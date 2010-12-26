dnl
dnl Copyright (c) 2005, Oracle and/or its affiliates. All rights reserved.
dnl
dnl Permission is hereby granted, free of charge, to any person obtaining a
dnl copy of this software and associated documentation files (the "Software"),
dnl to deal in the Software without restriction, including without limitation
dnl the rights to use, copy, modify, merge, publish, distribute, sublicense,
dnl and/or sell copies of the Software, and to permit persons to whom the
dnl Software is furnished to do so, subject to the following conditions:
dnl
dnl The above copyright notice and this permission notice (including the next
dnl paragraph) shall be included in all copies or substantial portions of the
dnl Software.
dnl
dnl THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
dnl IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
dnl FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
dnl THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
dnl LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
dnl FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
dnl DEALINGS IN THE SOFTWARE.
dnl

# XTRANS_TCP_FLAGS()
# ------------------
# Find needed libraries for TCP sockets, and check for IPv6 support
AC_DEFUN([XTRANS_TCP_FLAGS],[
 # SVR4 hides these in libraries other than libc
 AC_SEARCH_LIBS(socket, [socket])
 AC_SEARCH_LIBS(gethostbyname, [nsl])
 if test "$ac_cv_search_socket$ac_cv_search_gethostbyname" = "nono"; then
   AC_HAVE_LIBRARY([ws2_32])
 fi

 # Needs to come after above checks for libsocket & libnsl for SVR4 systems
 AC_ARG_ENABLE(ipv6, 
	AC_HELP_STRING([--enable-ipv6],[Enable IPv6 support]),
	[IPV6CONN=$enableval], 
	[AC_CHECK_FUNC(getaddrinfo,[IPV6CONN=yes],[IPV6CONN=no])])
 AC_MSG_CHECKING([if IPv6 support should be built])
 if test "$IPV6CONN" = "yes"; then
	AC_DEFINE(IPv6,1,[Support IPv6 for TCP connections])
 fi
 AC_MSG_RESULT($IPV6CONN)

 # 4.3BSD-Reno added a new member to struct sockaddr_in
 AC_CHECK_MEMBER([struct sockaddr_in.sin_len], 
	AC_DEFINE([BSD44SOCKETS],1,
 	    [Define to 1 if `struct sockaddr_in' has a `sin_len' member]), [], [
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
 ])

 # POSIX.1g changed the type of pointer passed to getsockname/getpeername/etc.
 AC_CHECK_TYPES([socklen_t], [], [], [
AC_INCLUDES_DEFAULT
#include <sys/socket.h>])
 
]) # XTRANS_TCP_FLAGS

# XTRANS_CONNECTION_FLAGS()
# -------------------------
# Standard checks for which Xtrans transports to use by the Xorg packages
# that use Xtrans functions
AC_DEFUN([XTRANS_CONNECTION_FLAGS],[
 AC_REQUIRE([AC_CANONICAL_HOST])
 AC_REQUIRE([AC_TYPE_SIGNAL])
 [case $host_os in
	mingw*)	unixdef="no"   ;;
	*)	unixdef="yes"  ;;
 esac]
 AC_ARG_ENABLE(unix-transport,
	AC_HELP_STRING([--enable-unix-transport],[Enable UNIX domain socket transport]),
	[UNIXCONN=$enableval], [UNIXCONN=$unixdef])
 AC_MSG_CHECKING([if Xtrans should support UNIX socket connections])
 if test "$UNIXCONN" = "yes"; then
	AC_DEFINE(UNIXCONN,1,[Support UNIX socket connections])
 fi
 AC_MSG_RESULT($UNIXCONN)
 AC_ARG_ENABLE(tcp-transport, 
	AC_HELP_STRING([--enable-tcp-transport],[Enable TCP socket transport]),
	[TCPCONN=$enableval], [TCPCONN=yes])
 AC_MSG_CHECKING([if Xtrans should support TCP socket connections])
 AC_MSG_RESULT($TCPCONN)
 if test "$TCPCONN" = "yes"; then
	AC_DEFINE(TCPCONN,1,[Support TCP socket connections])
	XTRANS_TCP_FLAGS
 fi
 [case $host_os in
	solaris*|sco*|sysv4*)	localdef="yes" ;;
	*)			localdef="no"  ;;
 esac]
 AC_ARG_ENABLE(local-transport,
	AC_HELP_STRING([--enable-local-transport],[Enable os-specific local transport]),
	[LOCALCONN=$enableval], [LOCALCONN=$localdef])
 AC_MSG_CHECKING([if Xtrans should support os-specific local connections])
 AC_MSG_RESULT($LOCALCONN)
 if test "$LOCALCONN" = "yes"; then
	AC_DEFINE(LOCALCONN,1,[Support os-specific local connections])
 fi
 
]) # XTRANS_CONNECTION_FLAGS


# XTRANS_SECURE_RPC_FLAGS()
# -------------------------
# Check for Secure RPC functions - must come after XTRANS_TCP_FLAGS
# so that any necessary networking libraries are already found
AC_DEFUN([XTRANS_SECURE_RPC_FLAGS],
[AC_REQUIRE([XTRANS_TCP_FLAGS])
 AC_ARG_ENABLE(secure-rpc, 
	AC_HELP_STRING([--enable-secure-rpc],[Enable Secure RPC]),
        [SECURE_RPC=$enableval], [SECURE_RPC="try"])

 if test "x$SECURE_RPC" = "xyes" -o "x$SECURE_RPC" = "xtry" ; then
	FOUND_SECURE_RPC="no"
	AC_CHECK_FUNCS([authdes_seccreate authdes_create],
			[FOUND_SECURE_RPC="yes"])
	if test "x$FOUND_SECURE_RPC" = "xno" ; then
		if test "x$SECURE_RPC" = "xyes" ; then
	AC_MSG_ERROR([Secure RPC requested, but required functions not found])
		fi	
		SECURE_RPC="no"
	else
		dnl FreeBSD keeps getsecretkey in librpcsvc
		AC_SEARCH_LIBS(getsecretkey, [rpcsvc])
		SECURE_RPC="yes"
	fi
 fi
 AC_MSG_CHECKING([if Secure RPC authentication ("SUN-DES-1") should be supported])
 if test "x$SECURE_RPC" = "xyes" ; then
	AC_DEFINE(SECURE_RPC, 1, [Support Secure RPC ("SUN-DES-1") authentication for X11 clients])
 fi
 AC_MSG_RESULT($SECURE_RPC)
]) # XTRANS_SECURE_RPC_FLAGS

