/*  This file is part of mhmake.
 *
 *  Copyright (C) 2001-2010 marha@sourceforge.net
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

/* -------------- declaration section -------------- */
%{

#include "fileinfo.h"
#include "rule.h"
#include "util.h"

static uint32 LoadMakMd5(fileinfo *pTarget)
{
  string FileName=pTarget->GetFullFileName();
  FileName+=".md5_32";
  FILE *pFile=fopen(FileName.c_str(),"rb");
  if (!pFile)
    return 0;
  uint32 Md5_32=0;
  fread(&Md5_32,sizeof(Md5_32),1,pFile);
  fclose(pFile);
  pTarget->SetCommandsMd5_32(Md5_32);
  return Md5_32;
}

static void SaveMakMd5(fileinfo *pTarget)
{
  string FileName=pTarget->GetFullFileName();
  FileName+=".md5_32";
  FILE *pFile=fopen(FileName.c_str(),"wb");
  if (!pFile)
  {
    throw string("Error creating file ")+FileName;
  }
  pTarget->WriteMd5_32(pFile);
  fclose(pFile);
}

static void ReplaceCurlyBraces(string &String)
{
  int Pos=String.find_first_of('{',0);
  if (Pos!=string::npos)
  {
    /* if not { found, } will not be there eather (or it is a valid syntax, so it may generate an error) */
    do
    {
      String.replace(Pos,1,1,'(');
      Pos=String.find_first_of('{',Pos);
    }
    while (Pos!=string::npos);
    Pos=0;
    while ((Pos=String.find_first_of('}',Pos))!=string::npos)
      String.replace(Pos,1,1,')');
        }
}

#define YY_DECL int mhmakeFlexLexer::yylex(TOKENVALUE &theValue)

%}

%option prefix="mhmake"
%option never-interactive
%option 8bit
%option c++
%option full
%option noyywrap
%option warn

/* -------------- rules section -------------- */
%x INCLUDE IFDEF IF IFNDEF SKIPUNTILELSEORENDIF QUOTE MAKEEXPRES SINGLEQUOTE COMMANDPARSE
%x IFEQ IFNEQ ERRORMACRO MESSAGEMACRO REPARSEMACRO LOAD_MAKEFILE
%x DEFINE

%%

 /*---------------------------------------------------------------------------*/
[ \t\r]*\n[ ][ \t]* {
  yy_set_bol(1); // Make sure the next rule also matches the ^
  yylineno++;
  return mhmakeparser::NEWLINE;
}

[ \t\r]*\n {
  PRINTF(("%s %d: NEWLINE:\n",m_InputFileName.c_str(),yylineno));
  yylineno++;
  return mhmakeparser::NEWLINE;
}

 /*---------------------------------------------------------------------------*/
^[s\-]?include {
  PRINTF(("%s %d: INCLUDE: ",m_InputFileName.c_str(),yylineno));
  BEGIN(INCLUDE);
  unsigned i=0;
  while (strchr(" \t",yytext[i])) i++;
  if (strchr("-s",yytext[i]))
    m_IgnoreIncludeError=true;
  else
    m_IgnoreIncludeError=false;
  return mhmakeparser::INCLUDEMAK;  // Return a newline to be sure that the previous line is completely parse by yacc (in case it is a variable definition)
}

 /*****************************************************************************/
<INCLUDE>[ \t]*      /* eat the whitespace */
 /*---------------------------------------------------------------------------*/
<INCLUDE>[^\r\n]+ { /* got the include file name */
  mhmakeparser *pParser=GetParser();

  /* replace the {} by () before expanding */
  string IncludeFileNames(yytext);
  ReplaceCurlyBraces(IncludeFileNames);
  IncludeFileNames=pParser->ExpandExpression(IncludeFileNames);
  PRINTF(("%s -> %s\n",yytext,IncludeFileNames.c_str()));

  const char *pTmp=IncludeFileNames.c_str();
  while (*pTmp)
  {
    string IncludeFileName;
    pTmp=NextItem(pTmp,IncludeFileName);
    if (!IncludeFileName.empty())
    {
      PRINTF(("%s -> %s\n",yytext,IncludeFileName.c_str()));
      fileinfo *pInclude=GetFileInfo(IncludeFileName,pParser->GetMakeDir());
      /* Already build the include file, in case we already have a rule for it. */
      if (pInclude->GetRule())
      {
        uint32 Md5_32=LoadMakMd5(pInclude);
        pParser->BuildTarget(pInclude);
        if (!pInclude->CompareMd5_32(Md5_32))
          SaveMakMd5(pInclude);
      }

      pParser->AddIncludedMakefile(pInclude);

      string strToInclude=pInclude->GetFullFileName();
      INSTACK *pStackElem=new INSTACK(YY_CURRENT_BUFFER, strToInclude, m_InputFileName, yylineno);
      if ( pStackElem->fail() )
      {
        delete pStackElem;
        if (!m_IgnoreIncludeError)
        {
          mystack::reverse_iterator StackIt=m_IncludeStack.rbegin();
          while (StackIt!=m_IncludeStack.rend())
          {
            cout<<" in "<<(*StackIt)->m_FileName<<" ("<<(*StackIt)->yylineno<<")";
            StackIt++;
          }
          cout<<endl;
          cout<<"Warning error opening file "<<strToInclude<<" in "<<m_InputFileName<<" ("<<yylineno<<")\n";
          pParser->IncludeAfterBuild(strToInclude);
        }
        else
         pInclude->SetPhony();  /* To be sure that no message is printed when mhmake is trying to build the file later */
      }
      else
      {
        m_IncludeStack.push(pStackElem);
        yylineno=1;

        m_InputFileName=strToInclude;

        yypush_buffer_state(yy_create_buffer( pStackElem->GetStream(), YY_BUF_SIZE ));
        yyrestart(pStackElem->GetStream());
      }

    }
  }

  BEGIN(INITIAL);
}

 /*---------------------------------------------------------------------------*/
