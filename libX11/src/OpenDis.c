/* $Xorg: OpenDis.c,v 1.4 2001/02/09 02:03:34 xorgcvs Exp $ */
/*

Copyright 1985, 1986, 1998  The Open Group

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
/* $XFree86: xc/lib/X11/OpenDis.c,v 3.16 2003/07/04 16:24:23 eich Exp $ */

#define NEED_REPLIES
#define NEED_EVENTS
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "Xlibint.h"
#if USE_XCB
#include "Xxcbint.h"
#else /* !USE_XCB */
#include <X11/Xtrans/Xtrans.h>
#include <X11/extensions/bigreqstr.h>
#endif /* USE_XCB */
#include <X11/Xatom.h>
#include <X11/Xresource.h>
#include <stdio.h>
#include "Xintconn.h"

#ifdef XKB
#include "XKBlib.h"
#endif /* XKB */

#if !USE_XCB
#ifdef X_NOT_POSIX
#define Size_t unsigned int
#else
#define Size_t size_t
#endif

#define bignamelen (sizeof(XBigReqExtensionName) - 1)

typedef struct {
    unsigned long seq;
    int opcode;
} _XBigReqState;
#endif /* !USE_XCB */

#ifdef XTHREADS
#include "locking.h"
int  (*_XInitDisplayLock_fn)(Display *dpy) = NULL;
void (*_XFreeDisplayLock_fn)(Display *dpy) = NULL;

#define InitDisplayLock(d)	(_XInitDisplayLock_fn ? (*_XInitDisplayLock_fn)(d) : Success)
#define FreeDisplayLock(d)	if (_XFreeDisplayLock_fn) (*_XFreeDisplayLock_fn)(d)
#else
#define InitDisplayLock(dis) Success
#define FreeDisplayLock(dis)
#endif /* XTHREADS */

static xReq _dummy_request = {
	0, 0, 0
};

static void OutOfMemory(Display *dpy, char *setup);
#if !USE_XCB
static Bool _XBigReqHandler(Display *dpy, xReply *rep, char *buf, int len,
				XPointer data);
#endif /* !USE_XCB */

/*
 * Connects to a server, creates a Display object and returns a pointer to
 * the newly created Display back to the caller.
 */
