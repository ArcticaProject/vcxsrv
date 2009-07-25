/* $Xorg: imTransR.c,v 1.4 2000/08/17 19:45:16 cpqbld Exp $ */
/******************************************************************

              Copyright 1992 by Sun Microsystems, Inc.
              Copyright 1992, 1993, 1994 by FUJITSU LIMITED

Permission to use, copy, modify, distribute, and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear
in supporting documentation, and that the name of Sun Microsystems, Inc.
and FUJITSU LIMITED not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.
Sun Microsystems, Inc. and FUJITSU LIMITED makes no representations about
the suitability of this software for any purpose.
It is provided "as is" without express or implied warranty.

Sun Microsystems Inc. AND FUJITSU LIMITED DISCLAIMS ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS, IN NO EVENT SHALL Sun Microsystems, Inc. AND FUJITSU LIMITED
BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

  Author: Hideki Hiura (hhiura@Sun.COM) Sun Microsystems, Inc.
          Takashi Fujiwara     FUJITSU LIMITED
                               fujiwara@a80.tech.yk.fujitsu.co.jp

******************************************************************/
/* $XFree86: xc/lib/X11/imTransR.c,v 3.6 2003/04/17 02:39:56 dawes Exp $ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "Xlibint.h"
#include "Xlcint.h"
#include "XimTrInt.h"
#include "Ximint.h"

Public TransportSW _XimTransportRec[] = {
    { "X",          _XimXConf },  /* 1st entry must be X.
					This will be a fallback */
#ifdef TCPCONN
    { "tcp",        _XimTransConf }, /* use X transport lib */
#endif /* TCPCONN */
#if defined(UNIXCONN) || defined(LOCALCONN)
    { "local",      _XimTransConf }, /* use X transport lib */
#endif /* UNIXCONN */
#ifdef DNETCONN
    { "dnet",     _XimTransConf }, /* use X transport lib */
#endif /* DNETCONN */
#ifdef STREAMSCONN
    { "streams",    _XimTransConf }, /* use X transport lib */
#endif /* STREAMSCONN */
    { (char *)NULL, (Bool (*)(Xim, char *))NULL },
};

Public Bool
_XimConnect(Xim im)
{
    return im->private.proto.connect(im);
}

Public Bool
_XimShutdown(Xim im)
{
    return im->private.proto.shutdown(im);
}

Public Bool
_XimWrite(Xim im, INT16 len, XPointer data)
{
    return im->private.proto.write(im, len, data);
}

Private int
_CheckProtocolData(
    Xim		  im,
    char	 *recv_buf)
{
    int		 data_len;

    data_len = (int)(((*((CARD16 *)recv_buf + 1)) * 4) + XIM_HEADER_SIZE);
    return data_len;
}

