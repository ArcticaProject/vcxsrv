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

/* -------------- declaration section -------------- */
%name mhmakelexer
%header{
#include "mhmakeparser.h"
#include "fileinfo.h"
#include "rule.h"
#include "util.h"

static uint32 LoadMakMd5(refptr<fileinfo> &Target)
{
  string FileName=Target->GetFullFileName();
  FileName+=".md5_32";
  FILE *pFile=fopen(FileName.c_str(),"rb");
  if (!pFile)
    return 0;
  uint32 Md5_32=0;
  fread(&Md5_32,sizeof(Md5_32),1,pFile);
  fclose(pFile);
  Target->SetCommandsMd5_32(Md5_32);
  return Md5_32;
}

static void SaveMakMd5(refptr<fileinfo> &Target)
{
  string FileName=Target->GetFullFileName();
  FileName+=".md5_32";
  FILE *pFile=fopen(FileName.c_str(),"wb");
  if (!pFile)
  {
    throw string("Error creating file ")+FileName;
  }
  Target->WriteMd5_32(pFile);
  fclose(pFile);
}

%}

%define LEX_PARAM TOKENVALUE &theValue
%define MEMBERS public: \
        struct INSTACK\
        {\
          YY_BUFFER_STATE m_BufferState;\
          string m_FileName;\
          int m_Line;\
          INSTACK(YY_BUFFER_STATE BufferState,string FileName,int Line) :\
            m_BufferState(BufferState),m_FileName(FileName),m_Line(Line) {}\
        };\
        int m_Line;\
        int m_BraceIndent;\
        size_t m_IndentSkip;\
        iterstack<int> m_IndentStack;\
        bool m_IgnoreIncludeError;\
        string m_InputFileName; \
        string m_curtoken; \
        iterstack<INSTACK> m_IncludeStack; \
        mhmakeparser *m_pParser;\
        mhmakeparser *GetParser()\
        {\
          return m_pParser;\
        }

%define CONSTRUCTOR_INIT : m_Line(1), m_BraceIndent(0)

/* -------------- rules section -------------- */
%x INCLUDE IFDEF IF IFNDEF SKIPUNTILELSEORENDIF QUOTE MAKEEXPRES SINGLEQUOTE COMMANDPARSE
%x IFEQ IFNEQ ERRORMACRO MESSAGEMACRO REPARSEMACRO LOAD_MAKEFILE

%%

 /*---------------------------------------------------------------------------*/
[ \t\r]*\n[ ][ \t]* |
[ \t\r]*\n {
  PRINTF(("%s %d: NEWLINE:\n",m_InputFileName.c_str(),m_Line));
  m_Line++;
  return mhmakeparser::NEWLINE;
}

 /*---------------------------------------------------------------------------*/
^[s\-]?include {
  PRINTF(("%s %d: INCLUDE: ",m_InputFileName.c_str(),m_Line));
  BEGIN(INCLUDE);
  if (yytext[0]=='-' || yytext[0]=='s')
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

  string IncludeFileNames=pParser->ExpandExpression((const char*)yytext);
  PRINTF(("%s -> %s\n",yytext,IncludeFileNames.c_str()));

  const char *pTmp=IncludeFileNames.c_str();
  while (*pTmp)
  {
    string IncludeFileName;
    pTmp=NextItem(pTmp,IncludeFileName);
    if (!IncludeFileName.empty())
    {
      PRINTF(("%s -> %s\n",yytext,IncludeFileName.c_str()));
      refptr<fileinfo> pInclude=GetFileInfo(IncludeFileName,pParser->GetMakeDir());
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
      FILE *pTmp = ::fopen(strToInclude.c_str(), "r" );
      if ( ! pTmp )
      {
        if (!m_IgnoreIncludeError)
        {
          iterstack<INSTACK>::reverse_iterator StackIt=m_IncludeStack.rbegin();
          while (StackIt!=m_IncludeStack.rend())
          {
            cout<<" in "<<StackIt->m_FileName<<" ("<<StackIt->m_Line<<")";
            StackIt++;
          }
          cout<<endl;
          cout<<"Warning error opening file "<<strToInclude<<" in "<<m_InputFileName<<" ("<<m_Line<<")\n";
          pParser->IncludeAfterBuild(strToInclude);
        }
      }
      else
      {
        m_IncludeStack.push(INSTACK(YY_mhmakelexer_CURRENT_BUFFER,m_InputFileName,m_Line));
        m_Line=1;

        yyin=pTmp;
        m_InputFileName=strToInclude;

        YY_mhmakelexer_SWITCH_TO_BUFFER(YY_mhmakelexer_CREATE_BUFFER( yyin, YY_BUF_SIZE ) );
      }

    }
  }

  BEGIN(INITIAL);
}

 /*---------------------------------------------------------------------------*/
