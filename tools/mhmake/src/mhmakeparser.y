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

%name mhmakeparser

%define LLOC m_yyloc
%define LTYPE int            // This is the type of LLOC (defined in mhmakefileparser)

%define LVAL m_theTokenValue
%define STYPE TOKENVALUE     // This is the type of LVAL (defined in mhmakefileparser)

%define INHERIT : public mhmakefileparser

%define CONSTRUCTOR_PARAM  const map<string,string> &CommandLineVariables
%define CONSTRUCTOR_INIT   : mhmakefileparser(CommandLineVariables)

%header{
#include "mhmakefileparser.h"
#include "rule.h"
#include "util.h"
%}

%token <theString> COMMAND
%token <theString> COMMA OPENBRACE CLOSEBRACE
%token <theString> STRING DOLLAREXPR EQUAL COLON DOUBLECOLON
%token IMEQUAL PEQUAL OPTEQUAL PHONY EXPORT NEWLINE INCLUDEMAK SPACE

%type <theString> expression nonspaceexpression simpleexpression
%type <theString> maybeemptyexpression
%type <theString> expression_nocolorequal simpleexpression_nocolorequal nonspaceexpression_nocolorequal
%type <ival> rulecolon

%start file

/* -------------- rules section -------------- */
/* Sample parser. Does count Chars in a line, and lines in file */
%%
file : statements
       {
         if (m_pCurrentItems)
         {
           PRINTF(("Adding rule : %s\n",(*m_pCurrentItems)[0]->GetQuotedFullFileName().c_str()));
           AddRule();
         }
       }
;

statements :
             | statements statement
;

statement: NEWLINE |
           includemak |
           ruledef |
           phonyrule |
           varassignment |
           imvarassignment |
           pvarassignment |
           optvarassignment |
           exportrule |
           COMMAND
           {
             if (!m_pCurrentRule)
             {
               m_pCurrentRule=refptr<rule>(new rule(this));
             }
             m_pCurrentRule->AddCommand($1);
             PRINTF(("Adding command : %s\n",$1.c_str()));
           }
;

includemak:
         {
           if (m_pCurrentItems)
           {
             PRINTF(("Adding rule : %s\n",(*m_pCurrentItems)[0]->GetQuotedFullFileName().c_str()));
             AddRule();
           }
         } INCLUDEMAK
;

ruledef: expression_nocolorequal rulecolon maybeemptyexpression
         {
           if (m_pCurrentItems)
           {
             PRINTF(("Adding rule : %s\n",(*m_pCurrentItems)[0]->GetQuotedFullFileName().c_str()));
             AddRule();
           }

           m_pCurrentItems=new fileinfoarray;
           m_pCurrentDeps=new fileinfoarray;
           #ifdef _DEBUG
           if (!ExpandExpression($1).size())
           {
             throw string("Empty left hand side in rule: ") + $1 + " : " + $3;
           }
           #endif
           SplitToItems(ExpandExpression($1),*m_pCurrentItems);
           SplitToItems(ExpandExpression($3),*m_pCurrentDeps);
           m_DoubleColonRule= ($2==1) ;
           PRINTF(("Defining rule %s : %s\n",$1.c_str(),$3.c_str()));
           PRINTF(("  Expanded to %s : %s\n",ExpandExpression($1).c_str(),ExpandExpression($3).c_str()));
         }
;

rulecolon: COLON {$$=0;} |
           DOUBLECOLON {$$=1;}
;

phonyrule: PHONY COLON expression
           {
             vector< refptr<fileinfo> > Items;
             SplitToItems(ExpandExpression($3),Items);
             vector< refptr<fileinfo> >::iterator pIt=Items.begin();
             while (pIt!=Items.end())
             {
               (*pIt)->SetPhony();
               pIt++;
             }
             PRINTF(("Defining phony rule : %s\n",$3.c_str()));
             PRINTF(("  Expanded to : %s\n",ExpandExpression($3).c_str()));
           }
           NEWLINE
;

exportrule: EXPORT SPACE exportstrings NEWLINE
;

exportstrings : exportstring |
                exportstring SPACE exportstrings
;

exportstring : STRING
               {
                 SetExport($1,ExpandExpression(ExpandVar($1)));
                 PRINTF(("Exporting %s : %s\n",$1.c_str(),ExpandExpression(ExpandVar($1)).c_str()));
               }
;

varassignment: STRING EQUAL maybeemptyexpression
               {
                 m_Variables[$1]=$3;
                 PRINTF(("Setting variable %s to %s\n",$1.c_str(), $3.c_str()));
               }
;

imvarassignment: STRING IMEQUAL maybeemptyexpression
               {
                 m_Variables[$1]=ExpandExpression($3);
                 PRINTF(("Setting variable %s to %s\n",$1.c_str(), m_Variables[$1].c_str()));
               }
;

pvarassignment: STRING PEQUAL maybeemptyexpression
               {
                 m_Variables[$1]=m_Variables[$1]+g_SpaceString+$3;
                 PRINTF(("Setting variable %s to %s\n",$1.c_str(), $3.c_str()));
               }
;

optvarassignment: STRING OPTEQUAL maybeemptyexpression
               {
                 if (!IsDefined($1))
                 {
                   m_Variables[$1]=$3;
                   PRINTF(("Setting variable %s to %s\n",$1.c_str(), $3.c_str()));
                 }
               }
;

maybeemptyexpression: NEWLINE {$$=g_EmptyString;} |
                      expression NEWLINE |
                      expression SPACE NEWLINE 
;

expression: nonspaceexpression |
            expression SPACE nonspaceexpression {$$=$1+g_SpaceString+$3;}
;

expression_nocolorequal: nonspaceexpression_nocolorequal |
                         expression_nocolorequal SPACE nonspaceexpression_nocolorequal {$$=$1+g_SpaceString+$3;}
;

nonspaceexpression: simpleexpression |
                    nonspaceexpression simpleexpression {$$=$1+$2;}
;

nonspaceexpression_nocolorequal: simpleexpression_nocolorequal |
                    nonspaceexpression_nocolorequal simpleexpression_nocolorequal {$$=$1+$2;}
;

simpleexpression: simpleexpression_nocolorequal |
                  EQUAL |
                  COLON |
                  DOUBLECOLON |
                  COMMA
;

simpleexpression_nocolorequal: STRING {$$=$1;} |
                               DOLLAREXPR {$$=$1;}
;

%%
/* -------------- body section -------------- */

