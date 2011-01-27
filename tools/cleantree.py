#!/usr/bin/python
__doc__="""%prog [options] [<directory>]

Removes all unversioned files from a subversion working copy directory
<directory>.
When <directory> is not specified the current directory is used.
<directory can also be a white space seperated list of directories.
"""
import os,sys,re,subprocess
from optparse import OptionParser

parser = OptionParser(__doc__)
parser.add_option("-r", "--recursive", action='store_true', dest="Recursive", default=False, help="Also clean subdirectories.")
parser.add_option("-s", "--skiputil", action='store_true', dest="SkipUtil", default=False, help="Skip util and tools directory.")
parser.add_option("-v", "--noverbose", action='store_true', dest="NoVerbose", default=False, help="No output.")

(g_Options, g_Args) = parser.parse_args()

if g_Args:
  for i in range(len(g_Args)):
    g_Args[i]=os.path.abspath(g_Args[i])
    if not os.path.isdir(g_Args[i]):
      parser.error("%s is not a valid directory"%g_Args[i])

NotWorkRe=re.compile("'\.' is not a working copy")

################################################################################

def Print (Message,NoVerbose=None,Append='\n'):
  if NoVerbose!=1:
    Message+=Append
    sys.stdout.write(Message)

################################################################################
class RunCommandException(Exception):
  def __init__(this,CommandLine,Output=None):
    this.CommandLine=CommandLine
    this.Output=Output
  def __str__(this):
    Ret="\n!!! RunCommand exception:\n-> Commandline:\n"
    Ret+=this.CommandLine
    if this.Output:
      Ret+="\n-> Output:\n"
      Ret+=this.Output
    else:
      Ret+="\n-> Command not found"
    Ret+="\n"
    return Ret
################################################################################
PIPE=subprocess.PIPE
STDOUT=subprocess.STDOUT

def RunCommand(Command):
  try:
    if sys.platform=='linux2':
      Process=subprocess.Popen(Command, stdin=PIPE, stdout=PIPE, stderr=STDOUT, shell=True)
    else:
      CREATE_NO_WINDOW=0x8000000
      Process=subprocess.Popen(Command, stdin=PIPE, stdout=PIPE, stderr=STDOUT, creationflags=CREATE_NO_WINDOW, shell=True)
      import msvcrt
      msvcrt.setmode(Process.stdout.fileno(), os.O_TEXT)  # Threat output as ascii noy binary
  except:
    raise RunCommandException(Command)
  return Process

################################################################################
def DelTreeNoJunctions(Dir):
  try:
    os.rmdir(Dir)
    # empty dir or it was a junction, so we can return immediately
    return
  except:
    pass

  for root, dirs, files in os.walk(Dir, topdown=True):
    for name in files:
      os.remove(os.path.join(root,name))
    for dir in dirs:
      # first try just to remove the directory. This is to be sure that when it is a symbolic link
      # just the link is removed and not the complete contents where it points to
      try:
        os.rmdir(os.path.join(root,dir))
        del dirs[dir]  # do not walk this directory
      except:
        pass # go further in the directory tree
  # Now delete the empty directories left over from the previous walk
  for root, dirs, files in os.walk(Dir, topdown=False):
    for dir in dirs:
      os.rmdir(os.path.join(root,dir))
  os.rmdir(Dir)

################################################################################
def ProcessSvnLine(line,NoVerbose=None):
  if line:
    if NotWorkRe.search(line):
      Print(NotWorkRe.sub("'%s' is not a working copy directory. Not cleaning this directory."%(re.sub(r"\\",r"\\\\",Dir)),line))
      sys.exit(1)
    if line[0]=='?' or line[0]=='I':
      Item=re.sub("^\s+","",line[2:])
      if g_Options.SkipUtil:
        if Item[:5]=='util'+os.sep:
          return
        if Item[:6]=='tools'+os.sep:
          return
      if os.path.isdir(Item):
        Print('Deleting directory %s'%(os.path.abspath(Item)),NoVerbose)
        try:
          DelTreeNoJunctions(Item)
        except:
          print "Error deleting directory %s. Contains read-only files?"%Item
      else:
        Print('Deleting file %s'%(os.path.abspath(Item)),NoVerbose)
        try:
          os.remove(Item)
        except Exception,Details:
          print "Error deleting file %s. Is read-only? "%Item
          print Details
################################################################################
def CleanTree(Dir, NoVerbose=None):
  Print("Cleaning '"+Dir+"'")
  if g_Options.Recursive:
    Command='svn st --no-ignore'
  else:
    Command='svn st -N --no-ignore'
  Process=RunCommand(Command)
  StdOut=Process.stdout
  all_data=""
  while 1:
    done = Process.poll()
    if done == None or done == 0:
      line=StdOut.readline()
      all_data+=line
      line=line.rstrip()
      ProcessSvnLine(line,NoVerbose)
    if done == 0:
      data=StdOut.read()
      all_data+=data
      for line in data.splitlines():
        ProcessSvnLine(line,NoVerbose)
      break
    elif done != None:
      #error!
      print "error running svn command:", Command
      print all_data, StdOut.read()
      sys.exit(1)
################################################################################
if not g_Args:
  g_Args=["."]

for Dir in g_Args:
  os.chdir(Dir)
  CleanTree(Dir, g_Options.NoVerbose)