load_makefile {
  PRINTF(("%s %d: LOAD_MAKEFILE:\n",m_InputFileName.c_str(),m_Line));
  BEGIN(LOAD_MAKEFILE);
  return mhmakeparser::NEWLINE;  // Return a newline to be sure that the previous line is completely parse by yacc (in case it is a variable definition)
}

 /*****************************************************************************/
<LOAD_MAKEFILE>[^\r\n]+ {
  string ListOfMakefiles=GetParser()->ExpandExpression((const char*)yytext);
  PRINTF(("%s %d: LOAD_MAKEFILE: '%s'\n",m_InputFileName.c_str(),m_Line,ListOfMakefiles.c_str()));

  const char *pTmp=ListOfMakefiles.c_str();
  while (*pTmp)
  {
    string Item;
    pTmp=NextCharItem(pTmp,Item,';');
    if (Item.empty())
    {
      throw m_InputFileName + "(" + stringify(m_Line) + "): Error in load_makefile statement";
    }
    GetParser()->AddMakefileToMakefilesToLoad(Item);
  }

}
 /*---------------------------------------------------------------------------*/
<LOAD_MAKEFILE>\r?\n {
  m_Line++;
  BEGIN(INITIAL);
  return mhmakeparser::NEWLINE;
}

 /*---------------------------------------------------------------------------*/
[\ \t]+ {
  PRINTF(("%s %d: SPACE:\n",m_InputFileName.c_str(),m_Line));
  return mhmakeparser::SPACE;
}

 /*---------------------------------------------------------------------------*/
[\ t]*=[ \t]*\\[ \t\r]*\n[ \t]* {
  PRINTF(("%s %d: EQUAL: %s\n",m_InputFileName.c_str(),m_Line,yytext));
  m_Line++;
  theValue.theString=(const char *)yytext;
  return mhmakeparser::EQUAL;
}

[\ t]*=[ \t]* {
  PRINTF(("%s %d: EQUAL: %s\n",m_InputFileName.c_str(),m_Line,yytext));
  theValue.theString=(const char *)yytext;
  return mhmakeparser::EQUAL;
}

 /*---------------------------------------------------------------------------*/
[\ t]*:=[ \t]*\\[ \t\r]*\n[ \t]* {
  m_Line++;
  PRINTF(("%s %d: IMEQUAL: %s\n",m_InputFileName.c_str(),m_Line,yytext));
  return mhmakeparser::IMEQUAL;
}

[ \t]*:=[ \t]* {
  PRINTF(("%s %d: IMEQUAL: %s\n",m_InputFileName.c_str(),m_Line,yytext));
  return mhmakeparser::IMEQUAL;
}

 /*---------------------------------------------------------------------------*/
[\ t]*\?=[ \t]*\\[ \t\r]*\n[ \t]* {
  m_Line++;
  PRINTF(("%s %d: IMEQUAL: %s\n",m_InputFileName.c_str(),m_Line,yytext));
  return mhmakeparser::OPTEQUAL;
}

[ \t]*\?=[ \t]* {
  PRINTF(("%s %d: IMEQUAL: %s\n",m_InputFileName.c_str(),m_Line,yytext));
  return mhmakeparser::OPTEQUAL;
}

 /*---------------------------------------------------------------------------*/
[\ t]*\+=[ \t]*\\[ \t\r]*\n[ \t]* {
  PRINTF(("%s %d: PEQUAL: %s\n",m_InputFileName.c_str(),m_Line,yytext));
  m_Line++;
  return mhmakeparser::PEQUAL;
}