load_makefile {
  PRINTF(("%s %d: LOAD_MAKEFILE:\n",m_InputFileName.c_str(),yylineno));
  BEGIN(LOAD_MAKEFILE);
  return mhmakeparser::NEWLINE;  // Return a newline to be sure that the previous line is completely parse by yacc (in case it is a variable definition)
}

 /*****************************************************************************/
<LOAD_MAKEFILE>[^\r\n]+ {
  string ListOfMakefiles((const char*)yytext);
  ReplaceCurlyBraces(ListOfMakefiles);
  ListOfMakefiles=GetParser()->ExpandExpression(ListOfMakefiles);
  PRINTF(("%s %d: LOAD_MAKEFILE: '%s'\n",m_InputFileName.c_str(),yylineno,ListOfMakefiles.c_str()));

  const char *pTmp=ListOfMakefiles.c_str();
  while (*pTmp)
  {
    string Item;
    pTmp=NextCharItem(pTmp,Item,';');
    if (Item.empty())
    {
      throw m_InputFileName + "(" + stringify(yylineno) + "): Error in load_makefile statement";
    }
    GetParser()->AddMakefileToMakefilesToLoad(Item);
  }

}
 /*---------------------------------------------------------------------------*/
<LOAD_MAKEFILE>\r?\n {
  yylineno++;
  BEGIN(INITIAL);
  return mhmakeparser::NEWLINE;
}

 /*---------------------------------------------------------------------------*/
[ \t]+ {
  PRINTF(("%s %d: SPACE:\n",m_InputFileName.c_str(),yylineno));
  return mhmakeparser::SPACE;
}

 /*---------------------------------------------------------------------------*/
[ \t]*=[ \t]*\\[ \t\r]*\n[ \t]* {
  PRINTF(("%s %d: EQUAL: %s\n",m_InputFileName.c_str(),yylineno,yytext));
  yylineno++;
  theValue.theString=(const char *)yytext;
  return mhmakeparser::EQUAL;
}

[ \t]*=[ \t]* {
  PRINTF(("%s %d: EQUAL: %s\n",m_InputFileName.c_str(),yylineno,yytext));
  theValue.theString=(const char *)yytext;
  return mhmakeparser::EQUAL;
}

 /*---------------------------------------------------------------------------*/
[ \t]*:=[ \t]*\\[ \t\r]*\n[ \t]* {
  yylineno++;
  PRINTF(("%s %d: IMEQUAL: %s\n",m_InputFileName.c_str(),yylineno,yytext));
  return mhmakeparser::IMEQUAL;
}

[ \t]*:=[ \t]* {
  PRINTF(("%s %d: IMEQUAL: %s\n",m_InputFileName.c_str(),yylineno,yytext));
  return mhmakeparser::IMEQUAL;
}

 /*---------------------------------------------------------------------------*/
[ \t]*\?=[ \t]*\\[ \t\r]*\n[ \t]* {
  yylineno++;
  PRINTF(("%s %d: OPTEQUAL: %s\n",m_InputFileName.c_str(),yylineno,yytext));
  return mhmakeparser::OPTEQUAL;
}

[ \t]*\?=[ \t]* {
  PRINTF(("%s %d: OPTEQUAL: %s\n",m_InputFileName.c_str(),yylineno,yytext));
  return mhmakeparser::OPTEQUAL;
}

 /*---------------------------------------------------------------------------*/
[ \t]*\+=[ \t]*\\[ \t\r]*\n[ \t]* {
  PRINTF(("%s %d: PEQUAL: %s\n",m_InputFileName.c_str(),yylineno,yytext));
  yylineno++;
  return mhmakeparser::PEQUAL;
}

