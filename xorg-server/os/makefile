ifeq ($(DEBUG),1)
DEFINES += XSERVER_DTRACE
endif

SECURERPC_SRCS = rpcauth.c
XDMCP_SRCS = xdmcp.c
STRLCAT_SRCS = strlcat.c strlcpy.c
XORG_SRCS = log.c

CSRCS = 	\
	WaitFor.c	\
	access.c	\
	auth.c		\
	backtrace.c	\
	connection.c	\
	io.c		\
	mitauth.c	\
	oscolor.c	\
	osinit.c	\
	utils.c		\
	strcasecmp.c	\
	strcasestr.c	\
	xdmauth.c	\
	xsha1.c		\
	xstrans.c	\
	xprintf.c	\
	$(XORG_SRCS)

CSRCS += $(SECURERPC_SRCS)

CSRCS += $(XDMCP_SRCS)

CSRCS += $(STRLCAT_SRCS)

LIBRARY=libos

