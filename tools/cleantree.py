#!/usr/bin/python
import os,sys,re,shutil
from optparse import OptionParser

parser = OptionParser()
parser.add_option("-r", "--recursive", action='store_true', dest="Recursive", default=False, help="Also clean subdirectories.")
parser.add_option("-s", "--skiputil", action='store_true', dest="SkipUtil", default=False, help="Skip util and tools directory.")
parser.add_option("-v", "--noverbose", action='store_true', dest="NoVerbose", default=False, help="No output.")

(g_Options, g_Args) = parser.parse_args()

################################################################################

def Print (Message,NoVerbose=None,Append='\n'):
#  global pLOG
  if NoVerbose!=1:
    Message+=Append
    sys.stdout.write(Message)
#  if NoVerbose!=2:
#    pLOG.write(Message)

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
def CleanTree(NoVerbose=None):
  if g_Options.Recursive:
    Command='svn st --no-ignore'
  else:
    Command='svn st -N --no-ignore'
  StdIn,StdOut=os.popen4(Command)
  NotWorkRe=re.compile('is not a working copy')
  while 1:
    line=StdOut.readline()
    if not line:
      break
    if NotWorkRe.search(line):
      Print(line)
      sys.exit(1)
    if line[0]=='?' or line[0]=='I':
      Item=re.sub("^\s+","",line[2:-1])
      if g_Options.SkipUtil:
        if Item[:5]=='util'+os.sep:
          continue
        if Item[:6]=='tools'+os.sep:
          continue
      if os.path.isdir(Item):
        Print('Deleting directory %s'%Item,NoVerbose)
        try:
          shutil.rmtree(Item)
        except:
          print "Error deleting directory %s. Contains read-only files?"%Item
      else:
        Print('Deleting file %s'%Item,NoVerbose)
        try:
          os.remove(Item)
        except:
          print "Error deleting file %s. Is read-only?"%Item
CleanTree(g_Options.NoVerbose)
