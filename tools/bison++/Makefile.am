bin_PROGRAMS = bison++
AUTOMAKE_OPTIONS = foreign 

bison___SOURCES = closure.cc \
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

info_TEXINFOS = bison.texinfo
man_MANS = bison++.1 bison.1 bison++.yacc.1
pkgdata_DATA = bison.cc bison.hairy bison.h Example
CXX=g++
PFILE = bison.cc
PFILE1 = bison.hairy
HFILE = bison.h
INCLUDES = -DXPFILE=\"$(datadir)/bison++/$(PFILE)\" \
	 -DXHFILE=\"$(datadir)/bison++/$(HFILE)\" \
	 -DXPFILE1=\"$(datadir)/bison++/$(PFILE1)\"

install-exec-hook:
	cp bison $(bindir)
	cp bison++.yacc $(bindir)

uninstall-hook:
	rm $(bindir)/bison++.yacc
	rm $(bindir)/bison





