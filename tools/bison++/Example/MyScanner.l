%{ 
#ifndef FLEXFIX
#define FLEXFIX YY_MyParser_STYPE *val
#define FLEXFIX2 val
#endif
#include "MyParser.h" // Make sure the flexer can communicate with bison++ 
                      //using return values
%}

digit	[0-9]
integer	[1-9]{digit}*
ws	[ \t\n]+
%%
{ws}				{ /* no action */ }
{integer}	{  val->num = atoi(yytext); return MyParser::INTEGER; }
"AND"		{return(MyParser::AND);}
"OR"		{return(MyParser::OR);}
"NOT"		{return(MyParser::NOT);}
"TRUE"	{val->statement=true; return MyParser::BOOLEAN; }
"FALSE"	{val->statement=false; return MyParser::BOOLEAN; }
"-"		{return(MyParser::MINUS);}
"+"		{return(MyParser::PLUS);}
"("		{return(MyParser::LPARA);}
")"		{return(MyParser::RPARA);}

<<EOF>> { yyterminate();}
%%
int yywrap()
{
        return(1);
}