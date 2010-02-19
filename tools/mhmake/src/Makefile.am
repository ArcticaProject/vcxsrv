SRCS = mhmakeparser.y mhmakelexer.l mhmake.cpp mhmakefileparser.cpp util.cpp \
       functions.cpp fileinfo.cpp rule.cpp md5.c build.cpp curdir.cpp \
       commandqueue.cpp

if DEBUG
bin_PROGRAMS=mhmake_dbg
mhmake_dbg_SOURCES = $(SRCS)
else
bin_PROGRAMS=mhmake
mhmake_SOURCES = $(SRCS)
endif

LEX=flex++
AM_LFLAGS=-8

YACC=bison++
AM_YFLAGS=-d

mhmakelexer.o: mhmakelexer.cpp mhmakelexer.h
mhmakelexer.cpp: mhmakelexer.l
mhmakelexer.h: mhmakelexer.l

mhmakeparser.o: mhmakeparser.cpp mhmakeparser.h
mhmakeparser.cpp: mhmakeparser.y
mhmakeparser.h: mhmakeparser.y

.l.cpp:
	$(LEXCOMPILE) -S$(srcdir)/flexskel.cc -H$(srcdir)/flexskel.h -h$(@:%.cpp=%.h) -otemp1234.456 $<
	echo '#include "stdafx.h"' > $@
	cat temp1234.456 >> $@
	rm temp1234.456

.y.cpp:
	$(YACCCOMPILE) -S$(srcdir)/bison.cc -H$(srcdir)/bison.h -h$(@:%.cpp=%.h) -otemp1234.456 $<
	echo '#include "stdafx.h"' > $@
	cat temp1234.456 >> $@
	rm temp1234.456


.l.h:
	$(LEXCOMPILE) -S$(srcdir)/flexskel.cc -H$(srcdir)/flexskel.h -h$@ -otemp1234.456 $<
	echo '#include "stdafx.h"' > $(@:%.h=%.cpp)
	cat temp1234.456 >> $(@:%.h=%.cpp)
	rm temp1234.456

.y.h:
	$(YACCCOMPILE) -S$(srcdir)/bison.cc -H$(srcdir)/bison.h -h$@ -otemp1234.456 $<
	echo '#include "stdafx.h"' > $(@:%.h=%.cpp)
	cat temp1234.456 >> $(@:%.h=%.cpp)
	rm temp1234.456

# set the include path found by configure
INCLUDES= $(all_includes)

LDADD = /usr/lib/libpopt.a

# the library search path.
if DEBUG
mhmake_dbg_LDFLAGS = $(all_libraries)
else
mhmake_LDFLAGS = $(all_libraries)
endif
