/*

Copyright 1989, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

*/

/*
 * This file contains operating system dependencies.
 */

#define NEED_EVENTS

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <X11/Xlibint.h>
#include <X11/Xtrans/Xtrans.h>
#include <X11/Xauth.h>
#ifdef HASXDMAUTH
#include <X11/Xdmcp.h>
#endif
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>

#if !defined(WIN32)
#ifndef Lynx
#include <sys/socket.h>
#else
#include <socket.h>
#endif
#else
#include <X11/Xwindows.h>
#endif

#ifndef X_CONNECTION_RETRIES		/* number retries on ECONNREFUSED */
#define X_CONNECTION_RETRIES 5
#endif

#include "Xintconn.h"

/* prototypes */
static void GetAuthorization(
    XtransConnInfo trans_conn,
    int family,
    char *saddr,
    int saddrlen,
    int idisplay,
    char **auth_namep,
    int *auth_namelenp,
    char **auth_datap,
    int *auth_datalenp);

/* functions */
static char *copystring (const char *src, int len)
{
    char *dst = Xmalloc (len + 1);

    if (dst) {
	strncpy (dst, src, len);
	dst[len] = '\0';
    }

    return dst;
}

#define Xstrdup(s)	copystring(s, strlen(s))

#ifdef TCPCONN
# define TCP_TRANS	"tcp"
#endif
#ifdef UNIXCONN
# define UNIX_TRANS	"unix"
#endif
#if defined(LOCALCONN) || defined(OS2PIPECONN)
# define LOCAL_TRANS	"local"
#endif

/*
 * Attempts to connect to server, given display name. Returns file descriptor
 * (network socket) or -1 if connection fails.  Display names may be of the
 * following format:
 *
 *     [protocol/] [hostname] : [:] displaynumber [.screennumber]
 *
 * A string with exactly two colons seperating hostname from the display
 * indicates a DECnet style name.  Colons in the hostname may occur if an
 * IPv6 numeric address is used as the hostname.  An IPv6 numeric address
 * may also end in a double colon, so three colons in a row indicates an
 * IPv6 address ending in :: followed by :display.  To make it easier for
 * people to read, an IPv6 numeric address hostname may be surrounded by
 * [ ] in a similar fashion to the IPv6 numeric address URL syntax defined
 * by IETF RFC 2732.
 *
 * If no hostname and no protocol is specified, the string is interpreted
 * as the most efficient local connection to a server on the same machine.
 * This is usually:
 *
 *     o  shared memory
 *     o  local stream
 *     o  UNIX domain socket
 *     o  TCP to local host
 *
 * This function will eventually call the X Transport Interface functions
 * which expects the hostname in the format:
 *
 *	[protocol/] [hostname] : [:] displaynumber
 *
 */
