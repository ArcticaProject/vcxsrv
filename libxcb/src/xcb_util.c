/* Copyright (C) 2001-2004 Bart Massey and Jamey Sharp.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * Except as contained in this notice, the names of the authors or their
 * institutions shall not be used in advertising or otherwise to promote the
 * sale, use or other dealings in this Software without prior written
 * authorization from the authors.
 */

/* Utility functions implementable using only public APIs. */

#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#ifdef DNETCONN
#include <netdnet/dnetdb.h>
#include <netdnet/dn.h>
#endif
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "xcb.h"
#include "xcbext.h"
#include "xcbint.h"

static const int error_connection = 1;

int xcb_popcount(uint32_t mask)
{
    uint32_t y;
    y = (mask >> 1) & 033333333333;
    y = mask - y - ((y >> 1) & 033333333333);
    return ((y + (y >> 3)) & 030707070707) % 077;
}

static int _xcb_parse_display(const char *name, char **host, char **protocol,
                      int *displayp, int *screenp)
{
    int len, display, screen;
    char *slash, *colon, *dot, *end;
    if(!name || !*name)
        name = getenv("DISPLAY");
    if(!name)
        return 0;
    slash = strrchr(name, '/');
    if (slash) {
        len = slash - name;
        if (protocol) {
            *protocol = malloc(len + 1);
            if(!*protocol)
                return 0;
            memcpy(*protocol, name, len);
            (*protocol)[len] = '\0';
        }
        name = slash + 1;
    } else
        if (protocol)
            *protocol = NULL;

    colon = strrchr(name, ':');
    if(!colon)
        return 0;
    len = colon - name;
    ++colon;
    display = strtoul(colon, &dot, 10);
    if(dot == colon)
        return 0;
    if(*dot == '\0')
        screen = 0;
    else
    {
        if(*dot != '.')
            return 0;
        ++dot;
        screen = strtoul(dot, &end, 10);
        if(end == dot || *end != '\0')
            return 0;
    }
    /* At this point, the display string is fully parsed and valid, but
     * the caller's memory is untouched. */

    *host = malloc(len + 1);
    if(!*host)
        return 0;
    memcpy(*host, name, len);
    (*host)[len] = '\0';
    *displayp = display;
    if(screenp)
        *screenp = screen;
    return 1;
}

int xcb_parse_display(const char *name, char **host, int *displayp,
                             int *screenp)
{
    return _xcb_parse_display(name, host, NULL, displayp, screenp);
}

static int _xcb_open_tcp(char *host, char *protocol, const unsigned short port);
static int _xcb_open_unix(char *protocol, const char *file);
#ifdef DNETCONN
static int _xcb_open_decnet(const char *host, char *protocol, const unsigned short port);
#endif

static int _xcb_open(char *host, char *protocol, const int display)
{
    int fd;
    static const char base[] = "/tmp/.X11-unix/X";
    char file[sizeof(base) + 20];

    if(*host)
    {
#ifdef DNETCONN
        /* DECnet displays have two colons, so _xcb_parse_display will have
           left one at the end.  However, an IPv6 address can end with *two*
           colons, so only treat this as a DECnet display if host ends with
           exactly one colon. */
        char *colon = strchr(host, ':');
        if(colon && *(colon+1) == '\0')
        {
            *colon = '\0';
            return _xcb_open_decnet(host, protocol, display);
        }
        else
#endif
            if (protocol
                || strcmp("unix",host)) { /* follow the old unix: rule */

                /* display specifies TCP */
                unsigned short port = X_TCP_PORT + display;
                return _xcb_open_tcp(host, protocol, port);
            }
    }

    /* display specifies Unix socket */
    snprintf(file, sizeof(file), "%s%d", base, display);
    return  _xcb_open_unix(protocol, file);


    return fd;
}