[ \t]*\+=[ \t]* {
  PRINTF(("%s %d: PEQUAL: %s\n",m_InputFileName.c_str(),yylineno,yytext));
  return mhmakeparser::PEQUAL;
}

 /*---------------------------------------------------------------------------*/
[ \t]*;[ \t]*\\[ \t\r]*\n[ \t]* {
  PRINTF(("%s %d: -SEMICOLON (NEWLINE): %s\n",m_InputFileName.c_str(),yylineno,yytext));
  m_curtoken=g_EmptyString;
  yylineno++;
  BEGIN(COMMANDPARSE);
  return mhmakeparser::NEWLINE;
}

[ \t]*;[ \t]* {
  PRINTF(("%s %d: -SEMICOLON (NEWLINE): %s\n",m_InputFileName.c_str(),yylineno,yytext));
  m_curtoken=g_EmptyString;
  BEGIN(COMMANDPARSE);
  return mhmakeparser::NEWLINE;
}

 /*---------------------------------------------------------------------------*/
[ \t]*::[ \t]*\\[ \t\r]*\n[ \t]* {
  PRINTF(("%s %d: DOUBLECOLON: %s\n",m_InputFileName.c_str(),yylineno,yytext));
  yylineno++;
  theValue.theString=(const char *)yytext;
  return mhmakeparser::DOUBLECOLON;
}

[ \t]*::[ \t]* {
  PRINTF(("%s %d: DOUBLECOLON: %s\n",m_InputFileName.c_str(),yylineno,yytext));
  theValue.theString=(const char *)yytext;
  return mhmakeparser::DOUBLECOLON;
}

 /*---------------------------------------------------------------------------*/
[ \t]*:[ \t]*\\[ \t\r]*\n[ \t]* {
  PRINTF(("%s %d: COLON: %s\n",m_InputFileName.c_str(),yylineno,yytext));
  yylineno++;
  theValue.theString=(const char *)yytext;
  return mhmakeparser::COLON;
}

[ \t]*:[ \t]* {
  PRINTF(("%s %d: COLON: %s\n",m_InputFileName.c_str(),yylineno,yytext));
  theValue.theString=(const char *)yytext;
  return mhmakeparser::COLON;
}

 /*---------------------------------------------------------------------------*/
, {
  PRINTF(("%s %d: COMMA: %s\n",m_InputFileName.c_str(),yylineno,yytext));
  theValue.theString=(const char *)yytext;
  return mhmakeparser::COMMA;
}

 /*---------------------------------------------------------------------------*/
^endif {
  if (m_IndentStack.size())
  {
    m_IndentStack.pop();
    PRINTF(("%s %d: %s: depth %d\n",m_InputFileName.c_str(),yylineno,yytext,m_IndentStack.size()));
  }
  else
  {
     throw string("Unexpected endif at line ") + stringify(yylineno) + " of " + m_InputFileName;
  }
}

 /*---------------------------------------------------------------------------*/
^ifdef[ \t]*\\[ \t\r]*\n[ \t]* {
  BEGIN(IFDEF);
  yylineno++;
  return mhmakeparser::NEWLINE;
}

^ifdef[ \t]+ {
  BEGIN(IFDEF);
  return mhmakeparser::NEWLINE;
}

 /*---------------------------------------------------------------------------*/
^if[ \t]*\\[ \t\r]*\n[ \t]* {
  BEGIN(IF);
  yylineno++;
  m_curtoken=g_EmptyString;
  return mhmakeparser::NEWLINE;
}

^if[ \t]+ {
  BEGIN(IF);
  m_curtoken=g_EmptyString;
  return mhmakeparser::NEWLINE;
}

 /*---------------------------------------------------------------------------*/
^ifndef[ \t]*\\[ \t\r]*\n[ \t]* {
  BEGIN(IFNDEF);
  yylineno++;
  return mhmakeparser::NEWLINE;
}

^ifndef[ \t]+ {
  BEGIN(IFNDEF);
  return mhmakeparser::NEWLINE;
}

 /*---------------------------------------------------------------------------*/
^ifeq[ \t]*\\[ \t\r]*\n[ \t]* {
  BEGIN(IFEQ);
  m_curtoken=g_EmptyString;
  yylineno++;
  return mhmakeparser::NEWLINE;
}

^ifeq[ \t]+ {
  BEGIN(IFEQ);
  m_curtoken=g_EmptyString;
  return mhmakeparser::NEWLINE;
}

 /*---------------------------------------------------------------------------*/
^ifneq[ \t]*\\[ \t\r]*\n[ \t]* {
  BEGIN(IFNEQ);
  m_curtoken=g_EmptyString;
  yylineno++;
  return mhmakeparser::NEWLINE;
}

^ifneq[ \t]+ {
  BEGIN(IFNEQ);
  m_curtoken=g_EmptyString;
  return mhmakeparser::NEWLINE;
}

 /*---------------------------------------------------------------------------*/
