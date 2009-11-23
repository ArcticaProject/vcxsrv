SLNFILE=mesa.sln

all: Release\swrast_dri.dll Debug\swrast_dri.dll

ifneq $(which devenv.com),
VCSTUDIO_DBG=devenv.com $(SLNFILE) /build Debug
VCSTUDIO_REL=devenv.com $(SLNFILE) /build Release
endif
ifneq $(which vcbuild.exe),
VCSTUDIO_DBG=vcbuild $(SLNFILE) "Debug|Win32"
VCSTUDIO_REL=vcbuild $(SLNFILE) "Release|Win32"
endif

ifndef VCSTUDIO_DBG
$(error Environment not set correctly for building Visual Studio projects)
endif

Debug\swrast_dri.dll:
	$(VCSTUDIO_DBG) 

Release\swrast_dri.dll:
	$(VCSTUDIO_REL) 

clean: cleanthis

cleanthis:
	del -e Debug
	del -e Release
