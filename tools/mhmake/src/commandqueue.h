#ifndef __COMMANDQUEUE_H__
#define __COMMANDQUEUE_H__

#include "fileinfo.h"

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
  HANDLE                    *m_pActiveProcesses;
  activeentry               *m_pActiveEntries;
  unsigned                   m_NrActiveEntries;

private:
  void ThrowCommandExecutionError(activeentry *pActiveEntry);
  void RemoveActiveEntry(unsigned Entry);
  bool StartExecuteCommands(const refptr<fileinfo> &pTarget);
  bool StartExecuteNextCommand(activeentry *pActiveEntry, HANDLE *pActiveProcess);
  void TargetBuildFinished(activeentry *pActiveEntry);

public:
  commandqueue();
  ~commandqueue();

  bool QueueTarget(const refptr<fileinfo> &pTarget);  // Returns true if target has been queued, false when commands are executed upon return
  mh_time_t WaitForTarget(const refptr<fileinfo> &pTarget);
  void SetNrParallelBuilds(unsigned NrParallelBuilds);
};


#endif
