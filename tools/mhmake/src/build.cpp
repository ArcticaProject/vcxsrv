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

#include "mhmakefileparser.h"
#include "mhmakeparser.h"
#include "rule.h"
#include "util.h"

/* Calling py2exe is only implemented on windows for now. */
#ifdef WIN32

/* Python exe create script in parts:
import zipfile,tempfile,shutil,os

def UpdateZipFile(SrcZip,DestZip):
  NewZip=tempfile.mktemp('.zip')

  Src=zipfile.ZipFile(SrcZip)

  New=zipfile.ZipFile(NewZip,'w')

  InFile={}
  for name in Src.namelist():
    InFile[name]=1
    New.writestr(name,Src.read(name))
  Src.close()

  try:
    Dest=zipfile.ZipFile(DestZip)

    for name in Dest.namelist():
      if not InFile.has_key(name):
        New.writestr(name,Dest.read(name))
    Dest.close()
  except IOError:
    pass
  New.close()

  shutil.move(NewZip,DestZip)

OutFileName=tempfile.mktemp('.py')
pOut=open(OutFileName,'w')

Script=r'<PYTHONSCRIPT>'
DirScript=os.path.split(Script)[0]
pOut.write(r'''

from distutils.core import setup
import py2exe
import sys
sys.path.append(r'%s')
setup(zipfile=None, console=[r'%s'])
'''%(DirScript,Script))

pOut.close()

import os

stdin,stdout=os.popen4(r'"<PYTHONEXE>" %s py2exe'%OutFileName);
stdout.read()
stdout.close()
stdin.close()

OutDir=os.path.split(Script)[0]

import shutil
def CopyFiles(Src,Dest):
  for File in os.listdir(Src):
    SrcDir=os.path.join(Src,File)
    if File=='library.zip':
      UpdateZipFile(os.path.join(Src,File),os.path.join(Dest,File))
    elif os.path.isdir(SrcDir):
      DestDir=os.path.join(Dest,File)
      os.mkdir(DestDir)
      CopyFiles(SrcDir,DestDir)
    else:
      shutil.copy(os.path.join(Src,File),os.path.join(Dest,File))

if os.path.isdir('dist'):
  CopyFiles('dist',OutDir)

  try:
    shutil.rmtree('dist')
  except:
    pass
  try:
    shutil.rmtree('build')
  except:
    pass
os.remove(OutFileName)
*/

static const string PythonScriptPart1=
"import zipfile,tempfile,shutil,os\n"
"\n"
"def UpdateZipFile(SrcZip,DestZip):\n"
"  NewZip=tempfile.mktemp('.zip')\n"
"\n"
"  Src=zipfile.ZipFile(SrcZip)\n"
"\n"
"  New=zipfile.ZipFile(NewZip,'w')\n"
"\n"
"  InFile={}\n"
"  for name in Src.namelist():\n"
"    InFile[name]=1\n"
"    New.writestr(name,Src.read(name))\n"
"  Src.close()\n"
"\n"
"  try:\n"
"    Dest=zipfile.ZipFile(DestZip)\n"
"\n"
"    for name in Dest.namelist():\n"
"      if not InFile.has_key(name):\n"
"        New.writestr(name,Dest.read(name))\n"
"    Dest.close()\n"
"  except IOError:\n"
"    pass\n"
"  New.close()\n"
"\n"
"  shutil.move(NewZip,DestZip)\n"
"\n"
"OutFileName=tempfile.mktemp('.py')\n"
"pOut=open(OutFileName,'w')\n"
"\n"
"Script=r'"
;

static const string PythonScriptPart2=
"'\n"
"DirScript=os.path.split(Script)[0]\n"
"\n"
"pOut.write(r'''\n"
"\n"
"from distutils.core import setup\n"
"import py2exe\n"
"import sys\n"
"sys.path.append(r'%s')\n"
"setup(console=[r'%s'])\n"
"'''%(DirScript,Script))\n"
"\n"
"pOut.close()\n"
"\n"
"import os\n"
"\n"
"stdin,stdout=os.popen4(r'"
;

static const string PythonScriptPart3=
"%s py2exe'%OutFileName);\n"
"stdout.read()\n"
"stdout.close()\n"
"stdin.close()\n"
"\n"
"OutDir=os.path.split(Script)[0]\n"
"\n"
"import shutil\n"
"def CopyFiles(Src,Dest):\n"
"  for File in os.listdir(Src):\n"
"    SrcDir=os.path.join(Src,File)\n"
"    if File=='library.zip':\n"
"      UpdateZipFile(os.path.join(Src,File),os.path.join(Dest,File))\n"
"    elif os.path.isdir(SrcDir):\n"
"      DestDir=os.path.join(Dest,File)\n"
"      os.mkdir(DestDir)\n"
"      CopyFiles(SrcDir,DestDir)\n"
"    else:\n"
"      shutil.copy(os.path.join(Src,File),os.path.join(Dest,File))\n"
"\n"
"if os.path.isdir('dist'):\n"
"  CopyFiles('dist',OutDir)\n"
"\n"
"  try:\n"
"    shutil.rmtree('dist')\n"
"  except:\n"
"    pass\n"
"  try:\n"
"    shutil.rmtree('build')\n"
"  except:\n"
"    pass\n"
"os.remove(OutFileName)\n"
;
/*****************************************************************************/
/* Converts a python script to an executable if py2exe is installed */

void mhmakefileparser::CreatePythonExe(const string &FullCommand)
{
  /* First create a python script to run */
  cout << "Converting "<<FullCommand<<endl;

  string PythonScript;
  PythonScript+=PythonScriptPart1;
  PythonScript+=FullCommand;
  PythonScript+=PythonScriptPart2;
  PythonScript+=GetPythonExe();
  PythonScript+=PythonScriptPart3;

  char Filename[MAX_PATH];
  int Nr=0;
  FILE *pFile=(FILE*)1;
  while (1)
  {
    sprintf(Filename,"%s\\tmp%d.py",m_MakeDir->GetFullFileName().c_str(),Nr);
    pFile=fopen(Filename,"r");
    if (!pFile)
      break;
    fclose(pFile);
    Nr++;
  }
  pFile=fopen(Filename,"w");
  fprintf(pFile,"%s",PythonScript.c_str());
  fclose(pFile);

  string GenExeCommand=GetPythonExe();
  GenExeCommand+=QuoteFileName(Filename);

  string Output;
  ExecuteCommand(GenExeCommand,&Output);

  remove(Filename);
}
#endif