[ \t]*\+=[ \t]* {
  PRINTF(("%s %d: PEQUAL: %s\n",m_InputFileName.c_str(),m_Line,yytext));
  return mhmakeparser::PEQUAL;
}

 /*---------------------------------------------------------------------------*/
[\ t]*;[ \t]*\\[ \t\r]*\n[ \t]* {
  PRINTF(("%s %d: -SEMICOLON (NEWLINE): %s\n",m_InputFileName.c_str(),m_Line,yytext));
  m_curtoken=g_EmptyString;
  m_Line++;
  BEGIN(COMMANDPARSE);
  return mhmakeparser::NEWLINE;
}

[ \t]*;[ \t]* {
  PRINTF(("%s %d: -SEMICOLON (NEWLINE): %s\n",m_InputFileName.c_str(),m_Line,yytext));
  m_curtoken=g_EmptyString;
  BEGIN(COMMANDPARSE);
  return mhmakeparser::NEWLINE;
}

 /*---------------------------------------------------------------------------*/
[\ t]*::[ \t]*\\[ \t\r]*\n[ \t]* {
  PRINTF(("%s %d: DOUBLECOLON: %s\n",m_InputFileName.c_str(),m_Line,yytext));
  m_Line++;
  theValue.theString=(const char *)yytext;
  return mhmakeparser::DOUBLECOLON;
}

[ \t]*::[ \t]* {
  PRINTF(("%s %d: DOUBLECOLON: %s\n",m_InputFileName.c_str(),m_Line,yytext));
  theValue.theString=(const char *)yytext;
  return mhmakeparser::DOUBLECOLON;
}

 /*---------------------------------------------------------------------------*/
[\ t]*:[ \t]*\\[ \t\r]*\n[ \t]* {
  PRINTF(("%s %d: COLON: %s\n",m_InputFileName.c_str(),m_Line,yytext));
  m_Line++;
  theValue.theString=(const char *)yytext;
  return mhmakeparser::COLON;
}

[ \t]*:[ \t]* {
  PRINTF(("%s %d: COLON: %s\n",m_InputFileName.c_str(),m_Line,yytext));
  theValue.theString=(const char *)yytext;
  return mhmakeparser::COLON;
}

 /*---------------------------------------------------------------------------*/
, {
  PRINTF(("%s %d: COMMA: %s\n",m_InputFileName.c_str(),m_Line,yytext));
  theValue.theString=(const char *)yytext;
  return mhmakeparser::COMMA;
}

 /*---------------------------------------------------------------------------*/
^[ \t]*endif {
  if (m_IndentStack.size())
  {
    m_IndentStack.pop();
    PRINTF(("%s %d: %s: depth %d\n",m_InputFileName.c_str(),m_Line,yytext,m_IndentStack.size()));
  }
  else
  {
     throw string("Unexpected endif at line ") + stringify(m_Line) + " of " + m_InputFileName;
  }
}

 /*---------------------------------------------------------------------------*/
^[ \t]*ifdef[ \t]*\\[ \t\r]*\n[ \t]* {
  BEGIN(IFDEF);
  m_Line++;
  return mhmakeparser::NEWLINE;
}

^[ \t]*ifdef[ \t]+ {
  BEGIN(IFDEF);
  return mhmakeparser::NEWLINE;
}

 /*---------------------------------------------------------------------------*/
^[ \t]*if[ \t]*\\[ \t\r]*\n[ \t]* {
  BEGIN(IF);
  m_Line++;
  return mhmakeparser::NEWLINE;
}

^[ \t]*if[ \t]+ {
  BEGIN(IF);
  return mhmakeparser::NEWLINE;
}

 /*---------------------------------------------------------------------------*/
^[ \t]*ifndef[ \t]*\\[ \t\r]*\n[ \t]* {
  BEGIN(IFNDEF);
  m_Line++;
  return mhmakeparser::NEWLINE;
}

^[ \t]*ifndef[ \t]+ {
  BEGIN(IFNDEF);
  return mhmakeparser::NEWLINE;
}

 /*---------------------------------------------------------------------------*/
