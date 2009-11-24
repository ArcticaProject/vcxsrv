MHMAKESLNFILE=mhmake.sln

.PHONY: all mhmake_dbg mhmake cleanthis

all: debug\mhmake_dbg.exe release\mhmake.exe

DEPS=$(wildcard src\*)

debug\mhmake_dbg.exe: $(DEPS)
	vcbuild $(MHMAKESLNFILE) "Debug|Win32" 

release\mhmake.exe: $(DEPS)
	vcbuild $(MHMAKESLNFILE) "Release|Win32" 

clean: cleanthis

cleanthis:
	del -e Debug
	del -e Release
