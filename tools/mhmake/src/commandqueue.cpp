/*  This file is part of mhmake.
 *
 *  Copyright (C) 2001-2009 Marc Haesen
 *
 *  Mhmake is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Mhmake is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Mhmake.  If not, see <http://www.gnu.org/licenses/>.
*/

/* $Rev$ */

#include "stdafx.h"

#include "commandqueue.h"
#include "mhmakeparser.h"

#ifndef WIN32

#define INFINITE 0
#define FALSE 0

static int Status;
unsigned WaitForMultipleObjects(int Nbr, mh_pid_t *phProcesses, int, int)
{
  mh_pid_t ID=waitpid(0,&Status,0);
  for (int i=0; i<Nbr; i++)
  {
    if (ID==phProcesses[i])
      return i;
  }
  return -1;
}

typedef unsigned DWORD;

int GetExitCodeProcess(mh_pid_t hProcess, DWORD *pExitCode)
{
  *pExitCode=Status;
  return true;
}

#define CloseHandle(Handle)

#endif


commandqueue::commandqueue() :
    m_NrActiveEntries(0)
{
#ifdef WIN32
  SYSTEM_INFO SysInfo;
  GetSystemInfo(&SysInfo);
  m_MaxNrCommandsInParallel=SysInfo.dwNumberOfProcessors;
#else
  FILE *pFile=fopen("/proc/cpuinfo","r");
  const char *pProc="\nprocessor";
  int cur=1;
  int NrProcs=0;
  while (!feof(pFile))
  {
    char In=fgetc(pFile);
    if (In==pProc[cur])
    {
      cur++;
      if (!pProc[cur])
      {
        NrProcs++;
        cur=0;
      }
    }
    else
      cur=0;
  }
  m_MaxNrCommandsInParallel=NrProcs;
#endif

  m_pActiveProcesses=new mh_pid_t[m_MaxNrCommandsInParallel];
  m_pActiveEntries= new activeentry[m_MaxNrCommandsInParallel];
}

commandqueue::~commandqueue()
{
  delete [] m_pActiveProcesses;
  delete [] m_pActiveEntries;
}

void commandqueue::SetNrParallelBuilds(unsigned NrParallelBuilds)
{
  if (m_NrActiveEntries)
    throw("Changing of number of parallel builds is only allowed when no commands are executing");
  NrParallelBuilds=max(1u,NrParallelBuilds);
  m_MaxNrCommandsInParallel=NrParallelBuilds;
  delete [] m_pActiveProcesses;
  delete [] m_pActiveEntries;
  m_pActiveProcesses=new mh_pid_t[NrParallelBuilds];
  m_pActiveEntries= new activeentry[NrParallelBuilds];
}

void commandqueue::ThrowCommandExecutionError(activeentry *pActiveEntry)
{
  refptr<fileinfo> pTarget=pActiveEntry->pTarget;
  const string    &Command=pActiveEntry->Command;
  mhmakeparser    *pMakefile=pTarget->GetRule()->GetMakefile();

  string ErrorMessage = string("Error running command: ")+ Command +"\n";
  ErrorMessage += "Command defined in makefile: " + pMakefile->GetMakeDir()->GetQuotedFullFileName();
  pTarget->SetCommandsMd5_32(0);  /* Clear the md5 to make sure that the target is rebuild the next time mhmake is ran */
  pMakefile->SetAutoDepsDirty();  /* We need to update the autodeps file if the md5 has been changed */
  throw ErrorMessage;
}

void commandqueue::RemoveActiveEntry(unsigned Entry)
{
//cout << "Remove entry "<<Entry<<" of "<<m_NrActiveEntries<<" from queue:"<<m_pActiveEntries[Entry].pTarget->GetQuotedFullFileName()<<endl;
  unsigned NrToMove=m_NrActiveEntries-Entry-1;
  if (NrToMove)
  {
    for (unsigned i=0; i<NrToMove; i++)
    {
      unsigned EntryP1=Entry+1;
      m_pActiveEntries[Entry]=m_pActiveEntries[EntryP1];
      m_pActiveProcesses[Entry]=m_pActiveProcesses[EntryP1];
      Entry=EntryP1;
    }
  }
  m_pActiveEntries[Entry]=activeentry();
  m_pActiveProcesses[Entry]=NULL;
  m_NrActiveEntries--;
}