/*****************************************************************************/
int mhmakefileparser::SearchPath(void *NotUsed, const char *szCommand, const char *pExt, int Len, char *szFullCommand,char **pFilePart) const
{
  static vector< refptr<fileinfo> > vSearchPath;

  string Command(szCommand);
  if (pExt)
    Command+=pExt;
  vector< refptr<fileinfo> >::iterator It;
  vector< refptr<fileinfo> >::iterator ItEnd;

  refptr<fileinfo> CommandFile=GetFileInfo(Command,m_MakeDir);
  if (CommandFile->Exists())
  {
    goto found;
  }
  CommandFile->InvalidateDate(); // It could be created in the makefile later
  if (!vSearchPath.size())
  {
     char Path[1024];
     char *pPath=getenv(PATH);
     if (!pPath)
       return 0;
     strcpy(Path,pPath); // To be able to use strtok
     char *pTok=strtok(Path,OSPATHENVSEPSTR);
     while (pTok)
     {
       vSearchPath.push_back(GetFileInfo(pTok,m_MakeDir));
       pTok=strtok(NULL,OSPATHENVSEPSTR);
     }
  }
  It=vSearchPath.begin();
  ItEnd=vSearchPath.end();
  while (It!=ItEnd)
  {
    CommandFile=GetFileInfo(Command,*It);
    if (CommandFile->Exists())
      goto found;
    It++;
  }

  return 0;

found:
  string FullCommand=CommandFile->GetFullFileName();
  int CommandLen=FullCommand.size();
  if (CommandLen>Len-1)
  {
    throw string("Command to long: ") + FullCommand;
  }
  strcpy(szFullCommand,FullCommand.c_str());
  return 1;
}

