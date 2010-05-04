if UNITTESTS
check_PROGRAMS =  \
	protocol-xiqueryversion \
	protocol-xiquerydevice \
	protocol-xiselectevents \
	protocol-xigetselectedevents \
        protocol-xisetclientpointer \
        protocol-xigetclientpointer \
        protocol-xiquerypointer \
        protocol-xiwarppointer \
        protocol-eventconvert

TESTS=$(check_PROGRAMS)

AM_CFLAGS = $(DIX_CFLAGS) $(GLIB_CFLAGS) @XORG_CFLAGS@
INCLUDES = @XORG_INCS@
TEST_LDADD=../libxservertest.la $(XORG_SYS_LIBS) $(XSERVER_SYS_LIBS) $(GLIB_LIBS)
COMMON_SOURCES=protocol-common.h protocol-common.c

if SPECIAL_DTRACE_OBJECTS
TEST_LDADD += $(OS_LIB) $(DIX_LIB)
endif

protocol_xiqueryversion_LDADD=$(TEST_LDADD)
protocol_xiquerydevice_LDADD=$(TEST_LDADD)
protocol_xiselectevents_LDADD=$(TEST_LDADD)
protocol_xigetselectedevents_LDADD=$(TEST_LDADD)
protocol_xisetclientpointer_LDADD=$(TEST_LDADD)
protocol_xigetclientpointer_LDADD=$(TEST_LDADD)
protocol_xiquerypointer_LDADD=$(TEST_LDADD)
protocol_xiwarppointer_LDADD=$(TEST_LDADD)
protocol_eventconvert_LDADD=$(TEST_LDADD)

protocol_xiqueryversion_LDFLAGS=$(AM_LDFLAGS) -Wl,-wrap,WriteToClient
protocol_xiquerydevice_LDFLAGS=$(AM_LDFLAGS) -Wl,-wrap,WriteToClient
protocol_xiselectevents_LDFLAGS=$(AM_LDFLAGS) -Wl,-wrap,dixLookupWindow -Wl,-wrap,XISetEventMask
protocol_xigetselectedevents_LDFLAGS=$(AM_LDFLAGS) -Wl,-wrap,WriteToClient -Wl,-wrap,dixLookupWindow -Wl,-wrap,AddResource
protocol_xisetclientpointer_LDFLAGS=$(AM_LDFLAGS) -Wl,-wrap,dixLookupClient
protocol_xigetclientpointer_LDFLAGS=$(AM_LDFLAGS) -Wl,-wrap,WriteToClient -Wl,-wrap,dixLookupClient
protocol_xiquerypointer_LDFLAGS=$(AM_LDFLAGS) -Wl,-wrap,WriteToClient -Wl,-wrap,dixLookupWindow
protocol_xiwarppointer_LDFLAGS=$(AM_LDFLAGS) -Wl,-wrap,WriteToClient -Wl,-wrap,dixLookupWindow

protocol_xiqueryversion_SOURCES=$(COMMON_SOURCES) protocol-xiqueryversion.c
protocol_xiquerydevice_SOURCES=$(COMMON_SOURCES) protocol-xiquerydevice.c
protocol_xiselectevents_SOURCES=$(COMMON_SOURCES) protocol-xiselectevents.c
protocol_xigetselectedevents_SOURCES=$(COMMON_SOURCES) protocol-xigetselectedevents.c
protocol_xisetclientpointer_SOURCES=$(COMMON_SOURCES) protocol-xisetclientpointer.c
protocol_xigetclientpointer_SOURCES=$(COMMON_SOURCES) protocol-xigetclientpointer.c
protocol_xiquerypointer_SOURCES=$(COMMON_SOURCES) protocol-xiquerypointer.c
protocol_xiwarppointer_SOURCES=$(COMMON_SOURCES) protocol-xiwarppointer.c
endif