/* Start to execute next command, return true when command is completely executed
   upon return */
bool commandqueue::StartExecuteNextCommand(activeentry *pActiveEntry, mh_pid_t *pActiveProcess)
{
  refptr<fileinfo> pTarget=pActiveEntry->pTarget;
  mhmakeparser *pMakefile=pTarget->GetRule()->GetMakefile();

  pMakefile->SetRuleThatIsBuild(pTarget); // Make sure that the command expension is correct
  string Command=pMakefile->ExpandExpression(*pActiveEntry->CurrentCommandIt);
  pMakefile->ClearRuleThatIsBuild();  /* Make sure that further expansion is not taking this rule into account.*/
  md5_update( &pActiveEntry->md5ctx, (uint8 *)Command.c_str(), (unsigned long)Command.size());

  pActiveEntry->Command=Command;

  #ifdef _DEBUG
  if (g_pPrintDependencyCheck)
  {
    cout<<"-> "<<Command<<endl;
  }
  if (!g_GenProjectTree)
  {
  #endif
    mh_pid_t hProcess=pMakefile->ExecuteCommand(Command,pActiveEntry->IgnoreError);
    if (hProcess==(mh_pid_t)-1)
    {
      ThrowCommandExecutionError(pActiveEntry);
    }
    else if (!hProcess)
    {
      // Command already finished, so execute next command
      return true;
    }
    else
    {
         // Command still executing, so add it to the list of handles to wait for
      *pActiveProcess=hProcess;
      return false;
    }
  #ifdef _DEBUG
  }
  #endif
  return true;
}

void commandqueue::TargetBuildFinished(activeentry *pActiveEntry)
{
  refptr<fileinfo> pTarget=pActiveEntry->pTarget;

  // Building of this target finished
  uint32 Md5_32=md5_finish32( &pActiveEntry->md5ctx);
  #ifdef _DEBUG
  if (g_DoNotExecute||g_GenProjectTree)
    pTarget->SetDateToNow();
  else
  #endif
    pTarget->InvalidateDate();

  pTarget->SetCommandsMd5_32(Md5_32);  /* If the rule of the target was added with an implicit rule the targets in the rule is empty */

  refptr<rule> pRule=pTarget->GetRule();
  mhmakeparser *pMakefile=pRule->GetMakefile();

  pMakefile->AddTarget(pTarget);
  pMakefile->SetAutoDepsDirty();
  pRule->SetTargetsIsBuild(Md5_32);

  #ifdef _DEBUG
  if (g_pPrintDependencyCheck)
  {
    cout<<"Building "<<pTarget->GetQuotedFullFileName()<<" finished : "<< pTarget->GetDate() << endl;
  }

  if (!pMakefile->ImplicitSearch() && !pTarget->Exists() && !pTarget->IsPhony() && !g_DoNotExecute && !g_GenProjectTree)
  {
    // This is only a warning for phony messages
    cout<<"Warning: don't know how to make "<<pTarget->GetQuotedFullFileName()<<"\nMake the rule a phony rule to avoid this warning (but only do it when it is really phony).\n";;
  }
  #endif

  pTarget->ClearBuilding();
}

