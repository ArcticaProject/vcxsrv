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

#include "fileinfo.h"
#include "rule.h"
#include "util.h"
#include "mhmakeparser.h"

const string NullString;
refptr<fileinfo> NullFileInfo;

#ifdef WIN32
ZEROTIME g_ZeroTime;
#endif

///////////////////////////////////////////////////////////////////////////////
string QuoteFileName(const string &Filename)
{
  string Ret(Filename);
#if OSPATHSEP=='\\'
  /* Put quotes around the string if there are spaces in it */
  if (Ret.find_first_of(' ')!=string::npos && Ret[0]!='"')
  {
    Ret=g_QuoteString+Ret+g_QuoteString;
  }
#else
  int Pos=0;
  /* Escape the spaces with a backslash */
  while ((Pos=Ret.find_first_of(' ',Pos))!=string::npos)
  {
    Ret=Ret.replace(Pos,1,"\\ ");
    Pos+=2;
  }
#endif
  return Ret;
}

///////////////////////////////////////////////////////////////////////////////
string UnquoteFileName(const string &Filename)
{
  size_t Pos=0;
  string Name(Filename);
#if OSPATHSEP=='\\'
  /* Remove all the quotes from the filename */
  while ((Pos=Name.find_first_of('"',Pos))!=string::npos)
  {
    Name=Name.replace(Pos,1,"");
  }
#else
  /* Remove the escaped spaces */
  while ((Pos=Name.find_first_of("\\",Pos))!=string::npos)
  {
    if (Name[Pos+1]==' ')
      Name=Name.replace(Pos,2," ");
    Pos+=1;
  }
#endif
  return Name;
}

///////////////////////////////////////////////////////////////////////////////
refptr<fileinfo> fileinfo::GetDir() const
{
  return GetAbsFileInfo(m_AbsFileName.substr(0,m_AbsFileName.find_last_of(OSPATHSEP)));
}

///////////////////////////////////////////////////////////////////////////////
string fileinfo::GetName() const
{
  return m_AbsFileName.substr(m_AbsFileName.find_last_of(OSPATHSEP)+1);
}

