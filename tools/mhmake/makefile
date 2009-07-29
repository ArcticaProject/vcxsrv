ifneq ($(BASEAUTOMAK),makefile)
ifneq ($(BASEAUTOMAK),linux)
$(error Please specify BASEAUTOMAK=makefile or BASEAUTOMAK=linux on the command-line or in the environment)
endif
endif
ifneq ($(CONFIG),LINESIM)
$(error Please specify CONFIG=LINESIM on the command-line or in the environment)
endif
NOAUTOINCLIBS:=1

ifdef VS2008
MHMAKESLNFILE=mhmake.sln
else
MHMAKESLNFILE=mhmakevc6.sln
endif

.PHONY: all mhmake_dbg mhmake cleanthis

all: debug\mhmake_dbg.exe release\mhmake.exe

DEPS=$(wildcard src\*)

debug\mhmake_dbg.exe: $(DEPS)
	devenv.com $(MHMAKESLNFILE) /build Debug

release\mhmake.exe: $(DEPS)
	devenv.com $(MHMAKESLNFILE) /build Release

clean: cleanthis

cleanthis:
	del -e Debug
	del -e Release