^else[ \t]* {
  if (m_IndentStack.size() && (!m_IndentStack.top()))
  {
    PRINTF(("%s %d: skipping else: depth %d\n",m_InputFileName.c_str(),yylineno,m_IndentStack.size()));
    m_IndentSkip=m_IndentStack.size();
    m_IndentStack.top()=1;
    BEGIN(SKIPUNTILELSEORENDIF);
  }
  else
  {
     throw string("Unexpected else at line ") + stringify(yylineno) + " of file " + m_InputFileName;
  }
}

 /*****************************************************************************/
<IFEQ>\n {
  yyless(0);
  m_IndentStack.push(0);
  if (GetParser()->IsEqual(m_curtoken))
  {
    PRINTF(("%s %d: Not Skipping ifeq %s: depth %d\n",m_InputFileName.c_str(),yylineno,m_curtoken.c_str(),m_IndentStack.size()));
    BEGIN(INITIAL);
  }
  else
  {
    PRINTF(("%s %d: Skipping ifeq %s: depth %d\n",m_InputFileName.c_str(),yylineno,m_curtoken.c_str(),m_IndentStack.size()));
    m_IndentSkip=m_IndentStack.size();
    BEGIN(SKIPUNTILELSEORENDIF);
  }
}

<IF,IFEQ,IFNEQ>[ \t]*\\[ \t\r]*\n[ \t]* { yylineno++; m_curtoken += g_SpaceString;}

<IF,IFEQ,IFNEQ>\r    /* skip */

<IF,IFEQ,IFNEQ>[^\\\r\n{}\$]+ |
<IF,IFEQ,IFNEQ>\$ |
<IF,IFEQ,IFNEQ>\\                        { m_curtoken += (const char *)yytext; }

 /*****************************************************************************/
<IFNEQ>\n {
  yyless(0);
  m_IndentStack.push(0);
  if (!GetParser()->IsEqual(m_curtoken))
  {
    PRINTF(("%s %d: Not Skipping ifneq %s: depth %d\n",m_InputFileName.c_str(),yylineno,m_curtoken.c_str(),m_IndentStack.size()));
    BEGIN(INITIAL);
  }
  else
  {
    PRINTF(("%s %d: Skipping ifneq %s: depth %d\n",m_InputFileName.c_str(),yylineno,m_curtoken.c_str(),m_IndentStack.size()));
    m_IndentSkip=m_IndentStack.size();
    BEGIN(SKIPUNTILELSEORENDIF);
  }
}

 /*****************************************************************************/
<IF>[ \t\r]*[a-zA-Z0-9_]+[ \t\r]*\n {
  yyless(yyleng-1);
  m_IndentStack.push(0);
#ifndef WIN32
  int lastidx=yyleng-1;
  if (yytext[lastidx]=='\r')
    yytext[lastidx]='\0';
#endif
  string Val=GetParser()->ExpandVar((const char *)yytext);
  if (Val.empty() || Val=="0")
  {
    PRINTF(("%s %d: Skipping if %s: depth %d\n",m_InputFileName.c_str(),yylineno,yytext,m_IndentStack.size()));
    m_IndentSkip=m_IndentStack.size();
    BEGIN(SKIPUNTILELSEORENDIF);
  }
  else
  {
    PRINTF(("%s %d: Not Skipping if %s: depth %d\n",m_InputFileName.c_str(),yylineno,yytext,m_IndentStack.size()));
    BEGIN(INITIAL);
  }
}

 /*---------------------------------------------------------------------------*/
<IF>\n {
  yyless(0);
  m_IndentStack.push(0);
  if (GetParser()->IsExprTrue(m_curtoken))
  {
    PRINTF(("%s %d: Not Skipping ifeq %s: depth %d\n",m_InputFileName.c_str(),yylineno,m_curtoken.c_str(),m_IndentStack.size()));
    BEGIN(INITIAL);
  }
  else
  {
    PRINTF(("%s %d: Skipping ifeq %s: depth %d\n",m_InputFileName.c_str(),yylineno,m_curtoken.c_str(),m_IndentStack.size()));
    m_IndentSkip=m_IndentStack.size();
    BEGIN(SKIPUNTILELSEORENDIF);
  }
}

 /*****************************************************************************/
<IFDEF,IFNDEF>[ \t\r]*  /* skip */

 /*---------------------------------------------------------------------------*/
<IFDEF>[a-zA-Z0-9_]+ {
  m_IndentStack.push(0);
  if (GetParser()->IsDefined((const char *)yytext))
  {
    PRINTF(("%s %d: Not Skipping ifdef %s: depth %d\n",m_InputFileName.c_str(),yylineno,yytext,m_IndentStack.size()));
    BEGIN(INITIAL);
  }
  else
  {
    PRINTF(("%s %d: Skipping ifdef %s: depth %d\n",m_InputFileName.c_str(),yylineno,yytext,m_IndentStack.size()));
    m_IndentSkip=m_IndentStack.size();
    BEGIN(SKIPUNTILELSEORENDIF);
  }
}

 /*****************************************************************************/
