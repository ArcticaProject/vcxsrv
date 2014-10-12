# Copyright 2012 Intel Corporation
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

LOCAL_PATH := $(call my-dir)

#
# The libmesa_glsl_utils libraries allow us to avoid a circular dependency
# between core mesa and glsl.
#

# ---------------------------------------
# libmesa_glsl_utils.a for target
# ---------------------------------------

include $(CLEAR_VARS)

LOCAL_MODULE := libmesa_glsl_utils

LOCAL_C_INCLUDES := \
	$(MESA_TOP)/src \
	$(MESA_TOP)/src/glsl \
	$(MESA_TOP)/src/mapi

LOCAL_SRC_FILES := \
	main/imports.c \
	program/prog_hash_table.c \
	program/symbol_table.c

include $(MESA_COMMON_MK)
include $(BUILD_STATIC_LIBRARY)

# ---------------------------------------
# libmesa_glsl_utils.a for host
# ---------------------------------------

include $(CLEAR_VARS)

LOCAL_MODULE := libmesa_glsl_utils
LOCAL_IS_HOST_MODULE := true
LOCAL_CFLAGS := -D_POSIX_C_SOURCE=199309L

LOCAL_C_INCLUDES := \
	$(MESA_TOP)/src \
	$(MESA_TOP)/src/glsl \
	$(MESA_TOP)/src/mapi

LOCAL_SRC_FILES := \
	main/imports.c \
	program/prog_hash_table.c \
	program/symbol_table.c

include $(MESA_COMMON_MK)
include $(BUILD_HOST_STATIC_LIBRARY)
