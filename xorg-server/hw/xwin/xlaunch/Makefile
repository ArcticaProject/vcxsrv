#
# Copyright (c) 2005 Alexander Gottwald
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
#
# Except as contained in this notice, the name(s) of the above copyright
# holders shall not be used in advertising or otherwise to promote the sale,
# use or other dealings in this Software without prior written authorization.
#
WINDRES=windres

TARGET=mingw
#DEBUG_FLAGS=-D_DEBUG

OS_FLAGS_mingw=-mno-cygwin
OS_FLAGS=$(OS_FLAGS_$(TARGET)) $(DEBUG_FLAGS)

X11_DIR_$(TARGET)=/usr/X11R6
X11_DIR_mingw=../../../../../exports
X11_DIR=$(X11_DIR_$(TARGET))
X11_INCLUDE=-I$(X11_DIR)/include
X11_LIBDIR=-L$(X11_DIR)/lib
X11_LIBS_$(TARGET)=-lX11
X11_LIBS_mingw=-lX11 -lwsock32
X11_LIBS=$(X11_LIBS_$(TARGET))

PROGRAMFILES:=$(shell cygpath -u $(PROGRAMFILES))
#MSXML_DIR=$(PROGRAMFILES)/MSXML 4.0
MSXML_DIR=$(PROGRAMFILES)/Microsoft XML Parser SDK
MSXML_INCLUDE="-I$(MSXML_DIR)/inc"
MSXML_LIBDIR="-L$(MSXML_DIR)/lib"
MSXML_LIBS=


CXXFLAGS=-g $(OS_FLAGS) $(X11_INCLUDE) $(MSXML_INCLUDE)
LDFLAGS=-mwindows $(X11_LIBDIR) $(MSXML_LIBDIR)
LIBS=-lcomctl32 -lole32 -loleaut32 $(X11_LIBS) $(MSXML_LIBS)
all:xlaunch.exe
%.res: %.rc
	$(WINDRES) -O coff -o $@ $<

WINDOW_PARTS=window util dialog wizard
WINDOW_OBJECTS=$(foreach file,$(WINDOW_PARTS),window/$(file).o) 

RESOURCES_IMAGES=resources/multiwindow.bmp resources/fullscreen.bmp \
	resources/windowed.bmp resources/nodecoration.bmp

resources/resources.res: resources/resources.rc resources/resources.h \
	resources/images.rc resources/dialog.rc resources/strings.rc \
	$(RESOURCES_IMAGES)
xlaunch.exe: $(WINDOW_OBJECTS) main.o config.o resources/resources.res
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)


window/dialog.o: window/dialog.cc window/dialog.h window/window.h window/util.h
window/frame.o: window/frame.cc window/frame.h window/window.h
window/util.o: window/util.cc window/util.h
window/window.o: window/window.cc window/window.h window/util.h
window/wizard.o: window/wizard.cc window/wizard.h window/dialog.h \
  window/window.h window/util.h
main.o: main.cc window/util.h window/wizard.h window/dialog.h \
  window/window.h resources/resources.h config.h
config.o: config.cc config.h