^[ \t]*ifeq[ \t]*\\[ \t\r]*\n[ \t]* {
  BEGIN(IFEQ);
  m_curtoken=g_EmptyString;
  m_Line++;
  return mhmakeparser::NEWLINE;
}

^[ \t]*ifeq[ \t]+ {
  BEGIN(IFEQ);
  m_curtoken=g_EmptyString;
  return mhmakeparser::NEWLINE;
}

 /*---------------------------------------------------------------------------*/
^[ \t]*ifneq[ \t]*\\[ \t\r]*\n[ \t]* {
  BEGIN(IFNEQ);
  m_curtoken=g_EmptyString;
  m_Line++;
  return mhmakeparser::NEWLINE;
}

^[ \t]*ifneq[ \t]+ {
  BEGIN(IFNEQ);
  m_curtoken=g_EmptyString;
  return mhmakeparser::NEWLINE;
}

 /*---------------------------------------------------------------------------*/
^[ \t]*else[ \t]* {
  if (m_IndentStack.size() && (!m_IndentStack.top()))
  {
    PRINTF(("%s %d: skipping else: depth %d\n",m_InputFileName.c_str(),m_Line,m_IndentStack.size()));
    m_IndentSkip=m_IndentStack.size();
    m_IndentStack.top()=1;
    BEGIN(SKIPUNTILELSEORENDIF);
  }
  else
  {
     throw string("Unexpected else at line ") + stringify(m_Line) + " of file " + m_InputFileName;
  }
}

 /*****************************************************************************/
<IFEQ>\n {
  unput( yytext[0] );
  m_IndentStack.push(0);
  if (GetParser()->IsEqual(m_curtoken))
  {
    PRINTF(("%s %d: Not Skipping ifeq %s: depth %d\n",m_InputFileName.c_str(),m_Line,m_curtoken.c_str(),m_IndentStack.size()));
    BEGIN(INITIAL);
  }
  else
  {
    PRINTF(("%s %d: Skipping ifeq %s: depth %d\n",m_InputFileName.c_str(),m_Line,m_curtoken.c_str(),m_IndentStack.size()));
    m_IndentSkip=m_IndentStack.size();
    BEGIN(SKIPUNTILELSEORENDIF);
  }
}

<IFEQ,IFNEQ>[ \t]*\\[ \t\r]*\n[ \t]* { m_Line++; m_curtoken += g_SpaceString;}

<IFEQ,IFNEQ>\r    /* skip */

<IFEQ,IFNEQ>[^\\\r\n]+ |
<IFEQ,IFNEQ>\\                        { m_curtoken += (const char *)yytext; }

 /*****************************************************************************/
<IFNEQ>\n {
  unput( yytext[0] );
  m_IndentStack.push(0);
  if (!GetParser()->IsEqual(m_curtoken))
  {
    PRINTF(("%s %d: Not Skipping ifneq %s: depth %d\n",m_InputFileName.c_str(),m_Line,m_curtoken.c_str(),m_IndentStack.size()));
    BEGIN(INITIAL);
  }
  else
  {
    PRINTF(("%s %d: Skipping ifneq %s: depth %d\n",m_InputFileName.c_str(),m_Line,m_curtoken.c_str(),m_IndentStack.size()));
    m_IndentSkip=m_IndentStack.size();
    BEGIN(SKIPUNTILELSEORENDIF);
  }
}

 /*****************************************************************************/
<IF,IFDEF,IFNDEF>[ \t\r]*  /* skip */

 /*---------------------------------------------------------------------------*/
<IF>[a-zA-Z0-9_]+ {
  m_IndentStack.push(0);
  string Val=GetParser()->ExpandVar((const char *)yytext);
  if (Val.empty() || Val=="0")
  {
    PRINTF(("%s %d: Skipping if %s: depth %d\n",m_InputFileName.c_str(),m_Line,yytext,m_IndentStack.size()));
    m_IndentSkip=m_IndentStack.size();
    BEGIN(SKIPUNTILELSEORENDIF);
  }
  else
  {
    PRINTF(("%s %d: Not Skipping if %s: depth %d\n",m_InputFileName.c_str(),m_Line,yytext,m_IndentStack.size()));
    BEGIN(INITIAL);
  }
}

 /*---------------------------------------------------------------------------*/