<IFNDEF>[a-zA-Z0-9_]+ {
  m_IndentStack.push(0);
  if (!GetParser()->IsDefined((const char *)yytext)) {
    PRINTF(("%s %d: Not Skipping ifndef %s: depth %d\n",m_InputFileName.c_str(),yylineno,yytext,m_IndentStack.size()));
    BEGIN(INITIAL);
  }
  else
  {
    PRINTF(("%s %d: Skipping ifndef %s: depth %d\n",m_InputFileName.c_str(),yylineno,yytext,m_IndentStack.size()));
    m_IndentSkip=m_IndentStack.size();
    BEGIN(SKIPUNTILELSEORENDIF);
  }
}

 /*****************************************************************************/
<SKIPUNTILELSEORENDIF>\n[ ]*endif {
  yylineno++;
  if (!m_IndentStack.size())
  {
    throw string("Unexpected endif at line ") + stringify(yylineno) + " of " + m_InputFileName;
  }
  else
  {
    m_IndentStack.pop();
    PRINTF(("%s %d: endif: depth %d\n",m_InputFileName.c_str(),yylineno,m_IndentStack.size()));
    if (m_IndentStack.size()==m_IndentSkip-1) BEGIN(INITIAL);
  }
}

 /*---------------------------------------------------------------------------*/
<SKIPUNTILELSEORENDIF>\n[ ]*else {
  yylineno++;
  PRINTF(("%s %d: else: depth %d\n",m_InputFileName.c_str(),yylineno,m_IndentStack.size()));
  if (m_IndentStack.top())
  {
    throw string("Unexpected else at line ") + stringify(yylineno) + " of file " + m_InputFileName;
  }
  m_IndentStack.top()=1;
  if (m_IndentStack.size()==m_IndentSkip)
  {
    BEGIN(INITIAL);
  }
}

 /*---------------------------------------------------------------------------*/
<SKIPUNTILELSEORENDIF>\n[ ]*if(def|ndef|eq|neq)? {
  yylineno++;
  m_IndentStack.push(0);
  PRINTF(("%s %d: %s: depth %d\n",m_InputFileName.c_str(),yylineno,yytext+1,m_IndentStack.size()));
}

 /*---------------------------------------------------------------------------*/
<SKIPUNTILELSEORENDIF>[a-zA-Z]+    /* skip */
 /*---------------------------------------------------------------------------*/
<SKIPUNTILELSEORENDIF>[^a-zA-Z\n]+ /* skip */
 /*---------------------------------------------------------------------------*/
<SKIPUNTILELSEORENDIF>\n[ ]*[a-zA-Z]+ yylineno++;
 /*---------------------------------------------------------------------------*/
<SKIPUNTILELSEORENDIF>\n {
  yylineno++;
}

 /*---------------------------------------------------------------------------*/
[ \t]*#[^\n]* {
  PRINTF(("%s %d: -COMMENT: %s\n",m_InputFileName.c_str(),yylineno,yytext));
}

 /*---------------------------------------------------------------------------*/
[ \t]*\\[ \t\r]*\n[ \t]* {
  PRINTF(("%s %d: SPACE:\n",m_InputFileName.c_str(),yylineno));
  yylineno++;
  return mhmakeparser::SPACE;
}

 /*---------------------------------------------------------------------------*/
\.PHONY {
  PRINTF(("%s %d: .PHONY: %s\n",m_InputFileName.c_str(),yylineno,yytext));
  return mhmakeparser::PHONY;
}

 /*---------------------------------------------------------------------------*/
\.AUTODEPS {
  PRINTF(("%s %d: .AUTODEPS: %s\n",m_InputFileName.c_str(),yylineno,yytext));
  return mhmakeparser::AUTODEPS;
}

 /*---------------------------------------------------------------------------*/
export {
  PRINTF(("%s %d: export: %s\n",m_InputFileName.c_str(),yylineno,yytext));
  return mhmakeparser::EXPORT;
}

 /*---------------------------------------------------------------------------*/
^vpath {
  PRINTF(("%s %d: vpath\n",m_InputFileName.c_str(),yylineno));
  return mhmakeparser::VPATH;
}

 /*---------------------------------------------------------------------------*/
[a-zA-Z]:[a-zA-Z0-9\\\._\~\-%\@<&/]+\\[ \t\r]*\n {
  size_t EndIndex=yyleng;
  while (strchr(" \t\r\n\\",yytext[--EndIndex]));
  yyless(EndIndex+1);

  PRINTF(("%s %d: STRING: %s\n",m_InputFileName.c_str(),yylineno,yytext));
  theValue.theString=(const char *)yytext;
  return mhmakeparser::STRING;
}

