#include <windows.h>

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  int argc=1;
  char ProgramName[255];
  #define MAXNRARGS 100
  char *argv[MAXNRARGS]={"plink"};
  char *pTmp=lpCmdLine;
  
  GetModuleFileName(NULL,ProgramName,255);
  argv[0]=ProgramName;
  while (*pTmp && argc<MAXNRARGS-1)
  {
    char *pEnd;
    if (*pTmp=='"')
    {
      pEnd=strchr(pTmp+1,'"');
    }
    else if (*pTmp!=' ')
    {
      pEnd=strchr(pTmp,' ');
    }
    else
    {
      pTmp++;
      continue;
    }
    if (pEnd)
    {
      *pEnd=0;
      argv[argc++]=pTmp;
      pTmp=pEnd+1;
    }
    else
    {
      argv[argc++]=pTmp;
      break;
    }
  }
  
  return main(argc,argv);
}