<IFDEF>[a-zA-Z0-9_]+ {
  m_IndentStack.push(0);
  if (GetParser()->IsDefined((const char *)yytext))
  {
    PRINTF(("%s %d: Not Skipping ifdef %s: depth %d\n",m_InputFileName.c_str(),m_Line,yytext,m_IndentStack.size()));
    BEGIN(INITIAL);
  }
  else
  {
    PRINTF(("%s %d: Skipping ifdef %s: depth %d\n",m_InputFileName.c_str(),m_Line,yytext,m_IndentStack.size()));
    m_IndentSkip=m_IndentStack.size();
    BEGIN(SKIPUNTILELSEORENDIF);
  }
}

 /*****************************************************************************/
<IFNDEF>[a-zA-Z0-9_]+ {
  m_IndentStack.push(0);
  if (!GetParser()->IsDefined((const char *)yytext)) {
    PRINTF(("%s %d: Not Skipping ifndef %s: depth %d\n",m_InputFileName.c_str(),m_Line,yytext,m_IndentStack.size()));
    BEGIN(INITIAL);
  }
  else
  {
    PRINTF(("%s %d: Skipping ifndef %s: depth %d\n",m_InputFileName.c_str(),m_Line,yytext,m_IndentStack.size()));
    m_IndentSkip=m_IndentStack.size();
    BEGIN(SKIPUNTILELSEORENDIF);
  }
}

 /*****************************************************************************/
<SKIPUNTILELSEORENDIF>\n[ ]*endif {
  m_Line++;
  if (!m_IndentStack.size())
  {
    throw string("Unexpected endif at line ") + stringify(m_Line) + " of " + m_InputFileName;
  }
  else
  {
    m_IndentStack.pop();
    PRINTF(("%s %d: endif: depth %d\n",m_InputFileName.c_str(),m_Line,m_IndentStack.size()));
    if (m_IndentStack.size()==m_IndentSkip-1) BEGIN(INITIAL);
  }
}

 /*---------------------------------------------------------------------------*/
<SKIPUNTILELSEORENDIF>\n[ ]*else {
  m_Line++;
  PRINTF(("%s %d: else: depth %d\n",m_InputFileName.c_str(),m_Line,m_IndentStack.size()));
  if (m_IndentStack.top())
  {
    throw string("Unexpected else at line ") + stringify(m_Line) + " of file " + m_InputFileName;
  }
  m_IndentStack.top()=1;
  if (m_IndentStack.size()==m_IndentSkip)
  {
    BEGIN(INITIAL);
  }
}

 /*---------------------------------------------------------------------------*/
<SKIPUNTILELSEORENDIF>\n[ ]*if(def|ndef|eq|neq) {
  m_Line++;
  m_IndentStack.push(0);
  PRINTF(("%s %d: %s: depth %d\n",m_InputFileName.c_str(),m_Line,yytext+1,m_IndentStack.size()));
}

 /*---------------------------------------------------------------------------*/
<SKIPUNTILELSEORENDIF>[a-zA-Z]+    /* skip */
 /*---------------------------------------------------------------------------*/
<SKIPUNTILELSEORENDIF>[^a-zA-Z\n]+ /* skip */
 /*---------------------------------------------------------------------------*/
<SKIPUNTILELSEORENDIF>\n[ ]*[a-zA-Z]+ m_Line++;
 /*---------------------------------------------------------------------------*/
<SKIPUNTILELSEORENDIF>\n {
  m_Line++;
}

 /*---------------------------------------------------------------------------*/
[ \t]*#[^\n]* {
  PRINTF(("%s %d: -COMMENT: %s\n",m_InputFileName.c_str(),m_Line,yytext));
}

 /*---------------------------------------------------------------------------*/
[ \t]*\\[ \t\r]*\n[ \t]* {
  PRINTF(("%s %d: SPACE:\n",m_InputFileName.c_str(),m_Line));
  m_Line++;
  return mhmakeparser::SPACE;
}

 /*---------------------------------------------------------------------------*/
\.PHONY {
  PRINTF(("%s %d: .PHONY: %s\n",m_InputFileName.c_str(),m_Line,yytext));
  return mhmakeparser::PHONY;
}

 /*---------------------------------------------------------------------------*/
