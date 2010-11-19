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

#ifndef __FILEINFO_H
#define __FILEINFO_H

#include "curdir.h"
#include "rule.h"
#include "md5.h"

#ifdef WIN32
#define OSPATHSEP '\\'
#define OSPATHSEPSTR "\\"
#define OSPATHENVSEP      ';'
#define OSPATHENVSEPSTR   ";"
#else
#define OSPATHSEP '/'
#define OSPATHSEPSTR "/"
#define OSPATHENVSEP      ':'
#define OSPATHENVSEPSTR   ":"
#endif

extern bool g_DumpOnError;
extern bool g_PrintVarsAndRules;
extern bool g_DoNotExecute;
extern bool g_GenProjectTree;
extern bool g_Quiet;
extern bool g_RebuildAll;
extern bool g_PrintAdditionalInfo;
extern bool g_pPrintDependencyCheck;
extern bool g_CheckCircularDeps;
extern bool g_ForceAutoDepRescan;
extern bool g_PrintLexYacc;
extern bool g_Clean;
extern bool g_PrintMultipleDefinedRules;

extern const string g_EmptyString;
extern const string g_SpaceString;

#define TIMESAFETY 3
class mh_time
{
  enum
  {
    DATENOTVALID=0,
    NOTEXISTTIME=1,
    DIRTIME     =2+TIMESAFETY
  };
  unsigned long m_Time;

  bool operator < (const mh_time &Src);
public:
  mh_time(){m_Time=DATENOTVALID;}
#ifdef WIN32
  mh_time(unsigned __int64 Time) : m_Time((unsigned long)(Time&0xffffffff)) {}
  mh_time(__int64 Time) : m_Time((unsigned long)(Time&0xffffffff)) {}
#endif
  mh_time(unsigned long Time) : m_Time(Time) {}
  mh_time(const mh_time &Time) : m_Time(Time.m_Time) {}

  void SetDir(void)
  {
    m_Time=DIRTIME;
  }
  bool IsDir(void) const
  {
    return m_Time==DIRTIME;
  }
  void SetNotExist(void)
  {
    m_Time=NOTEXISTTIME;
  }
  bool IsExistingFile(void) const
  {
    return m_Time>DIRTIME;
  }
  bool DoesExist(void) const
  {
    return m_Time!=NOTEXISTTIME;
  }
  void Invalidate(void)
  {
    m_Time=DATENOTVALID;
  }
  bool IsDateValid(void) const
  {
    return m_Time!=DATENOTVALID;
  }
  friend ostream& operator<<(ostream& out,const mh_time &Src);

  mh_time &operator = (const mh_time &Src) { m_Time=Src.m_Time; return *this;}

  bool IsOlder(const mh_time &Src) const { return m_Time<Src.m_Time-TIMESAFETY; }

  bool IsNewer(const mh_time &Src) const { return m_Time>Src.m_Time+TIMESAFETY; }
  bool IsNewerOrSame(const mh_time &Src) const { return m_Time>=Src.m_Time-TIMESAFETY; }
};
typedef mh_time mh_time_t;

inline ostream& operator<<(ostream& out,const mh_time &Src)
{
  out << hex << (unsigned long)Src.m_Time;
  return out;
}

class fileinfo : public refbase
{
  string m_AbsFileName;
  bool m_IsPhony;
  bool m_IsBuild;
  refptr<rule> m_pRule;
  vector< refptr<fileinfo> > m_Deps;
  mh_time_t m_Date;
  uint32 m_CommandsMd5_32;  // 32-bit Md5 checksum of commands to build this target

  fileinfo(const fileinfo &Src);
  fileinfo(void);
public:

  fileinfo(const string &AbsFileName,uint32 Md5_32)
  {
    m_IsPhony=false;
    m_IsBuild=false;
    m_AbsFileName=AbsFileName;
    InvalidateDate();
    m_CommandsMd5_32=Md5_32;
    #ifdef _DEBUG
    if (g_PrintAdditionalInfo)
      cout << "Initialising Md5 of "<<GetFullFileName().c_str()<<" to 0x"<<hex<<Md5_32<<endl;
    #endif
  }
  fileinfo(const string &AbsFileName)
  {
    new (this) fileinfo(AbsFileName,0);
  }
  /* The following constructor is only used for name comparisons, and should only be used for that */
  fileinfo(int Dummy,const string &AbsFileName)
  {
    m_AbsFileName=AbsFileName;
  }