#ifdef DNETCONN
static int _xcb_open_decnet(const char *host, const char *protocol, const unsigned short port)
{
    int fd;
    struct sockaddr_dn addr;
    struct accessdata_dn accessdata;
    struct nodeent *nodeaddr = getnodebyname(host);

    if(!nodeaddr)
        return -1;
    if (protocol && strcmp("dnet",protocol))
        return -1;
    addr.sdn_family = AF_DECnet;

    addr.sdn_add.a_len = nodeaddr->n_length;
    memcpy(addr.sdn_add.a_addr, nodeaddr->n_addr, addr.sdn_add.a_len);

    sprintf((char *)addr.sdn_objname, "X$X%d", port);
    addr.sdn_objnamel = strlen((char *)addr.sdn_objname);
    addr.sdn_objnum = 0;

    fd = socket(PF_DECnet, SOCK_STREAM, 0);
    if(fd == -1)
        return -1;

    memset(&accessdata, 0, sizeof(accessdata));
    sprintf((char*)accessdata.acc_acc, "%d", getuid());
    accessdata.acc_accl = strlen((char *)accessdata.acc_acc);
    setsockopt(fd, DNPROTO_NSP, SO_CONACCESS, &accessdata, sizeof(accessdata));

    if(connect(fd, (struct sockaddr *) &addr, sizeof(addr)) == -1)
        return -1;
    return fd;
}
#endif

static int _xcb_open_tcp(char *host, char *protocol, const unsigned short port)
{
    int fd = -1;
    struct addrinfo hints = { 0
#ifdef AI_ADDRCONFIG
                              | AI_ADDRCONFIG
#endif
#ifdef AI_NUMERICSERV
                              | AI_NUMERICSERV
#endif
                              , AF_UNSPEC, SOCK_STREAM };
    char service[6]; /* "65535" with the trailing '\0' */
    struct addrinfo *results, *addr;
    char *bracket;

    if (protocol && strcmp("tcp",protocol))
        return -1;

    /* Allow IPv6 addresses enclosed in brackets. */
    if(host[0] == '[' && (bracket = strrchr(host, ']')) && bracket[1] == '\0')
    {
        *bracket = '\0';
        ++host;
        hints.ai_flags |= AI_NUMERICHOST;
        hints.ai_family = AF_INET6;
    }

    snprintf(service, sizeof(service), "%hu", port);
    if(getaddrinfo(host, service, &hints, &results))
        /* FIXME: use gai_strerror, and fill in error connection */
        return -1;

    for(addr = results; addr; addr = addr->ai_next)
    {
        fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if(fd >= 0 && connect(fd, addr->ai_addr, addr->ai_addrlen) >= 0)
            break;
        fd = -1;
    }
    freeaddrinfo(results);
    return fd;
}

static int _xcb_open_unix(char *protocol, const char *file)
{
    int fd;
    struct sockaddr_un addr = { AF_UNIX };

    if (protocol && strcmp("unix",protocol))
        return -1;

    strcpy(addr.sun_path, file);

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(fd == -1)
        return -1;
    if(connect(fd, (struct sockaddr *) &addr, sizeof(addr)) == -1)
        return -1;
    return fd;
}

xcb_connection_t *xcb_connect(const char *displayname, int *screenp)
{
    int fd, display = 0;
    char *host;
    char *protocol;
    xcb_connection_t *c;
    xcb_auth_info_t auth;

    if(!_xcb_parse_display(displayname, &host, &protocol, &display, screenp))
        return (xcb_connection_t *) &error_connection;
    fd = _xcb_open(host, protocol, display);
    free(host);
    if(fd == -1)
        return (xcb_connection_t *) &error_connection;

    if(_xcb_get_auth_info(fd, &auth, display))
    {
        c = xcb_connect_to_fd(fd, &auth);
        free(auth.name);
        free(auth.data);
    }
    else
        c = xcb_connect_to_fd(fd, 0);
    return c;
}

xcb_connection_t *xcb_connect_to_display_with_auth_info(const char *displayname, xcb_auth_info_t *auth, int *screenp)
{
    int fd, display = 0;
    char *host;
    char *protocol;

    if(!_xcb_parse_display(displayname, &host, &protocol, &display, screenp))
        return (xcb_connection_t *) &error_connection;
    fd = _xcb_open(host, protocol, display);
    free(host);
    if(fd == -1)
        return (xcb_connection_t *) &error_connection;

    return xcb_connect_to_fd(fd, auth);
}