Private int
_XimReadData(
    Xim		 im,
    INT16	*len,
    XPointer	 buf,
    int		 buf_size)
{
    char	*hold_buf;
    char	*tmp;
    int		 data_len;
    int		 packet_size;
    int		 ret_len;
    register int i;

    if (buf_size < XIM_HEADER_SIZE) {
	*len = (INT16)XIM_HEADER_SIZE;
	return XIM_OVERFLOW;
    }

    bzero(buf, buf_size);
    packet_size = 0;
    data_len = 0;

    if ((hold_buf = im->private.proto.hold_data)) {
	data_len = im->private.proto.hold_data_len;
	if (data_len >= XIM_HEADER_SIZE) {
	    packet_size = _CheckProtocolData(im, hold_buf);
	    if (packet_size > buf_size) {
		*len = (INT16)packet_size;
		return XIM_OVERFLOW;
	    }
	    if (packet_size <= data_len) {
		memcpy(buf, hold_buf, packet_size);
		for (i = packet_size; i < data_len; i++) {
		    if (hold_buf[i])
			break;
		}
		data_len -= i;

		if (data_len) {
		    if (!(tmp = (char *)Xmalloc(data_len))) {
			return XIM_FALSE;
		    }
		    memcpy(tmp, &hold_buf[i], data_len);
		    im->private.proto.hold_data = tmp;
		    im->private.proto.hold_data_len = data_len;
		} else {
		    im->private.proto.hold_data = 0;
		    im->private.proto.hold_data_len = 0;
		}
		Xfree(hold_buf);
		*len = (INT16)packet_size;
		return XIM_TRUE;
	    }
	}
	memcpy(buf, hold_buf, data_len);
	buf_size -= data_len;
	Xfree(hold_buf);
	im->private.proto.hold_data = 0;
	im->private.proto.hold_data_len = 0;
    }

    if (!packet_size) {
	while (data_len < XIM_HEADER_SIZE) {
	    if (!(im->private.proto.read(im,
			(XPointer)&buf[data_len], buf_size, &ret_len))) {
		return XIM_FALSE;
	    }
	    data_len += ret_len;
	    buf_size -= ret_len;
	}
	packet_size = _CheckProtocolData(im, buf);
    }

    if (packet_size > buf_size) {
	if (!(tmp = (char *)Xmalloc(data_len))) {
	    return XIM_FALSE;
	}
	memcpy(tmp, buf, data_len);
	bzero(buf, data_len);
	im->private.proto.hold_data = tmp;
	im->private.proto.hold_data_len = data_len;
	*len = (INT16)packet_size;
	return XIM_OVERFLOW;
    }

    while (data_len < packet_size) {
	if (!(im->private.proto.read(im,
			(XPointer)&buf[data_len], buf_size, &ret_len))) {
	    return XIM_FALSE;
	}
	data_len += ret_len;
	buf_size -= ret_len;
    }

    for (i = packet_size; i < data_len; i++) {
	if (buf[i])
	    break;
    }
    data_len -= i;

    if (data_len) {
	if (!(tmp = (char *)Xmalloc(data_len))) {
	    return XIM_FALSE;
	}
	memcpy(tmp, &buf[i], data_len);
	bzero(&buf[i], data_len);
	im->private.proto.hold_data = tmp;
	im->private.proto.hold_data_len = data_len;
    } else {
	im->private.proto.hold_data = 0;
	im->private.proto.hold_data_len = 0;
    }
    *len = (INT16)packet_size;
    return XIM_TRUE;
}

Private Bool
_XimCallDispatcher(
    Xim		 im,
    INT16	 len,
    XPointer	 data)
{
    return im->private.proto.call_dispatcher(im, len, data);
}

Public int
_XimRead(Xim im, INT16 *len, XPointer buf, int buf_size,
	 Bool (*predicate)(Xim, INT16, XPointer, XPointer), XPointer arg)
{
    INT16	 read_len;
    int		 ret_code;

    for (;;) {
	ret_code = _XimReadData(im, &read_len, buf, buf_size);
	if(ret_code != XIM_TRUE) {
	    return ret_code;
	}
	if ((*predicate)(im, read_len, buf, arg))
	    break;
	if (_XimCallDispatcher(im, read_len, buf))
	    continue;
	_XimError(im, 0, XIM_BadProtocol, (INT16)0, (CARD16)0, (char *)NULL);
    }
    *len = read_len;
    return True;
}

Public Bool
_XimRegisterDispatcher(
    Xim		 im,
    Bool	 (*callback)(
			     Xim, INT16, XPointer, XPointer
			     ),
    XPointer	 call_data)
{
    return im->private.proto.register_dispatcher(im, callback, call_data);
}

Public void
_XimFlush(Xim im)
{
    im->private.proto.flush(im);
    return;
}

Public Bool
_XimFilterWaitEvent(Xim im)
{
    INT16	 read_len;
    CARD32	 reply32[BUFSIZE/4];
    char	*reply = (char *)reply32;
    XPointer	 preply;
    int		 buf_size;
    int		 ret_code;

    buf_size = BUFSIZE;
    ret_code = _XimReadData(im, &read_len, (XPointer)reply, buf_size);
    if(ret_code == XIM_TRUE) {
	preply = reply;
    } else if(ret_code == XIM_OVERFLOW) {
	if(read_len <= 0) {
	    preply = reply;
	} else {
	    buf_size = (int)read_len;
	    preply = (XPointer)Xmalloc(buf_size);
	    ret_code = _XimReadData(im, &read_len, preply, buf_size);
	    if(ret_code != XIM_TRUE) {
		if (preply != reply)
		    Xfree(preply);
		return False;
	    }
	}
    } else {
	return False;
    }
    if (_XimCallDispatcher(im, read_len, preply)) {
	if(reply != preply)
	    Xfree(preply);
	return True;
    }
    _XimError(im, 0, XIM_BadProtocol, (INT16)0, (CARD16)0, (char *)NULL);
    if(reply != preply)
	Xfree(preply);
    return True;
}
