
ifdef x11thislocaledir

$(x11thislocaledir)\%: %.pre
	cl /nologo /EP $< -DXCOMM\#\# > $@

$(x11thislocaledir):
	$(CREATEDIR)

all: $(x11thislocaledir)
endif

$(eval $(locales:%=$(X11_LOCALEDATADIR)\%\XLC_LOCALE : %\XLC_LOCALE.pre$n	cl /nologo /EP $$< -DXCOMM\#\# > $$@$n))

$(eval $(locales:%=$(X11_LOCALEDATADIR)\%\Compose : %\Compose.pre$n	cl /nologo /EP $$< -DXCOMM\#\# > $$@$n))

$(eval $(locales:%=$(X11_LOCALEDATADIR)\%\XI18N_OBJS : $(X11_LOCALEDATADIR)\% %\XI18N_OBJS$n	copy %\XI18N_OBJS $$@$n))

$(eval $(locales:%=$(X11_LOCALEDATADIR)\% :$n	mkdir $$@$n ))
