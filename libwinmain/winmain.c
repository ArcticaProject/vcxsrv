#include <windows.h>

#include <stdio.h>
#include <fcntl.h>
#include <io.h>

static void CreateConsole()
{
  static int ConsoleCreated=0;
  va_list arglist;
  if (!ConsoleCreated)
  {
    int hConHandle;
    long lStdHandle;
    CONSOLE_SCREEN_BUFFER_INFO coninfo;

    FILE *fp;
    const unsigned int MAX_CONSOLE_LINES = 500;
    ConsoleCreated=1;
    if (!AttachConsole(ATTACH_PARENT_PROCESS))
      AllocConsole();

      // set the screen buffer to be big enough to let us scroll text
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),	&coninfo);
    coninfo.dwSize.Y = MAX_CONSOLE_LINES;
    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE),	coninfo.dwSize);

    // redirect unbuffered STDOUT to the console
    lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
    fp = _fdopen( hConHandle, "w" );
    *stdout = *fp;
    setvbuf( stdout, NULL, _IONBF, 0 );

    // redirect unbuffered STDIN to the console
    lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
    fp = _fdopen( hConHandle, "r" );
    *stdin = *fp;
    setvbuf( stdin, NULL, _IONBF, 0 );

    // redirect unbuffered STDERR to the console
    lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
    fp = _fdopen( hConHandle, "w" );
    *stderr = *fp;
    setvbuf( stderr, NULL, _IONBF, 0 );

  }
}

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  int argc=1;
  char ProgramName[255];
  #define MAXNRARGS 100
  char *argv[MAXNRARGS];
  char *pTmp;
  char *pProgramName;

  if (!strncmp(lpCmdLine,"-console",8))
  {
    CreateConsole();
    lpCmdLine+=9;
  }
  
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