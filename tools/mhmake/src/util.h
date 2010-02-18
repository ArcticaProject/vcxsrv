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

#ifndef __UTIL_H__
#define __UTIL_H__

#include "fileinfo.h"

// List of pre-defined variables
#define WC_REVISION  "WC_REVISION"
#define WC_URL       "WC_URL"
#define AUTODEPFILE  "AUTODEPFILE"
#define OBJEXTVAR    "OBJEXT"
#define EXEEXTVAR    "EXEEXT"
#define SKIPHEADERS  "SKIPHEADERS"
#define MAKE         "MAKE"
#define MHMAKECONF   "MHMAKECONF"
#define BASEAUTOMAK  "BASEAUTOMAK"
#define CURDIR       "CURDIR"
#define USED_ENVVARS "USED_ENVVARS"
#define PATH         "PATH"
#ifdef WIN32
#define COMSPEC      "COMSPEC"
#define PYTHONEXE    "python.exe"
#define EXEEXT       ".exe"
#define OBJEXT       ".obj"
#define PLATFORM     "win32"
#else
#define COMSPEC      "SHELL"
#define PYTHONEXE    "python"
#define EXEEXT       ""
#define OBJEXT       ".o"
#define PLATFORM     "linux"
#endif

#define MHMAKEVER    "2.0.0_rc"

class makecommand
{
  string m_BuildCommand;
public:
  makecommand();
  operator string()
  {
    return m_BuildCommand;
  }
};

extern makecommand g_MakeCommand;

///////////////////////////////////////////////////////////////////////////////
inline const char *NextItem(const char *pTmp,string &Output, const char *pToks=" \t")
{
  const char *pStart;
  const char *pStop;
  while (strchr(pToks,*pTmp)&&*pTmp) pTmp++;
  pStart=pTmp;
  while (1)
  {
    if (*pTmp=='"')
    {
      pTmp++;
      while (*pTmp && *pTmp!='"') pTmp++;
      if (*pTmp) pTmp++;
      pStop=pTmp;
      if (!*pTmp || strchr(pToks,*pTmp))
        break;
    }
    else if (*pTmp=='\'')
    {
      pTmp++;
      while (*pTmp && *pTmp!='\'') pTmp++;
      if (*pTmp) pTmp++;
      pStop=pTmp;
      if (!*pTmp || strchr(pToks,*pTmp))
        break;
    }
    else if (!*pTmp)
    {
      pStop=pTmp;
      break;
    }
    else
    {
      pTmp++;
      #if OSPATHSEP=='/'
      while (*pTmp)
      {
        if (!strchr(pToks,*pTmp) || (*(pTmp-1)=='\\'))
          pTmp++;
        else
          break;
      }
      #else
      while (!strchr(pToks,*pTmp)) pTmp++;
      #endif
      pStop=pTmp;
      break;
    }
  }
  Output=string(pStart,pStop);
  // skip trailing space
  while (strchr(pToks,*pTmp)&&*pTmp) pTmp++;
  return pTmp;
}

///////////////////////////////////////////////////////////////////////////////
inline const char *NextCharItem(const char *pTmp,string &Output,char Char)
{
  const char *pStart=pTmp;
  while (*pTmp && *pTmp!=Char) pTmp++;
  const char *pStop=pTmp;
  if (*pTmp) pTmp++;

  while (pStart<pStop && (*pStart==' ' || *pStart == '\t')) pStart++;
  pStop--;
  while (pStart<=pStop && (*pStop==' ' || *pStop == '\t')) pStop--;
  pStop++;

  Output=string(pStart,pStop);
  return pTmp;
}

///////////////////////////////////////////////////////////////////////////////

string Substitute(const string &ToSubst,const string &SrcStr,const string &ToStr);

struct matchres
{
  string m_First;
  string m_Stem;
  string m_Last;
};

bool PercentMatch(const string &String,const string &Expr,matchres *pRes=NULL,const char Char='%');
bool PercentMatchList(const string &String,const string &ExprList,matchres *pRes=NULL);
string ReplaceWithStem(const string &String,const string &Stem);

struct loadedmakefile : public refbase
{
  struct loadedmakefile_statics
  {
    map<string,string>   m_GlobalCommandLineVars;
    refptr<fileinfo>     m_MhMakeConf;

    loadedmakefile_statics();
  };
  static loadedmakefile_statics sm_Statics;

  refptr<fileinfo>     m_Makefile;
  refptr<fileinfo>     m_MakeDir;
  map<string,string>   m_CommandLineVars;

  vector<string>       m_CommandLineTargets;
  refptr<mhmakeparser> m_pParser;

  loadedmakefile(const refptr<fileinfo> &pDir, vector<string> &Args,const string &Makefile=g_EmptyString);

  void LoadMakefile();
  void AddCommandLineVarsToEnvironment()
  {
    map<string,string>::const_iterator It=m_CommandLineVars.begin();
    map<string,string>::const_iterator ItEnd=m_CommandLineVars.end();
    while (It!=ItEnd)
    {
      sm_Statics.m_GlobalCommandLineVars.insert(*It++);
    }
  }

  int operator==(const loadedmakefile &Other)
  {
    if (m_Makefile!=Other.m_Makefile)
      return 0;
    if (m_MakeDir!=Other.m_MakeDir)
      return 0;
    if (m_CommandLineTargets.size()!=Other.m_CommandLineTargets.size())
      return 0;
    if (m_CommandLineVars.size()!=Other.m_CommandLineVars.size())
      return 0;
    map<string,string>::iterator VarIt=m_CommandLineVars.begin();
    while (VarIt!=m_CommandLineVars.end())
    {
      map<string,string>::const_iterator pFound=Other.m_CommandLineVars.find(VarIt->first);
      if (pFound==Other.m_CommandLineVars.end())
        return 0;
      if (pFound->second!=VarIt->second)
        return 0;
      VarIt++;
    }
    vector<string>::iterator TarIt=m_CommandLineTargets.begin();
    while (TarIt!=m_CommandLineTargets.end())
    {
      vector<string>::const_iterator OtherIt=Other.m_CommandLineTargets.begin();
      while (OtherIt!=Other.m_CommandLineTargets.begin())
      {
        if (*TarIt==*OtherIt)
          break;
        OtherIt++;
      }
      if (OtherIt==Other.m_CommandLineTargets.end())
        return 0;
      TarIt++;
    }
    return 1;
  }
};

class LOADEDMAKEFILES : public vector<refptr<loadedmakefile> >
{
public:
  refptr<loadedmakefile> find(const loadedmakefile &pToSearch);
  typedef vector<refptr<loadedmakefile> >::iterator iterator;
};

extern LOADEDMAKEFILES g_LoadedMakefiles;

void DumpVarsAndRules();

#endif

