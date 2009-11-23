SLNFILE=mesa.sln

all: Release\swrast_dri.dll Debug\swrast_dri.dll

ifneq $(which devenv.com),
VCSTUDIO=devenv.com
endif
ifneq $(which vcexpress.exe),
VCSTUDIO=vcexpress
endif

ifndef VCSTUDIO
$(error Environment not set correctly for building Visual Studio projects)
endif

Debug\swrast_dri.dll:
	$(VCSTUDIO) $(SLNFILE) /build Debug

Release\swrast_dri.dll:
	$(VCSTUDIO) $(SLNFILE) /build Release

clean: cleanthis

cleanthis:
	del -e Debug
	del -e Release
