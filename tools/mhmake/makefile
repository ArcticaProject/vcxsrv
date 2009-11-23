MHMAKESLNFILE=mhmake.sln

.PHONY: all mhmake_dbg mhmake cleanthis

all: debug\mhmake_dbg.exe release\mhmake.exe

DEPS=$(wildcard src\*)

ifneq $(which devenv.com),
VCSTUDIO=devenv.com
endif
ifneq $(which vcexpress.exe),
VCSTUDIO=vcexpress
endif

ifndef VCSTUDIO
$(error Environment not set correctly for building Visual Studio projects)
endif

debug\mhmake_dbg.exe: $(DEPS)
	$(VCSTUDIO) $(MHMAKESLNFILE) /build Debug

release\mhmake.exe: $(DEPS)
	$(VCSTUDIO) $(MHMAKESLNFILE) /build Release

clean: cleanthis

cleanthis:
	del -e Debug
	del -e Release