[a-zA-Z]:[a-zA-Z0-9\\\._\~\-%\@<&/]+ {
  PRINTF(("%s %d: STRING: %s\n",m_InputFileName.c_str(),yylineno,yytext));
  theValue.theString=(const char *)yytext;
  return mhmakeparser::STRING;
}

 /*---------------------------------------------------------------------------*/
([a-zA-Z0-9\\\._\~\-\+%\@<&;/\*\|]|\\\ |\\#)+\\[ \t\r]*\n  {
  size_t EndIndex=yyleng;
  while (strchr(" \t\r\n\\",yytext[--EndIndex]));
  yyless(EndIndex+1);

  PRINTF(("%s %d: STRING: %s\n",m_InputFileName.c_str(),yylineno,yytext));
  theValue.theString=(const char *)yytext;
  return mhmakeparser::STRING;
}

([a-zA-Z0-9\\\._\~\-\+%\@<&;/\*\|]|\\\ |\\#)+\+=  {
  PRINTF(("%s %d: STRING: %s\n",m_InputFileName.c_str(),yylineno,yytext));
  yyless(yyleng-2);
  theValue.theString=(const char *)yytext;
  return mhmakeparser::STRING;
}

([a-zA-Z0-9\\\._\~\-\+%\@<&;/\*\|]|\\\ |\\#)+  {
  PRINTF(("%s %d: STRING: %s\n",m_InputFileName.c_str(),yylineno,yytext));
  theValue.theString=(const char *)yytext;
  return mhmakeparser::STRING;
}

^define[ \t]+[a-zA-Z0-9_\.]+[ \t]*\n {
  const char *pVar=(const char *)yytext;
  while (strchr(" \t",*pVar)) pVar++;
  pVar+=6;
  theValue.theString=pVar;
  BEGIN(DEFINE);
  m_curtoken=g_EmptyString;
  PRINTF(("%s %d: VARDEF: %s\n",m_InputFileName.c_str(),yylineno,m_curtoken.c_str()));
  yylineno++;
  return mhmakeparser::VARDEF;
}

<DEFINE>[ \t]*\\[ \t\r]*\n[ \t]* {
  yylineno++;
  m_curtoken+=g_SpaceString;
}

<DEFINE>. {
  m_curtoken+=(const char *)yytext;
}

<DEFINE>[ \t]*\n[ \t]*endef {
  yylineno++;
  theValue.theString=m_curtoken;
  PRINTF(("%s %d: VARVAL: %s\n",m_InputFileName.c_str(),yylineno,m_curtoken.c_str()));
  BEGIN(INITIAL);
  return mhmakeparser::VARVAL;
}

 /*---------------------------------------------------------------------------*/
\"  {
  BEGIN(QUOTE);
  yymore();
}

 /*---------------------------------------------------------------------------*/
\' {
  BEGIN(SINGLEQUOTE);
  yymore();
}

 /*---------------------------------------------------------------------------*/
