MHMAKESLNFILE=mhmake.sln

.PHONY: all mhmake_dbg mhmake cleanthis

all: debug\mhmake_dbg.exe release\mhmake.exe

DEPS=$(wildcard src\*)

VCSTUDIO:=vcexpress # choose devenv.com  if you have the commercial edition

debug\mhmake_dbg.exe: $(DEPS)
	$(VCSTUDIO) $(MHMAKESLNFILE) /build Debug

release\mhmake.exe: $(DEPS)
	$(VCSTUDIO) $(MHMAKESLNFILE) /build Release

clean: cleanthis

cleanthis:
	del -e Debug
	del -e Release
