#! /usr/bin/python

import urllib2
import sys
from lxml import html

fp = urllib2.urlopen('http://unicode.org/cldr/utility/list-unicodeset.jsp?a=[%3AGC%3DZs%3A][%3ADI%3A]&abb=on&ucd=on&esc=on&g')
data = fp.read()
fp.close()

dom = html.fromstring(data)
x = dom.xpath('/html/body/form/p/text()')
p = x[1]
if p[0] == '[' and p[-1] == ']':
    p = p.replace('[', '').replace(']', '')
else:
    sys.exit(1)
fescape = False
funicode = False
frange = False
fprocess = False
v = 0
vbegin = 0
vend = 0
n = 0
l = []

def insert(db, begin, end):
    db.append([begin, end])

for i in p:
    if i == '\\':
        if n > 0:
            if frange == True and funicode == True:
                vend = v
                insert(l, vbegin, vend)
                fprocess = True
            elif funicode == True:
                vbegin = v
                vend = v
                insert(l, vbegin, vend)
                fprocess = True
        funicode = False
        fescape = True
    elif i.lower() == 'u' and fescape == True:
        funicode = True
        fescape = False
    elif i >= '0' and i <= '9' or i.lower() >= 'a' and i.lower() <= 'f':
        if fescape == True:
            raise RuntimeError, "Unexpected escape code"
        if funicode == True:
            v <<= 4
            v += int(i, 16)
        else:
            raise RuntimeError, "Unable to parse Unicode"
    elif i == ' ':
        if fescape == True:
            funicode = True
            fescape = False
            v = 0x20
        if frange == True and funicode == True:
            vend = v
            insert(l, vbegin, vend)
            fprocess = True
        elif funicode == True:
            vbegin = v
            vend = v
            insert(l, vbegin, vend)
            fprocess = True
        funicode = False
        frange = False
    elif i == '-':
        if fescape == True:
            raise RuntimeError, "Unexpected escape code"
        vbegin = v
        v = 0
        funicode = False
        frange = True
    else:
        raise RuntimeError, "Unable to parse Unicode: %s" % i

    if fprocess == True:
        vbegin = 0
        vend = 0
        v = 0
        fprocess = False
        funicode = False
        frange = False
    n += 1

if frange == True and funicode == True:
    vend = v
    insert(l, vbegin, vend)
elif funicode == True:
    vbegin = vend = v
    insert(l, vbegin, vend)

ncode = 0
for i in l:
    ncode += (i[1] - i[0] + 1)

a = int(x[0].split(' ')[0].replace(',', ''))
if a != ncode:
    sys.stderr.write("Unexpected the amount of code points: %d (expected %d)\n" % (ncode, a))
    sys.exit(1)

# exception; BRAILLE PATTERN BLANK
insert(l, 0x2800, 0x2800)

while True:
    s = sys.stdin.readline().rstrip()
    if s == "@@@":
        break
    print s

print "static FcChar32 _fcBlanks[%s] = {" % (ncode + 1)
k = 0
for i in sorted(l, key=lambda(a): a[0]):
    for j in range(i[0], i[1] + 1):
        if k != 0:
            print ","
        print "    0x%04x" % j,
        k += 1

print "};"
print '''
static FcBlanks fcBlanks = {
    %s,
    -1,
    _fcBlanks
};
''' % (ncode + 1)