XtransConnInfo
_X11TransConnectDisplay (
    char *display_name,
    char **fullnamep,			/* RETURN */
    int *dpynump,			/* RETURN */
    int *screenp,			/* RETURN */
    char **auth_namep,			/* RETURN */
    int *auth_namelenp,			/* RETURN */
    char **auth_datap,			/* RETURN */
    int *auth_datalenp)			/* RETURN */
{
    int family;
    int saddrlen;
    Xtransaddr *saddr;
    char *lastp, *lastc, *p;		/* char pointers */
    char *pprotocol = NULL;		/* start of protocol name */
    char *phostname = NULL;		/* start of host of display */
    char *pdpynum = NULL;		/* start of dpynum of display */
    char *pscrnum = NULL;		/* start of screen of display */
    Bool dnet = False;			/* if true, then DECnet format */
    int idisplay = 0;			/* required display number */
    int iscreen = 0;			/* optional screen number */
    /*  int (*connfunc)(); */		/* method to create connection */
    int len, hostlen;			/* length tmp variable */
    int retry;				/* retry counter */
    char addrbuf[128];			/* final address passed to
					   X Transport Interface */
    char* address = addrbuf;
    XtransConnInfo trans_conn = NULL;	/* transport connection object */
    int connect_stat;
#if defined(LOCALCONN) || defined(UNIXCONN) || defined(TCPCONN)
    Bool reset_hostname = False;	/* Reset hostname? */
    char *original_hostname = NULL;
    int local_transport_index = -1;
    const char *local_transport[] = { LOCAL_TRANSPORT_LIST, NULL };
#endif

    p = display_name;

    saddrlen = 0;			/* set so that we can clear later */
    saddr = NULL;

    /*
     * Step 0, find the protocol.  This is delimited by the optional
     * slash ('/').
     */
    for (lastp = p; *p && *p != ':' && *p != '/'; p++) ;
    if (!*p) return NULL;		/* must have a colon */

    if (p != lastp && *p != ':') {	/* protocol given? */
	pprotocol = copystring (lastp, p - lastp);
	if (!pprotocol) goto bad;	/* no memory */
	p++;				/* skip the '/' */
    } else
	p = display_name;		/* reset the pointer in
					   case no protocol was given */

    /*
     * Step 1, find the hostname.  This is delimited by either one colon,
     * or two colons in the case of DECnet (DECnet Phase V allows a single
     * colon in the hostname).  (See note above regarding IPv6 numeric
     * addresses with triple colons or [] brackets.)
     */

    lastp = p;
    lastc = NULL;
    for (; *p; p++)
	if (*p == ':')
	    lastc = p;

    if (!lastc) return NULL;		/* must have a colon */

    if ((lastp != lastc) && (*(lastc - 1) == ':')
#if defined(IPv6) && defined(AF_INET6)
      && ( ((lastc - 1) == lastp) || (*(lastc - 2) != ':'))
#endif
	) {
	/* DECnet display specified */

#ifndef DNETCONN
	goto bad;
#else
	dnet = True;
	/* override the protocol specified */
	if (pprotocol)
	    Xfree (pprotocol);
	pprotocol = copystring ("dnet", 4);
	hostlen = lastc - 1 - lastp;
#endif
    }
    else
	hostlen = lastc - lastp;

    if (hostlen > 0) {		/* hostname given? */
	phostname = copystring (lastp, hostlen);
	if (!phostname) goto bad;	/* no memory */
    }

    p = lastc;

#if defined(LOCALCONN) || defined(UNIXCONN) || defined(TCPCONN)
    /* check if phostname == localnodename AND protocol not specified */
    if (!pprotocol && phostname) {
	char localhostname[256];

	if ((_XGetHostname (localhostname, sizeof localhostname) > 0)
	    && (strcmp(phostname, localhostname) == 0)) {
	    original_hostname = phostname;
	    phostname = NULL;
	    reset_hostname = True;
	}
    }
#endif


    /*
     * Step 2, find the display number.  This field is required and is
     * delimited either by a nul or a period, depending on whether or not
     * a screen number is present.
     */

    for (lastp = ++p; *p && isascii(*p) && isdigit(*p); p++) ;
    if ((p == lastp) ||			/* required field */
	(*p != '\0' && *p != '.') ||	/* invalid non-digit terminator */
	!(pdpynum = copystring (lastp, p - lastp)))  /* no memory */
      goto bad;
    idisplay = atoi (pdpynum);


    /*
     * Step 3, find the screen number.  This field is optional.  It is
     * present only if the display number was followed by a period (which
     * we've already verified is the only non-nul character).
     */

    if (*p) {
	for (lastp = ++p; *p && isascii(*p) && isdigit (*p); p++) ;
	if (p != lastp) {
	    if (*p ||			/* non-digits */
		!(pscrnum = copystring (lastp, p - lastp))) /* no memory */
		goto bad;
	    iscreen = atoi (lastp);
	}
    }

    /*
     * At this point, we know the following information:
     *
     *     pprotocol                protocol string or NULL
     *     phostname                hostname string or NULL
     *     idisplay                 display number
     *     iscreen                  screen number
     *     dnet                     DECnet boolean
     *
     * We can now decide which transport to use based on the defined
     * connection types and the hostname string.
     * If phostname & pprotocol are NULL, then choose the best transport.
     * If phostname is "unix" & pprotocol is NULL, then choose UNIX domain
     * sockets (if configured).
     */

#if defined(TCPCONN) || defined(UNIXCONN) || defined(LOCALCONN) || defined(MNX_TCPCONN) || defined(OS2PIPECONN)
    if (!pprotocol) {
#if defined(UNIXCONN)
	if (phostname && (strcmp (phostname, "unix") == 0)) {
	    Xfree(pprotocol);
	    pprotocol = copystring ("unix", 4);
	} else
#endif
#ifdef HAVE_LAUNCHD
	if (phostname && phostname[0]=='/') {
		pprotocol = copystring ("local", 5);
	}
#endif
	if (!phostname)
	{
	    if (local_transport[0] != NULL) {
		pprotocol = Xstrdup(local_transport[0]);
		local_transport_index = 0;
	    }
	}

	if (!pprotocol) { /* if still not found one, tcp is our last resort */
	    pprotocol = copystring ("tcp", 3);
	}
    }
#endif


  connect:
    /*
     * This seems kind of backwards, but we need to put the protocol,
     * host, and port back together to pass to _X11TransOpenCOTSClient().
     */

    {
	int olen = 3 + (pprotocol ? strlen(pprotocol) : 0) +
		       (phostname ? strlen(phostname) : 0) +
		       (pdpynum   ? strlen(pdpynum)   : 0);
	if (olen > sizeof addrbuf) address = Xmalloc (olen);
    }
    if (!address) goto bad;

    sprintf(address,"%s/%s:%d",
	pprotocol ? pprotocol : "",
	phostname ? phostname : "",
	idisplay );

    /*
     * Make the connection, also need to get the auth address info for
     * the connection.  Do retries in case server host has hit its
     * backlog (which, unfortunately, isn't distinguishable from there not
     * being a server listening at all, which is why we have to not retry
     * too many times).
     */
    for(retry=X_CONNECTION_RETRIES; retry>=0; retry-- )
    {
	if ( (trans_conn = _X11TransOpenCOTSClient(address)) == NULL )
	{
	    break;
	}
	if ((connect_stat = _X11TransConnect(trans_conn,address)) < 0 )
	{
	    _X11TransClose(trans_conn);
	    trans_conn = NULL;

	    if (connect_stat == TRANS_TRY_CONNECT_AGAIN)
		continue;
	    else
		break;
	}

	_X11TransGetPeerAddr(trans_conn, &family, &saddrlen, &saddr);

	/*
	 * The family is given in a socket format (ie AF_INET). This
	 * will convert it to the format used by the authorization and
	 * X protocol (ie FamilyInternet).
	 */

	if( _X11TransConvertAddress(&family, &saddrlen, &saddr) < 0 )
	{
	    _X11TransClose(trans_conn);
	    trans_conn = NULL;
	    if (saddr)
	    {
		free ((char *) saddr);
		saddr = NULL;
	    }
	    continue;
	}

	break;
    }

    if (address != addrbuf) Xfree (address);
    address = addrbuf;

    if( trans_conn == NULL )
      goto bad;

    /*
     * Set close-on-exec so that programs that fork() doesn't get confused.
     */

    _X11TransSetOption(trans_conn,TRANS_CLOSEONEXEC,1);

    /*
     * Build the expanded display name:
     *
     *     [host] : [:] dpy . scr \0
     */
#if defined(LOCALCONN) || defined(TCPCONN) || defined(UNIXCONN)
    /*
     *  If we computed the host name, get rid of it so that
     *  XDisplayString() and XDisplayName() agree.
     */
    if (reset_hostname && (phostname != original_hostname)) {
	Xfree (phostname);
	phostname = original_hostname;
	original_hostname = NULL;
    }
#endif
    len = ((phostname ? strlen(phostname) : 0) + 1 + (dnet ? 1 : 0) +
	   strlen(pdpynum) + 1 + (pscrnum ? strlen(pscrnum) : 1) + 1);
    *fullnamep = (char *) Xmalloc (len);
    if (!*fullnamep) goto bad;

#ifdef HAVE_LAUNCHD
    if (phostname && strlen(phostname) > 11 && !strncmp(phostname, "/tmp/launch", 11))
    	sprintf (*fullnamep, "%s%s%d",
	     (phostname ? phostname : ""),
	     (dnet ? "::" : ":"),
	     idisplay);
    else
#endif
    sprintf (*fullnamep, "%s%s%d.%d",
	     (phostname ? phostname : ""),
	     (dnet ? "::" : ":"),
	     idisplay, iscreen);

    *dpynump = idisplay;
    *screenp = iscreen;
    if (pprotocol) Xfree (pprotocol);
    if (phostname) Xfree (phostname);
    if (pdpynum) Xfree (pdpynum);
    if (pscrnum) Xfree (pscrnum);
#if defined(LOCALCONN) || defined(UNIXCONN) || defined(TCPCONN)
    if (original_hostname) Xfree (original_hostname);
#endif

    GetAuthorization(trans_conn, family, (char *) saddr, saddrlen, idisplay,
		     auth_namep, auth_namelenp, auth_datap, auth_datalenp);
    return trans_conn;


    /*
     * error return; make sure everything is cleaned up.
     */
  bad:
    if (trans_conn) (void)_X11TransClose(trans_conn);
    if (saddr) free ((char *) saddr);
    if (pprotocol) Xfree (pprotocol);
    if (phostname) Xfree (phostname);
    if (address && address != addrbuf) { Xfree(address); address = addrbuf; }

#if defined(LOCALCONN) || defined(UNIXCONN) || defined(TCPCONN)
    /* If connecting to the local machine, and we failed, try again with
     * the next transport type available, if there is one.
     */
    if (local_transport_index >= 0) {
	if (local_transport[++local_transport_index] != NULL) {
	    pprotocol = Xstrdup(local_transport[local_transport_index]);
#ifdef TCPCONN
	    if (strcmp(pprotocol, "tcp") == 0) {
		if (original_hostname != NULL) {
		    phostname = original_hostname;
		    original_hostname = NULL;
		} else {
		    phostname = copystring("localhost", 9);
		}
	    } else
#endif /* TCPCONN */
	    {
		if ((phostname != NULL) && (original_hostname == NULL)) {
		    original_hostname = phostname;
		}
		phostname = NULL;
	    }
	    goto connect;
	}
    }

    /* No more to try, we've failed all available local transports */
    if (original_hostname) Xfree(original_hostname);
#endif /* LOCALCONN || UNIXCONN || TCPCONN */

    if (pdpynum) Xfree (pdpynum);
    if (pscrnum) Xfree (pscrnum);
    return NULL;

}

