# $XdotOrg: lib/X11/src/util/Makefile.am,v 1.4 2006-02-19 02:14:12 jamey Exp $

noinst_PROGRAMS=makekeys

makekeys_CFLAGS=$(X11_CFLAGS)

CC = @CC_FOR_BUILD@

EXTRA_DIST = mkks.sh

if LINT
# Check source code with tools like lint & sparse

ALL_LINT_FLAGS=$(LINT_FLAGS) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
		$(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS)

lint:
	$(LINT) $(ALL_LINT_FLAGS) makekeys.c

endif LINT
