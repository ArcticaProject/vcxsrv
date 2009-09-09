AM_CFLAGS = @DIX_CFLAGS@

noinst_LTLIBRARIES = libconfig.la
libconfig_la_SOURCES = config.c config-backends.h

if CONFIG_NEED_DBUS
AM_CFLAGS += @DBUS_CFLAGS@
libconfig_la_SOURCES += dbus-core.c
endif

if CONFIG_DBUS_API
dbusconfigdir = $(sysconfdir)/dbus-1/system.d
dbusconfig_DATA = xorg-server.conf

libconfig_la_SOURCES += dbus.c
endif

if CONFIG_HAL
libconfig_la_SOURCES += hal.c
endif

EXTRA_DIST = xorg-server.conf x11-input.fdi