/*
 * This is gross, but we need it for compatiblity.
 * The test suite relies on the following interface.
 *
 */

int _XConnectDisplay (
    char *display_name,
    char **fullnamep,			/* RETURN */
    int *dpynump,			/* RETURN */
    int *screenp,			/* RETURN */
    char **auth_namep,			/* RETURN */
    int *auth_namelenp,			/* RETURN */
    char **auth_datap,			/* RETURN */
    int *auth_datalenp)			/* RETURN */
{
   XtransConnInfo trans_conn;

   trans_conn = _X11TransConnectDisplay (
		      display_name, fullnamep, dpynump, screenp,
		      auth_namep, auth_namelenp, auth_datap, auth_datalenp);

   if (trans_conn)
   {
       int fd = _X11TransGetConnectionNumber (trans_conn);
       _X11TransFreeConnInfo (trans_conn);
       return (fd);
   }
   else
       return (-1);
}


/*****************************************************************************
 *                                                                           *
 *			  Connection Utility Routines                        *
 *                                                                           *
 *****************************************************************************/

/*
 * Disconnect from server.
 */

int _XDisconnectDisplay (trans_conn)

XtransConnInfo	trans_conn;

{
    _X11TransDisconnect(trans_conn);
    _X11TransClose(trans_conn);
    return 0;
}