/*****************************************************************************/
string mhmakefileparser::SearchCommand(const string &Command, const string &Extension) const
{
  char FullCommand[MAX_PATH]="";
  unsigned long Size=sizeof(FullCommand);
  const char *pExt;
  if (Extension.empty())
    pExt=NULL;
  else
    pExt=Extension.c_str();
  if (SearchPath(NULL,UnquoteFileName(Command).c_str(),pExt,MAX_PATH,FullCommand,NULL))
    return FullCommand;
#ifdef WIN32
  /* See if we have a path for python.exe in the registry */
  HKEY hKey;
  string RegEntry=Command;
  if (pExt)
  {
    RegEntry+=Extension;
  }
  string KeyName=string("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\")+RegEntry;
  if (ERROR_SUCCESS!=RegOpenKey(HKEY_LOCAL_MACHINE,KeyName.c_str(),&hKey))
    return FullCommand;

  RegQueryValueEx(hKey,NULL,NULL,NULL,(LPBYTE)FullCommand,&Size);

  RegCloseKey(hKey);
#endif

  return FullCommand;
}

/*****************************************************************************/
/* Deletes a complete directory or files with wildcard. It can be assumed that the Directory passed does exist */
static bool DeleteDir(const string &Dir,const string WildSearch="*",bool bRemoveDir=true)
{
  bool Error=false;
  string Pattern=Dir+OSPATHSEP+WildSearch;
#ifdef WIN32
  WIN32_FIND_DATA FindData;
  HANDLE hFind=FindFirstFile(Pattern.c_str(),&FindData);
  if (hFind==INVALID_HANDLE_VALUE)
  {
    return Error;
  }

  do
  {
    /* Only handle items which are not . and .. */
    if (FindData.cFileName[0]!='.' || (FindData.cFileName[1] && (FindData.cFileName[1]!='.' || FindData.cFileName[2])) )
    {
      string FileName=Dir+OSPATHSEP+FindData.cFileName;
      if (FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
      {
        Error = DeleteDir(FileName);
      }
      else
      {
        Error = (-1==remove(FileName.c_str()));
      }
    }
  } while (FindNextFile(hFind,&FindData));

  FindClose(hFind);

  if (bRemoveDir)
    Error = (0==RemoveDirectory(Dir.c_str()));
#else
  glob_t Res;
  if (glob (Pattern.c_str(), GLOB_ERR|GLOB_NOSORT|GLOB_MARK, NULL, &Res))
    return Error;

  for (int i=0; i<Res.gl_pathc; i++)
  {
    int Len=strlen(Res.gl_pathv[i])-1;
    if (Res.gl_pathv[i][Len]=='/')
    {
      Res.gl_pathv[i][Len]=0;
      Error = DeleteDir(Res.gl_pathv[i]);
    }
    else
    {
      Error = (-1==remove(Res.gl_pathv[i]));
    }
  }

  globfree(&Res);

  if (bRemoveDir)
    Error = (-1==remove(Dir.c_str()));
#endif

  return Error;
}

/*****************************************************************************/
HANDLE mhmakefileparser::DeleteFiles(const string &Params) const
{
  bool IgnoreError=false;
  vector< refptr<fileinfo> > Files;

  // First check if the first parameter is -e meaning don't give an error when file does not exist
  if (Params[1]=='-')
  {
    if (Params[2]=='e')
    {
      IgnoreError=true;
      SplitToItems(Params.substr(4),Files);
    }
    else
    {
      cerr << "Invalid option "<<Params[1]<<" in del statement\n";
      return (HANDLE)-1;
    }
  }
  else
  {
    SplitToItems(Params,Files);
  }

  vector< refptr<fileinfo> >::const_iterator It=Files.begin();
  while (It!=Files.end())
  {
    refptr<fileinfo> pFile=*It++;
    string DirSearch="*";
    bool bRemoveDir=true;

    /* Now check if there is a wildcard */
    if (pFile->GetFullFileName().find('*')!=string::npos)
    {
      DirSearch=pFile->GetName();
      pFile=pFile->GetDir();
      bRemoveDir=false;
    }

    pFile->InvalidateDate();
    if (IgnoreError && !pFile->Exists() && !pFile->IsDir())
    {
      continue;
    }

    const string &FileName=pFile->GetFullFileName();
    if (pFile->IsDir())
    {
      if (DeleteDir(FileName,DirSearch,bRemoveDir) && !IgnoreError)
      {
        cerr << "Error deleting "<<FileName<<endl;
        return false;
      }
    }
    else
    {
      if (-1==remove(FileName.c_str()) && !IgnoreError)
      {
        cerr << "Error deleting "<<FileName<<endl;
        return (HANDLE)-1;
      }
    }
  }
  return (HANDLE)0;
}

/*****************************************************************************/
/* pDest can be a directory or a file */
static bool CopyFile(refptr<fileinfo> pSrc, refptr<fileinfo> pDest)
{
  if (pDest->IsDir())
  {
    pDest=GetFileInfo(pSrc->GetName(),pDest);
  }
  string SrcFileName=pSrc->GetFullFileName();
  string DestFileName=pDest->GetFullFileName();

  /* Now copy the file */
  FILE *pSrcFile=fopen(SrcFileName.c_str(),"rb");
  if (!pSrcFile)
  {
    cerr << "copy: error opening file "<<SrcFileName<<endl;
    return false;
  }
  FILE *pDestFile=fopen(DestFileName.c_str(),"wb");
  if (!pDestFile)
  {
    cerr << "copy: error creating file "<<DestFileName<<endl;
    return false;
  }
  char Buf[4096];
  size_t Ret;

  while ( (Ret=fread(Buf,1,sizeof(Buf),pSrcFile)) > 0)
  {
    fwrite(Buf,1,Ret,pDestFile);
  }
  fclose(pSrcFile);
  fclose(pDestFile);
  pDest->InvalidateDate();
  return true;
}

/*****************************************************************************/
/* Copies a complete directory to a destination (currenlty not recursive */
static bool CopyDir(refptr<fileinfo> pDir,refptr<fileinfo> pDest,const string WildSearch="*")
{
  bool Error=true;
  string Pattern=pDir->GetFullFileName()+OSPATHSEP+WildSearch;
#ifdef WIN32
  WIN32_FIND_DATA FindData;
  HANDLE hFind=FindFirstFile(Pattern.c_str(),&FindData);
  if (hFind==INVALID_HANDLE_VALUE)
  {
    return false;
  }

  do
  {
    /* Only handle items which are not . and .. */
    if (FindData.cFileName[0]!='.' || (FindData.cFileName[1] && (FindData.cFileName[1]!='.' || FindData.cFileName[2])) )
    {
      if (FindData.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN)
        continue;
      refptr<fileinfo> pSrc=GetFileInfo(FindData.cFileName,pDir);

      if (pSrc->IsDir())
      {
        refptr<fileinfo> pNewDest=GetFileInfo(FindData.cFileName,pDest);
        if (!pNewDest->IsDir())
        {
          if (pNewDest->Exists())
          {
            cerr << pNewDest->GetFullFileName() << " exists and is not a directory.\n";
            Error = false;
            goto exit;
          }
          if (!CreateDirectory(pNewDest->GetFullFileName().c_str(),NULL))
          {
            cerr << "Error creating directory " << pNewDest->GetFullFileName() << endl;
            Error = false;
            goto exit;
          }
          pNewDest->InvalidateDate();
        }
        Error = CopyDir(pSrc,pNewDest);
        if (!Error) goto exit;
      }
      else
      {
        Error = CopyFile(pSrc,pDest);
        if (!Error) goto exit;
      }
    }
  } while (FindNextFile(hFind,&FindData));

exit:
  FindClose(hFind);
#else
  glob_t Res;
  if (glob (Pattern.c_str(), GLOB_ERR|GLOB_NOSORT|GLOB_MARK, NULL, &Res))
    return Error;

  for (int i=0; i<Res.gl_pathc; i++)
  {
    refptr<fileinfo> pSrc=GetFileInfo(Res.gl_pathv[i]);
    if (pSrc->IsDir())
    {
      *(strrchr(Res.gl_pathv[i],'/'))='\0';
      const char *SrcDirName=strrchr(Res.gl_pathv[i],'/')+1;

      if (SrcDirName[0]=='.')
        continue;

      refptr<fileinfo> pNewDest=GetFileInfo(SrcDirName,pDest);
      if (!pNewDest->IsDir())
      {
        if (pNewDest->Exists())
        {
          cerr << pNewDest->GetQuotedFullFileName() << " exists and is not a directory.\n";
          Error = false;
          goto exit;
        }
        if (-1==mkdir(pNewDest->GetFullFileName().c_str(),0777))
        {
          cerr << "Error creating directory " << pNewDest->GetQuotedFullFileName() << endl;
          Error = false;
          goto exit;
        }
        pNewDest->InvalidateDate();
      }
      Error = CopyDir(pSrc,pNewDest);
      if (!Error) goto exit;
    }
    else
    {
      Error = CopyFile(GetFileInfo(Res.gl_pathv[i]),pDest);
      if (!Error) goto exit;
    }
  }

exit:
  globfree(&Res);
#endif

  return Error;
}

/*****************************************************************************/
HANDLE mhmakefileparser::EchoCommand(const string &Params) const
{
    // Find the first > character
  size_t Pos=Params.find_first_of('>');
  if (Pos==string::npos)
  {
    // Just echo it
    cout << Params << endl;
  }
  else
  {
    FILE *pfFile;
     /* Extra the filename */
    string Filename;
    if (Params[Pos+1]=='>')
    {
      NextItem(Params.substr(Pos+2).c_str(),Filename);
      refptr<fileinfo> pFile=GetFileInfo(Filename,m_MakeDir);
        // Open file in append
      pfFile=fopen(pFile->GetFullFileName().c_str(),"a");
    }
    else
    {
      NextItem(Params.substr(Pos+1).c_str(),Filename);
      refptr<fileinfo> pFile=GetFileInfo(Filename,m_MakeDir);
      pfFile=fopen(pFile->GetFullFileName().c_str(),"w");
    }
    if (!pfFile)
    {
      cerr << "Error opening file "<<Filename<<endl;
      return (HANDLE)-1;
    }
    int Begin=0;
    while (Params[Begin]==' ' || Params[Begin] == '\t') Begin++;  // Strip leading white space
    string EchoStr=Params.substr(Begin,Pos-1)+"\n";
    if (EchoStr.length()!=fwrite(EchoStr.c_str(),1,EchoStr.length(),pfFile))
    {
      cerr << "Error writing file "<<Filename<<endl;
      return (HANDLE)-1;
    }
    fclose(pfFile);
  }
  return (HANDLE)0;
}

/*****************************************************************************/
HANDLE mhmakefileparser::CopyFiles(const string &Params) const
{
  vector< refptr<fileinfo> > Files;

  SplitToItems(Params,Files);

  size_t NrSrcs=Files.size()-1;

  if (NrSrcs<1)
  {
    cerr << "Wrong number of arguments in copy: "<<Params<<endl;
    return false;
  }

  refptr<fileinfo> pDest=Files[NrSrcs];
  if (NrSrcs>1 && !pDest->IsDir())
  {
    cerr << "copy: Destination must be a directory when more then one source : "<<Params<<endl;
    return (HANDLE)-1;
  }

  for (size_t i=0; i<NrSrcs; i++)
  {
    refptr<fileinfo> pSrc=Files[i];

    string SrcFileName=pSrc->GetFullFileName();

    if (pSrc->IsDir())
    {
      SrcFileName+=OSPATHSEPSTR"*";
      pSrc=GetFileInfo(SrcFileName,m_MakeDir);
    }

    //cerr << "copy "<<pSrc->GetFullFileName()<<" "<<pDest->GetFullFileName()<<endl;
    /* Now check if there is a wildcard */
    if (SrcFileName.find('*')!=string::npos)
    {
      if (!CopyDir(pSrc->GetDir(), pDest, pSrc->GetName()))
      {
        cerr << "copy: Error copying directory: " << Params << endl;
        return (HANDLE)-1;
      }
    }
    else
    {
      if (!CopyFile(pSrc,pDest))
      {
        cerr << "copy: Error copying file: " << Params << endl;
        return (HANDLE)-1;
      }
    }
  }

  return (HANDLE)0;

}

/*****************************************************************************/
HANDLE mhmakefileparser::TouchFiles(const string &Params) const
{
  vector< refptr<fileinfo> > Files;

  SplitToItems(Params,Files);

  vector< refptr<fileinfo> >::const_iterator It=Files.begin();
  while (It!=Files.end())
  {
    refptr<fileinfo> pFile=*It++;
    const string &FileName=pFile->GetFullFileName();

    /* Since this can be part of a list of commands for a certain rule, and it is possible that the file
     * was generated by one on the previous commands, we first need the invalidate the date so that the
     * existance checking is done again */
    pFile->InvalidateDate();

    if (pFile->IsDir())
    {
      cerr << "touch: Cannot touch a directory: " << FileName << endl;
      return (HANDLE)-1;
    }
    if (pFile->Exists())
    {
      int fd;
      char c;
      int status = 0;
      struct stat st;
      int Ret;
      int saved_errno = 0;

      fd = open (FileName.c_str(), O_RDWR);
      if (fd<0)
      {
        st.st_size=0;
      }
      else
      {
        if (fstat (fd, &st) < 0)
        {
          cerr << "touch: Cannot stat file " << FileName << endl;
          return (HANDLE)-1;
        }
      }

      if (st.st_size == 0)
      {
        FILE *pFile;
        if (fd>=0 && close(fd) < 0)
        {
          cerr << "touch: Error closing file " << FileName << endl;
          return (HANDLE)-1;
        }
        /*Re-Create an empty file */
        pFile=fopen(FileName.c_str(),"wb");
        if (!pFile)
        {
          cerr << "touch: Cannot create file: " << FileName << endl;
          return (HANDLE)-1;
        }
        fclose(pFile);
      }
      else
      {
        Ret=read (fd, &c, sizeof(c));
        if (Ret!=sizeof(c) && Ret!=EOF)
        {
          cerr << "touch: Cannot read file " << FileName << ": "<<Ret<<endl;
          return (HANDLE)-1;
        }
        if (lseek (fd, (off_t) 0, SEEK_SET) < 0)
        {
          cerr << "touch: Error changing file pointer " << FileName << endl;
          return (HANDLE)-1;
        }
        if (write (fd, &c, sizeof c) != sizeof(c))
        {
          cerr << "touch: Error writing file " << FileName << endl;
          return (HANDLE)-1;
        }
        if (close (fd) < 0)
        {
          cerr << "touch: Error closing file " << FileName << endl;
          return (HANDLE)-1;
        }
      }
    }
    else
    {
      /* Create an empty file */
      FILE *pFile=fopen(FileName.c_str(),"wb");
      if (!pFile)
      {
        cerr << "touch: Cannot create file: " << FileName << endl;
        return (HANDLE)-1;
      }
      fclose(pFile);
    }
    pFile->InvalidateDate();
  }
  return (HANDLE)0;
}

/*****************************************************************************/
const string &mhmakefileparser::GetPythonExe() const
{
  static string PythonExe;
  if (PythonExe.empty())
  {
    string FullCommand=SearchCommand(PYTHONEXE);
    if (!FullCommand.empty())
    {
      PythonExe=QuoteFileName(FullCommand)+" ";
    }
    else
    {
      cerr<<"python executable not found in path and registry.\n";
      exit(1);
    }
  }
  return PythonExe;
}

/*****************************************************************************/
static const string &GetComspec()
{
  static string Comspec;
  if (Comspec.empty())
  {
    const char *pComspec=getenv(COMSPEC);
    if (pComspec)
    {
      Comspec=getenv(COMSPEC);
      #ifdef WIN32
      Comspec+=" /c ";
      #else
      Comspec+=" -c \"";
      #endif
    }
    else
    {
      #ifdef WIN32
      Comspec="cmd.exe /c ";
      #else
      Comspec="sh -c \"";
      #endif
    }
  }
  return Comspec;
}

/*****************************************************************************/
string mhmakefileparser::GetFullCommand(string Command)
{
  map<string,string>::iterator pFound=m_CommandCache.find(Command);
  string OriCommand=Command;
  if (pFound==m_CommandCache.end())
  {
    bool Found=false;
    // Not found in the stack, search in the environment path
    // Check if an extension is specified
    const char *pBeg=Command.c_str();
    const char *pEnd=pBeg+Command.length()-1;
    bool HasExt=false;
    while (pEnd>pBeg && *pEnd!=OSPATHSEP)
    {
      if (*pEnd=='.')
      {
        HasExt=true;
        break;
      }
      pEnd--;
    }
    if (HasExt)
    {
      string FullCommand=SearchCommand(Command);
      if (!FullCommand.empty())
      {
        Found=true;
        Command=FullCommand;
      }
    }
    else
    {
      static bool s_Py2ExeInstalled=true;
      /* First check for special internal commands */
      if (OriCommand=="del")
      {
        m_CommandCache[OriCommand]="del";
        return Command;
      }
      // Try with different extensions
      string FullCommand=SearchCommand(Command,EXEEXT);
      if (!FullCommand.empty())
      {
        Found=true;
        #ifdef WIN32
        /* Check if a python script also exists, is so try generating the executable again. */
        string PythonFullCommand=SearchCommand(Command,".py");
        Command=FullCommand;
        if (!PythonFullCommand.empty()&&s_Py2ExeInstalled)
        {
          refptr<fileinfo> pExeFile=GetFileInfo(FullCommand,m_MakeDir);
          refptr<fileinfo> pPyFile=GetFileInfo(PythonFullCommand,m_MakeDir);
          bool bBuild=false;
          if (pExeFile->GetDate().IsOlder(pPyFile->GetDate()))
          {
            bBuild=true;
          }
          if (!bBuild)
          {
            set< refptr<fileinfo> > Autodeps;
            GetAutoDeps(pPyFile, Autodeps);
            set< refptr<fileinfo> >::iterator It=Autodeps.begin();
            while (It!=Autodeps.end())
            {
              if (pExeFile->GetDate().IsOlder((*It)->GetDate()))
              {
                bBuild=true;
                break;
              }
              It++;
            }
          }
          if (bBuild)
          {
            if (pExeFile->Exists())
              remove(pExeFile->GetFullFileName().c_str());
            CreatePythonExe(PythonFullCommand);
            // Invalidate the exe date since it could have been recreated by the CreatePythonExe
            pExeFile->InvalidateDate();
          }

        }
        #else
        Command=FullCommand;
        #endif
      }
      else
      {
        FullCommand=SearchCommand(Command,".py");
        if (!FullCommand.empty())
        {
          Found=true;
          #ifdef WIN32
          /* Now first try to create an executable for it */
          if (s_Py2ExeInstalled)
          {
            refptr<fileinfo> pExeFile;
            CreatePythonExe(FullCommand);
            string ExeFullCommand=SearchCommand(Command,EXEEXT);
            if (!ExeFullCommand.empty())
            {
              pExeFile=GetFileInfo(ExeFullCommand,m_MakeDir);
              pExeFile->InvalidateDate(); // The file was just generated, make sure the correct date is taken.
            }
            if (ExeFullCommand.empty() || !pExeFile->Exists())
            {
              s_Py2ExeInstalled=false;
              cout << "\nWarning: cannot convert "<<FullCommand<<".\nCompilation will be faster by installing py2exe.\n\n";
              Command=GetPythonExe()+QuoteFileName(FullCommand);
            }
            else
              Command=ExeFullCommand;
          }
          else
          #endif
            Command=GetPythonExe()+QuoteFileName(FullCommand);
        }
      }
    }
    if (!Found)
    {
      Command=GetComspec()+QuoteFileName(Command);
    }
    m_CommandCache[OriCommand]=Command;
    return Command;
  }
  return pFound->second;
}

HANDLE mhmakefileparser::OsExeCommand(const string &Command, const string &Params, bool IgnoreError, string *pOutput) const
{
  string FullCommandLine;
  string ComSpec=GetComspec();
#ifdef WIN32
  STARTUPINFO StartupInfo;
  memset(&StartupInfo,0,sizeof(StartupInfo));
  StartupInfo.cb=sizeof(STARTUPINFO);
  PROCESS_INFORMATION ProcessInfo;

  if (Command.substr(0,ComSpec.size())==ComSpec)
  {
    string tmpCommand=Command.substr(ComSpec.size(),Command.size());
    FullCommandLine=ComSpec;
    FullCommandLine+=g_QuoteString+QuoteFileName(tmpCommand)+Params+g_QuoteString;
  }
  else
  {
    const string PythonExe=GetPythonExe();
    if (!(Command.substr(0,PythonExe.size())==PythonExe))
      FullCommandLine=QuoteFileName(Command)+Params;
    else
      FullCommandLine=Command+Params;
  }
  char *pFullCommand=new char[FullCommandLine.length()+1];
  strcpy(pFullCommand,FullCommandLine.c_str());

  if (pOutput || g_Quiet)
  {
    HANDLE hChildStdinRd;
    HANDLE hChildStdinWr;
    HANDLE hChildStdoutRd;
    HANDLE hChildStdoutWr;
    HANDLE hChildStdinWrDup;
    HANDLE hChildStdoutRdDup;
    SECURITY_ATTRIBUTES saAttr;
    BOOL fSuccess;

    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&hChildStdinRd, &hChildStdinWr, &saAttr, 0))
      return false;

        /* Create new output read handle and the input write handle. Set
    * the inheritance properties to FALSE. Otherwise, the child inherits
    * the these handles; resulting in non-closeable handles to the pipes
        * being created. */
    fSuccess = DuplicateHandle(GetCurrentProcess(), hChildStdinWr,
                               GetCurrentProcess(), &hChildStdinWrDup, 0,
                               FALSE, DUPLICATE_SAME_ACCESS);
    if (!fSuccess) return false;
    /* Close the inheritable version of ChildStdin that we're using. */
    CloseHandle(hChildStdinWr);

    if (!CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0))
      return false;

    fSuccess = DuplicateHandle(GetCurrentProcess(), hChildStdoutRd,
                               GetCurrentProcess(), &hChildStdoutRdDup, 0,
                               FALSE, DUPLICATE_SAME_ACCESS);
    if (!fSuccess) return false;
    CloseHandle(hChildStdoutRd);

    int hStdIn = _open_osfhandle((long)hChildStdinWrDup, _O_WRONLY|_O_TEXT);
    FILE *pStdIn = _fdopen(hStdIn, "w");
    int hStdOut = _open_osfhandle((long)hChildStdoutRdDup, _O_RDONLY|_O_TEXT);
    FILE *pStdOut = _fdopen(hStdOut, "r");

    StartupInfo.dwFlags = STARTF_USESTDHANDLES;
    StartupInfo.hStdInput = hChildStdinRd;
    StartupInfo.hStdOutput = hChildStdoutWr;
    StartupInfo.hStdError = hChildStdoutWr;

    if (!CreateProcess(NULL,pFullCommand,NULL,NULL,TRUE,CREATE_NO_WINDOW,m_pEnv,m_MakeDir->GetFullFileName().c_str(),&StartupInfo,&ProcessInfo))
    {
      delete[] pFullCommand;
      string ErrorMessage=string("Error starting command: ") + FullCommandLine + " : " + stringify(GetLastError());
      if (IgnoreError)
        cerr << ErrorMessage << endl;
      else
        throw ErrorMessage;
    }
    delete[] pFullCommand;
    if (!CloseHandle(hChildStdinRd)) return false;
    if (!CloseHandle(hChildStdoutWr)) return false;

    CloseHandle(ProcessInfo.hThread);
    char Buf[256];
    size_t Nbr;
    while ( (Nbr=fread(Buf,1,sizeof(Buf)-1,pStdOut)) > 0)
    {
      if (pOutput)
      {
        Buf[Nbr]=0;
        *pOutput+=Buf;
      }
    }
    WaitForSingleObject(ProcessInfo.hProcess,INFINITE);
    fclose(pStdIn);
    fclose(pStdOut);

    DWORD ExitCode=0;
    if (!GetExitCodeProcess(ProcessInfo.hProcess,&ExitCode) || ExitCode)
    {
      if (IgnoreError)
      {
        cerr << "Error running command: "<<Command<<", but ignoring error\n";
        return (HANDLE)0; // Ignore error
      }
      else
        return (HANDLE)-1;
    }
    CloseHandle(ProcessInfo.hProcess);
    return (HANDLE)0;
  }
  else
  {
    if (!CreateProcess(NULL,pFullCommand,NULL,NULL,TRUE,0,m_pEnv,m_MakeDir->GetFullFileName().c_str(),&StartupInfo,&ProcessInfo))
    {
      delete[] pFullCommand;
      string ErrorMessage=string("Error starting command: ") + Command + " : " + stringify(GetLastError());
      if (IgnoreError)
        cerr << ErrorMessage << endl;
      else
        throw ErrorMessage;
    }
    delete[] pFullCommand;
    CloseHandle(ProcessInfo.hThread);
    return ProcessInfo.hProcess;
  }