\$\(  {
  m_BraceIndent++;
  PRINTF(("%s %d: BEGIN MACRO $(: %d\n",m_InputFileName.c_str(),yylineno,m_BraceIndent));
  BEGIN(MAKEEXPRES);
  m_curtoken=(const char *)yytext;
}

 /*---------------------------------------------------------------------------*/
\$\([ \t]*error[ \t]+ {
  m_BraceIndent++;
  PRINTF(("%s %d: BEGIN ERROR MACRO $(: %d\n",m_InputFileName.c_str(),yylineno,m_BraceIndent));
  BEGIN(ERRORMACRO);
  m_curtoken=g_EmptyString;
  return mhmakeparser::NEWLINE; // Make sure that the previous lines are matched by the bison parser (so that all variables until here are defined)
}

 /*---------------------------------------------------------------------------*/
\$\([ \t]*(message|info)[ \t]+ {
  m_BraceIndent++;
  PRINTF(("%s %d: BEGIN MESSAGE MACRO $(: %d\n",m_InputFileName.c_str(),yylineno,m_BraceIndent));
  BEGIN(MESSAGEMACRO);
  m_curtoken=g_EmptyString;
  return mhmakeparser::NEWLINE; // Make sure that the previous lines are matched by the bison parser (so that all variables until here are defined)
}

 /*---------------------------------------------------------------------------*/
\$\([ \t]*reparse[ \t]+ {
  m_BraceIndent++;
  PRINTF(("%s %d: BEGIN REPARSE MACRO $(: %d\n",m_InputFileName.c_str(),yylineno,m_BraceIndent));
  BEGIN(REPARSEMACRO);
  m_curtoken=g_EmptyString;
  return mhmakeparser::NEWLINE;
}

 /*---------------------------------------------------------------------------*/
\(   {
  PRINTF(("%s %d: OPENBRACE: %s\n",m_InputFileName.c_str(),yylineno,yytext));
  return mhmakeparser::OPENBRACE;
}

 /*---------------------------------------------------------------------------*/
\)   {
  PRINTF(("%s %d: CLOSEBRACE: %s\n",m_InputFileName.c_str(),yylineno,yytext));
  return mhmakeparser::CLOSEBRACE;
}

 /*---------------------------------------------------------------------------*/
\$[<@/$] {
  PRINTF(("%s %d: DOLLAREXPR: %s\n",m_InputFileName.c_str(),yylineno,yytext));
  theValue.theString=(const char *)yytext;
  return mhmakeparser::DOLLAREXPR;
}

 /*---------------------------------------------------------------------------*/
[ \t\r]*\n\t[ \t]* {
  /* token newline */
  PRINTF(("%s %d: NEWLINE\n",m_InputFileName.c_str(),yylineno));
  yylineno++;
  m_curtoken=g_EmptyString;
  BEGIN(COMMANDPARSE);
  return mhmakeparser::NEWLINE;
}

 /*---------------------------------------------------------------------------*/
<*>\$\{ {
  unput('(');
  unput('$');
}
 /*---------------------------------------------------------------------------*/
<*>\} {
  unput(')');
}

 /*---------------------------------------------------------------------------*/
[^\n] {
  PRINTF(("%s %d: ANYCHAR: %d: %s\n",m_InputFileName.c_str(),yylineno,yylineno,yytext));
}

 /*****************************************************************************/

<COMMANDPARSE>[ \t\r]*\n {
  PRINTF(("%s %d: COMMAND: %d: %s\n",m_InputFileName.c_str(),yylineno,yylineno,m_curtoken.c_str()));
  theValue.theString=m_curtoken;
  yylineno++;
  BEGIN(INITIAL);
  return mhmakeparser::COMMAND;
}

 /*---------------------------------------------------------------------------*/
<COMMANDPARSE>[ \t\r]*\n\t[ \t]* {
  PRINTF(("%s %d: COMMAND: %s\n",m_InputFileName.c_str(),yylineno,m_curtoken.c_str()));
  theValue.theString=m_curtoken;
  yylineno++;
  m_curtoken=g_EmptyString;
  return mhmakeparser::COMMAND;
}

 /*---------------------------------------------------------------------------*/
<COMMANDPARSE>[ \t]*\\[ \t\r]*\n[ \t]* {
  yylineno++;
  m_curtoken+=g_SpaceString;
}

 /*---------------------------------------------------------------------------*/
<COMMANDPARSE>[ ]+ {
  m_curtoken+=g_SpaceString;
}

 /*---------------------------------------------------------------------------*/
<COMMANDPARSE>[^ \r\n#\\{}$]+ |
<COMMANDPARSE>\$ |
<COMMANDPARSE>\\ {
  m_curtoken+=(const char *)yytext;
}

 /*---------------------------------------------------------------------------*/
<COMMANDPARSE>[ \t]*\\#[^\n]* {
  int nChars=(int)((strchr((const char *)yytext,'#')-(char*)yytext))+1;
  yyless(nChars);
  m_curtoken+=string(yytext,nChars-2);
  m_curtoken+='#';
}

 /*---------------------------------------------------------------------------*/
<COMMANDPARSE>[ \t]*#[^\n]* {
  PRINTF(("%s %d: -COMMENT: %s\n",m_InputFileName.c_str(),yylineno,yytext));
}

 /*****************************************************************************/
<QUOTE>\" {
  PRINTF(("%s %d: QUOTEDSTRING: %s\n",m_InputFileName.c_str(),yylineno,yytext));
  BEGIN(INITIAL);
  theValue.theString=(const char *)yytext;
  return mhmakeparser::STRING;
}

 /*---------------------------------------------------------------------------*/
<QUOTE>\r /* skip */

<QUOTE>[^\\\"\r\n{}$]+  |
<QUOTE>\$            |
<QUOTE>\\            |
<QUOTE>\\\"          |
<QUOTE>\\#  {
  yymore();
}

 /*****************************************************************************/
<SINGLEQUOTE>\' {
  PRINTF(("%s %d: QUOTEDSTRING: %s\n",m_InputFileName.c_str(),yylineno,yytext));
  BEGIN(INITIAL);
  theValue.theString=(const char *)yytext;
  return mhmakeparser::STRING;
}

 /*---------------------------------------------------------------------------*/
<SINGLEQUOTE>\r /* skip */

<SINGLEQUOTE>[^\\\'\r\n{}$]+ |
<SINGLEQUOTE>\$           |
<SINGLEQUOTE>\\           |
<SINGLEQUOTE>\\\'         |
<SINGLEQUOTE>\\# {
  yymore();
}

 /*****************************************************************************/
<ERRORMACRO>\) {
  m_BraceIndent--;
  PRINTF(("%s %d: CLOSE BRACE ERROR MACRO ): %d\n",m_InputFileName.c_str(),yylineno,m_BraceIndent));
  if (!m_BraceIndent)
  {
    PRINTF(("%s %d: ERRORMACRO: %s\n",m_InputFileName.c_str(),yylineno,m_curtoken.c_str()));
    throw string("\n-> ")+m_InputFileName.c_str()+"("+stringify(yylineno)+") : "+GetParser()->ExpandExpression(m_curtoken);
  } else {
    m_curtoken+=(const char *)yytext;
}
}

 /*****************************************************************************/