Bool
_XSendClientPrefix (dpy, client, auth_proto, auth_string, prefix)
     Display *dpy;
     xConnClientPrefix *client;		/* contains count for auth_* */
     char *auth_proto, *auth_string;	/* NOT null-terminated */
     xConnSetupPrefix *prefix;		/* prefix information */
{
    int auth_length = client->nbytesAuthProto;
    int auth_strlen = client->nbytesAuthString;
    static char padbuf[3];		/* for padding to 4x bytes */
    int pad;
    struct iovec iovarray[5], *iov = iovarray;
    int niov = 0;
    int len = 0;

#define add_to_iov(b,l) \
  { iov->iov_base = (b); iov->iov_len = (l); iov++, niov++; len += (l); }

    add_to_iov ((caddr_t) client, SIZEOF(xConnClientPrefix));

    /*
     * write authorization protocol name and data
     */
    if (auth_length > 0) {
	add_to_iov (auth_proto, auth_length);
	pad = -auth_length & 3; /* pad auth_length to a multiple of 4 */
	if (pad) add_to_iov (padbuf, pad);
    }
    if (auth_strlen > 0) {
	add_to_iov (auth_string, auth_strlen);
	pad = -auth_strlen & 3; /* pad auth_strlen to a multiple of 4 */
	if (pad) add_to_iov (padbuf, pad);
    }

#undef add_to_iov

    len -= _X11TransWritev (dpy->trans_conn, iovarray, niov);

    /*
     * Set the connection non-blocking since we use select() to block.
     */

    _X11TransSetOption(dpy->trans_conn, TRANS_NONBLOCKING, 1);

    if (len != 0)
	return -1;

#ifdef K5AUTH
    if (auth_length == 14 &&
	!strncmp(auth_proto, "MIT-KERBEROS-5", 14))
    {
	return k5_clientauth(dpy, prefix);
    } else
#endif
    return 0;
}


#ifdef STREAMSCONN
#ifdef SVR4
#include <tiuser.h>
#else
#undef HASXDMAUTH
#endif
#endif

#ifdef SECURE_RPC
#include <rpc/rpc.h>
#ifdef ultrix
#include <time.h>
#include <rpc/auth_des.h>
#endif
#endif

#ifdef HASXDMAUTH
#include <time.h>
#define Time_t time_t
#endif

/*
 * First, a routine for setting authorization data
 */
static int xauth_namelen = 0;
static char *xauth_name = NULL;	 /* NULL means use default mechanism */
static int xauth_datalen = 0;
static char *xauth_data = NULL;	 /* NULL means get default data */

/*
 * This is a list of the authorization names which Xlib currently supports.
 * Xau will choose the file entry which matches the earliest entry in this
 * array, allowing us to prioritize these in terms of the most secure first
 */

static char *default_xauth_names[] = {
#ifdef K5AUTH
    "MIT-KERBEROS-5",
#endif
#ifdef SECURE_RPC
    "SUN-DES-1",
#endif
#ifdef HASXDMAUTH
    "XDM-AUTHORIZATION-1",
#endif
    "MIT-MAGIC-COOKIE-1"
};

static _Xconst int default_xauth_lengths[] = {
#ifdef K5AUTH
    14,     /* strlen ("MIT-KERBEROS-5") */
#endif
#ifdef SECURE_RPC
    9,	    /* strlen ("SUN-DES-1") */
#endif
#ifdef HASXDMAUTH
    19,	    /* strlen ("XDM-AUTHORIZATION-1") */
#endif
    18	    /* strlen ("MIT-MAGIC-COOKIE-1") */
};

