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
#include "util.h"
#include "md5.h"

#include "mhmakefileparser.h"
#include "rule.h"
#include "mhmakelexer.h"

///////////////////////////////////////////////////////////////////////////////
int mhmakefileparser::yylex(void)
{
  m_yyloc=m_ptheLexer->m_Line;
  return m_ptheLexer->yylex(m_theTokenValue);
}

///////////////////////////////////////////////////////////////////////////////
void mhmakefileparser::yyerror(const char *m)
{
  cerr << this->m_ptheLexer->m_InputFileName<< " ("<<m_yyloc<<"): "<<m<<endl;
}

///////////////////////////////////////////////////////////////////////////////
int mhmakefileparser::ParseFile(const refptr<fileinfo> &FileInfo,bool SetMakeDir)
{
  mhmakelexer theLexer;
  m_ptheLexer=&theLexer;
  if (SetMakeDir)
  {
    m_MakeDir=curdir::GetCurDir();
    m_Variables[CURDIR]=m_MakeDir->GetQuotedFullFileName();
  }
  theLexer.m_InputFileName=FileInfo->GetFullFileName();
  theLexer.m_pParser=(mhmakeparser*)this;
  theLexer.yyin=::fopen(FileInfo->GetFullFileName().c_str(),"r");
  if (!theLexer.yyin)
  {
    cerr << "Error opening makefile: "<<FileInfo->GetQuotedFullFileName()<<endl;
    return 1;
  }
  int Ret=yyparse();
  ::fclose(theLexer.yyin);
  return Ret;
}


///////////////////////////////////////////////////////////////////////////////
bool mhmakefileparser::IsDefined(const string &Var) const
{
  bool Ret = m_Variables.find(Var)!=m_Variables.end();
  if (!Ret)
  {
    string Env=GetFromEnv(Var);
    if (!Env.empty())
    {
      Ret=true;
    }
  }
  return Ret;
}

///////////////////////////////////////////////////////////////////////////////
static inline int SkipUntilQuote(const string &Expr,int i,char Char)
{
  while (Expr[i++]!=Char) ;
  return i;
}

///////////////////////////////////////////////////////////////////////////////
static inline int SkipMakeExpr(const string &Expr,int i)
{
  char Char=Expr[i++];
  if (Char!='(')
    return i;
  Char=Expr[i++];
  while (Char!=')')
  {
    if (Char=='$')
    {
      i=SkipMakeExpr(Expr,i);
    }
    Char=Expr[i++];
  }
  return i;
}

///////////////////////////////////////////////////////////////////////////////
// Splits expression on the Item, but the item may not occur within
// a macro or quoted string
static pair<string,string> SplitExpr(const string &Expr,char Item)
{
  int i=0;
  char Char=Expr[i++];
  while (Char!=Item)
  {
    if (Char=='"' || Char=='\'')
    {
      i=SkipUntilQuote(Expr,i,Char);
    }
    else if (Char=='$')
    {
      i=SkipMakeExpr(Expr,i);
    }
    Char=Expr[i++];
  }
  return pair<string,string>(Expr.substr(0,i-1),Expr.substr(i));
}