#else
  int pipeto[2];      /* pipe to feed the exec'ed program input */
  int pipefrom[2];    /* pipe to get the exec'ed program output */

  if (Command.substr(0,ComSpec.size())==ComSpec)
  {
    string tmpCommand=Command.substr(ComSpec.size(),Command.size());
    FullCommandLine=ComSpec;
    FullCommandLine+=QuoteFileName(tmpCommand)+Params;
  }
  else
  {
    FullCommandLine=Command+Params;
  }

  if (pOutput || g_Quiet)
  {
    pipe( pipeto );
    pipe( pipefrom );
  }

  pid_t ID=vfork();
  if (ID==-1)
  {
    if (IgnoreError)
    {
      cerr << "Error forking when try to run command: "<<Command<<", but ignoring error\n";
      return true; // Ignore error
    }
    else
      return false;
  }
  else if (ID==0)
  {
    int argc;
    const char **pargv;

    if (pOutput || g_Quiet)
    {
      dup2( pipeto[0], STDIN_FILENO );
      dup2( pipefrom[1], STDOUT_FILENO  );
      /* close unnecessary pipe descriptors for a clean environment */
      close( pipeto[0] );
      close( pipeto[1] );
      close( pipefrom[0] );
      close( pipefrom[1] );
    }

    poptParseArgvString(FullCommandLine.c_str(), &argc, &pargv);
    execv(pargv[0],(char *const*)pargv);
    _exit (EXIT_FAILURE);
  }
  else
  {
    if (pOutput || g_Quiet)
    {
        /* Close unused pipe ends. This is especially important for the
      * pipefrom[1] write descriptor, otherwise readFromPipe will never
        * get an EOF. */
      close( pipeto[0] );
      close( pipefrom[1] );

      pid_t ID2=vfork();
      if (ID2==-1)
      {
        if (IgnoreError)
        {
          cerr << "Error forking when try to run command: "<<Command<<", but ignoring error\n";
          return true; // Ignore error
        }
        else
          return false;
      }
      else if (ID2==0)
      {
        /* Close pipe write descriptor, or we will never know when the
         * writer process closes its end of the pipe and stops feeding the
         * exec'ed program. */
        close( pipeto[1] );
        char Buf[256];
        int Nbr;
        while ( (Nbr=read(pipefrom[0],Buf,sizeof(Buf)-1)) > 0)
        {
          if (pOutput)
          {
            Buf[Nbr]=0;
            *pOutput+=Buf;
          }
        }
        close( pipefrom[0]);
        _exit (EXIT_FAILURE);
      }
      else
      {
        /* close unused pipe end */
        close( pipefrom[0] );
        close( pipeto[1] );

        int Status;
        waitpid(ID2,&Status,0); // Wait until the reading of the output is finished
      }
    }
    int Status;
    int Ret=waitpid(ID,&Status,0);
    if (Ret!=ID || Status)
    {
      if (IgnoreError)
      {
        cerr << "Error running command: "<<Command<<", but ignoring error\n";
        return true; // Ignore error
      }
      else
        return false;
    }
  }
  return true;