#define NUM_DEFAULT_AUTH    (sizeof (default_xauth_names) / sizeof (default_xauth_names[0]))

static char **xauth_names = default_xauth_names;
static _Xconst int  *xauth_lengths = default_xauth_lengths;

static int  xauth_names_length = NUM_DEFAULT_AUTH;

void XSetAuthorization (name, namelen, data, datalen)
    int namelen, datalen;		/* lengths of name and data */
    char *name, *data;			/* NULL or arbitrary array of bytes */
{
    char *tmpname, *tmpdata;

    _XLockMutex(_Xglobal_lock);
    if (xauth_name) Xfree (xauth_name);	 /* free any existing data */
    if (xauth_data) Xfree (xauth_data);

    xauth_name = xauth_data = NULL;	/* mark it no longer valid */
    xauth_namelen = xauth_datalen = 0;
    _XUnlockMutex(_Xglobal_lock);

    if (namelen < 0) namelen = 0;	/* check for bogus inputs */
    if (datalen < 0) datalen = 0;	/* maybe should return? */

    if (namelen > 0)  {			/* try to allocate space */
	tmpname = Xmalloc ((unsigned) namelen);
	if (!tmpname) return;
	memcpy (tmpname, name, namelen);
    } else {
	tmpname = NULL;
    }

    if (datalen > 0)  {
	tmpdata = Xmalloc ((unsigned) datalen);
	if (!tmpdata) {
	    if (tmpname) (void) Xfree (tmpname);
	    return;
	}
	memcpy (tmpdata, data, datalen);
    } else {
	tmpdata = NULL;
    }

    _XLockMutex(_Xglobal_lock);
    xauth_name = tmpname;		/* and store the suckers */
    xauth_namelen = namelen;
    if (tmpname)
    {
	xauth_names = &xauth_name;
	xauth_lengths = &xauth_namelen;
	xauth_names_length = 1;
    }
    else
    {
	xauth_names = default_xauth_names;
	xauth_lengths = default_xauth_lengths;
	xauth_names_length = NUM_DEFAULT_AUTH;
    }
    xauth_data = tmpdata;
    xauth_datalen = datalen;
    _XUnlockMutex(_Xglobal_lock);
    return;
}

#ifdef SECURE_RPC
/*
 * Create a credential that we can send to the X server.
 */
static int
auth_ezencode(
    char           *servername,
    int             window,
    char	   *cred_out,
    int            *len)
{
        AUTH           *a;
        XDR             xdr;

#if defined(SVR4) && defined(sun)
        a = authdes_seccreate(servername, window, NULL, NULL);
#else
        a = (AUTH *)authdes_create(servername, window, NULL, NULL);
#endif
        if (a == (AUTH *)NULL) {
                perror("auth_create");
                return 0;
        }
        xdrmem_create(&xdr, cred_out, *len, XDR_ENCODE);
        if (AUTH_MARSHALL(a, &xdr) == FALSE) {
                perror("auth_marshall");
                AUTH_DESTROY(a);
                return 0;
        }
        *len = xdr_getpos(&xdr);
        AUTH_DESTROY(a);
	return 1;
}
#endif

#ifdef K5AUTH
#include <com_err.h>

extern krb5_flags krb5_kdc_default_options;

/*
 * k5_clientauth
 *
 * Returns non-zero if the setup prefix has been read,
 * so we can tell XOpenDisplay to not bother looking for it by
 * itself.
 */
