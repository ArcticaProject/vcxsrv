#include <windows.h>

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  int argc=1;
  char ProgramName[255];
  #define MAXNRARGS 100
  char *argv[MAXNRARGS];
  char *pTmp;
  char *pProgramName;
  
  GetModuleFileName(NULL,ProgramName,255);
  pTmp=strrchr(ProgramName,'\\');
  if (pTmp)
    pProgramName=pTmp+1;
  else
    pProgramName=ProgramName;
  pTmp=strrchr(pProgramName,'.');
  if (pTmp)
    *pTmp=0;
  argv[0]=pProgramName;

  pTmp=lpCmdLine;
  while (*pTmp && argc<MAXNRARGS-1)
  {
    char *pEnd;
    if (*pTmp=='"')
    {
      pTmp++;
      pEnd=strchr(pTmp,'"');
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