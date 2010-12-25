CFLAGS += -Wall -pedantic -fPIC
OPT_CFLAGS = -O3
LDFLAGS += -shared -fPIC
OBJS = txc_compress_dxtn.o txc_fetch_dxtn.o
LIB = libtxc_dxtn.so

$(LIB): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

%.o: %.c txc_dxtn.h
	$(CC) $(CFLAGS) $(OPT_CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(LIB)

install: $(LIB)
	install -d $(DESTDIR)/usr/lib
	install -m 755 $(LIB) $(DESTDIR)/usr/lib