export {
  PRINTF(("%s %d: export: %s\n",m_InputFileName.c_str(),m_Line,yytext));
  return mhmakeparser::EXPORT;
}


 /*---------------------------------------------------------------------------*/
[a-zA-Z]:[a-zA-Z0-9\\\._\~\-%\@<&/]+\\[ \t\r]*\n {
  size_t EndIndex=::strlen((const char*)yytext);
  while (strchr(" \t\r\n\\",yytext[--EndIndex]));
  yyless(EndIndex+1);

  PRINTF(("%s %d: STRING: %s\n",m_InputFileName.c_str(),m_Line,yytext));
  theValue.theString=(const char *)yytext;
  return mhmakeparser::STRING;
}

[a-zA-Z]:[a-zA-Z0-9\\\._\~\-%\@<&/]+ {
  PRINTF(("%s %d: STRING: %s\n",m_InputFileName.c_str(),m_Line,yytext));
  theValue.theString=(const char *)yytext;
  return mhmakeparser::STRING;
}

 /*---------------------------------------------------------------------------*/
([a-zA-Z0-9\\\._\~\-\+%\@<&;/\*\|]|\\\ |\\#)+\\[ \t\r]*\n  {
  size_t EndIndex=::strlen((const char*)yytext);
  while (strchr(" \t\r\n\\",yytext[--EndIndex]));
  yyless(EndIndex+1);

  PRINTF(("%s %d: STRING: %s\n",m_InputFileName.c_str(),m_Line,yytext));
  theValue.theString=(const char *)yytext;
  return mhmakeparser::STRING;
}

([a-zA-Z0-9\\\._\~\-\+%\@<&;/\*\|]|\\\ |\\#)+\+=  {
  size_t Len;
  PRINTF(("%s %d: STRING: %s\n",m_InputFileName.c_str(),m_Line,yytext));
  Len=strlen((const char *)yytext)-2;
  yyless(Len);
  theValue.theString=(const char *)yytext;
  return mhmakeparser::STRING;
}

([a-zA-Z0-9\\\._\~\-\+%\@<&;/\*\|]|\\\ |\\#)+  {
  PRINTF(("%s %d: STRING: %s\n",m_InputFileName.c_str(),m_Line,yytext));
  theValue.theString=(const char *)yytext;
  return mhmakeparser::STRING;
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
  PRINTF(("%s %d: BEGIN MACRO $(: %d\n",m_InputFileName.c_str(),m_Line,m_BraceIndent));
  BEGIN(MAKEEXPRES);
  yymore();
}

 /*---------------------------------------------------------------------------*/
