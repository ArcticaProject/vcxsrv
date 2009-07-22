import sys

pOUT=open(sys.argv[1],"wb")
pHDR=open("HDR","r")
for item in sys.argv[2:]:
  if item == 'HDR':
    pOUT.write("\n")
    pOUT.write(pHDR.readline())
  else:
    pOUT.write(open(item,"rb").read())

pOUT.close()