Display *
XOpenDisplay (
	register _Xconst char *display)
{
	register Display *dpy;		/* New Display object being created. */
	register int i;
	int j, k;			/* random iterator indexes */
#if !USE_XCB
	char *display_name;		/* pointer to display name */
	int endian;			/* to determine which endian. */
	xConnClientPrefix client;	/* client information */
	int idisplay;			/* display number */
	int prefixread = 0;             /* setup prefix already read? */
	char *conn_auth_name, *conn_auth_data;
	int conn_auth_namelen, conn_auth_datalen;
#endif /* !USE_XCB */
	char *setup = NULL;		/* memory allocated at startup */
	char *fullname = NULL;		/* expanded name of display */
	int iscreen;			/* screen number */
	xConnSetupPrefix prefix;	/* prefix information */
	int vendorlen;			/* length of vendor string */
	union {
		xConnSetup *setup;
		char *failure;
		char *vendor;
		xPixmapFormat *sf;
		xWindowRoot *rp;
		xDepth *dp;
		xVisualType *vp;
	} u;				/* proto data returned from server */
	long setuplength;	/* number of bytes in setup message */
	long usedbytes = 0;     /* number of bytes we have processed */
	unsigned long mask;
       long int conn_buf_size;
       char *xlib_buffer_size;

#if !USE_XCB
	bzero((char *) &client, sizeof(client));
	bzero((char *) &prefix, sizeof(prefix));

	/*
	 * If the display specifier string supplied as an argument to this
	 * routine is NULL or a pointer to NULL, read the DISPLAY variable.
	 */
	if (display == NULL || *display == '\0') {
		if ((display_name = getenv("DISPLAY")) == NULL) {
			/* Oops! No DISPLAY environment variable - error. */
			return(NULL);
		}
	}
	else {
		/* Display is non-NULL, copy the pointer */
		display_name = (char *)display;
	}
#endif /* !USE_XCB */

/*
 * Set the default error handlers.  This allows the global variables to
 * default to NULL for use with shared libraries.
 */
	if (_XErrorFunction == NULL) (void) XSetErrorHandler (NULL);
	if (_XIOErrorFunction == NULL) (void) XSetIOErrorHandler (NULL);

/*
 * Attempt to allocate a display structure. Return NULL if allocation fails.
 */
	if ((dpy = (Display *)Xcalloc(1, sizeof(Display))) == NULL) {
		return(NULL);
	}

/*
 * Call the Connect routine to get the transport connection object.
 * If NULL is returned, the connection failed. The connect routine
 * will set fullname to point to the expanded name.
 */

#if USE_XCB
	if(!_XConnectXCB(dpy, display, &fullname, &iscreen)) {
		dpy->display_name = fullname;
		OutOfMemory(dpy, NULL);
		return NULL;
	}
#else /* !USE_XCB */
	if ((dpy->trans_conn = _X11TransConnectDisplay (
					 display_name, &fullname, &idisplay,
					 &iscreen, &conn_auth_name,
					 &conn_auth_namelen, &conn_auth_data,
					 &conn_auth_datalen)) == NULL) {
		Xfree ((char *) dpy);
		return(NULL);
	}

	dpy->fd = _X11TransGetConnectionNumber (dpy->trans_conn);
#endif /* USE_XCB */

	/* Initialize as much of the display structure as we can.
	 * Initialize pointers to NULL so that XFreeDisplayStructure will
	 * work if we run out of memory before we finish initializing.
	 */
	dpy->display_name	= fullname;
	dpy->keysyms		= (KeySym *) NULL;
	dpy->modifiermap	= NULL;
	dpy->lock_meaning	= NoSymbol;
	dpy->keysyms_per_keycode = 0;
	dpy->xdefaults		= (char *)NULL;
	dpy->scratch_length	= 0L;
	dpy->scratch_buffer	= NULL;
	dpy->key_bindings	= NULL;
	dpy->ext_procs		= (_XExtension *)NULL;
	dpy->ext_data		= (XExtData *)NULL;
	dpy->ext_number 	= 0;
	dpy->event_vec[X_Error] = _XUnknownWireEvent;
	dpy->event_vec[X_Reply] = _XUnknownWireEvent;
	dpy->wire_vec[X_Error]  = _XUnknownNativeEvent;
	dpy->wire_vec[X_Reply]  = _XUnknownNativeEvent;
	for (i = KeyPress; i < LASTEvent; i++) {
	    dpy->event_vec[i] 	= _XWireToEvent;
	    dpy->wire_vec[i] 	= NULL;
	}
	for (i = LASTEvent; i < 128; i++) {
	    dpy->event_vec[i] 	= _XUnknownWireEvent;
	    dpy->wire_vec[i] 	= _XUnknownNativeEvent;
	}
	dpy->resource_id	= 0;
	dpy->db 		= (struct _XrmHashBucketRec *)NULL;
	dpy->cursor_font	= None;
	dpy->flags		= 0;
	dpy->async_handlers	= NULL;
	dpy->screens		= NULL;
	dpy->vendor		= NULL;
	dpy->buffer		= NULL;
	dpy->atoms		= NULL;
	dpy->error_vec		= NULL;
	dpy->context_db		= NULL;
	dpy->free_funcs		= NULL;
	dpy->pixmap_format	= NULL;
	dpy->cms.clientCmaps	= NULL;
	dpy->cms.defaultCCCs	= NULL;
	dpy->cms.perVisualIntensityMaps = NULL;
	dpy->im_filters		= NULL;
 	dpy->bigreq_size	= 0;
	dpy->lock		= NULL;
	dpy->lock_fns		= NULL;
	dpy->qfree		= NULL;
	dpy->next_event_serial_num = 1;
	dpy->im_fd_info		= NULL;
	dpy->im_fd_length	= 0;
	dpy->conn_watchers	= NULL;
	dpy->watcher_count	= 0;
	dpy->filedes		= NULL;
	dpy->flushes		= NULL;
	dpy->xcmisc_opcode	= 0;
	dpy->xkb_info		= NULL;

/*
 * Setup other information in this display structure.
 */
	dpy->vnumber = X_PROTOCOL;
	dpy->resource_alloc = _XAllocID;
	dpy->idlist_alloc = _XAllocIDs;
	dpy->synchandler = NULL;
	dpy->savedsynchandler = NULL;
	dpy->request = 0;
	dpy->last_request_read = 0;
	dpy->default_screen = iscreen;  /* Value returned by ConnectDisplay */
	dpy->last_req = (char *)&_dummy_request;

	/* Initialize the display lock */
	if (InitDisplayLock(dpy) != 0) {
	        OutOfMemory (dpy, setup);
		return(NULL);
	}

	if (!_XPollfdCacheInit(dpy)) {
	        OutOfMemory (dpy, setup);
		return(NULL);
	}

	/* Set up the output buffers. */
#ifndef XLIBDEFAULTBUFSIZE
#define XLIBDEFAULTBUFSIZE 16384 /* 16k */
#endif
#ifndef XLIBMINBUFSIZE
#define XLIBMINBUFSIZE BUFSIZE /* old default buffer size */
#endif
    if ((xlib_buffer_size = getenv("XLIBBUFFERSIZE")) == NULL)
        conn_buf_size = XLIBDEFAULTBUFSIZE;
    else
        conn_buf_size = 1024 * strtol(xlib_buffer_size, NULL, 10);
    if (conn_buf_size < XLIBMINBUFSIZE)
        conn_buf_size = XLIBMINBUFSIZE;

    if ((dpy->bufptr = dpy->buffer = Xcalloc(1, conn_buf_size)) == NULL) {
         OutOfMemory (dpy, setup);
         return(NULL);
    }
    dpy->bufmax = dpy->buffer + conn_buf_size;
#if USE_XCB
    dpy->xcb->real_bufmax = dpy->bufmax;
    dpy->bufmax = dpy->buffer;
#endif

	/* Set up the input event queue and input event queue parameters. */
	dpy->head = dpy->tail = NULL;
	dpy->qlen = 0;

	/* Set up free-function record */
	if ((dpy->free_funcs = (_XFreeFuncRec *)Xcalloc(1,
							sizeof(_XFreeFuncRec)))
	    == NULL) {
	    OutOfMemory (dpy, setup);
	    return(NULL);
	}

#if !USE_XCB
/*
 * The xConnClientPrefix describes the initial connection setup information
 * and is followed by the authorization information.  Sites that are interested
 * in security are strongly encouraged to use an authentication and
 * authorization system such as Kerberos.
 */
	endian = 1;
	if (*(char *) &endian)
	    client.byteOrder = '\154'; /* 'l' */
	else
	    client.byteOrder = '\102'; /* 'B' */
	client.majorVersion = X_PROTOCOL;
	client.minorVersion = X_PROTOCOL_REVISION;
	client.nbytesAuthProto = conn_auth_namelen;
	client.nbytesAuthString = conn_auth_datalen;
	prefixread = _XSendClientPrefix(dpy, &client,
					conn_auth_name, conn_auth_data,
					&prefix);
	if (prefixread < 0)
	{
	    _XDisconnectDisplay (dpy->trans_conn);
	    Xfree ((char *)dpy);
	    return(NULL);
	}
	if (conn_auth_name) Xfree(conn_auth_name);
	if (conn_auth_data) Xfree(conn_auth_data);
/*
 * Now see if connection was accepted...
 */
	/* these internal functions expect the display to be locked */
	LockDisplay(dpy);

	if (prefixread == 0)
	    _XRead (dpy, (char *)&prefix,(long)SIZEOF(xConnSetupPrefix));

	/* an Authenticate reply we weren't expecting? */
	if (prefix.success != xTrue && prefix.success != xFalse) {
	    fprintf (stderr,
      "Xlib: unexpected connection setup reply from server, type %d.\r\n",
		     prefix.success);
	    _XDisconnectDisplay (dpy->trans_conn);
	    Xfree ((char *)dpy);
	    return(NULL);
	}

	if (prefix.majorVersion != X_PROTOCOL) {
	    /* XXX - printing messages marks a bad programming interface */
	    fprintf (stderr,
     "Xlib: client uses different protocol version (%d) than server (%d)!\r\n",
		     X_PROTOCOL, prefix.majorVersion);
	    _XDisconnectDisplay (dpy->trans_conn);
	    Xfree ((char *)dpy);
	    return(NULL);
	}
#endif /* !USE_XCB */

#if USE_XCB
	{
		const struct xcb_setup_t *xcbsetup = xcb_get_setup(dpy->xcb->connection);
		memcpy(&prefix, xcbsetup, sizeof(prefix));
		setuplength = prefix.length << 2;
		setup = (char *) xcbsetup;
		setup += SIZEOF(xConnSetupPrefix);
		u.setup = (xConnSetup *) setup;
	}
#else /* !USE_XCB */
	setuplength = prefix.length << 2;
	if ( (u.setup = (xConnSetup *)
	      (setup =  Xmalloc ((unsigned) setuplength))) == NULL) {
		_XDisconnectDisplay (dpy->trans_conn);
		Xfree ((char *)dpy);
		return(NULL);
	}
	_XRead (dpy, (char *)u.setup, setuplength);

/*
 * If the connection was not accepted by the server due to problems,
 * give error message to the user....
 */
	if (prefix.success != xTrue) {
		/* XXX - printing messages marks a bad programming interface */
		fprintf (stderr,
		      "Xlib: connection to \"%s\" refused by server\r\nXlib: ",
			 fullname);

		if (prefix.lengthReason > setuplength) {
		    fprintf (stderr, "Xlib: Broken initial reply: length of reason > length of packet\r\n");
		}else{
		    (void) fwrite (u.failure, (Size_t)sizeof(char),
			       (Size_t)prefix.lengthReason, stderr);
		    (void) fwrite ("\r\n", sizeof(char), 2, stderr);
		}

		OutOfMemory(dpy, setup);
		return (NULL);
	}
#endif /* USE_XCB */

/*
 * Check if the reply was long enough to get any information out of it.
 */
	usedbytes = sz_xConnSetup;
	if (setuplength < usedbytes ) {
	    fprintf (stderr, "Xlib: Broken initial reply: Too short (%ld)\n", setuplength);
	    OutOfMemory(dpy, setup);
	    return (NULL);
	}

/*
 * We succeeded at authorization, so let us move the data into
 * the display structure.
 */
	dpy->proto_major_version= prefix.majorVersion;
	dpy->proto_minor_version= prefix.minorVersion;
	dpy->release 		= u.setup->release;
	dpy->resource_base	= u.setup->ridBase;
	dpy->resource_mask	= u.setup->ridMask;
	dpy->min_keycode	= u.setup->minKeyCode;
	dpy->max_keycode	= u.setup->maxKeyCode;
	dpy->motion_buffer	= u.setup->motionBufferSize;
	dpy->nformats		= u.setup->numFormats;
	dpy->nscreens		= u.setup->numRoots;
	dpy->byte_order		= u.setup->imageByteOrder;
	dpy->bitmap_unit	= u.setup->bitmapScanlineUnit;
	dpy->bitmap_pad		= u.setup->bitmapScanlinePad;
	dpy->bitmap_bit_order   = u.setup->bitmapBitOrder;
	dpy->max_request_size	= u.setup->maxRequestSize;
	mask = dpy->resource_mask;
	dpy->resource_shift	= 0;
	if (!mask)
	{
	    fprintf (stderr, "Xlib: connection to \"%s\" invalid setup\n",
		     fullname);
	    OutOfMemory(dpy, setup);
	    return (NULL);
	}

	while (!(mask & 1)) {
	    dpy->resource_shift++;
	    mask = mask >> 1;
	}
	dpy->resource_max = (dpy->resource_mask >> dpy->resource_shift) - 5;
/*
 * now extract the vendor string...  String must be null terminated,
 * padded to multiple of 4 bytes.
 */
	/* Check for a sane vendor string length */
	if (u.setup->nbytesVendor > 256) {
	    OutOfMemory(dpy, setup);
	    return (NULL);
	}

	dpy->vendor = (char *) Xmalloc((unsigned) (u.setup->nbytesVendor + 1));
	if (dpy->vendor == NULL) {
	    OutOfMemory(dpy, setup);
	    return (NULL);
	}
	vendorlen = u.setup->nbytesVendor;

/*
 * validate setup length
 */
	usedbytes += (vendorlen + 3) & ~3;
	if (setuplength < usedbytes) {
	    fprintf (stderr, "Xlib: Broken initial reply: Too short (%ld)\n", setuplength);
	    OutOfMemory(dpy, setup);
	    return (NULL);
	}

 	u.setup = (xConnSetup *) (((char *) u.setup) + sz_xConnSetup);
  	(void) strncpy(dpy->vendor, u.vendor, vendorlen);
	dpy->vendor[vendorlen] = '\0';
 	vendorlen = (vendorlen + 3) & ~3;	/* round up */
	u.vendor += vendorlen;

/*
 * Now iterate down setup information.....
 */
	dpy->pixmap_format =
	    (ScreenFormat *)Xmalloc(
		(unsigned) (dpy->nformats *sizeof(ScreenFormat)));
	if (dpy->pixmap_format == NULL) {
	        OutOfMemory (dpy, setup);
		return(NULL);
	}
/*
 * First decode the Z axis Screen format information.
 */
	usedbytes += dpy->nformats * sz_xPixmapFormat;

	if (setuplength < usedbytes) {
	    fprintf (stderr, "Xlib: Broken initial reply: Too short (%ld)\n", setuplength);
	    OutOfMemory (dpy, setup);
	    return(NULL);
	}

	for (i = 0; i < dpy->nformats; i++) {
	    register ScreenFormat *fmt = &dpy->pixmap_format[i];
	    fmt->depth = u.sf->depth;
	    fmt->bits_per_pixel = u.sf->bitsPerPixel;
	    fmt->scanline_pad = u.sf->scanLinePad;
	    fmt->ext_data = NULL;
	    u.sf = (xPixmapFormat *) (((char *) u.sf) + sz_xPixmapFormat);
	}

/*
 * next the Screen structures.
 */
	dpy->screens =
	    (Screen *)Xmalloc((unsigned) dpy->nscreens*sizeof(Screen));
	if (dpy->screens == NULL) {
	        OutOfMemory (dpy, setup);
		return(NULL);
	}

/*
 * Now go deal with each screen structure.
 */
	for (i = 0; i < dpy->nscreens; i++) {
	    register Screen *sp = &dpy->screens[i];
	    VisualID root_visualID;

	    usedbytes += sz_xWindowRoot;
	    if (setuplength < usedbytes) {
		fprintf (stderr, "Xlib: Broken initial reply: Too short (%ld)\n", setuplength);
		OutOfMemory (dpy, setup);
		return(NULL);
	    }

	    root_visualID = u.rp->rootVisualID;
	    sp->display	    = dpy;
	    sp->root 	    = u.rp->windowId;
	    sp->cmap 	    = u.rp->defaultColormap;
	    sp->white_pixel = u.rp->whitePixel;
	    sp->black_pixel = u.rp->blackPixel;
	    sp->root_input_mask = u.rp->currentInputMask;
	    sp->width	    = u.rp->pixWidth;
	    sp->height	    = u.rp->pixHeight;
	    sp->mwidth	    = u.rp->mmWidth;
	    sp->mheight	    = u.rp->mmHeight;
	    sp->min_maps    = u.rp->minInstalledMaps;
	    sp->max_maps    = u.rp->maxInstalledMaps;
	    sp->backing_store= u.rp->backingStore;
	    sp->save_unders = u.rp->saveUnders;
	    sp->root_depth  = u.rp->rootDepth;
	    sp->ndepths	    = u.rp->nDepths;
	    sp->ext_data   = NULL;
	    u.rp = (xWindowRoot *) (((char *) u.rp) + sz_xWindowRoot);
/*
 * lets set up the depth structures.
 */
	    sp->depths = (Depth *)Xmalloc(
			(unsigned)sp->ndepths*sizeof(Depth));
	    if (sp->depths == NULL) {
		OutOfMemory (dpy, setup);
		return(NULL);
	    }
	    /*
	     * for all depths on this screen.
	     */
	    for (j = 0; j < sp->ndepths; j++) {
		Depth *dp = &sp->depths[j];

		usedbytes += sz_xDepth;
		if (setuplength < usedbytes) {
		    fprintf (stderr, "Xlib: Broken initial reply: Too short (%ld)\n", setuplength);
		    OutOfMemory (dpy, setup);
		    return(NULL);
		}

		dp->depth = u.dp->depth;
		dp->nvisuals = u.dp->nVisuals;
		u.dp = (xDepth *) (((char *) u.dp) + sz_xDepth);
		if (dp->nvisuals > 0) {
		    dp->visuals =
		      (Visual *)Xmalloc((unsigned)dp->nvisuals*sizeof(Visual));
		    if (dp->visuals == NULL) {
			OutOfMemory (dpy, setup);
			return(NULL);
		    }
		    for (k = 0; k < dp->nvisuals; k++) {
			register Visual *vp = &dp->visuals[k];

			usedbytes += sz_xVisualType;
			if (setuplength < usedbytes) {
			    fprintf (stderr, "Xlib: Broken initial reply: Too short (%ld)\n", setuplength);
			    OutOfMemory (dpy, setup);
			    return(NULL);
			}

			vp->visualid	= u.vp->visualID;
			vp->class	= u.vp->class;
			vp->bits_per_rgb= u.vp->bitsPerRGB;
			vp->map_entries	= u.vp->colormapEntries;
			vp->red_mask	= u.vp->redMask;
			vp->green_mask	= u.vp->greenMask;
			vp->blue_mask	= u.vp->blueMask;
			vp->ext_data	= NULL;
			u.vp = (xVisualType *) (((char *) u.vp) +
						sz_xVisualType);
		    }
		    if (dp->depth == 32 && getenv ("XLIB_SKIP_ARGB_VISUALS"))
		    {
			Xfree (dp->visuals);
			dp->visuals = NULL;
			dp->nvisuals = 0;
		    }
		} else {
		    dp->visuals = (Visual *) NULL;
		}
	    }
	    sp->root_visual = _XVIDtoVisual(dpy, root_visualID);
	}

	if(usedbytes != setuplength){
	    /* Sanity check, shouldn't happen. */
	    fprintf(stderr, "Xlib: Did not parse entire setup message: "
	                    "parsed: %ld, message: %ld\n",
		    usedbytes, setuplength);
	    OutOfMemory(dpy, setup);
	    return(NULL);
	}

/*
 * Now start talking to the server to setup all other information...
 */

#if !USE_XCB
	Xfree (setup);	/* all finished with setup information */
#endif /* !USE_XCB */

/*
 * Make sure default screen is legal.
 */
	if (iscreen >= dpy->nscreens) {
	    OutOfMemory(dpy, (char *) NULL);
	    return(NULL);
	}

#if !USE_XCB
/*
 * finished calling internal routines, now unlock for external routines
 */
	UnlockDisplay(dpy);
#endif /* !USE_XCB */

#if USE_XCB
	dpy->bigreq_size = xcb_get_maximum_request_length(dpy->xcb->connection);
	if(dpy->bigreq_size <= dpy->max_request_size)
		dpy->bigreq_size = 0;
#endif /* USE_XCB */

/*
 * Set up other stuff clients are always going to use.
 */
	for (i = 0; i < dpy->nscreens; i++) {
	    register Screen *sp = &dpy->screens[i];
	    XGCValues values;
	    values.foreground = sp->black_pixel;
	    values.background = sp->white_pixel;
	    if ((sp->default_gc = XCreateGC (dpy, sp->root,
					     GCForeground|GCBackground,
					     &values)) == NULL) {
		OutOfMemory(dpy, (char *) NULL);
		return (NULL);
	    }
	}
/*
 * call into synchronization routine so that all programs can be
 * forced synchronous
 */
	(void) XSynchronize(dpy, _Xdebug);

/*
 * get availability of large requests, and
 * get the resource manager database off the root window.
 */
	LockDisplay(dpy);
	{
	    xGetPropertyReply reply;
	    xGetPropertyReq *req;
#if !USE_XCB
	    _XAsyncHandler async;
	    _XBigReqState async_state;
	    xQueryExtensionReq *qreq;
	    xBigReqEnableReq *breq;
	    xBigReqEnableReply brep;

	    GetReq(QueryExtension, qreq);
	    async_state.seq = dpy->request;
	    async_state.opcode = 0;
	    async.next = dpy->async_handlers;
	    async.handler = _XBigReqHandler;
	    async.data = (XPointer)&async_state;
	    dpy->async_handlers = &async;
	    qreq->nbytes = bignamelen;
	    qreq->length += (bignamelen+3)>>2;
	    Data(dpy, XBigReqExtensionName, bignamelen);
#endif /* !USE_XCB */

	    GetReq (GetProperty, req);
	    req->window = RootWindow(dpy, 0);
	    req->property = XA_RESOURCE_MANAGER;
	    req->type = XA_STRING;
	    req->delete = False;
	    req->longOffset = 0;
	    req->longLength = 100000000L;

	    if (_XReply (dpy, (xReply *) &reply, 0, xFalse)) {
		if (reply.format == 8 && reply.propertyType == XA_STRING &&
		    (reply.nItems + 1 > 0) &&
		    (reply.nItems <= req->longLength * 4) &&
		    (dpy->xdefaults = Xmalloc (reply.nItems + 1))) {
		    _XReadPad (dpy, dpy->xdefaults, reply.nItems);
		    dpy->xdefaults[reply.nItems] = '\0';
		}
		else if (reply.propertyType != None)
		    _XEatData(dpy, reply.nItems * (reply.format >> 3));
	    }
#if !USE_XCB
	    DeqAsyncHandler(dpy, &async);
	    if (async_state.opcode) {
		GetReq(BigReqEnable, breq);
		breq->reqType = async_state.opcode;
		breq->brReqType = X_BigReqEnable;
		if (_XReply(dpy, (xReply *)&brep, 0, xFalse))
		    dpy->bigreq_size = brep.max_request_size;
	    }
#endif /* !USE_XCB */
	}
	UnlockDisplay(dpy);

#ifdef MOTIFBC
	{
	    extern Display *_XHeadOfDisplayList;
	    _XHeadOfDisplayList = dpy;
	}
#endif
#ifdef XKB
	XkbUseExtension(dpy,NULL,NULL);
#endif
/*
 * and return successfully
 */
 	return(dpy);
}