\$\([ \t]*error[ \t]+ {
  m_BraceIndent++;
  PRINTF(("%s %d: BEGIN ERROR MACRO $(: %d\n",m_InputFileName.c_str(),m_Line,m_BraceIndent));
  BEGIN(ERRORMACRO);
  m_curtoken=g_EmptyString;
}

 /*---------------------------------------------------------------------------*/
\$\([ \t]*message[ \t]+ {
  m_BraceIndent++;
  PRINTF(("%s %d: BEGIN MESSAGE MACRO $(: %d\n",m_InputFileName.c_str(),m_Line,m_BraceIndent));
  BEGIN(MESSAGEMACRO);
  m_curtoken=g_EmptyString;
}

 /*---------------------------------------------------------------------------*/
\$\([ \t]*reparse[ \t]+ {
  m_BraceIndent++;
  PRINTF(("%s %d: BEGIN REPARSE MACRO $(: %d\n",m_InputFileName.c_str(),m_Line,m_BraceIndent));
  BEGIN(REPARSEMACRO);
  m_curtoken=g_EmptyString;
  return mhmakeparser::NEWLINE;
}

 /*---------------------------------------------------------------------------*/
\(   {
  PRINTF(("%s %d: OPENBRACE: %s\n",m_InputFileName.c_str(),m_Line,yytext));
  return mhmakeparser::OPENBRACE;
}

 /*---------------------------------------------------------------------------*/
\)   {
  PRINTF(("%s %d: CLOSEBRACE: %s\n",m_InputFileName.c_str(),m_Line,yytext));
  return mhmakeparser::CLOSEBRACE;
}

 /*---------------------------------------------------------------------------*/
\$[<@/$] {
  PRINTF(("%s %d: DOLLAREXPR: %s\n",m_InputFileName.c_str(),m_Line,yytext));
  theValue.theString=(const char *)yytext;
  return mhmakeparser::DOLLAREXPR;
}

 /*---------------------------------------------------------------------------*/
[ \t\r]*\n\t[ \t]* {
  /* token newline */
  PRINTF(("%s %d: NEWLINE\n",m_InputFileName.c_str(),m_Line));
  m_Line++;
  m_curtoken=g_EmptyString;
  BEGIN(COMMANDPARSE);
  return mhmakeparser::NEWLINE;
}

 /*---------------------------------------------------------------------------*/
[^\n] {
  PRINTF(("%s %d: ANYCHAR: %d: %s\n",m_InputFileName.c_str(),m_Line,m_Line,yytext));
}

 /*****************************************************************************/

<COMMANDPARSE>[ \t\r]*\n {
  PRINTF(("%s %d: COMMAND: %d: %s\n",m_InputFileName.c_str(),m_Line,m_Line,m_curtoken.c_str()));
  theValue.theString=m_curtoken;
  m_Line++;
  BEGIN(INITIAL);
  return mhmakeparser::COMMAND;
}

 /*---------------------------------------------------------------------------*/
<COMMANDPARSE>[ \t\r]*\n\t[ \t]* {
  PRINTF(("%s %d: COMMAND: %s\n",m_InputFileName.c_str(),m_Line,m_curtoken.c_str()));
  theValue.theString=m_curtoken;
  m_Line++;
  m_curtoken=g_EmptyString;
  return mhmakeparser::COMMAND;
}

 /*---------------------------------------------------------------------------*/
<COMMANDPARSE>[ \t]*\\[ \t\r]*\n[ \t]* {
  m_Line++;
  m_curtoken+=g_SpaceString;
}

 /*---------------------------------------------------------------------------*/
<COMMANDPARSE>[ ]+ {
  m_curtoken+=g_SpaceString;
}

 /*---------------------------------------------------------------------------*/
<COMMANDPARSE>[^ \r\n#\\]+ |
<COMMANDPARSE>\\ {
  m_curtoken+=(const char *)yytext;
}

 /*---------------------------------------------------------------------------*/
<COMMANDPARSE>[ \t]*\\#[^\n]* {
  char ToAdd[100];
  int i=0;
  int nChars=(int)((strchr((const char *)yytext,'#')-(char*)yytext))+1;
  while (strchr(" \t",yytext[i]))
  {
    ToAdd[i]=yytext[i];
    i++;
  }
  ToAdd[i++]='#';
  ToAdd[i++]=0;
  m_curtoken+=ToAdd;
  yyless(nChars);
}

 /*---------------------------------------------------------------------------*/
<COMMANDPARSE>[ \t]*#[^\n]* {
  PRINTF(("%s %d: -COMMENT: %s\n",m_InputFileName.c_str(),m_Line,yytext));
}

 /*****************************************************************************/
<QUOTE>\" {
  PRINTF(("%s %d: QUOTEDSTRING: %s\n",m_InputFileName.c_str(),m_Line,yytext));
  BEGIN(INITIAL);
  theValue.theString=(const char *)yytext;
  return mhmakeparser::STRING;
}

 /*---------------------------------------------------------------------------*/
<QUOTE>\r /* skip */

<QUOTE>[^\\\"\r\n]+  |
<QUOTE>\\            |
<QUOTE>\\\"          |
<QUOTE>\\#  {
  yymore();
}

 /*****************************************************************************/
<SINGLEQUOTE>\' {
  PRINTF(("%s %d: QUOTEDSTRING: %s\n",m_InputFileName.c_str(),m_Line,yytext));
  BEGIN(INITIAL);
  theValue.theString=(const char *)yytext;
  return mhmakeparser::STRING;
}

 /*---------------------------------------------------------------------------*/
<SINGLEQUOTE>\r /* skip */

