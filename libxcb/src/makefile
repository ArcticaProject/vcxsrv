
LIBRARY=libxcb

CSRCS = \
		xcb_conn.c xcb_out.c xcb_in.c xcb_ext.c xcb_xid.c \
		xcb_list.c xcb_util.c xcb_auth.c

DEFINES += PTW32_STATIC_LIB

XCBPROTO_XCBINCLUDEDIR = ..\xcb-proto\src

XMLFILES := $(notdir $(wildcard $(XCBPROTO_XCBINCLUDEDIR)\*.xml))

EXTSOURCES := $(XMLFILES:%.xml=%.c)
CSRCS += $(EXTSOURCES)

EXTHEADERS = $(XMLFILES:%.xml=%.h)

$(EXTHEADERS) $(EXTSOURCES): c_client.py

load_makefile NORELDBG=1 makefile.srcs
