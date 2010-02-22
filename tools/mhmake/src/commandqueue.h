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

#ifndef __COMMANDQUEUE_H__
#define __COMMANDQUEUE_H__

#include "fileinfo.h"

#ifdef WIN32
typedef HANDLE mh_pid_t;
#else
typedef pid_t mh_pid_t;
#endif

class commandqueue
{
  struct activeentry
  {
    refptr<fileinfo>         pTarget;
    vector<string>::iterator CurrentCommandIt;
    string                   Command;
    md5_context              md5ctx;
    bool                     IgnoreError;
  };
private:
  queue< refptr<fileinfo> >  m_Queue;
  unsigned                   m_MaxNrCommandsInParallel;
  mh_pid_t                  *m_pActiveProcesses;
  activeentry               *m_pActiveEntries;
  unsigned                   m_NrActiveEntries;

private:
  void ThrowCommandExecutionError(activeentry *pActiveEntry);
  void AddActiveEntry(activeentry &ActiveEntry, mh_pid_t ActiveProcess);
  void RemoveActiveEntry(unsigned Entry);
  bool StartExecuteCommands(const refptr<fileinfo> &pTarget);
  bool StartExecuteNextCommand(activeentry *pActiveEntry, mh_pid_t *pActiveProcess);
  void TargetBuildFinished(activeentry *pActiveEntry);

public:
  commandqueue();
  ~commandqueue();

  bool QueueTarget(const refptr<fileinfo> &pTarget);  // Returns true if target has been queued, false when commands are executed upon return
  mh_time_t WaitForTarget(const refptr<fileinfo> &pTarget);
  void SetNrParallelBuilds(unsigned NrParallelBuilds);
};


#endif
