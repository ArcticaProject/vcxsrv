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

#include "fileinfo.h"
#include "curdir.h"
#include "util.h"

set<refptr<fileinfo>,less_refptrfileinfo> g_FileInfos;  // declare here since it is important that it is constructed before m_pcurrentdir
curdir::initcurdir curdir::m_pCurrentDir;

///////////////////////////////////////////////////////////////////////////////
curdir::initcurdir &curdir::initcurdir::operator=(const refptr<fileinfo>& Src)
{
  return (curdir::initcurdir&)refptr<fileinfo>::operator=(Src);
}

///////////////////////////////////////////////////////////////////////////////
curdir::initcurdir::initcurdir()
{
  char CurDir[MAX_PATH];
  getcwd(CurDir,MAX_PATH);
  *this=GetFileInfo(CurDir,refptr<fileinfo>());
}

///////////////////////////////////////////////////////////////////////////////
void curdir::ChangeCurDir(const refptr<fileinfo>&NewDir)
{
  if (NewDir!=m_pCurrentDir)
  {
    #ifdef _DEBUG
    if (g_PrintAdditionalInfo)
      cout << "Changing to dir "<<NewDir->GetFullFileName()<<endl;
    #endif
    if (-1==chdir(NewDir->GetFullFileName().c_str()))
    {
      cerr<<"Error changing to directory "<<NewDir->GetQuotedFullFileName()<<endl;
      throw(1);
    }
    m_pCurrentDir=NewDir;
  }
}

