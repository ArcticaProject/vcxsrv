ifdef VS2010
MHMAKESLNFILE=mesavc10.sln
BUILDCMD=devenv.com $(MHMAKESLNFILE) /build
else
MHMAKESLNFILE=mesa.sln
BUILDCMD=vcbuild $(MHMAKESLNFILE)
endif
SLNFILE=

all: Release\swrast_dri.dll Debug\swrast_dri.dll

Debug\swrast_dri.dll:
	$(BUILDCMD) "Debug|Win32" 

Release\swrast_dri.dll:
	$(BUILDCMD) "Release|Win32" 

clean: cleanthis

cleanthis:
	del -e Debug
	del -e Release
