#include "MyParser.h"
#define YY_DECL int yyFlexLexer::yylex(YY_MyParser_STYPE *val)
#include "FlexLexer.h"
#include <stdio.h>

class MyCompiler : public MyParser
{
private:
  yyFlexLexer theScanner;
 public:
  virtual int yylex();
  virtual void yyerror(char *m);
  MyCompiler(){;}
};

int MyCompiler::yylex()
{
 return theScanner.yylex(&yylval);
}

void MyCompiler::yyerror(char *m)
{ fprintf(stderr,"%d: %s at token '%s'\n",yylloc.first_line, m,yylloc.text);
}

int main(int argc,char **argv)
{
 MyCompiler aCompiler;
 int result=aCompiler.yyparse();
 printf("Resultat Parsing=%s\n",result?"Erreur":"OK");
 return 0;
};