  fileinfo(const char *szFile)
  {
    new (this) fileinfo(string(szFile));
  }
  fileinfo(const char *szFile,uint32 Md5_32)
  {
    new (this) fileinfo(string(szFile),Md5_32);
  }
  ~fileinfo()
  {
  }
  const string &GetFullFileName(void) const
  {
    return m_AbsFileName;
  }
  void SetFullFileName(const string &strAbsName)
  {
    m_AbsFileName=strAbsName;
    // If the last char is path sep strip it
    if (!m_AbsFileName.empty() && m_AbsFileName[m_AbsFileName.length()-1]==OSPATHSEP)
      m_AbsFileName.resize(m_AbsFileName.length()-1);
  }
  fileinfo &operator = (const fileinfo &Src)
  {
    new (this) fileinfo(Src);
    return *this;
  }

  refptr<fileinfo> GetDir(void) const;
  string GetName() const;
  bool IsDir() const;

  void DumpErrorMessageDuplicateRule(const refptr<rule> &pRule);

  void SetRule(refptr<rule> &pRule)
  {
    #if defined(_DEBUG) && defined(_MSC_VER)
    if (m_pRule && m_pRule->GetCommands().size()) {
      _asm int 3;
    }
    #endif
    m_pRule=pRule;
  }

  void SetRuleIfNotExist(refptr<rule> &pRule)
  {
    if (pRule)
    {
      if (!m_pRule)
      {
        SetRule(pRule);
        pRule->AddTarget(this);
      }
      #ifdef _DEBUG
      else
      {
        if (*m_pRule!=*pRule)
        {
          DumpErrorMessageDuplicateRule(pRule);
          throw(1);
        }
        else if (g_PrintMultipleDefinedRules)
        {
          DumpErrorMessageDuplicateRule(pRule);
        }
      }
      #endif
    }
  }

  refptr<rule> GetRule(void)
  {
    return m_pRule;
  }
  void AddDep(const refptr<fileinfo> &Dep)
  {
    if (&*Dep==this)
    {
      #ifdef _DEBUG
      cout << GetFullFileName()<<" is directly dependent on itself\n";
      #endif
      return;
    }
    m_Deps.push_back(Dep);
  }
  void AddDeps(vector< refptr<fileinfo> > &Deps);

  void InsertDeps(vector< refptr<fileinfo> > &Deps)
  {
    vector< refptr<fileinfo> > NewDeps;
    vector< refptr<fileinfo> >::const_iterator It=Deps.begin();
    vector< refptr<fileinfo> >::const_iterator ItEnd=Deps.end();
    while (It!=ItEnd)
    {
      if (&**It==this)
      {
        #ifdef _DEBUG
        cout << GetFullFileName()<<" is directly dependent on itself\n";
        #endif
      }
      else
        NewDeps.push_back(*It);
      It++;
    }
    if (NewDeps.size())
      m_Deps.insert(m_Deps.begin(),NewDeps.begin(),NewDeps.end());
  }
  void AddMainDep(refptr<fileinfo> &MainDep)
  {
    if (&*MainDep==this)
    {
      #ifdef _DEBUG
      cout << GetFullFileName()<<" is directly dependent on itself\n";
      #endif
      return;
    }
    m_Deps.insert(m_Deps.begin(),MainDep);
  }
  vector< refptr<fileinfo> > &GetDeps(void)
  {
    return m_Deps;
  }
  string GetPrerequisits(void) const;
  void SetPhony(void)
  {
    m_IsPhony=true;
    m_Date.SetNotExist(); // This will sure that this target will always be build (even if a corresponding file exists)
  }
  bool IsPhony(void)
  {
    return m_IsPhony;
  }
  mh_time_t realGetDate(void);
  void SetDateToNow(void);

  void SetDate(mh_time_t Date)
  {
    m_Date=Date;
  }

  bool IsDateValid() const
  {
    return m_Date.IsDateValid();
  }
  void InvalidateDate(void)
  {
    m_Date.Invalidate();
  }