/* Start executing the commands of a target
*/
bool commandqueue::StartExecuteCommands(const refptr<fileinfo> &pTarget)
{
  cout << "Building " << pTarget->GetQuotedFullFileName()<<endl;
  // We do not have to put it in the queue, we can start executing directly
  refptr<rule> pRule=pTarget->GetRule();
  mhmakeparser *pMakefile=pRule->GetMakefile();
  vector<string>::iterator CommandIt=pRule->GetCommands().begin();
  activeentry *pActiveEntry=&m_pActiveEntries[m_NrActiveEntries];
  md5_starts( &pActiveEntry->md5ctx );

  pActiveEntry->pTarget=pTarget;
  pActiveEntry->CurrentCommandIt=CommandIt;
  pActiveEntry->pTarget=pTarget;

  while (1)
  {
    if (StartExecuteNextCommand(pActiveEntry, &m_pActiveProcesses[m_NrActiveEntries]))
    {
      pActiveEntry->CurrentCommandIt++;
      if (pActiveEntry->CurrentCommandIt==pRule->GetCommands().end())
      {
         // All commands executed
         break;
      }
    }
    else
    {
//cout << "Adding entry "<<m_NrActiveEntries<<" to queue:"<<pActiveEntry->pTarget->GetQuotedFullFileName()<<endl;
      m_NrActiveEntries++;
      return false;
    }
  }
  TargetBuildFinished(pActiveEntry);
  *pActiveEntry=activeentry();
  return true;
}


/* put the target in the execution queue or start executing immediately
*/
bool commandqueue::QueueTarget(const refptr<fileinfo> &pTarget)
{
  pTarget->SetBuilding();
  // First check if there is place in the active entries
  if (m_NrActiveEntries==m_MaxNrCommandsInParallel)
  {
     // commands cannot be started yet
     m_Queue.push(pTarget);
     return true;
  }
  else
  {
    return !StartExecuteCommands(pTarget);
  }
}

/* Wait for all the commands being executed of a target. In the mean time also continue
   executing all other commands in the queue
*/
mh_time_t commandqueue::WaitForTarget(const refptr<fileinfo> &pTarget)
{
  if (!pTarget->IsBuilding())
    return pTarget->GetDate();

  while (1)
  {
    // First wait until one of the processes that are running is finished
    unsigned Ret=WaitForMultipleObjects(m_NrActiveEntries,m_pActiveProcesses,FALSE,INFINITE);
    if (Ret>=m_NrActiveEntries)
      throw("fatal error: unexpected return value of WaitForMultipleObjects " + stringify(Ret));
    activeentry *pActiveEntry=&m_pActiveEntries[Ret];
    refptr<fileinfo> pCurrentTarget=pActiveEntry->pTarget;
    refptr<rule> pRule=pCurrentTarget->GetRule();

      // First check the error code of the command
    DWORD ExitCode=0;
    mh_pid_t hProcess=m_pActiveProcesses[Ret];
    if (!GetExitCodeProcess(hProcess,&ExitCode) || ExitCode)
    {
      if (pActiveEntry->IgnoreError)
      {
        cerr << "Error running command: "<<pActiveEntry->Command<<", but ignoring error\n";
      }
      else
        ThrowCommandExecutionError(pActiveEntry);
    }
    CloseHandle(hProcess);

       // Now check if we have to execute more commands for this target
    pActiveEntry->CurrentCommandIt++;
    while (pActiveEntry->CurrentCommandIt!=pRule->GetCommands().end())
    {
      if (!StartExecuteNextCommand(pActiveEntry, &m_pActiveProcesses[Ret]))
        break; // We have to wait for end of command execution
      pActiveEntry->CurrentCommandIt++;
    }

    if (pActiveEntry->CurrentCommandIt==pRule->GetCommands().end())
    {
      TargetBuildFinished(pActiveEntry);
      // Target building finished, so remove it from the list of active entries
      RemoveActiveEntry(Ret);

      bool Return=false;
      if (pTarget==pCurrentTarget)
      {
        Return = true;
      }
      // Check if we have other entries in the queue to start
      while (1)
      {
        if (m_Queue.empty())
        {
          // There should still be active entries, otherwise this is a serious bug
          if (!m_NrActiveEntries && !Return)
            throw("Fatal error: WaitForTarget "+pTarget->GetQuotedFullFileName()+": no active targets anymore.");
          else
            break;
        }
        else
        {
          refptr<fileinfo> pNewTarget=m_Queue.front();
          m_Queue.pop();
          if (StartExecuteCommands(pNewTarget))
          {
            if (pNewTarget==pTarget)
              Return=true;
          }
          else
            break;
        }
      }
      if (Return)
        return pTarget->GetDate();
    }
  }

}
