# Mesa 3-D graphics library
#
# Copyright (C) 2010-2011 Chia-I Wu <olvaffe@gmail.com>
# Copyright (C) 2010-2011 LunarG Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

# use c99 compiler by default
ifeq ($(LOCAL_CC),)
ifeq ($(LOCAL_IS_HOST_MODULE),true)
LOCAL_CC := $(HOST_CC) -std=c99
else
LOCAL_CC := $(TARGET_CC) -std=c99
endif
endif

LOCAL_C_INCLUDES += \
	$(MESA_TOP)/include

MESA_VERSION=$(shell cat $(MESA_TOP)/VERSION)
# define ANDROID_VERSION (e.g., 4.0.x => 0x0400)
LOCAL_CFLAGS += \
	-DPACKAGE_VERSION=\"$(MESA_VERSION)\" \
	-DPACKAGE_BUGREPORT=\"https://bugs.freedesktop.org/enter_bug.cgi?product=Mesa\" \
	-DANDROID_VERSION=0x0$(MESA_ANDROID_MAJOR_VERSION)0$(MESA_ANDROID_MINOR_VERSION)

LOCAL_CFLAGS += \
	-DHAVE_PTHREAD=1 \
	-fvisibility=hidden \
	-Wno-sign-compare

ifeq ($(strip $(MESA_ENABLE_ASM)),true)
ifeq ($(TARGET_ARCH),x86)
LOCAL_CFLAGS += \
	-DUSE_X86_ASM \
	-DHAVE_DLOPEN \

endif
endif

LOCAL_CPPFLAGS += \
	-Wno-error=non-virtual-dtor \
	-Wno-non-virtual-dtor

# uncomment to keep the debug symbols
#LOCAL_STRIP_MODULE := false

ifeq ($(strip $(LOCAL_MODULE_TAGS)),)
LOCAL_MODULE_TAGS := optional
endif