<MESSAGEMACRO>\) {
  m_BraceIndent--;
  PRINTF(("%s %d: CLOSE BRACE MESSAGE MACRO ): %d\n",m_InputFileName.c_str(),yylineno,m_BraceIndent));
  if (!m_BraceIndent)
  {
    PRINTF(("%s %d: MESSAGEMACRO: %s\n",m_InputFileName.c_str(),yylineno,yytext));
    cerr<<m_InputFileName.c_str()<<"("<<stringify(yylineno)+") : "<<GetParser()->ExpandExpression(m_curtoken)<<endl;
    BEGIN(INITIAL);
  } else {
    m_curtoken+=(const char *)yytext;
  }
}

 /*****************************************************************************/
<REPARSEMACRO>\) {
  m_BraceIndent--;
  PRINTF(("%s %d: CLOSE BRACE REPARSE MACRO ): %d\n",m_InputFileName.c_str(),yylineno,m_BraceIndent));
  if (!m_BraceIndent)
  {
    PRINTF(("%s %d: REPARSEMACRO: %s\n",m_InputFileName.c_str(),yylineno,yytext));
    string Deps=GetParser()->ExpandExpression(m_curtoken);
    PRINTF(("%s %d: REPARSEMACRO expanded: %s\n",m_InputFileName.c_str(),yylineno,Deps.c_str()));
    string::const_reverse_iterator It=Deps.rbegin()+1; // +1 because we don't want the latest brace
    string::const_reverse_iterator ItBeg=Deps.rend();
    while (It!= ItBeg)
    {
      char Char=*It++;
      if (Char==';') Char='\n';
      unput(Char);
    }
    BEGIN(INITIAL);
  }
  else
  {
    m_curtoken+=(const char *)yytext;
  }
}

 /*****************************************************************************/
<MAKEEXPRES>\) {
  m_BraceIndent--;
  PRINTF(("%s %d: CLOSE BRACE MAKEEXPRES MACRO ): %d\n",m_InputFileName.c_str(),yylineno,m_BraceIndent));
  if (!m_BraceIndent)
  {
    BEGIN(INITIAL);
    m_curtoken+=(const char *)yytext;
    theValue.theString=m_curtoken;
    PRINTF(("%s %d: DOLLAREXPR: %s\n",m_InputFileName.c_str(),yylineno,m_curtoken.c_str()));
    return mhmakeparser::DOLLAREXPR;
  }
  else
  {
    m_curtoken+=(const char *)yytext;
  }
}

 /*---------------------------------------------------------------------------*/
<MAKEEXPRES,ERRORMACRO,MESSAGEMACRO,REPARSEMACRO>\$\( {
  m_BraceIndent++;
  PRINTF(("%s %d: MACRO extra $(: %d\n",m_InputFileName.c_str(),yylineno,m_BraceIndent));
  m_curtoken+=(const char *)yytext;
}

 /*---------------------------------------------------------------------------*/
<MAKEEXPRES,ERRORMACRO,MESSAGEMACRO,REPARSEMACRO>[^$\(\){}\r\n\\]+ |
<MAKEEXPRES,ERRORMACRO,MESSAGEMACRO,REPARSEMACRO>\$        |
<MAKEEXPRES,ERRORMACRO,MESSAGEMACRO,REPARSEMACRO>\\        |
<MAKEEXPRES,ERRORMACRO,MESSAGEMACRO,REPARSEMACRO>\( {
  m_curtoken+=(const char *)yytext;
}
<MAKEEXPRES,ERRORMACRO,MESSAGEMACRO,REPARSEMACRO>[ \t\r]*\\[ \t\r]*\n[ \t\r]* {
  yylineno++;
  m_curtoken+=g_SpaceString;
}

<SKIPUNTILELSEORENDIF><<EOF>> {
  throw string("Missing endif or else statement. #else or #endif used?");
}

<<EOF>> {
  if (m_BraceIndent)
  {
    throw string("Missing closing ) of macro usage in ") + m_InputFileName;
  }
  if (!m_IncludeStack.size())
  {
    if (m_IndentStack.size())
    {
      throw string("Missing endif or else statement in ") + m_InputFileName + ". #else or #endif used";
    }
    yyterminate();
  }
  else
  {
    INSTACK *pInStack=m_IncludeStack.top();
    yypop_buffer_state();
    m_InputFileName=pInStack->m_FileName;
    yylineno=pInStack->yylineno;
    m_IncludeStack.pop();
  }
}
%%