static int k5_clientauth(dpy, sprefix)
    Display *dpy;
    xConnSetupPrefix *sprefix;
{
    krb5_error_code retval;
    xReq prefix;
    char *buf;
    CARD16 plen, tlen;
    krb5_data kbuf;
    krb5_ccache cc;
    krb5_creds creds;
    krb5_principal cprinc, sprinc;
    krb5_ap_rep_enc_part *repl;

    krb5_init_ets();
    /*
     * stage 0: get encoded principal and tgt from server
     */
    _XRead(dpy, (char *)&prefix, sz_xReq);
    if (prefix.reqType != 2 && prefix.reqType != 3)
	/* not an auth packet... so deal */
	if (prefix.reqType == 0 || prefix.reqType == 1)
	{
	    memcpy((char *)sprefix, (char *)&prefix, sz_xReq);
	    _XRead(dpy, (char *)sprefix + sz_xReq,
		   sz_xConnSetupPrefix - sz_xReq); /* ewww... gross */
	    return 1;
	}
	else
	{
	    fprintf(stderr,
		    "Xlib: Krb5 stage 0: got illegal connection setup success code %d\n",
		    prefix.reqType);
	    return -1;
	}
    if (prefix.data != 0)
    {
	fprintf(stderr, "Xlib: got out of sequence (%d) packet in Krb5 auth\n",
		prefix.data);
	return -1;
    }
    buf = (char *)malloc((prefix.length << 2) - sz_xReq);
    if (buf == NULL)		/* malloc failed.  Run away! */
    {
	fprintf(stderr, "Xlib: malloc bombed in Krb5 auth\n");
	return -1;
    }
    tlen = (prefix.length << 2) - sz_xReq;
    _XRead(dpy, buf, tlen);
    if (prefix.reqType == 2 && tlen < 6)
    {
	fprintf(stderr, "Xlib: Krb5 stage 0 reply from server too short\n");
	free(buf);
	return -1;
    }
    if (prefix.reqType == 2)
    {
	plen = *(CARD16 *)buf;
	kbuf.data = buf + 2;
	kbuf.length = (plen > tlen) ? tlen : plen;
    }
    else
    {
	kbuf.data = buf;
	kbuf.length = tlen;
    }
    if (XauKrb5Decode(kbuf, &sprinc))
    {
	free(buf);
	fprintf(stderr, "Xlib: XauKrb5Decode bombed\n");
	return -1;
    }
    if (prefix.reqType == 3)	/* do some special stuff here */
    {
	char *sname, *hostname = NULL;

	sname = (char *)malloc(krb5_princ_component(sprinc, 0)->length + 1);
	if (sname == NULL)
	{
	    free(buf);
	    krb5_free_principal(sprinc);
	    fprintf(stderr, "Xlib: malloc bombed in Krb5 auth\n");
	    return -1;
	}
	memcpy(sname, krb5_princ_component(sprinc, 0)->data,
	       krb5_princ_component(sprinc, 0)->length);
	sname[krb5_princ_component(sprinc, 0)->length] = '\0';
	krb5_free_principal(sprinc);
	if (dpy->display_name[0] != ':') /* hunt for a hostname */
	{
	    char *t;

	    if ((hostname = (char *)malloc(strlen(dpy->display_name)))
		== NULL)
	    {
		free(buf);
		free(sname);
		fprintf(stderr, "Xlib: malloc bombed in Krb5 auth\n");
		return -1;
	    }
	    strcpy(hostname, dpy->display_name);
	    t = strchr(hostname, ':');
	    if (t == NULL)
	    {
		free(buf);
		free(sname);
		free(hostname);
		fprintf(stderr,
			"Xlib: shouldn't get here! malformed display name.");
		return -1;
	    }
	    if ((t - hostname + 1 < strlen(hostname)) && t[1] == ':')
		t++;
	    *t = '\0';		/* truncate the dpy number out */
	}
	retval = krb5_sname_to_principal(hostname, sname,
					 KRB5_NT_SRV_HST, &sprinc);
	free(sname);
	if (hostname)
	    free(hostname);
	if (retval)
	{
	    free(buf);
	    fprintf(stderr, "Xlib: krb5_sname_to_principal failed: %s\n",
		    error_message(retval));
	    return -1;
	}
    }
    if (retval = krb5_cc_default(&cc))
    {
	free(buf);
	krb5_free_principal(sprinc);
	fprintf(stderr, "Xlib: krb5_cc_default failed: %s\n",
		error_message(retval));
	return -1;
    }
    if (retval = krb5_cc_get_principal(cc, &cprinc))
    {
	free(buf);
	krb5_free_principal(sprinc);
	fprintf(stderr, "Xlib: cannot get Kerberos principal from \"%s\": %s\n",
		krb5_cc_default_name(), error_message(retval));
	return -1;
    }
    bzero((char *)&creds, sizeof(creds));
    creds.server = sprinc;
    creds.client = cprinc;
    if (prefix.reqType == 2)
    {
	creds.second_ticket.length = tlen - plen - 2;
	creds.second_ticket.data = buf + 2 + plen;
	retval = krb5_get_credentials(KRB5_GC_USER_USER |
				      krb5_kdc_default_options,
				      cc, &creds);
    }
    else
	retval = krb5_get_credentials(krb5_kdc_default_options,
				      cc, &creds);
    if (retval)
    {
	free(buf);
	krb5_free_cred_contents(&creds);
	fprintf(stderr, "Xlib: cannot get Kerberos credentials: %s\n",
		error_message(retval));
	return -1;
    }
    /*
     * now format the ap_req to send to the server
     */
    if (prefix.reqType == 2)
	retval = krb5_mk_req_extended(AP_OPTS_USE_SESSION_KEY |
				      AP_OPTS_MUTUAL_REQUIRED, NULL,
				      0, 0, NULL, cc,
				      &creds, NULL, &kbuf);
    else
	retval = krb5_mk_req_extended(AP_OPTS_MUTUAL_REQUIRED, NULL,
				      0, 0, NULL, cc, &creds, NULL,
				      &kbuf);
    free(buf);
    if (retval)			/* Some manner of Kerberos lossage */
    {
	krb5_free_cred_contents(&creds);
	fprintf(stderr, "Xlib: krb5_mk_req_extended failed: %s\n",
		error_message(retval));
	return -1;
    }
    prefix.reqType = 1;
    prefix.data = 0;
    prefix.length = (kbuf.length + sz_xReq + 3) >> 2;
    /*
     * stage 1: send ap_req to server
     */
    _XSend(dpy, (char *)&prefix, sz_xReq);
    _XSend(dpy, (char *)kbuf.data, kbuf.length);
    free(kbuf.data);
    /*
     * stage 2: get ap_rep from server to mutually authenticate
     */
    _XRead(dpy, (char *)&prefix, sz_xReq);
    if (prefix.reqType != 2)
	if (prefix.reqType == 0 || prefix.reqType == 1)
	{
	    memcpy((char *)sprefix, (char *)&prefix, sz_xReq);
	    _XRead(dpy, (char *)sprefix + sz_xReq,
		   sz_xConnSetupPrefix - sz_xReq);
	    return 1;
	}
	else
	{
	    fprintf(stderr,
		    "Xlib: Krb5 stage 2: got illegal connection setup success code %d\n",
		    prefix.reqType);
	    return -1;
	}
    if (prefix.data != 2)
	return -1;
    kbuf.length = (prefix.length << 2) - sz_xReq;
    kbuf.data = (char *)malloc(kbuf.length);
    if (kbuf.data == NULL)
    {
	fprintf(stderr, "Xlib: malloc bombed in Krb5 auth\n");
	return -1;
    }
    _XRead(dpy, (char *)kbuf.data, kbuf.length);
    retval = krb5_rd_rep(&kbuf, &creds.keyblock, &repl);
    if (retval)
    {
	free(kbuf.data);
	fprintf(stderr, "Xlib: krb5_rd_rep failed: %s\n",
		error_message(retval));
	return -1;
    }
    free(kbuf.data);
    /*
     * stage 3: send a short ack to the server and return
     */
    prefix.reqType = 3;
    prefix.data = 0;
    prefix.length = sz_xReq >> 2;
    _XSend(dpy, (char *)&prefix, sz_xReq);
    return 0;
}
#endif /* K5AUTH */