  mh_time_t GetDate(void)
  {
    if (m_Date.IsDateValid())
      return m_Date;
    else
      return realGetDate();
  }
  void SetNotExist(void)
  {  // this is used to make sure that this item is rebuild, even if it really exists
    m_Date.SetNotExist();
  }
  bool Exists(void)
  {
    return GetDate().DoesExist();
  }
  bool IsBuild(void) const
  {
    return m_IsBuild;
  }
  void SetBuild(void)
  {
    m_IsBuild=true;
  }
  void ClearBuild(void)
  {
    m_IsBuild=false;
  }
  bool IsAutoDepExtention(void) const;

  void SetCommandsMd5_32(uint32 Md5_32)
  {
    #ifdef _DEBUG
    if (g_PrintAdditionalInfo)
      cout << "Setting Md5 of "<<GetFullFileName().c_str()<<" to 0x"<<hex<<Md5_32<<endl;
    #endif
    m_CommandsMd5_32=Md5_32;
  }
#ifdef _DEBUG
  uint32 GetCommandsMd5_32(void) const
  {
    return m_CommandsMd5_32;
  }
#endif
  bool CompareMd5_32(uint32 Md5_32) const
  {
    return m_CommandsMd5_32==Md5_32;
  }
  void WriteMd5_32(FILE *pFile) const
  {
    fwrite(&m_CommandsMd5_32,sizeof(m_CommandsMd5_32),1,pFile);
  }
};

struct less_refptrfileinfo : public binary_function <refptr<fileinfo>, refptr<fileinfo>, bool>
{
  bool operator()(const refptr<fileinfo>& _Left, const refptr<fileinfo>& _Right) const
  {
    return less<string>().operator ()(_Left->GetFullFileName(),_Right->GetFullFileName());
  }
};

struct less_fileinfo : public binary_function <const fileinfo*, const fileinfo*, bool>
{
  bool operator()(const fileinfo *_Left, const fileinfo *_Right) const
  {
    return less<string>().operator ()(_Left->GetFullFileName(),_Right->GetFullFileName());
  }
};

extern const string NullString;
extern refptr<fileinfo> NullFileInfo;

const refptr<fileinfo> &GetFileInfo(const string &szName,const refptr<fileinfo> &pRelDir=curdir::GetCurDir());

extern set<refptr<fileinfo>,less_refptrfileinfo > g_FileInfos;

inline const refptr<fileinfo> &GetAbsFileInfo(const string &strAbsName)
{
  static refptr<fileinfo> SearchFileInfo(new fileinfo(""));
  SearchFileInfo->SetFullFileName(strAbsName);
  /* Using find is just an optimalisation, you could use insert immediately */
  set<refptr<fileinfo>,less_refptrfileinfo >::const_iterator pFind=g_FileInfos.find(SearchFileInfo);
  if (pFind==g_FileInfos.end())
  {
    pair <set<refptr<fileinfo>,less_refptrfileinfo >::iterator, bool> pPair=g_FileInfos.insert(new fileinfo(SearchFileInfo->GetFullFileName()));
    return *(pPair.first);
  }
  else
    return *pFind;
}


inline const refptr<fileinfo> &GetFileInfo(const string &szName,const string &RelDir)
{
  return GetFileInfo(szName,GetFileInfo(RelDir));
}

inline const refptr<fileinfo> &GetFileInfo(const char *szName,const char *RelDir)
{
  return GetFileInfo(string(szName),string(RelDir));
}

inline const refptr<fileinfo> &GetFileInfo(const char *szName,const refptr<fileinfo> &RelDir=curdir::GetCurDir())
{
  return GetFileInfo(string(szName),RelDir);
}

void PrintFileInfos();

#ifdef WIN32
class ZEROTIME
{
  __int64 m_ZeroTime;
public:
  ZEROTIME()
  {
    SYSTEMTIME SystemTime;

    memset(&SystemTime,0,sizeof(SystemTime));
    SystemTime.wYear=1970;
    SystemTime.wMonth=1;
    SystemTime.wDay=1;
    SystemTime.wDayOfWeek=4;

    SystemTimeToFileTime(&SystemTime,(FILETIME*)&m_ZeroTime);
  }

  __int64 GetZeroTime()
  {
    return m_ZeroTime;
  }

  mh_time_t ConvertTime(FILETIME *pFileTime)
  {
    return (mh_time_t)((*(__int64*)pFileTime-m_ZeroTime)/10000000);  /* filetime is in nano seconds*/
  }
};

extern ZEROTIME g_ZeroTime;
#endif

#endif