///////////////////////////////////////////////////////////////////////////////
mh_time_t fileinfo::realGetDate()
{
#ifdef WIN32
  WIN32_FIND_DATA FindData;
  HANDLE hFind=FindFirstFile(m_AbsFileName.c_str(),&FindData);
  if (hFind==INVALID_HANDLE_VALUE)
  {
    m_Date.SetNotExist();
  }
  else
  {
    FindClose(hFind);
    if (FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
    { // For directories we just take an old time since the lastwritetime is changed each time something
      // is added to the directory
      m_Date.SetDir();
    }
    else
    {
      m_Date=g_ZeroTime.ConvertTime(&FindData.ftLastWriteTime);
    }
  }
#else
  struct stat Buf;
  if (-1==stat(m_AbsFileName.c_str(),&Buf))
    m_Date.SetNotExist();
  else if (S_ISDIR(Buf.st_mode))
    m_Date.SetDir();
  else
    m_Date=Buf.st_mtime;
#endif
  return m_Date;
}

///////////////////////////////////////////////////////////////////////////////
bool fileinfo::IsDir() const
{
#ifdef WIN32
  WIN32_FIND_DATA FindData;
  HANDLE hFind=FindFirstFile(m_AbsFileName.c_str(),&FindData);
  if (hFind==INVALID_HANDLE_VALUE)
  {
    return false;
  }
  else
  {
    FindClose(hFind);
    if (FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
#else
  struct stat Buf;
  if (-1==stat(m_AbsFileName.c_str(),&Buf))
    return false; // File does not exist, so consider this as not a directory
  else
    return 0!=S_ISDIR (Buf.st_mode);
  return true;
#endif
}

///////////////////////////////////////////////////////////////////////////////
void fileinfo::SetDateToNow()
{
#ifdef WIN32
  FILETIME FileTime;
  GetSystemTimeAsFileTime(&FileTime);
  m_Date=g_ZeroTime.ConvertTime(&FileTime);
#else
  m_Date=time(NULL);
#endif
}

///////////////////////////////////////////////////////////////////////////////
string fileinfo::GetPrerequisits() const
{
  // Build a string with all prerequisits, but make sure that every dependency
  // is only in there once (we do this be building a set in parallel
  vector< refptr<fileinfo> >::const_iterator DepIt=m_Deps.begin();
  deps_t Deps;
  bool first=true;
  string Ret=g_EmptyString;
  while (DepIt!=m_Deps.end())
  {
    deps_t::iterator pFound=Deps.find(*DepIt);
    if (pFound==Deps.end())
    {
      if (first)
      {
        first=false;
      }
      else
      {
        Ret+=g_SpaceString;
      }
      Ret+=(*DepIt)->GetQuotedFullFileName();
    }
    Deps.insert(*DepIt);
    DepIt++;
  }
  return Ret;
}

///////////////////////////////////////////////////////////////////////////////
void fileinfo::AddDeps(vector< refptr<fileinfo> > &Deps)
{
  vector< refptr<fileinfo> >::iterator It=Deps.begin();
  vector< refptr<fileinfo> >::iterator ItEnd=Deps.end();
  while (It!=ItEnd)
  {
    AddDep(*It++);
  }
}

///////////////////////////////////////////////////////////////////////////////
bool fileinfo::IsAutoDepExtention(void) const
{
  const char *pName=GetFullFileName().c_str();
  const char *pExt=strrchr(pName,'.');
  if (!pExt)
    return false;
  pExt++;
  if (m_pRule)
  {
    string ObjExt=m_pRule->GetMakefile()->ExpandVar(OBJEXTVAR);
    return ((0==strcmp(pExt,ObjExt.c_str()+1)) || (0==strcmp(pExt,"h")));
  }
  else
    return ((0==strcmp(pExt,"obj")) || (0==strcmp(pExt,"doj")) || (0==strcmp(pExt,"o")) || (0==strcmp(pExt,"h")));
}

#ifdef _DEBUG
///////////////////////////////////////////////////////////////////////////////
string fileinfo::GetErrorMessageDuplicateRule(const refptr<rule>&pRule)
{
  string Ret;
  Ret = GetQuotedFullFileName() + ": rule is defined multiple times\n";
  Ret += "First (" + m_pRule->GetMakefile()->GetMakeDir()->GetQuotedFullFileName() + ") :\n";

  vector<string>::const_iterator It=m_pRule->GetCommands().begin();
  while (It!=m_pRule->GetCommands().end())
  {
    Ret+= "    " + m_pRule->GetMakefile()->ExpandExpression(*It) + "\n";
    It++;
  }
  Ret += "Second (" + pRule->GetMakefile()->GetMakeDir()->GetQuotedFullFileName() + ") :\n";
  It=pRule->GetCommands().begin();
  while (It!=pRule->GetCommands().end())
  {
    Ret += "    " + pRule->GetMakefile()->ExpandExpression(*It) +"\n";
    It++;
  }
  return Ret;
}
#endif

///////////////////////////////////////////////////////////////////////////////
static inline string &NormalizePathName(string &Name)
{
  const char *pPtr=Name.c_str();
  const char *pBeg=pPtr;
  const char *pLastSlash=NULL;
  char *pWr=(char*)pBeg;
  char Char=*pPtr++;

  while (Char)
  {
    if (Char=='\\' || Char=='/')
    {
      char Char2=pPtr[0];
      if (Char2=='.')
      {
        if (pPtr[1]=='.')
        {
          pPtr+=2;
          while ((pPtr[0]=='\\' || pPtr[0]=='/')  && pPtr[1]=='.' && pPtr[2]=='.')
          {
            pLastSlash--;
            while (*pLastSlash!='\\' && *pLastSlash!='/') pLastSlash--;
            if (pLastSlash<pBeg)
              pLastSlash=pBeg;  // This is a fault in the file name, just put it back at the beginning
            pPtr+=3;
          }
          if (pLastSlash)
            pWr=(char*)pLastSlash;
        }
        else
        {
          if (pPtr[1]=='\\' || pPtr[1]=='/')
          {
            pPtr++;
          }
          else
          {
            pLastSlash=pWr;
            *pWr++ = OSPATHSEP;
          }
        }
      }
      else if (Char2=='\\' || Char2=='/')
      {
      }
      else
      {
        pLastSlash=pWr;
        *pWr++ = OSPATHSEP;
      }
    }
    else
    {
      #ifdef WIN32
      *pWr++ = tolower(Char);
      #else
      *pWr++ = Char;
      #endif
    }
    Char=*pPtr++;
  }
  *pWr=0;
  Name.resize(pWr-pBeg);

  return Name;
}


///////////////////////////////////////////////////////////////////////////////
const refptr<fileinfo> &GetFileInfo(const string &NameIn,const refptr<fileinfo> &RelDir)
{
  string Name=UnquoteFileName(NameIn);
  bool DoesExist=true;

  //Only concatenate if szName is not already a full name
#ifdef WIN32
  if (!Name.empty() && Name[1]!=':')
#endif
  {
    if (Name[0]!=OSPATHSEP)
    {
      Name=RelDir->GetFullFileName()+OSPATHSEPSTR+Name;
      if (!RelDir->Exists())  /* if the directory does not exist, the file will not exist either */
        DoesExist=false;
    }
    #ifdef WIN32
    else
    {
      /* The filename is absolute but does not contain a driver letter. So add it (only on windows) */
      Name=RelDir->GetFullFileName().substr(0,2)+Name;
    }
    #endif
  }
  const refptr<fileinfo> &Ret=GetAbsFileInfo(NormalizePathName(Name));
  if (!DoesExist)
    Ret->SetNotExist();
  return Ret;
}

#ifdef _DEBUG
///////////////////////////////////////////////////////////////////////////////
void PrintFileInfos()
{
  set<refptr<fileinfo>,less_refptrfileinfo>::iterator pIt=g_FileInfos.begin();
  while (pIt!=g_FileInfos.end())
  {
    cout<<(*pIt)->GetQuotedFullFileName()<<" :";
    if ((*pIt)->IsPhony())
      cout<<" (phony)";
    vector< refptr<fileinfo> > &Deps=(*pIt)->GetDeps();
    vector< refptr<fileinfo> >::iterator pDepIt=Deps.begin();
    while (pDepIt!=Deps.end())
    {
      cout<<g_SpaceString<<(*pDepIt)->GetQuotedFullFileName();
      pDepIt++;
    }
    cout<<endl;
    // Write the commands
    refptr<rule> pRule=(*pIt)->GetRule();
    if (pRule)
    {
      cout<<g_SpaceString<<"Run in: "<<pRule->GetMakefile()->GetMakeDir()->GetQuotedFullFileName()<<endl;
      pRule->PrintCommands();
    }
    pIt++;
  }

}
#endif

