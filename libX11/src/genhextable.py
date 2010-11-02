import sys

HexTable={
    '0' : 0,  '1' : 1,
    '2' : 2,  '3' : 3,
    '4' : 4,  '5' : 5,
    '6' : 6,  '7' : 7,
    '8' : 8,  '9' : 9,
    'A' : 10, 'B' : 11,
    'C' : 12, 'D' : 13,
    'E' : 14, 'F' : 15,
    'a' : 10, 'b' : 11,
    'c' : 12, 'd' : 13,
    'e' : 14, 'f' : 15,

    ' ' : -1, ',' : -1,
    '}' : -1, '\n' : -1,
    '\t' : -1
}

OutHexTable=[0]*256

for Char,Val in HexTable.iteritems():
  OutHexTable[ord(Char)]=Val
  
print "static const short hexTable[256] = {"
i=0
for Item in OutHexTable:
  if i==0:
    PreFix="   "
  elif i%16 == 0:
    PreFix="\n  ,"
  else:
    PreFix=", "
  i+=1
  Val="%d"%Item
  if len(Val)==1: Val = " "+Val
  sys.stdout.write("%s%s"%(PreFix,Val))
print "\n};"  