///////////////////////////////////////////////////////////////////////////////
bool mhmakefileparser::IsEqual(const string &EqualExpr) const
{
  string Expr=ExpandExpression(EqualExpr);
  const char *pStr=Expr.c_str();
  const char *pTmp=pStr;
  while (*pTmp && *pTmp!=',') pTmp++;
  int Pos=pTmp-pStr;
  int Size=Expr.size();
  pTmp=pStr+Size-1;
  while (pTmp>pStr && strchr(" \t",*pTmp))
  {
    pTmp--;
  }
  if (2*Pos != pTmp-pStr)
  {
    return false;
  }
  pTmp=pStr;
  const char *pTmp2=pTmp+Pos+1;
  if (*pTmp=='(')
  {
    pTmp++;
    Pos--;
  }
  for (int i=0; i<Pos; i++)
  {
    if (pTmp[i]!=pTmp2[i])
    {
      return false;
    }
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
string mhmakefileparser::ExpandExpression(const string &Expr) const
{
  ((mhmakefileparser*)this)->m_InExpandExpression++;
  int i=0;
  int Length=Expr.size();
  string Ret;
  string ToAdd;
  while (i<Length)
  {
    char Char=Expr[i++];
    if (Char=='$')
    {
      char CharNext=Expr[i];
      if (CharNext=='$')
      {
        ToAdd="$$";
        i++;
      }
      else
      {
        int inew=SkipMakeExpr(Expr,i);
        i++;
        if (inew>i)
        {
          ToAdd=ExpandMacro(Expr.substr(i,inew-i-1));
          i=inew;
        }
        else
        {
          // This is a single character expression
          ToAdd=ExpandMacro(string(1,Expr[i-1]));
        }
      }
      Ret+=ToAdd;
    }
    else
    {
      Ret+=Char;
    }
  }
  if (m_InExpandExpression==1)
  {
      // Here we do a special case in case we still have a $ within a %
    if (Ret.find('$')!=string::npos)
      Ret=ExpandExpression(Ret);
    int Pos;
    while ((Pos=Ret.find("$$"))!=string::npos)
    {
      Ret=Ret.replace(Pos,2,"$");
    }
  }
  ((mhmakefileparser*)this)->m_InExpandExpression--;

  return Ret;
}

///////////////////////////////////////////////////////////////////////////////
string mhmakefileparser::ExpandMacro(const string &Expr) const
{
  if (Expr.find('%')!=string::npos && Expr.find('$')==string::npos && Expr.find(':')==string::npos)
    return string("$(")+Expr+")";
  string ExpandedExpr=ExpandExpression(Expr);

  const char *pTmp=ExpandedExpr.c_str();
  /* First remove leading spaces */
  while (*pTmp==' ' || *pTmp=='\t') pTmp++;
  const char *pVar=pTmp;
  while (*pTmp && *pTmp!=' ' && *pTmp!='\t' && *pTmp!=':') pTmp++;
  const char *pVarEnd=pTmp;
  char Type=*pTmp++;
  while (*pTmp && (*pTmp==' ' || *pTmp=='\t')) pTmp++;
  if (Type&&*pTmp)
  { // We have a match for the regular expression ^([^ \\t:]+)([: \\t])[ \\t]*(.+)
    if (Type==':')
    {
      #ifdef WIN32
      bool IsFileName=false;
      if (pVarEnd-pVar == 1 && (*pVar=='<' || *pVar =='@'))
        IsFileName=true;
      #endif
      string ToSubst=ExpandExpression(ExpandVar(string(pVar,pVarEnd)));
      const char *pSrc=pTmp;
      const char *pStop=pSrc;
      while (*pStop!='=') pStop++;
      const char *pTo=pStop+1;
      string SrcStr(pSrc,pStop);
      string ToStr(pTo);
      #ifdef WIN32
      if (IsFileName)
        return QuoteFileName(Substitute(UnquoteFileName(ToSubst),SrcStr,ToStr));
      #endif
      return Substitute(ToSubst,SrcStr,ToStr);
    }
    else if (Type==' ' || Type == '\t')
    {
      string Func(pVar,pVarEnd);
      string Arg(pTmp);
      if (Arg.find('%')!=string::npos && Arg.find('$')!=string::npos)
        return string("$(")+ExpandedExpr+")";
      function_f pFunc=m_Functions[Func];
      #ifdef _DEBUG
      if (pFunc)
      {
        return (this->*pFunc)(Arg);
      }
      else
      {
        throw string("Unknown function specified in macro: ")+Func;
      }
      #else
      return (this->*pFunc)(Arg);
      #endif
    }
    else
    {
    #ifdef _DEBUG
      throw string("Fatal error in ExpandMacro (bug in mhmake ? ? ?)");
    #else
      return g_EmptyString;
    #endif
    }
  }
  else
  {
    return ExpandExpression(ExpandVar(ExpandedExpr));
  }
}

///////////////////////////////////////////////////////////////////////////////
string mhmakefileparser::ExpandVar(const string &Var) const
{
  map<string,string>::const_iterator pIt=m_Variables.find(Var);
  if (pIt==m_Variables.end())
  {
    if (Var.size()==1)
    {
      char Char=Var[0];
      if (m_RuleThatIsBuild)
      {
        switch (Char)
        {
          case '<': // return first prerequisit
#ifdef _DEBUG
            if (!m_RuleThatIsBuild->GetDeps().size())
            {
              return "<No Dependencies defined.>";
            }
#endif
            return m_RuleThatIsBuild->GetDeps()[0]->GetQuotedFullFileName();
          case '@': // return full target file name
            return m_RuleThatIsBuild->GetQuotedFullFileName();
          case '*': // return stem
            return m_RuleThatIsBuild->GetRule()->GetStem();
          case '^': // return all prerequisits
            return m_RuleThatIsBuild->GetPrerequisits();
          case '/':
            return OSPATHSEPSTR;
          default:
            break;
        }
      }
      else
      {
        switch (Char)
        {
          case '<': // return first prerequisit
          case '@': // return full target file name
          case '*': // return stem
          case '^': // return all prerequisits
            return Var;  // To make comparing of rules more accurate
          case '/':
            return OSPATHSEPSTR;
          default:
            break;
        }
      }
    }
    string Env=GetFromEnv(Var);
    if (Env.empty())
    {
      #ifdef _DEBUG
      if (g_PrintAdditionalInfo)
      {
        cout<<"Warning: Variable "<<Var<<" not found\n";
      }
      #endif
      return g_EmptyString;
    }
    else
      return Env;
  }
  else
    return pIt->second;
}

///////////////////////////////////////////////////////////////////////////////
void SplitToItems(const string &String,vector< refptr<fileinfo> > &Items,refptr<fileinfo> Dir)
{
  const char *pTmp=String.c_str();
  while (*pTmp)
  {
    string Item;
    pTmp=NextItem(pTmp,Item);
    if (!Item.empty())
    {
      Items.push_back(GetFileInfo(Item,Dir));
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
void mhmakefileparser::ParseBuildedIncludeFiles()
{
  vector<string>::iterator It=m_ToBeIncludeAfterBuild.begin();
  while (It!=m_ToBeIncludeAfterBuild.end())
  {
    int result=ParseFile(GetFileInfo(*It));
    if (result)
    {
      throw string("Error parsing ")+*It;
    }
    It++;
  }
}

#ifdef _DEBUG
///////////////////////////////////////////////////////////////////////////////
void mhmakefileparser::PrintVariables(bool Expand) const
{
  map<string,string>::const_iterator It=m_Variables.begin();
  while (It!=m_Variables.end())
  {
    if (Expand)
    {
      try
      {
        cout<<It->first<<" : "<<ExpandExpression(It->second)<<endl;
      }
      catch (...)
      {
        cout<<endl;
      }
    }
    else
    {
      cout<<It->first<<" : "<<It->second<<endl;
    }
    It++;
  }
  cout << "Exported Variables:\n";
  vector<string>::const_iterator It2=m_Exports.begin();
  while (It2!=m_Exports.end())
  {
    cout<<*It2<<endl;
    It2++;
  }
}
#endif

//#define PAGETOSTRING(Nr)  #Nr
//#define PAGETONBR(Nr)  PAGETOSTRING(##Nr)
//#pragma message("ar=" PAGETONBR(NULL) ";")

///////////////////////////////////////////////////////////////////////////////
void mhmakefileparser::AddRule()
{
  vector< refptr<fileinfo> >::iterator pIt=m_pCurrentItems->begin();
  while (pIt!=m_pCurrentItems->end())
  {
    if ((*pIt)->GetFullFileName().find('%')!=string::npos)
    {
      IMPLICITRULE::AddImplicitRule(*pIt,*m_pCurrentDeps,m_pCurrentRule);
    }
    else
    {
      // If we had a double colon we must make sure that the target is always build
      if (m_DoubleColonRule)
      {
        (*pIt)->SetNotExist();
      }
      (*pIt)->SetRuleIfNotExist(m_pCurrentRule);
      if (!m_pCurrentRule)
        (*pIt)->AddDeps(*m_pCurrentDeps);
      else
        (*pIt)->InsertDeps(*m_pCurrentDeps);
      if (!m_FirstTarget)
      { // Only check this if the rule is not an implicit rule
        m_FirstTarget=(*m_pCurrentItems)[0];
      }
    }
    pIt++;
  }
  m_pCurrentItems=NULL;
  m_pCurrentRule=NullRule;
}

///////////////////////////////////////////////////////////////////////////////
void mhmakefileparser::GetAutoDeps(const refptr<fileinfo> &FirstDep, deps_t &Autodeps)
{
  /* Here we have to scan only c/c++ headers so skip certain extensions */
  const char *pFullName=FirstDep->GetFullFileName().c_str();
  const char *pExt=strrchr(pFullName,'.');
  bool bPython=false;
  if (pExt)
  {
    if (!_stricmp(pExt+1,"py"))
      bPython=true;
  }

  FILE *pIn=fopen(pFullName,"r");
  if (!pIn)
    return;

  refptr<fileinfo> CurDir=curdir::GetCurDir();  /* Since we are calling BuildTarget, the current directory might change, which is not what we should expecting */
  char IncludeList[255];
  int PrevRet=0;
  int Ret=fgetc(pIn);
  while(Ret!=EOF)
  {
    char Type[2];
    bool bFound=false;
    if (bPython)
    {
      Type[0]='"';
      if (Ret=='i')
      {
        Ret=fscanf(pIn,"mport %254[^\"\n]",IncludeList);
        if (Ret==1)
        {
          if (IncludeList[0]!='*')
            bFound=true;
        }
      }
    }
    else
    {
      if (PrevRet=='/')
      {
        if (Ret=='/')
        {
          /* This is a C++ command so read until the next line-feed */
          do
          {
            Ret=fgetc(pIn);
          } while (Ret!='\n' && Ret!=EOF);
        }
        else if (Ret=='*')
        {
          /* This is a standard C comment, so read until then end of the command */
          do
          {
            PrevRet=Ret;
            Ret=fgetc(pIn);
          } while ((PrevRet!='*' || Ret!='/') && Ret!=EOF);
        }
      }
      else if (Ret=='#' || Ret=='.')
      {
        if (Ret=='#')
          Ret=fscanf(pIn,"include %1[\"<]%254[^>\"]%*[\">]",&Type,IncludeList);
        else
          Ret=fscanf(pIn,"import %1[\"<]%254[^>\"]%*[\">]",&Type,IncludeList);
        if (Ret==2)
        {
          bFound=true;
        }
      }
    }
    if (bFound)
    {
      const char *pTmp=IncludeList;
      while (*pTmp)
      {
        string IncludeFile;
        pTmp=NextItem(pTmp,IncludeFile," \t,");
        if (bPython)
          IncludeFile+=".py";

        if (SkipHeaderFile(IncludeFile))
          continue;
        #ifdef _DEBUG
        m_ImplicitSearch++; // This is to avoid warnings of targets that does not exist
        #endif
        if (Type[0]=='"')
        {
          refptr<fileinfo> pInclude=GetFileInfo(IncludeFile,FirstDep->GetDir());
          /* Add the dependency when the file alrady exist or there is a rule available to be build */
          if (BuildTarget(pInclude).DoesExist())  // Try to build the target, and add it if it exists after building
          {
            deps_t::const_iterator pFind=Autodeps.find(pInclude);
            if (pFind==Autodeps.end())
            {
              Autodeps.insert(pInclude);
              GetAutoDepsIfNeeded(pInclude,pInclude);
            }
          }
        }
        const refptr<fileinfoarray> IncludeDirs=GetIncludeDirs();
        vector< refptr<fileinfo> >::const_iterator It=IncludeDirs->begin();
        while (It<IncludeDirs->end())
        {
          refptr<fileinfo> pInclude=GetFileInfo(IncludeFile,*It);
          if (BuildTarget(pInclude).DoesExist()) // Try to build the target, and add it if it exists after building
          {
            deps_t::const_iterator pFind=Autodeps.find(pInclude);
            if (pFind==Autodeps.end())
            {
              Autodeps.insert(pInclude);
              GetAutoDepsIfNeeded(pInclude,pInclude);
            }
            break;
          }
          It++;
        }
        #ifdef _DEBUG
        m_ImplicitSearch--;
        #endif
      }
    }
    PrevRet=Ret;
    Ret=fgetc(pIn);
  }
  fclose(pIn);
  curdir::ChangeCurDir(CurDir);
}

///////////////////////////////////////////////////////////////////////////////

void mhmakefileparser::GetAutoDepsIfNeeded(const refptr<fileinfo> &Target, const refptr<fileinfo>&FirstDep)
{
  autodeps_entry_t &Autodeps=m_AutoDeps[Target];
  if (!Autodeps.first)
  {
    Autodeps.first=true;
    /* We are going to rescan, so throw away the old. */
    Autodeps.second.clear();
    GetAutoDeps(FirstDep,Autodeps.second);
    // Now add these dependencies also to the rules
    deps_t::iterator It=Autodeps.second.begin();
    while (It!=Autodeps.second.end())
    {
      Target->AddDep(*It);
      It++;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

void mhmakefileparser::UpdateAutomaticDependencies(const refptr<fileinfo> &Target)
{
  m_AutoDepsDirty=true; /* Always assume dirty since in the autodeps file, the md5 strings are also saved. */
  if (Target->IsAutoDepExtention())
  {
    // we have to search for the include files in the first dependency of Target
    vector< refptr<fileinfo> > &Deps=Target->GetDeps();
    if (!Deps.size())
      return; // There is no first dep
    refptr<fileinfo> FirstDep=Deps[0];
    GetAutoDepsIfNeeded(Target,FirstDep);
  }
}

///////////////////////////////////////////////////////////////////////////////
const refptr<fileinfoarray> mhmakefileparser::GetIncludeDirs() const
{
  string Includes=ExpandExpression("$(INCLUDES)");
  if (!m_pIncludeDirs || Includes != m_IncludeDirs)
  {
    ((mhmakefileparser*)this)->m_IncludeDirs=Includes;
    ((mhmakefileparser*)this)->m_pIncludeDirs=refptr<fileinfoarray>(new fileinfoarray);
    if (Includes.empty())  // If not defined try a default path
      Includes="include inc .."OSPATHSEPSTR"include .."OSPATHSEPSTR"inc";
    const char *pTmp=Includes.c_str();
    while (*pTmp)
    {
      string Item;
      pTmp=NextItem(pTmp,Item);
      refptr<fileinfo> pIncDir=GetFileInfo(Item,m_MakeDir);
      if (pIncDir->Exists() || pIncDir->GetRule())
        ((mhmakefileparser*)this)->m_pIncludeDirs->push_back(pIncDir);
    }
  }
  return m_pIncludeDirs;
}

static void ReadStr(FILE *pFile,char *Str)
{
  int i=0;
  while (1)
  {
    Str[i]=fgetc(pFile);
#ifdef _DEBUG
    if (Str[i]==EOF)
    {
      cout<<"Premature end of depency file.\n";
      Str[i]=0;
      return;
    }
#endif
    if (Str[i]=='\n')
      break;
    i++;
  }
  Str[i]='\0';
}

void mhmakefileparser::LoadAutoDepsFile(refptr<fileinfo> &DepFile)
{
  if (m_AutoDepFileLoaded && m_AutoDepFileLoaded==DepFile)
    return;  /* This autodep file is already loaded. */

  m_AutoDepFileLoaded=DepFile;

  FILE *pIn=fopen(DepFile->GetFullFileName().c_str(),"rb");
#ifdef _DEBUG
  if (!pIn)
  {
    cerr << "Error opening autodep file "<<DepFile->GetQuotedFullFileName()<<endl;
    return;
  }
#endif
  fread(&m_EnvMd5_32,sizeof(m_EnvMd5_32),1,pIn);
#ifdef _DEBUG
  if (g_PrintAdditionalInfo)
    cout << "Reading Env Md5 from "<<DepFile->GetQuotedFullFileName()<<": "<<hex<<m_EnvMd5_32<<endl;
#endif
  char UsedEnvVars[1024];
  ReadStr(pIn,UsedEnvVars);
  SetVariable(USED_ENVVARS,UsedEnvVars);

  char FileName[MAX_PATH];
  ReadStr(pIn,FileName);
  while (FileName[0])
  {
    refptr<fileinfo> Target=GetFileInfo(FileName);
    autodeps_entry_t &Autodeps=m_AutoDeps[Target];
    ReadStr(pIn,FileName);
    while (FileName[0])
    {
      if (!g_ForceAutoDepRescan)  /* If we are forcing the autodepscan we do not have to load the dependencies. */
      {
        refptr<fileinfo> Dep=GetFileInfo(FileName);
        Autodeps.second.insert(Dep);
        Target->AddDep(Dep);
      }
      ReadStr(pIn,FileName);
    }
    ReadStr(pIn,FileName);
  }

  uint32 Md5_32;
  while (fread(&Md5_32,sizeof(Md5_32),1,pIn))
  {
    ReadStr(pIn,FileName);
    pair <set<refptr<fileinfo>,less_refptrfileinfo >::iterator, bool> pPair=g_FileInfos.insert(new fileinfo(FileName,Md5_32));
    if (!pPair.second)
    {
      #ifdef _DEBUG
      if (!(*pPair.first)->CompareMd5_32(Md5_32) && !(*pPair.first)->CompareMd5_32(0))
        cout << "Warning: trying to set to different md5's for Target "<<(*pPair.first)->GetQuotedFullFileName()<<" Old: "<<hex<<(*pPair.first)->GetCommandsMd5_32()<<" New: "<<Md5_32<<endl;
      if (g_PrintAdditionalInfo)
        cout << "Setting Md5 for Target "<<(*pPair.first)->GetQuotedFullFileName()<<" to "<<hex<<Md5_32<<endl;
      #endif
      (*pPair.first)->SetCommandsMd5_32(Md5_32);  // If it was already there, just update the md5 value
    }
    AddTarget(*pPair.first);
  }

  fclose(pIn);
}

static void MakeDirs(const refptr<fileinfo> & Dir)
{
  refptr<fileinfo> ParentDir=Dir->GetDir();
  if (!ParentDir->GetDate().DoesExist())
  {  /* First make parent dirs */
    MakeDirs(ParentDir);
  }
  if (!Dir->GetDate().DoesExist())
  { /* Create directory */
    mkdir(Dir->GetFullFileName().c_str(),S_IRWXU);
  }
}

void mhmakefileparser::SaveAutoDepsFile()
{
  if (!m_AutoDepsDirty)
    return;

  if (g_Clean
#ifdef _DEBUG
      || g_DoNotExecute || g_GenProjectTree
#endif
     )
    return;   // do not save on clean or if no commands are executed

  string DepFile=ExpandVar(AUTODEPFILE);
  if (!DepFile.size())
  {
    return;
  }
  refptr<fileinfo> pDepFile=GetFileInfo(DepFile);

#ifdef _DEBUG
  if (g_PrintAdditionalInfo)
    cout<<"Saving automatic dependency file "<<DepFile<<endl;
#endif

  FILE *pOut=fopen(pDepFile->GetFullFileName().c_str(),"wb");
  if (!pOut)
  {
    /* Maybe it is because the directory does not exist, so try to create this first */
    MakeDirs(pDepFile->GetDir());
    pOut=fopen(pDepFile->GetFullFileName().c_str(),"wb");

    if (!pOut)
    {
      #ifdef _DEBUG
      if (!g_DoNotExecute)
      #endif
        cerr << "Error creating file "<<DepFile<<endl;
      return;
    }
  }
  // First update the USER_ENVVARS variable and then save it to the dep file together with the md5 string
  CreateUSED_ENVVARS();
  uint32 Md5_32=CreateEnvMd5_32();
  fwrite(&Md5_32,sizeof(Md5_32),1,pOut);
  fprintf(pOut,"%s\n",m_Variables[USED_ENVVARS].c_str());

  autodeps_t::const_iterator It=m_AutoDeps.begin();
  while (It!=m_AutoDeps.end())
  {
    if (!It->second.second.empty())
    {
      fprintf(pOut,"%s\n",It->first->GetFullFileName().c_str());
      deps_t::const_iterator DepIt=It->second.second.begin();
      while (DepIt!=It->second.second.end())
      {
        fprintf(pOut,"%s\n",(*DepIt)->GetFullFileName().c_str());
        DepIt++;
      }
      fprintf(pOut,"\n");
    }
    It++;
  }
  /* Now save the Md5 strings */
  fprintf(pOut,"\n");

  set< const fileinfo * , less_fileinfo>::iterator pIt=m_Targets.begin();
  while (pIt!=m_Targets.end())
  {
    if (!(*pIt)->CompareMd5_32(0))
    {
      (*pIt)->WriteMd5_32(pOut);
      string FileName=(*pIt)->GetFullFileName();
      fwrite(FileName.c_str(),FileName.size(),1,pOut);
      fputc('\n',pOut);
    }
    pIt++;
  }

  fclose(pOut);
}

///////////////////////////////////////////////////////////////////////////////
// Uses the SKIPHEADERS variable to check if we have to skip the header in
// the automatic dependency scanning

bool mhmakefileparser::SkipHeaderFile(const string &FileName)
{
  if (!m_SkipHeadersInitialized)
  {
    m_SkipHeadersInitialized=true;
    string HeadersToSkip=ExpandVar(SKIPHEADERS);
    const char *pTmp=HeadersToSkip.c_str();
    while (*pTmp)
    {
      string Item;
      pTmp=NextItem(pTmp,Item);
      if (Item.find('%')==string::npos)
      {
        m_SkipHeaders.insert(Item);
      }
      else
      {
        m_PercentHeaders.push_back(Item);
      }
    }
  }
  if (m_SkipHeaders.find(FileName)!=m_SkipHeaders.end())
    return true;

  vector<string>::const_iterator It=m_PercentHeaders.begin();
  vector<string>::const_iterator ItEnd=m_PercentHeaders.end();
  while (It!=ItEnd)
  {
    if (PercentMatch(FileName,*It++))
      return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////
// Makes sure that the makefile exports are set in the environment

const mhmakefileparser *mhmakefileparser::m_spCurEnv; // Identifies which makefiles exports are currently set

void mhmakefileparser::InitEnv() const
{
  if (this!=m_spCurEnv)
  {
    /* First restore the previous set environment variables */
    if (m_spCurEnv)
      m_spCurEnv->RestoreEnv();

    ((mhmakefileparser*)this)->SaveEnv();
    vector<string>::const_iterator It=m_Exports.begin();
    vector<string>::const_iterator ItEnd=m_Exports.end();
    while (It!=ItEnd)
    {
      putenv((char *)It->c_str());
      It++;
    }
    m_spCurEnv=this;
  }
}

///////////////////////////////////////////////////////////////////////////////
void mhmakefileparser::SaveEnv()
{
  vector<string>::const_iterator It=m_Exports.begin();
  vector<string>::const_iterator ItEnd=m_Exports.end();
  while (It!=ItEnd)
  {
    string Export=*It++;
    const char *pTmp=Export.c_str();
    string Var;
    NextItem(pTmp,Var,"=");
    const char *pEnv=getenv(Var.c_str());
    string Env;
    if (pEnv)
      Env=pEnv;
    m_SavedExports[Var]=Env;
  }
}

///////////////////////////////////////////////////////////////////////////////
void mhmakefileparser::RestoreEnv() const
{
  if (m_spCurEnv && this!=m_spCurEnv)
  {
    m_spCurEnv->RestoreEnv();  // Make sure that environment variable that were set in the other makefile (m_spCurEnv) are cleared, otherwise the environment is not completely restored.
  }

  map<string,string>::const_iterator It=m_SavedExports.begin();
  map<string,string>::const_iterator ItEnd=m_SavedExports.end();
  while (It!=ItEnd)
  {
#ifdef WIN32
    char Env[1024];
    sprintf(Env,"%s=%s",It->first.c_str(),It->second.c_str());
    putenv(Env);
#else
    /* on linux, only use putenv if you know the supplied string will stay in memory */
    setenv(It->first.c_str(),It->second.c_str(),1);
#endif
    It++;
  }
  ((mhmakefileparser*)this)->m_SavedExports.clear();
  m_spCurEnv=NULL;
}


///////////////////////////////////////////////////////////////////////////////
//
//Checks if the variables retreived from the environment or command-line have been
//changed. Do this at late as possible because they can also be changed in theLexer
//makefiles.
//
void mhmakefileparser::CheckEnv(void)
{
  if (CompareEnv())
  {
    #ifdef _DEBUG
    if (!g_GenProjectTree)
      cout << "Rebuilding everything of "<< m_MakeDir->GetQuotedFullFileName() <<" because environment and/or command-line variables have been changed.\n";
    #endif
    SetRebuildAll();
  }

}

///////////////////////////////////////////////////////////////////////////////
//
//Create a Md5 string from m_GlobalCommandLineVars and USED_ENVVARS
//


static bool SkipVar(const string &Var)
{
  if (Var==WC_REVISION)
    return true;
  if (Var==WC_URL)
    return true;
  if (Var==MAKE)
    return true;
  return false;
}

#define DBGOUT(stuff)

uint32 mhmakefileparser::CreateEnvMd5_32() const
{
  md5_context ctx;
  string Md5;
  string EnvVars=ExpandVar(USED_ENVVARS);
  const char *pTmp=EnvVars.c_str();

  RestoreEnv();  // Restore the environment before creating the md5, so that we have the original environment when the makefile was parsed.

  // Now create the md5 string
  md5_starts( &ctx );

  DBGOUT(cout << "MD5 of " << m_MakeDir->GetQuotedFullFileName() << endl);

  while (*pTmp)
  {
    string Var;
    pTmp=NextItem(pTmp,Var,";");
    if (!SkipVar(Var))
    {
      string Val=ExpandVar(Var);
      transform(Val.begin(),Val.end(),Val.begin(),(int(__CDECL *)(int))toupper);
      DBGOUT(cout << GetMakeDir()->GetQuotedFullFileName() << " -> Setting GetFromEnv var " << Var << " to " << Val << endl);
      md5_update( &ctx, (uint8 *) Var.c_str(), (unsigned long)Var.size());
      md5_update( &ctx, (uint8 *) "=", 1);
      md5_update( &ctx, (uint8 *) Val.c_str(), (unsigned long)Val.size());
    }
  }

  return md5_finish32( &ctx);
}

///////////////////////////////////////////////////////////////////////////////
// Makes sure that the makefile exports are set in the environment

bool mhmakefileparser::CompareEnv() const
{
  uint32 Md5_32=CreateEnvMd5_32();
#ifdef _DEBUG
  if (!g_GenProjectTree && Md5_32!=m_EnvMd5_32)
    cout << "Environment has been changed: "<<hex<<m_EnvMd5_32<<" to "<<Md5_32<<endl;
#endif
  return Md5_32!=m_EnvMd5_32;
}

///////////////////////////////////////////////////////////////////////////////
// Makes sure that the makefile exports are set in the environment
mh_time_t mhmakefileparser::m_sBuildTime;

void mhmakefileparser::InitBuildTime()
{
#ifdef WIN32
  FILETIME FileTime;
  GetSystemTimeAsFileTime(&FileTime);
  m_sBuildTime=g_ZeroTime.ConvertTime(&FileTime);
#else
  m_sBuildTime=time(NULL);
#endif
}

///////////////////////////////////////////////////////////////////////////////
// Returns a variable from the environment or from the command line and adds it the m_UsedEnvVars
string mhmakefileparser::GetFromEnv(const string &Var, bool Cache) const
{
  /* First we look into the command line variables, before we are looking in the environment */
  map<string,string>::const_iterator pLineFind=m_CommandLineVars.find(Var);
  if (pLineFind!=m_CommandLineVars.end())
  {
    ((mhmakefileparser*)this)->m_UsedEnvVars.insert(Var);
    if (Cache)
      ((mhmakefileparser*)this)->m_Variables[Var]=pLineFind->second;
    return pLineFind->second;
  }

  const char *pEnv=getenv(Var.c_str());
  if (!pEnv)
  {
    return g_EmptyString;
  }
  ((mhmakefileparser*)this)->m_UsedEnvVars.insert(Var);

  if (Cache)
    ((mhmakefileparser*)this)->m_Variables[Var]=pEnv;
  return pEnv;
}

///////////////////////////////////////////////////////////////////////////////
// Creates the variable USER_ENVVARS to be saved in the autodeps file
//

void mhmakefileparser::CreateUSED_ENVVARS()
{

  set<string> Variables;

  set<string>::const_iterator It=m_UsedEnvVars.begin();
  set<string>::const_iterator ItEnd=m_UsedEnvVars.end();

  while (It!=ItEnd)
  {
    string Var=*It;
    if (!SkipVar(Var))
    {
      transform(Var.begin(),Var.end(),Var.begin(),(int(__CDECL *)(int))toupper);
      Variables.insert(Var);
    }
    It++;
  }

  map<string,string>::const_iterator CLItEnd=loadedmakefile::sm_Statics.m_GlobalCommandLineVars.end();
  map<string,string>::const_iterator CLIt=loadedmakefile::sm_Statics.m_GlobalCommandLineVars.begin();
  while (CLIt!=CLItEnd)
  {
    string Var=CLIt->first;
    if (!SkipVar(Var))
    {
      transform(Var.begin(),Var.end(),Var.begin(),(int(__CDECL *)(int))toupper);
      Variables.insert(Var);
    }
    CLIt++;
  }

  It=Variables.begin();
  ItEnd=Variables.end();
  string Val;
  while (It!=ItEnd)
  {
    Val+=*It;
    Val+=";";
    It++;
  }

  m_Variables[USED_ENVVARS]=Val;
}