#endif
}

///////////////////////////////////////////////////////////////////////////////
#ifndef WIN32
string EscapeQuotes(const string &Params)
{
  int OldPos=0;
  int Pos;
  string Quote("\\\"");
  string SemiColon(" ; ");
  string Ret;

  while (1)
  {
    int Pos=Params.find_first_of('"',OldPos);
    int Pos2=Params.find(" & ",OldPos);
    string ToReplace(Quote);
    int Inc=1;

    if (Pos==string::npos)
    {
      if (Pos2==string::npos)
        break;
      Pos=Pos2;
      ToReplace=SemiColon;
      Inc=3;
    }
    else
    {
      if (Pos2!=string::npos && Pos2<Pos)
      {
        Pos=Pos2;
        ToReplace=SemiColon;
        Inc=3;
      }
    }
    Ret+=Params.substr(OldPos,Pos-OldPos);
    Ret+=ToReplace;
    OldPos=Pos+Inc;
  }
  Ret+=Params.substr(OldPos);
  return Ret;
}
#endif

///////////////////////////////////////////////////////////////////////////////
HANDLE mhmakefileparser::ExecuteCommand(string Command, bool &IgnoreError, string *pOutput)
{
  bool Echo=true;
  IgnoreError=false;
  while (1)
  {
    if (Command[0]=='@')
    {
      Echo=false;
      Command=Command.substr(1);
      continue;
    }
    if (Command[0]=='-')
    {
      IgnoreError=true;
      Command=Command.substr(1);
      continue;
    }
    break;
  }
  string InCommand=Command;
  if (g_Quiet)
    Echo=false;

  const char *pCom=Command.c_str();
  int StartCommandPos;
  int EndCommandPos;
  int BeginParamPos;
  if (*pCom=='"')
  {
    StartCommandPos=1;
    EndCommandPos=1;
    while (pCom[EndCommandPos]!='"') EndCommandPos++;
  }
  else
  {
    StartCommandPos=0;
    EndCommandPos=0;
  }
  while (!strchr(" \t",pCom[EndCommandPos])) EndCommandPos++;
  BeginParamPos=EndCommandPos;
  string Params=Command.substr(BeginParamPos);
  Command=Command.substr(StartCommandPos,EndCommandPos-StartCommandPos);

  // If we have special characters in the params we always call the command via the shell
  unsigned i;
  for (i=0; i<Params.size(); i++)
  {
    if (strchr("<>|&",Params[i]))
    {
      break;
    }
  }
  if (i==Params.size())
  {
    if (Command!="del" && Command!="touch" && Command!="copy" && Command!="echo")
      Command=GetFullCommand(Command);
#ifndef WIN32
    if (Command.substr(0,GetComspec().size())==GetComspec())
    {
      Params=EscapeQuotes(Params);
      Params+="\"";
    }
#endif
  }
  else
  {
    if (Command!="echo")
    {
      string FullCommand=GetFullCommand(Command);
      string ComSpec=GetComspec();
      if (FullCommand.substr(0,ComSpec.size())!=ComSpec)
        Command=FullCommand; // Only use FullCommand when it was found and not prepending by the comspec.
      Command=ComSpec+Command;
#ifndef WIN32
      Params=EscapeQuotes(Params);
      Params+="\"";
#endif
    }
  }
  if (Echo
      #ifdef _DEBUG
      || g_DoNotExecute
      #endif
      )
  {
    #ifdef _DEBUG
    if (g_PrintAdditionalInfo)
      cout << Command << Params << endl;
    else
      cout << InCommand << endl;
    #endif
  }

  /* first we check special internal commands */
  #ifdef _DEBUG
  if (pOutput || !g_DoNotExecute)
  {
  #endif
    if (Command=="del")
    {
      return DeleteFiles(Params);
    }
    else if (Command=="touch")
    {
      return TouchFiles(Params);
    }
    else if (Command=="copy")
    {
      return CopyFiles(Params);
    }
    else if (Command=="echo")
    {
      return EchoCommand(Params);
    }

    return OsExeCommand(Command,Params,IgnoreError,pOutput);
  #ifdef _DEBUG
  }
  #endif
  return (HANDLE)0; /* No Error */
}

