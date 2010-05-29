ifdef VS2008
MHMAKESLNFILE=mhmake.sln
BUILDCMD=vcbuild $(MHMAKESLNFILE)
else
MHMAKESLNFILE=mhmakevc10.sln
BUILDCMD=devenv.com $(MHMAKESLNFILE) /build
endif

.PHONY: all mhmake_dbg mhmake cleanthis

all: debug\mhmake_dbg.exe release\mhmake.exe

DEPS=$(wildcard src\*)

debug\mhmake_dbg.exe: $(DEPS)
	$(BUILDCMD) "Debug|Win32" 

release\mhmake.exe: $(DEPS)
	$(BUILDCMD) "Release|Win32" 

clean: cleanthis

cleanthis:
	del -e Debug
	del -e Release
