
.SUFFIXES : .cc .y .l $(SUFFIXES)

.cc.o :
	g++ -g  -I . -I$(CENTERCCLIBDIR)/incl -c  $*.cc

.y.cc :
	bison++ -d  -o $*.cc -h $*.h  $*.y
.l.cc :
	flex++  -o$*.cc $*.l
.y.h :
	bison++ -d  -o $*.cc -h $*.h  $*.y
.l.h :
	flex++  -o$*.cc $*.l

# COMPILER SAMPLE

MyCompiler.o : MyCompiler.cc MyParser.h MyScanner.h

MyParser.o : MyParser.cc MyParser.h

MyScanner.o : MyScanner.cc MyScanner.h MyParser.h

MyParser.cc : MyParser.y

MyScanner.cc : MyScanner.l

MyParser.h : MyParser.y

MyScanner.h : MyScanner.l

compiler : MyCompiler.o MyParser.o MyScanner.o
	g++  -o $@ MyCompiler.o MyParser.o MyScanner.o

