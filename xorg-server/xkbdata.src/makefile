ifeq ($(MAKESERVER),1)
$(error Please do not specify MAKESERVER=1)
endif

SUBDIRS = semantics keycodes keymap compat geometry torture symbols rules types

load_makefile $(SUBDIRS:%=%\makefile MAKESERVER=0 DEBUG=$(DEBUG);)

all: $(SUBDIRS:%=%\all)