#if !USE_XCB
static Bool
_XBigReqHandler(
    register Display *dpy,
    register xReply *rep,
    char *buf,
    int len,
    XPointer data)
{
    _XBigReqState *state;
    xQueryExtensionReply replbuf;
    xQueryExtensionReply *repl;

    state = (_XBigReqState *)data;
    if (dpy->last_request_read != state->seq)
	return False;
    if (rep->generic.type == X_Error)
	return True;
    repl = (xQueryExtensionReply *)
	_XGetAsyncReply(dpy, (char *)&replbuf, rep, buf, len,
			(SIZEOF(xQueryExtensionReply) - SIZEOF(xReply)) >> 2,
			True);
    if (repl->present)
	state->opcode = repl->major_opcode;
    return True;
}
#endif /* !USE_XCB */


/* XFreeDisplayStructure frees all the storage associated with a
 * Display.  It is used by XOpenDisplay if it runs out of memory,
 * and also by XCloseDisplay.   It needs to check whether all pointers
 * are non-NULL before dereferencing them, since it may be called
 * by XOpenDisplay before the Display structure is fully formed.
 * XOpenDisplay must be sure to initialize all the pointers to NULL
 * before the first possible call on this.
 */

void _XFreeDisplayStructure(Display *dpy)
{
	while (dpy->ext_procs) {
	    _XExtension *ext = dpy->ext_procs;
	    dpy->ext_procs = ext->next;
	    if (ext->name)
		Xfree (ext->name);
	    Xfree ((char *)ext);
	}
	if (dpy->im_filters)
	   (*dpy->free_funcs->im_filters)(dpy);
	if (dpy->cms.clientCmaps)
	   (*dpy->free_funcs->clientCmaps)(dpy);
	if (dpy->cms.defaultCCCs)
	   (*dpy->free_funcs->defaultCCCs)(dpy);
	if (dpy->cms.perVisualIntensityMaps)
	   (*dpy->free_funcs->intensityMaps)(dpy);
	if (dpy->atoms)
	    (*dpy->free_funcs->atoms)(dpy);
	if (dpy->modifiermap)
	   (*dpy->free_funcs->modifiermap)(dpy->modifiermap);
	if (dpy->key_bindings)
	   (*dpy->free_funcs->key_bindings)(dpy);
	if (dpy->context_db)
	   (*dpy->free_funcs->context_db)(dpy);
	if (dpy->xkb_info)
	   (*dpy->free_funcs->xkb)(dpy);

	/* if RM database was allocated by XGetDefault() free it */
	if (dpy->db && (dpy->flags & XlibDisplayDfltRMDB))
	    XrmDestroyDatabase(dpy->db);

	if (dpy->screens) {
	    register int i;

            for (i = 0; i < dpy->nscreens; i++) {
		Screen *sp = &dpy->screens[i];

		if (sp->depths) {
		   register int j;

		   for (j = 0; j < sp->ndepths; j++) {
			Depth *dp = &sp->depths[j];

			if (dp->visuals) {
			   register int k;

			   for (k = 0; k < dp->nvisuals; k++)
			     _XFreeExtData (dp->visuals[k].ext_data);
			   Xfree ((char *) dp->visuals);
			   }
			}

		   Xfree ((char *) sp->depths);
		   }

		_XFreeExtData (sp->ext_data);
		}

	    Xfree ((char *)dpy->screens);
	    }

	if (dpy->pixmap_format) {
	    register int i;

	    for (i = 0; i < dpy->nformats; i++)
	      _XFreeExtData (dpy->pixmap_format[i].ext_data);
            Xfree ((char *)dpy->pixmap_format);
	    }

	if (dpy->display_name)
	   Xfree (dpy->display_name);
	if (dpy->vendor)
	   Xfree (dpy->vendor);

        if (dpy->buffer)
	   Xfree (dpy->buffer);
	if (dpy->keysyms)
	   Xfree ((char *) dpy->keysyms);
	if (dpy->xdefaults)
	   Xfree (dpy->xdefaults);
	if (dpy->error_vec)
	    Xfree ((char *)dpy->error_vec);

	_XFreeExtData (dpy->ext_data);
	if (dpy->free_funcs)
	    Xfree ((char *)dpy->free_funcs);
 	if (dpy->scratch_buffer)
 	    Xfree (dpy->scratch_buffer);
	FreeDisplayLock(dpy);

	if (dpy->qfree) {
	    register _XQEvent *qelt = dpy->qfree;

	    while (qelt) {
		register _XQEvent *qnxt = qelt->next;
		Xfree ((char *) qelt);
		qelt = qnxt;
	    }
	}
	while (dpy->im_fd_info) {
	    struct _XConnectionInfo *conni = dpy->im_fd_info;
	    dpy->im_fd_info = conni->next;
	    if (conni->watch_data)
		Xfree (conni->watch_data);
	    Xfree (conni);
	}
	if (dpy->conn_watchers) {
	    struct _XConnWatchInfo *watcher = dpy->conn_watchers;
	    dpy->conn_watchers = watcher->next;
	    Xfree (watcher);
	}
	if (dpy->filedes)
	    Xfree (dpy->filedes);

#if USE_XCB
	_XFreeX11XCBStructure(dpy);
#endif /* USE_XCB */

	Xfree ((char *)dpy);
}

/* OutOfMemory is called if malloc fails.  XOpenDisplay returns NULL
   after this returns. */

static void OutOfMemory(Display *dpy, char *setup)
{
#if USE_XCB
    if(dpy->xcb->connection)
	xcb_disconnect(dpy->xcb->connection);
#else /* !USE_XCB */
    _XDisconnectDisplay (dpy->trans_conn);
#endif /* USE_XCB */
    _XFreeDisplayStructure (dpy);
#if !USE_XCB
    if (setup) Xfree (setup);
#endif /* !USE_XCB */
}