///////////////////////////////////////////////////////////////////////////////
void mhmakefileparser::BuildDependencies(const refptr<rule> &pRule, const refptr<fileinfo> &Target, mh_time_t TargetDate, mh_time_t &YoungestDate, bool &MakeTarget)
{

  vector< refptr<fileinfo> > &Deps=Target->GetDeps();
  vector< refptr<fileinfo> >::iterator pDepIt=Deps.begin();
  while (pDepIt!=Deps.end())
  {
    StartBuildTarget(*pDepIt);
    pDepIt++;
  }
  pDepIt=Deps.begin();
  while (pDepIt!=Deps.end())
  {
    mh_time_t DepDate=WaitBuildTarget(*pDepIt);
    if (DepDate.IsNewer(YoungestDate))
      YoungestDate=DepDate;
    if (DepDate.IsNewer(TargetDate))
    {
      #ifdef _DEBUG
      if (pRule&&g_pPrintDependencyCheck && DepDate.IsExistingFile() && TargetDate.IsExistingFile())
        cout<<"Going to build "<<Target->GetQuotedFullFileName()<<" because "<<(*pDepIt)->GetQuotedFullFileName()<<" is more recent\n";
      #endif
      MakeTarget=true;
    }
    pDepIt++;
  }
}

///////////////////////////////////////////////////////////////////////////////
mh_time_t mhmakefileparser::StartBuildTarget(const refptr<fileinfo> &Target,bool bCheckTargetDir)
{
  #ifdef _DEBUG
  if (g_CheckCircularDeps)
  {
    deque< refptr<fileinfo> >::const_iterator pFind=find(m_TargetStack.begin(),m_TargetStack.end(),Target);
    if (pFind!=m_TargetStack.end())
    {
      cout << "Circular dependency detected.\n"<<Target->GetQuotedFullFileName()<<" depending on itself.\n";
      cout << "Dependency stack:\n";
      deque< refptr<fileinfo> >::const_iterator pIt=m_TargetStack.begin();
      while (pIt!=m_TargetStack.end())
      {
        cout << "  " << (*pIt)->GetQuotedFullFileName() << endl;
        pIt++;
      }
    }
    if (!Target->IsBuildStarted()) m_TargetStack.push_back(Target);
  }
  #endif

  #ifdef _DEBUG
  static int Indent;
  #endif

  if (g_StopCompiling)
  {
    throw string("Compilation Interrupted by user.");
  }

  if (Target->IsBuild())
  {
    #ifdef _DEBUG
    if (g_pPrintDependencyCheck)
    {
      for (int i=0; i<Indent; i++)
        cout<<"  ";
      cout<<"  Already build "<<Target->GetQuotedFullFileName()<<" : "<<Target->GetDate()<<endl;
    }
    #endif
    return Target->GetDate();
  }
  if (Target->IsBuilding())
    return mh_time_t();  // Target is still building, so we have to wait

  #ifdef _DEBUG
  if (g_GenProjectTree)
    cout << Target->GetQuotedFullFileName() << endl;

  Indent++;
  if (g_pPrintDependencyCheck)
  {
    for (int i=0; i<Indent; i++)
      cout<<"  ";
    cout<<"Building dependencies of "<<Target->GetQuotedFullFileName()<<endl;
  }
  #endif

  Target->SetBuild();

  /* Optimisation: do not build target when target dir does not exist,
     but first build the target dir, in case there exists a rule for it*/
  refptr<rule> pRule=Target->GetRule();

  if (!pRule && bCheckTargetDir)
  {
    refptr<fileinfo> TargetDir=Target->GetDir();
    mh_time_t TargetDirDate=BuildTarget(TargetDir,false);

    if (!TargetDir->Exists())
    {
      #ifdef _DEBUG
      Indent--;
      if (g_CheckCircularDeps)
      {
        m_TargetStack.pop_back();
      }
      #endif
      return TargetDirDate;
    }
  }

  mh_time_t TargetDate=Target->GetDate();
  bool MakeTarget=false;
  mh_time_t YoungestDate=TargetDate;

  if (!pRule || !pRule->GetCommands().size())
  {
    vector< pair<refptr<fileinfo>,refptr<rule> > > Result;


    IMPLICITRULE::SearchImplicitRule(Target,Result);

    vector< pair<refptr<fileinfo>,refptr<rule> > >::iterator ResultIt=Result.begin();
    while (ResultIt!=Result.end())
    {
      if (ResultIt->first==NullFileInfo)
      {
        pRule=ResultIt->second;
        Target->SetRule(pRule);
        #ifdef _DEBUG
        if (g_PrintAdditionalInfo)
        {
          cout<<"Found implicit rule for "<<Target->GetQuotedFullFileName()<<endl;
          pRule->PrintCommands(Target);
        }
        #endif
        break;
      }
      else
      {
        #ifdef _DEBUG
        m_ImplicitSearch++;
        #endif
        mh_time_t DepDate=BuildTarget(ResultIt->first);
        #ifdef _DEBUG
        m_ImplicitSearch--;
        #endif
        if (DepDate.DoesExist()) {
          if (DepDate.IsNewer(YoungestDate))
            YoungestDate=DepDate;
          pRule=ResultIt->second;
          Target->AddMainDep(ResultIt->first);
          Target->SetRule(pRule);  /* This is an implicit rule so do not add the target */
          #ifdef _DEBUG
          if (g_PrintAdditionalInfo)
          {
            cout<<"Found implicit rule for "<<Target->GetQuotedFullFileName()<<". Dependent "<<ResultIt->first->GetQuotedFullFileName()<<endl;
            pRule->PrintCommands(Target);
          }
          #endif
          if (DepDate.IsNewer(TargetDate))
          {
            #ifdef _DEBUG
            if (pRule,g_pPrintDependencyCheck && DepDate.IsExistingFile() && TargetDate.IsExistingFile())
              cout<<"Going to build "<<Target->GetQuotedFullFileName()<<" because "<<ResultIt->first->GetQuotedFullFileName()<<" is more recent\n";
            #endif
            MakeTarget=true;
          }
          break;
        }
      }
      ResultIt++;
    }
    if (pRule)
    {
      #ifdef _DEBUG
      Target->SetBuilding();
      Target->SetRule(pRule);
      Target->ClearBuilding();
      #else
      Target->SetRule(pRule);
      #endif
    }
  }

  mhmakeparser *pMakefile=NULL;
  if (pRule)
  {
    pMakefile=pRule->GetMakefile();
    if (pMakefile->ForceAutoDepRescan()||MakeTarget==true)
      pMakefile->UpdateAutomaticDependencies(Target);
  }

  BuildDependencies(pRule,Target,TargetDate,YoungestDate,MakeTarget);

  if (pRule)
  {
    #ifdef _DEBUG
    if (g_pPrintDependencyCheck)
    {
      for (int i=0; i<Indent; i++)
        cout<<"  ";
      cout<<"Building "<<Target->GetQuotedFullFileName()<<endl;
    }
    #endif
    if (!MakeTarget)
    {
      if (!TargetDate.DoesExist() || ( (g_RebuildAll || pMakefile->m_RebuildAll) && TargetDate.IsOlder(m_sBuildTime)))
      {
        #ifdef _DEBUG
        if (g_pPrintDependencyCheck)
        {
          if (!TargetDate.DoesExist())
          {
            if (!m_ImplicitSearch && !Target->IsPhony())
              cout<<"Building "<<Target->GetQuotedFullFileName()<<" because it does not exist yet\n";
          }
          else if (TargetDate.IsOlder(m_sBuildTime))
          {
            cout<<"Building "<<Target->GetQuotedFullFileName()<<" because need to rebuild all (-a)\n";
          }
        }
        #endif
        MakeTarget=true;
      }
    }

    // Now execute the commands
    vector<string> &Commands=pRule->GetCommands();

    if (!MakeTarget)
    {
      vector<string>::iterator CommandIt=Commands.begin();

      md5_context ctx;
      md5_starts( &ctx );
      while (CommandIt!=Commands.end())
      {
        pMakefile->SetRuleThatIsBuild(Target); // Make sure that the command expension is correct
        string Command=pMakefile->ExpandExpression(*CommandIt);
        pMakefile->ClearRuleThatIsBuild();  /* Make sure that further expansion is not taking this rule into account.*/
        md5_update( &ctx, (uint8 *)Command.c_str(), (unsigned long)Command.size());
        CommandIt++;
      }

      uint32 Md5_32=md5_finish32( &ctx);
      if (!Target->CompareMd5_32(Md5_32))
      {
        if (TargetDate.IsNewerOrSame(m_sBuildTime) || TargetDate.IsDir())
        {
          // Only rebuild if it is not yet rebuild in this current run. This may happen for implicit rules that have multiple targets (implicit rules that build more then one target at the same time
          Target->SetCommandsMd5_32(Md5_32);
          pMakefile->AddTarget(Target);
          pMakefile->SetAutoDepsDirty();  /* We need to update the autodeps file if the md5 has been changed */
        }
        else
        {
          #ifdef _DEBUG
          if (!g_GenProjectTree)
            cout << "Md5 is different for " << Target->GetQuotedFullFileName() << " Old:"<<hex<<Target->GetCommandsMd5_32()<<", New: "<<Md5_32<<". Commandline must have been changed so recompiling\n";
          #endif

          #ifdef _DEBUG
          Indent--;
          if (g_CheckCircularDeps)
          {
            m_TargetStack.pop_back();
          }
          #endif

          MakeTarget=true;
        }
      }
    }
    if (MakeTarget)
    {
      // Queue for execution
//      Target->SetDate(YoungestDate);
      if (sm_CommandQueue.QueueTarget(Target))
        return mh_time_t();
      mh_time_t NewDate=Target->GetDate();
      if (NewDate.IsNewer(YoungestDate))
        YoungestDate=NewDate;
    }
  }

  #ifdef _DEBUG
  if (g_pPrintDependencyCheck)
  {
    for (int i=0; i<Indent; i++)
      cout<<"  ";
    cout<<"Building "<<Target->GetQuotedFullFileName()<<" finished : "<< YoungestDate << endl;
  }
  Indent--;
  if (g_CheckCircularDeps)
  {
    m_TargetStack.pop_back();
  }

  if (!m_ImplicitSearch && !Target->Exists() && !Target->IsPhony() && !g_DoNotExecute && !g_GenProjectTree)
  {
    // This is only a warning for phony messages
    cout<<"Warning: don't know how to make "<<Target->GetQuotedFullFileName()<<"\nMake the rule a phony rule to avoid this warning (but only do it when it is really phony).\n";;
  }
  #endif
  Target->SetDate(YoungestDate); /* This is especially needed for phony targets in between real targets */
  return YoungestDate;
}

///////////////////////////////////////////////////////////////////////////////
mh_time_t mhmakefileparser::WaitBuildTarget(const refptr<fileinfo> &Target)
{
  return sm_CommandQueue.WaitForTarget(Target);
}

///////////////////////////////////////////////////////////////////////////////
void mhmakefileparser::BuildIncludedMakefiles()
{
  vector< refptr<fileinfo> >::iterator MakefileIt=m_IncludedMakefiles.begin();
  while (MakefileIt!=m_IncludedMakefiles.end())
  {
    #ifdef _DEBUG
    if (g_PrintAdditionalInfo)
      cout<<"Building include file "<<(*MakefileIt)->GetQuotedFullFileName()<<endl;
    #endif
    BuildTarget(*MakefileIt);
    MakefileIt++;
  }
}
