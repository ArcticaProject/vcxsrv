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
#include "mhmakeparser.h"
#include "rule.h"
#include "util.h"

bool g_Clean=false;

int __CDECL main(int argc, char* argv[])
{
  //__int64 Freq;
  //__int64 Start;

  //QueryPerformanceFrequency((LARGE_INTEGER*)&Freq);
  //QueryPerformanceCounter((LARGE_INTEGER*)&Start);

  #if defined(_DEBUG) && defined(_MSC_VER)
  int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );

  // Turn on leak-checking bit
  tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
  // Turn on defer freeing
  //tmpFlag |= _CRTDBG_DELAY_FREE_MEM_DF;
  // Turn on heap checking
  //tmpFlag |= _CRTDBG_CHECK_ALWAYS_DF;

  // Set flag to the new value
  _CrtSetDbgFlag( tmpFlag );

  //_CrtSetBreakAlloc(44);
  #endif

  try
  {
    mhmakefileparser::InitBuildTime();

    putenv("PLATFORM="PLATFORM);

    vector<string> CmdLineArgs;
    for (int i=1; i<argc; i++)
    {
      CmdLineArgs.push_back(argv[i]);
    }

    refptr<loadedmakefile> pFirstMakefile(new loadedmakefile(CmdLineArgs,"makefile"));
    // For the first makefile we add the defines passed on the command line to the
    // environment so that the other load_makefile see the same variables
    pFirstMakefile->AddCommandLineVarsToEnvironment();

    g_LoadedMakefiles.push_back(pFirstMakefile);

    pFirstMakefile->LoadMakefile();

    #ifdef _DEBUG
    if (g_PrintVarsAndRules)
    {
      DumpVarsAndRules();
    }
    #endif

    // Make sure that the included makefiles that have rules are build

    LOADEDMAKEFILES::iterator LoadMakIt=g_LoadedMakefiles.begin();
    while (LoadMakIt!=g_LoadedMakefiles.end())
    {
      (*LoadMakIt)->m_pParser->BuildIncludedMakefiles();
      (*LoadMakIt)->m_pParser->ParseBuildedIncludeFiles();
      LoadMakIt++;
    }

    if (pFirstMakefile->m_CommandLineTargets.size())
    {
      vector<string>::iterator It=pFirstMakefile->m_CommandLineTargets.begin();
      while (It!=pFirstMakefile->m_CommandLineTargets.end())
      {
        if (*It=="clean" || *It=="cleanall")
        {
          g_Clean=true;
        }
        pFirstMakefile->m_pParser->BuildTarget(GetFileInfo(*It,pFirstMakefile->m_MakeDir));
        It++;
      }
    }
    else
    {
      refptr<fileinfo> FirstTarget=pFirstMakefile->m_pParser->GetFirstTarget();
      if (FirstTarget)
        pFirstMakefile->m_pParser->BuildTarget(FirstTarget);
      else
        cout << "Warning: no targets in makefile. Nothing to be build.\nMHMAKECONF defined?\n";
    }

  }
  catch (int Error)
  {
    printf("Error occured %d\n",Error);
    #ifdef _DEBUG
    if (g_DumpOnError)
    {
      DumpVarsAndRules();
    }
    #endif
    return 1;
  }

  //__int64 Stop;
  //QueryPerformanceCounter((LARGE_INTEGER*)&Stop);
  //cout << (Stop-Start)*1000/Freq << endl;

  return 0;
}

