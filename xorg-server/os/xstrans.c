#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#define TRANS_REOPEN
#define TRANS_SERVER
#define XSERV_t
#ifndef TCPCONN
#define TCPCONN
#endif

#include <X11/Xtrans/transport.c>