static void
GetAuthorization(
    XtransConnInfo trans_conn,
    int family,
    char *saddr,
    int saddrlen,
    int idisplay,
    char **auth_namep,			/* RETURN */
    int *auth_namelenp,			/* RETURN */
    char **auth_datap,			/* RETURN */
    int *auth_datalenp)			/* RETURN */
{
#ifdef SECURE_RPC
    char rpc_cred[MAX_AUTH_BYTES];
#endif
#ifdef HASXDMAUTH
    unsigned char xdmcp_data[192/8];
#endif
    char *auth_name;
    int auth_namelen;
    unsigned char *auth_data;
    int auth_datalen;
    Xauth *authptr = NULL;

/*
 * Look up the authorization protocol name and data if necessary.
 */
    if (xauth_name && xauth_data) {
	auth_namelen = xauth_namelen;
	auth_name = xauth_name;
	auth_datalen = xauth_datalen;
	auth_data = (unsigned char *) xauth_data;
    } else {
	char dpynumbuf[40];		/* big enough to hold 2^64 and more */
	(void) sprintf (dpynumbuf, "%d", idisplay);

	authptr = XauGetBestAuthByAddr ((unsigned short) family,
				    (unsigned short) saddrlen,
				    saddr,
				    (unsigned short) strlen (dpynumbuf),
				    dpynumbuf,
				    xauth_names_length,
				    xauth_names,
				    xauth_lengths);
	if (authptr) {
	    auth_namelen = authptr->name_length;
	    auth_name = (char *)authptr->name;
	    auth_datalen = authptr->data_length;
	    auth_data = (unsigned char *) authptr->data;
	} else {
	    auth_namelen = 0;
	    auth_name = NULL;
	    auth_datalen = 0;
	    auth_data = NULL;
	}
    }
#ifdef HASXDMAUTH
    /*
     * build XDM-AUTHORIZATION-1 data
     */
    if (auth_namelen == 19 && !strncmp (auth_name, "XDM-AUTHORIZATION-1", 19))
    {
	int i, j;
	Time_t  now;
	int family, addrlen;
	Xtransaddr *addr = NULL;

	for (j = 0; j < 8; j++)
	    xdmcp_data[j] = auth_data[j];

	_X11TransGetMyAddr(trans_conn, &family, &addrlen, &addr);

	switch( family )
	{
#ifdef AF_INET
	case AF_INET:
	  {
	    /*
	     * addr will contain a sockaddr_in with all
	     * of the members already in network byte order.
	     */

	    for(i=4; i<8; i++)	/* do sin_addr */
		xdmcp_data[j++] = ((char *)addr)[i];
	    for(i=2; i<4; i++)	/* do sin_port */
		xdmcp_data[j++] = ((char *)addr)[i];
	    break;
	  }
#endif /* AF_INET */
#if defined(IPv6) && defined(AF_INET6)
	case AF_INET6:
	  /* XXX This should probably never happen */
	  {
	    unsigned char ipv4mappedprefix[] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff };

	    /* In the case of v4 mapped addresses send the v4
	       part of the address - addr is already in network byte order */
	    if (memcmp((char*)addr+8, ipv4mappedprefix, 12) == 0) {
		for (i = 20 ; i < 24; i++)
		    xdmcp_data[j++] = ((char *)addr)[i];

		/* Port number */
		for (i=2; i<4; i++)
		    xdmcp_data[j++] = ((char *)addr)[i];
	    } else {
		/* Fake data to keep the data aligned. Otherwise the
		   the server will bail about incorrect timing data */
		for (i = 0; i < 6; i++) {
		    xdmcp_data[j++] = 0;
		}
	    }
	    break;
	  }
#endif /* AF_INET6 */
#ifdef AF_UNIX
	case AF_UNIX:
	  {
	    /*
	     * We don't use the sockaddr_un for this encoding.
	     * Instead, we create a sockaddr_in filled with
	     * a decreasing counter for the address, and the
	     * pid for the port.
	     */

	    static unsigned long    unix_addr = 0xFFFFFFFF;
	    unsigned long	the_addr;
	    unsigned short	the_port;
	    unsigned long	the_utime;
	    struct timeval      tp;

	    X_GETTIMEOFDAY(&tp);
	    _XLockMutex(_Xglobal_lock);
	    the_addr = unix_addr--;
	    _XUnlockMutex(_Xglobal_lock);
	    the_utime = (unsigned long) tp.tv_usec;
	    the_port = getpid ();

	    xdmcp_data[j++] = (the_utime >> 24) & 0xFF;
	    xdmcp_data[j++] = (the_utime >> 16) & 0xFF;
	    xdmcp_data[j++] = ((the_utime >>  8) & 0xF0)
		| ((the_addr >>  8) & 0x0F);
	    xdmcp_data[j++] = (the_addr >>  0) & 0xFF;
	    xdmcp_data[j++] = (the_port >>  8) & 0xFF;
	    xdmcp_data[j++] = (the_port >>  0) & 0xFF;
	    break;
	  }
#endif /* AF_UNIX */
#ifdef AF_DECnet
	case AF_DECnet:
	    /*
	     * What is the defined encoding for this?
	     */
	    break;
#endif /* AF_DECnet */
	default:
	    /*
	     * Need to return some kind of errro status here.
	     * maybe a NULL auth??
	     */
	    break;
	} /* switch */

	if (addr)
	    free ((char *) addr);

	time (&now);
	xdmcp_data[j++] = (now >> 24) & 0xFF;
	xdmcp_data[j++] = (now >> 16) & 0xFF;
	xdmcp_data[j++] = (now >>  8) & 0xFF;
	xdmcp_data[j++] = (now >>  0) & 0xFF;
	while (j < 192 / 8)
	    xdmcp_data[j++] = 0;
	_XLockMutex(_Xglobal_lock);
	/* this function might use static data, hence the lock around it */
	XdmcpWrap (xdmcp_data, auth_data + 8,
		      xdmcp_data, j);
	_XUnlockMutex(_Xglobal_lock);
	auth_data = xdmcp_data;
	auth_datalen = j;
    }
