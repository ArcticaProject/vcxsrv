MHMAKESLNFILE=mhmake.sln

.PHONY: all mhmake_dbg mhmake cleanthis

all: debug\mhmake_dbg.exe release\mhmake.exe

DEPS=$(wildcard src\*)

ifneq $(which devenv.com),
VCSTUDIO_DBG=devenv.com $(MHMAKESLNFILE) /build Debug
VCSTUDIO_REL=devenv.com $(MHMAKESLNFILE) /build Release
endif
ifneq $(which vcbuild.exe),
VCSTUDIO_DBG=vcbuild $(MHMAKESLNFILE) "Debug|Win32"
VCSTUDIO_REL=vcbuild $(MHMAKESLNFILE) "Release|Win32"
endif

ifndef VCSTUDIO_DBG
$(error Environment not set correctly for building Visual Studio projects)
endif

debug\mhmake_dbg.exe: $(DEPS)
	$(VCSTUDIO_DBG) 

release\mhmake.exe: $(DEPS)
	$(VCSTUDIO_REL) 

clean: cleanthis

cleanthis:
	del -e Debug
	del -e Release
