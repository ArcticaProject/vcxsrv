#!/usr/bin/python
__doc__="""%prog [options] [<directory>]

Removes all unversioned files from a subversion working copy directory
<directory>.
When <directory> is not specified the current directory is used.
<directory can also be a white space seperated list of directories.
"""
import os,sys,re,shutil
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

################################################################################

def Print (Message,NoVerbose=None,Append='\n'):
  if NoVerbose!=1:
    Message+=Append
    sys.stdout.write(Message)

################################################################################
def RunCommand(Command,NoVerbose=None):
  #Print(Command)
  StdIn,StdOut=os.popen4(Command)
  while 1:
    line=StdOut.readline()
    if not line:
      break
    Print(line,NoVerbose,'')

################################################################################
def CleanTree(Dir, NoVerbose=None):
  Print("Cleaning '"+Dir+"'")
  if g_Options.Recursive:
    Command='svn st --no-ignore'
  else:
    Command='svn st -N --no-ignore'
  StdIn,StdOut=os.popen4(Command)
  NotWorkRe=re.compile("'\.' is not a working copy")
  while 1:
    line=StdOut.readline()
    if not line:
      break
    if NotWorkRe.search(line):
      Print(NotWorkRe.sub("'%s' is not a working copy directory. Not cleaning this directory."%(re.sub(r"\\",r"\\\\",Dir)),line))
      sys.exit(1)
    if line[0]=='?' or line[0]=='I':
      Item=re.sub("^\s+","",line[2:-1])
      if g_Options.SkipUtil:
        if Item[:5]=='util'+os.sep:
          continue
        if Item[:6]=='tools'+os.sep:
          continue
      if os.path.isdir(Item):
        Print('Deleting directory %s'%(os.path.abspath(Item)),NoVerbose)
        try:
          shutil.rmtree(Item)
        except:
          print "Error deleting directory %s. Contains read-only files?"%Item
      else:
        Print('Deleting file %s'%(os.path.abspath(Item)),NoVerbose)
        try:
          os.remove(Item)
        except:
          print "Error deleting file %s. Is read-only?"%Item

################################################################################
if not g_Args:
  g_Args=["."]

for Dir in g_Args:
  os.chdir(Dir)
  CleanTree(Dir, g_Options.NoVerbose)
