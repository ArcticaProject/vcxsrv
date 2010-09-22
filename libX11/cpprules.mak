
ifdef x11thislocaledir

$(x11thislocaledir)\%: %.pre
	cl /nologo /EP $< -DXCOMM\#\# > $@

$(x11thislocaledir):
	$(CREATEDIR)

all: $(x11thislocaledir)
endif

$(reparse $(locales:%=$(X11_LOCALEDATADIR)\%\XLC_LOCALE : %\XLC_LOCALE.pre;	cl /nologo /EP $$< -DXCOMM\#\# > $$@;; ))

$(reparse $(locales:%=$(X11_LOCALEDATADIR)\%\Compose : %\Compose.pre;	cl /nologo /EP $$< -DXCOMM\#\# > $$@;; ))

$(reparse $(locales:%=$(X11_LOCALEDATADIR)\%\XI18N_OBJS : %\XI18N_OBJS;	copy $$< $$@;; ))

