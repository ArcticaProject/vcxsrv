SLNFILE=mesa.sln

all: Release\swrast_dri.dll Debug\swrast_dri.dll

Debug\swrast_dri.dll:
	vcbuild $(SLNFILE) "Debug|Win32" 

Release\swrast_dri.dll:
	vcbuild $(SLNFILE) "Release|Win32" 

clean: cleanthis

cleanthis:
	del -e Debug
	del -e Release
