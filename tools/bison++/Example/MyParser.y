%{
#define YY_MyParser_STYPE yy_MyParser_stype
%}
%name MyParser
%define LSP_NEEDED
%define ERROR_BODY =0
%define LEX_BODY =0
%header{
#include <iostream>
#include <string>
  using namespace std;
#define YY_DECL int yyFlexLexer::yylex(YY_MyParser_STYPE *val)
#ifndef FLEXFIX
#define FLEXFIX YY_MyParser_STYPE *val
#define FLEXFIX2 val
#endif
%}

%union {
  int num;
  bool statement;
  }



%token <num> PLUS INTEGER MINUS  AND OR NOT LPARA RPARA
%token <statement> BOOLEAN
%type <num> exp result
%type <statement> bexp
%start result

%left OR
%left AND
%left PLUS MINUS
%left NOT
%left LPARA RPARA

%%

result          : exp {cout << "Result = " << $1 << endl;}
                | bexp {cout << "Result = " << $1 << endl;}

exp		: exp PLUS exp {$$ = $1 + $3;}
       		| INTEGER {$$ = $1;}
                | MINUS exp { $$ = -$2;}
                | exp MINUS exp {$$ = $1 - $3;}

bexp            : BOOLEAN {$$ = $1;}
		| bexp AND bexp { $$ = $1 && $3;}
		| bexp OR bexp { $$ = $1 || $3;}
		| NOT bexp {$$ = !$2;}
		| LPARA bexp RPARA {$$ = $2}
%%
/* -------------- body section -------------- */
// feel free to add your own C/C++ code here