#endif /* HASXDMAUTH */
#ifdef SECURE_RPC
    /*
     * The SUN-DES-1 authorization protocol uses the
     * "secure RPC" mechanism in SunOS 4.0+.
     */
    if (auth_namelen == 9 && !strncmp(auth_name, "SUN-DES-1", 9)) {
	char servernetname[MAXNETNAMELEN + 1];

	/*
	 * Copy over the server's netname from the authorization
	 * data field filled in by XauGetAuthByAddr().
	 */
	if (auth_datalen > MAXNETNAMELEN) {
	    auth_datalen = 0;
	    auth_data = NULL;
	} else {
	    memcpy(servernetname, auth_data, auth_datalen);
	    servernetname[auth_datalen] = '\0';

	    auth_datalen = sizeof (rpc_cred);
	    if (auth_ezencode(servernetname, 100, rpc_cred,
			      &auth_datalen))
		auth_data = (unsigned char *) rpc_cred;
	    else {
		auth_datalen = 0;
		auth_data = NULL;
	    }
	}
    }
#endif
    if (saddr) free ((char *) saddr);
    if ((*auth_namelenp = auth_namelen))
    {
	if ((*auth_namep = Xmalloc(auth_namelen)))
	    memcpy(*auth_namep, auth_name, auth_namelen);
	else
	    *auth_namelenp = 0;
    }
    else
	*auth_namep = NULL;
    if ((*auth_datalenp = auth_datalen))
    {
	if ((*auth_datap = Xmalloc(auth_datalen)))
	    memcpy(*auth_datap, auth_data, auth_datalen);
	else
	    *auth_datalenp = 0;
    }
    else
	*auth_datap = NULL;
    if (authptr) XauDisposeAuth (authptr);
}