<SINGLEQUOTE>[^\\\'\r\n]+ |
<SINGLEQUOTE>\\           |
<SINGLEQUOTE>\\\'         |
<SINGLEQUOTE>\\# {
  yymore();
}

 /*****************************************************************************/
<ERRORMACRO>\) {
  m_BraceIndent--;
  PRINTF(("%s %d: CLOSE BRACE ERROR MACRO ): %d\n",m_InputFileName.c_str(),m_Line,m_BraceIndent));
  if (!m_BraceIndent)
  {
    PRINTF(("%s %d: ERRORMACRO: %s\n",m_InputFileName.c_str(),m_Line,yytext));
    yyless(0);
    throw GetParser()->ExpandExpression((const char*)yytext);
  } else {
    yymore();
  }
}

 /*****************************************************************************/
<MESSAGEMACRO>\) {
  m_BraceIndent--;
  PRINTF(("%s %d: CLOSE BRACE MESSAGE MACRO ): %d\n",m_InputFileName.c_str(),m_Line,m_BraceIndent));
  if (!m_BraceIndent)
  {
    PRINTF(("%s %d: MESSAGEMACRO: %s\n",m_InputFileName.c_str(),m_Line,yytext));
    yytext[strlen((const char*)yytext)-1]=0;
    cerr<<GetParser()->ExpandExpression((const char*)yytext)<<endl;
    BEGIN(INITIAL);
  } else {
    yymore();
  }
}

 /*****************************************************************************/
<REPARSEMACRO>\) {
  m_BraceIndent--;
  PRINTF(("%s %d: CLOSE BRACE REPARSE MACRO ): %d\n",m_InputFileName.c_str(),m_Line,m_BraceIndent));
  if (!m_BraceIndent)
  {
    PRINTF(("%s %d: REPARSEMACRO: %s\n",m_InputFileName.c_str(),m_Line,yytext));
    string Deps=GetParser()->ExpandExpression((const char*)yytext);
    PRINTF(("%s %d: REPARSEMACRO expanded: %s\n",m_InputFileName.c_str(),m_Line,Deps.c_str()));
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
    yymore();
  }
}

 /*****************************************************************************/
<MAKEEXPRES>\) {
  m_BraceIndent--;
  PRINTF(("%s %d: CLOSE BRACE MAKEEXPRES MACRO ): %d\n",m_InputFileName.c_str(),m_Line,m_BraceIndent));
  if (!m_BraceIndent)
  {
    PRINTF(("%s %d: DOLLAREXPR: %s\n",m_InputFileName.c_str(),m_Line,yytext));
    BEGIN(INITIAL);
    theValue.theString=(const char *)yytext;
    return mhmakeparser::DOLLAREXPR;
  }
  else
  {
    yymore();
  }
}

 /*---------------------------------------------------------------------------*/
<MAKEEXPRES,ERRORMACRO,MESSAGEMACRO,REPARSEMACRO>\$\( {
  m_BraceIndent++;
  PRINTF(("%s %d: MACRO extra $(: %d\n",m_InputFileName.c_str(),m_Line,m_BraceIndent));
  yymore();
}

 /*---------------------------------------------------------------------------*/
<MAKEEXPRES,ERRORMACRO,MESSAGEMACRO,REPARSEMACRO>[^$\(\)\r\n]+ |
<MAKEEXPRES,ERRORMACRO,MESSAGEMACRO,REPARSEMACRO>\$        |
<MAKEEXPRES,ERRORMACRO,MESSAGEMACRO,REPARSEMACRO>\( {
  yymore();
}
<MAKEEXPRES,ERRORMACRO,MESSAGEMACRO,REPARSEMACRO>\r?\n {
  m_Line++;
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
    ::fclose(YY_mhmakelexer_CURRENT_BUFFER->yy_input_file);
    YY_mhmakelexer_DELETE_BUFFER(YY_mhmakelexer_CURRENT_BUFFER);
    INSTACK *pInStack=&m_IncludeStack.top();
    YY_mhmakelexer_SWITCH_TO_BUFFER(pInStack->m_BufferState);
    m_InputFileName=pInStack->m_FileName;
    m_Line=pInStack->m_Line;
    m_IncludeStack.pop();
  }
}
%%

