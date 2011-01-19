import sys,os,re,glob

SRCDIR=sys.argv[1]
HEAD=open(sys.argv[2],"w")
TAIL=open(sys.argv[3],"w")

#rm -f $HEAD $TAIL
TAIL.write("#if HAVE_GNUC_ATTRIBUTE\n")

InputFiles=sys.argv[4:]
Names=[]
for Input in InputFiles:
  buffer=open(Input,"r").read()
  Names.extend(re.findall(r'\n(Fc[^ ]*) *\(',buffer))

def SearchCFiles(Name):
  Files=glob.glob(os.path.join(SRCDIR,"*.c") )
  for File in Files:
    buffer=open(File,"r").read()
    res = re.findall(r'\n%s[ \(].*'%Name,buffer)
    if res:
      File=re.sub(r'^.*[\\/]','',File)
      return "__"+re.sub(r'\.c','__',File)

for Name in Names:
  if Name=="FcCacheDir" or Name=="FcCacheSubdir":
    continue

  alias="IA__"+Name
  HEAD.write("extern __typeof (%s) %s;\n"%(Name,alias))
  HEAD.write("#define %s %s\n"%(Name,alias))
  TAIL.write("#ifdef "+SearchCFiles(Name)+"\n")
  TAIL.write("#undef %s\n"%Name)
  TAIL.write("extern __typeof (%s) %s;\n"%(Name,Name))
  TAIL.write("#endif\n")
TAIL.write("#endif\n")
