TTYAPP = bison++

CSRCS = closure.cc \
			derives.cc \
			getargs.cc \
			getopt1.cc \
			lalr.cc  \
			lr0.cc \
			nullable.cc \
			print.cc \
			reduce.cc \
			version.cc \
			warshall.cc \
			allocate.cc \
			conflict.cc \
			files.cc \
			getopt.cc \
			gram.cc \
			lex.cc \
			main.cc \
			output.cc \
			reader.cc \
			symtab.cc\
			old.c 

PFILE = bison.cc
PFILE1 = bison.hairy
HFILE = bison.h

DEFINES += HAVE_STDLIB_H HAVE_STRING_H XPFILE="""$(PFILE)""" XHFILE="""$(HFILE)""" XPFILE1="""$(PFILE1)"""

CCFLAGS := $(subst -MD,-MT,$(CCFLAGS))  # to have static linking

