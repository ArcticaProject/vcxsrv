/*
 * Copyright (c) 2001, 2003  Motoyuki Kasahara
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef DUMMYIN6_H
#define DUMMYIN6_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#ifndef AF_INET6
#define AF_INET6 (AF_INET + 1)
#endif

#ifndef PF_INET6
#define PF_INET6 (PF_INET + 1)
#endif

#ifndef AF_UNSPEC
#define AF_UNSPEC AF_INET
#endif

#ifndef PF_UNSPEC
#define PF_UNSPEC PF_INET
#endif

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN	46
#endif

#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN		16
#endif

#ifndef HAVE_STRUCT_IN6_ADDR
struct in6_addr {
    unsigned char	s6_addr[16];
};
#endif

#ifndef HAVE_STRUCT_SOCKADDR_IN6
struct sockaddr_in6 {
    sa_family_t		sin6_family;
    in_port_t		sin6_port;
    unsigned long	sin6_flowinfo;
    struct in6_addr	sin6_addr;
    unsigned long	sin6_scope_id;
};
#endif

#if !defined(HAVE_STRUCT_SOCKADDR_STORAGE) && !defined(sockaddr_storage)
#define sockaddr_storage sockaddr_in
#endif

#ifndef IN6ADDR_ANY_DECLARED
extern const struct in6_addr in6addr_any;
#endif

#ifndef IN6ADDR_LOOPBACK_DECLARED
extern const struct in6_addr in6addr_loopback;
#endif

#ifndef IN6ADDR_ANY_INIT
#define IN6ADDR_ANY_INIT \
	{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}
#endif

#ifndef IN6ADDR_LOOPBACK_INIT
#define IN6ADDR_LOOPBACK_INIT \
	{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01}}
#endif

#ifndef IN6_IS_ADDR_UNSPECIFIED
#define IN6_IS_ADDR_UNSPECIFIED(a) \
	(   (a)->s6_addr[ 0] == 0 && (a)->s6_addr[ 1] == 0 \
	 && (a)->s6_addr[ 2] == 0 && (a)->s6_addr[ 3] == 0 \
	 && (a)->s6_addr[ 4] == 0 && (a)->s6_addr[ 5] == 0 \
	 && (a)->s6_addr[ 6] == 0 && (a)->s6_addr[ 7] == 0 \
	 && (a)->s6_addr[ 8] == 0 && (a)->s6_addr[ 9] == 0 \
	 && (a)->s6_addr[10] == 0 && (a)->s6_addr[11] == 0 \
	 && (a)->s6_addr[12] == 0 && (a)->s6_addr[13] == 0 \
	 && (a)->s6_addr[14] == 0 && (a)->s6_addr[15] == 0)
#endif

#ifndef IN6_IS_ADDR_LOOPBACK
#define IN6_IS_ADDR_LOOPBACK(a) \
	(   (a)->s6_addr[ 0] == 0 && (a)->s6_addr[ 1] == 0 \
	 && (a)->s6_addr[ 2] == 0 && (a)->s6_addr[ 3] == 0 \
	 && (a)->s6_addr[ 4] == 0 && (a)->s6_addr[ 5] == 0 \
	 && (a)->s6_addr[ 6] == 0 && (a)->s6_addr[ 7] == 0 \
	 && (a)->s6_addr[ 8] == 0 && (a)->s6_addr[ 9] == 0 \
	 && (a)->s6_addr[10] == 0 && (a)->s6_addr[11] == 0 \
	 && (a)->s6_addr[12] == 0 && (a)->s6_addr[13] == 0 \
	 && (a)->s6_addr[14] == 0 && (a)->s6_addr[15] == 1)
#endif

#ifndef IN6_IS_ADDR_MULTICAST
#define IN6_IS_ADDR_MULTICAST(a) \
	((a)->s6_addr[0] == 0xff)
#endif

#ifndef IN6_IS_ADDR_LINKLOCAL
#define IN6_IS_ADDR_LINKLOCAL(a) \
	(((a)->s6_addr[0] == 0xfe) && (((a)->s6_addr[1] & 0xc0) == 0x80))
#endif

#ifndef IN6_IS_ADDR_SITELOCAL
#define IN6_IS_ADDR_SITELOCAL(a) \
	(((a)->s6_addr[0] == 0xfe) && (((a)->s6_addr[1] & 0xc0) == 0xc0))
#endif

#ifndef IN6_IS_ADDR_V4MAPPED
#define IN6_IS_ADDR_V4MAPPED(a) \
	(   (a)->s6_addr[ 0] == 0    && (a)->s6_addr[ 1] == 0 \
	 && (a)->s6_addr[ 2] == 0    && (a)->s6_addr[ 3] == 0 \
	 && (a)->s6_addr[ 4] == 0    && (a)->s6_addr[ 5] == 0 \
	 && (a)->s6_addr[ 6] == 0    && (a)->s6_addr[ 7] == 0 \
	 && (a)->s6_addr[ 8] == 0    && (a)->s6_addr[ 9] == 0 \
	 && (a)->s6_addr[10] == 0xff && (a)->s6_addr[11] == 0xff)
#endif

#ifndef IN6_IS_ADDR_V4COMPAT
#define IN6_IS_ADDR_V4COMPAT(a) \
	(   (a)->s6_addr[ 0] == 0 && (a)->s6_addr[ 1] == 0 \
	 && (a)->s6_addr[ 2] == 0 && (a)->s6_addr[ 3] == 0 \
	 && (a)->s6_addr[ 4] == 0 && (a)->s6_addr[ 5] == 0 \
	 && (a)->s6_addr[ 6] == 0 && (a)->s6_addr[ 7] == 0 \
	 && (a)->s6_addr[ 8] == 0 && (a)->s6_addr[ 9] == 0 \
	 && (a)->s6_addr[10] == 0 && (a)->s6_addr[11] == 0 \
	 && ((a)->s6_addr[12] != 0 || (a)->s6_addr[13] != 0 \
	     || (a)->s6_addr[14] != 0 \
	     || ((a)->s6_addr[15] != 0 && (a)->s6_addr[15] != 1)))
#endif

#endif /* not DUMMYIN6_H */
