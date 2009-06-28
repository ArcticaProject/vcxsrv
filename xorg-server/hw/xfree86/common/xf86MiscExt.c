/*
 * Copyright (c) 1999-2003 by The XFree86 Project, Inc.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the copyright holder(s)
 * and author(s) shall not be used in advertising or otherwise to promote
 * the sale, use or other dealings in this Software without prior written
 * authorization from the copyright holder(s) and author(s).
 */

/*
 * This file contains the Pointer/Keyboard functions needed by the 
 * XFree86-Misc extension.
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <X11/X.h>
#include "os.h"
#include "xf86.h"
#include "xf86Priv.h"

#ifdef XF86MISC
#define _XF86MISC_SERVER_
#include <X11/extensions/xf86misc.h>
#include "xf86miscproc.h"
#endif

#define XF86_OS_PRIVS
#include "xf86_OSlib.h"

#ifdef XINPUT
#include <X11/extensions/XI.h>
#include <X11/extensions/XIproto.h>
#include "xf86Xinput.h"
#else
#include "inputstr.h"
#endif

#include "xf86OSmouse.h"

#ifdef DEBUG
# define DEBUG_P(x) ErrorF(x"\n");
#else
# define DEBUG_P(x) /**/
#endif

#ifdef XF86MISC

typedef struct {
	int	type;
	int	baudrate;
	int	samplerate;
	int	resolution;
	int	buttons;
	Bool	em3buttons;
	int	em3timeout;
	Bool	chordmiddle;
	int	flags;
        char*   device;
        pointer	private;
} mseParamsRec, *mseParamsPtr;

typedef struct {
	int	type;
	int	rate;
	int	delay;
	int	serverNumLock;	/* obsolete */
        pointer private;
} kbdParamsRec, *kbdParamsPtr;

/*
    Sigh...

    The extension should probably be changed to use protocol
    names instead of ID numbers
*/
static struct mouse_map {
    int mtype;
    MouseProtocolID proto;
} m_map[] =
{
	{ MTYPE_MICROSOFT, PROT_MS },
	{ MTYPE_MOUSESYS, PROT_MSC },
	{ MTYPE_MMSERIES, PROT_MM },
	{ MTYPE_LOGITECH, PROT_LOGI },
	{ MTYPE_LOGIMAN, PROT_LOGIMAN },
	{ MTYPE_MMHIT, PROT_MMHIT },
	{ MTYPE_GLIDEPOINT, PROT_GLIDE },
	{ MTYPE_IMSERIAL, PROT_IMSERIAL },
	{ MTYPE_THINKING, PROT_THINKING },
	{ MTYPE_ACECAD, PROT_ACECAD },
	{ MTYPE_PS_2, PROT_PS2 },
	{ MTYPE_IMPS2, PROT_IMPS2 },
	{ MTYPE_EXPPS2, PROT_EXPPS2 },
	{ MTYPE_THINKINGPS2, PROT_THINKPS2 },
	{ MTYPE_MMANPLUSPS2, PROT_MMPS2 },
	{ MTYPE_GLIDEPOINTPS2, PROT_GLIDEPS2 },
	{ MTYPE_NETPS2, PROT_NETPS2 },
	{ MTYPE_NETSCROLLPS2, PROT_NETSCPS2 },
	{ MTYPE_BUSMOUSE, PROT_BM },
	{ MTYPE_AUTOMOUSE, PROT_AUTO },
	{ MTYPE_SYSMOUSE, PROT_SYSMOUSE },
	{ MTYPE_UNKNOWN, PROT_UNKNOWN }
};

static int
MapMseProtoToMisc(MouseProtocolID proto)
{
    int i;
    
	for (i = 0; m_map[i].proto != PROT_UNKNOWN; i++)
	if (proto == m_map[i].proto)
	    return m_map[i].mtype;

	return MTYPE_UNKNOWN;
}

static MouseProtocolID
MapMseMiscToProto(int proto)
{
    int i;

	for (i = 0; m_map[i].mtype != MTYPE_UNKNOWN; i++)
	if (proto == m_map[i].mtype)
	    return m_map[i].proto;

	return PROT_UNKNOWN;
}

_X_EXPORT Bool
MiscExtGetMouseSettings(pointer *mouse, char **devname)
{
    mseParamsPtr mseptr;

    mseptr = MiscExtCreateStruct(MISC_POINTER);
    if (!mseptr)
	return FALSE;

    {
	InputInfoPtr pInfo = mseptr->private;
	MouseDevPtr pMse;

	*devname = xf86FindOptionValue(pInfo->options, "Device");
	pMse = pInfo->private;

	mseptr->type =		MapMseProtoToMisc(pMse->protocolID);
	mseptr->baudrate =	pMse->baudRate;
	mseptr->samplerate =	pMse->sampleRate;
	mseptr->resolution =	pMse->resolution;
	mseptr->buttons =	pMse->buttons;
	mseptr->em3buttons =	pMse->emulate3Buttons;
	mseptr->em3timeout =	pMse->emulate3Timeout;
	mseptr->chordmiddle =	pMse->chordMiddle;
	mseptr->flags =		pMse->mouseFlags;
    }
    *mouse = mseptr;
    return TRUE;
}

_X_EXPORT int
MiscExtGetMouseValue(pointer mouse, MiscExtMseValType valtype)
{
    mseParamsPtr mse = mouse;

    switch (valtype) {
	case MISC_MSE_PROTO:		return mse->type;
	case MISC_MSE_BAUDRATE:		return mse->baudrate;
	case MISC_MSE_SAMPLERATE:	return mse->samplerate;
	case MISC_MSE_RESOLUTION:	return mse->resolution;
	case MISC_MSE_BUTTONS:		return mse->buttons;
	case MISC_MSE_EM3BUTTONS:	return mse->em3buttons;
	case MISC_MSE_EM3TIMEOUT:	return mse->em3timeout;
	case MISC_MSE_CHORDMIDDLE:	return mse->chordmiddle;
	case MISC_MSE_FLAGS:		return mse->flags;
    }
    return 0;
}

_X_EXPORT Bool
MiscExtSetMouseValue(pointer mouse, MiscExtMseValType valtype, int value)
{
    mseParamsPtr mse = mouse;

    switch (valtype) {
	case MISC_MSE_PROTO:
	    mse->type = value;
		return TRUE;
	case MISC_MSE_BAUDRATE:
		mse->baudrate = value;
		return TRUE;
	case MISC_MSE_SAMPLERATE:
		mse->samplerate = value;
		return TRUE;
	case MISC_MSE_RESOLUTION:
		mse->resolution = value;
		return TRUE;
	case MISC_MSE_BUTTONS:
		mse->buttons = value;
		return TRUE;
	case MISC_MSE_EM3BUTTONS:
		mse->em3buttons = value;
		return TRUE;
	case MISC_MSE_EM3TIMEOUT:
		mse->em3timeout = value;
		return TRUE;
	case MISC_MSE_CHORDMIDDLE:
		mse->chordmiddle = value;
		return TRUE;
	case MISC_MSE_FLAGS:
		mse->flags = value;
		return TRUE;
    }
    return FALSE;
}

_X_EXPORT Bool
MiscExtSetMouseDevice(pointer mouse, char* device)
{
    mseParamsPtr mse = mouse;
    mse->device = device;
    
    return TRUE;
}

_X_EXPORT Bool
MiscExtGetKbdSettings(pointer *kbd)
{
    return FALSE;
}

_X_EXPORT int
MiscExtGetKbdValue(pointer keyboard, MiscExtKbdValType valtype)
{
    return 0;
}

_X_EXPORT Bool
MiscExtSetKbdValue(pointer keyboard, MiscExtKbdValType valtype, int value)
{
    return FALSE;
}

static void
MiscExtClientStateCallback(CallbackListPtr *callbacks,
			   pointer data, pointer args)
{
    NewClientInfoRec *clientinfo = (NewClientInfoRec*)args;

    if (clientinfo->client == xf86Info.grabInfo.override &&
	clientinfo->client->clientState == ClientStateGone) {
	xf86Info.grabInfo.override = NULL;
	xf86Info.grabInfo.disabled = 0;
	DeleteCallback(&ClientStateCallback, MiscExtClientStateCallback, NULL);
    }
}

#define MiscExtGrabStateSuccess	0	/* No errors */
#define MiscExtGrabStateLocked	1	/* A client already requested that
					 * grabs cannot be removed/killed */
#define MiscExtGrabStateAlready	2	/* Request for enabling/disabling
					 * grab removeal/kill already done */
_X_EXPORT int
MiscExtSetGrabKeysState(ClientPtr client, int state)
{
    if (xf86Info.grabInfo.override == NULL ||
	xf86Info.grabInfo.override == client) {
	if (state == 0 && xf86Info.grabInfo.disabled == 0) {
	    xf86Info.grabInfo.disabled = 1;
	    AddCallback(&ClientStateCallback,
			MiscExtClientStateCallback, NULL);
	    xf86Info.grabInfo.override = client;
	}
	else if (state == 1 && xf86Info.grabInfo.disabled == 1) {
	    xf86Info.grabInfo.disabled = 0;
	    DeleteCallback(&ClientStateCallback,
			   MiscExtClientStateCallback, NULL);
	    xf86Info.grabInfo.override = NULL;
	}
	else
	    return MiscExtGrabStateAlready;

	return MiscExtGrabStateSuccess;
    }

    return MiscExtGrabStateLocked;
}

_X_EXPORT pointer
MiscExtCreateStruct(MiscExtStructType mse_or_kbd)
{
    switch (mse_or_kbd) {
    case MISC_POINTER:
    {
	mseParamsPtr mseptr;
	InputInfoPtr pInfo = xf86InputDevs;
	
	while (pInfo) {
	    if (pInfo->dev == inputInfo.pointer)
		break;
	    pInfo = pInfo->next;
	}
	if (!pInfo)
	    return NULL;
	
	mseptr = xcalloc(sizeof(mseParamsRec),1);
	if (mseptr)
	    mseptr->private = pInfo;
	return mseptr;
    }
    case MISC_KEYBOARD:
	return xcalloc(sizeof(kbdParamsRec),1);
    }
    return 0;
}

_X_EXPORT void
MiscExtDestroyStruct(pointer structure, MiscExtStructType mse_or_kbd)
{
    switch (mse_or_kbd) {
	case MISC_POINTER:
	case MISC_KEYBOARD:
		xfree(structure);
    }
}

static Bool
MiscExtAuthorizeDevice(InputInfoPtr pInfo, char *device)
{
    Bool authorized = FALSE;
    char *elem;
    struct stat dev, dev_list;
    const char *olddev = xf86FindOptionValue(pInfo->options, "Device");
    
    if (stat(device,&dev))
	return FALSE;

    if (!S_ISCHR(dev.st_mode))
	return FALSE;

    if (!stat(olddev,&dev_list)) {
	if (dev_list.st_dev == dev.st_dev
	    && dev_list.st_ino == dev.st_ino) {
	    authorized = TRUE;
	}
    }

    if (!authorized) {
	char *path;

	if (!xf86InputDeviceList
	    || (path = strdup(xf86InputDeviceList)) == NULL) 
	    return FALSE;
	
	elem = strtok(path,",");
	
	while (elem) {
	    
	    if (!stat(elem,&dev_list)) {
		if (dev_list.st_dev == dev.st_dev
		    && dev_list.st_ino == dev.st_ino) {
		    authorized = TRUE;
		    break;

		}
	    }
	    elem = strtok(NULL,",");
	}
	xfree(path);
    }
#if 0
    ErrorF("AUTHORIZED: %s\n",authorized?"Yes":"No");
#endif
    return (authorized);
}

_X_EXPORT MiscExtReturn
MiscExtApply(pointer structure, MiscExtStructType mse_or_kbd)
{
    if (mse_or_kbd == MISC_POINTER) {
	Bool protoChanged = FALSE;
	int oldflags;
	Bool reopen = FALSE;
	MouseProtocolID newProtocol;
	mseParamsPtr mse = structure;
	InputInfoPtr pInfo;
	MouseDevPtr pMse;
	pointer xf86MouseProtocolIDToName
	    = LoaderSymbol("xf86MouseProtocolIDToName");
	if (!xf86MouseProtocolIDToName)
	    return MISC_RET_NOMODULE;
	if (mse->type < MTYPE_MICROSOFT
		|| (mse->type > MTYPE_EXPPS2
		    && (mse->type != MTYPE_OSMOUSE)))
	    return MISC_RET_BADMSEPROTO;
#ifdef OSMOUSE_ONLY
	if (mse->type != MTYPE_OSMOUSE)
	    return MISC_RET_BADMSEPROTO;
#else
	if (mse->type == MTYPE_OSMOUSE)
	    return MISC_RET_BADMSEPROTO;
#endif /* OSMOUSE_ONLY */

	if (mse->em3timeout < 0)
	    return MISC_RET_BADVAL;

	if (mse->type == MTYPE_LOGIMAN
	    && !(mse->baudrate == 0
		 || mse->baudrate == 1200
		 || mse->baudrate == 9600))
	    return MISC_RET_BADBAUDRATE;
	if (mse->type == MTYPE_LOGIMAN && mse->samplerate)
	    return MISC_RET_BADCOMBO;

	if (mse->flags & MF_REOPEN) {
	    reopen = TRUE;
	    mse->flags &= ~MF_REOPEN;
	}
	if (mse->type != MTYPE_OSMOUSE
		&& mse->type != MTYPE_PS_2
		&& mse->type != MTYPE_BUSMOUSE
		&& mse->type != MTYPE_IMPS2
		&& mse->type != MTYPE_THINKINGPS2
		&& mse->type != MTYPE_MMANPLUSPS2
		&& mse->type != MTYPE_GLIDEPOINTPS2
		&& mse->type != MTYPE_NETPS2
		&& mse->type != MTYPE_NETSCROLLPS2
		&& mse->type != MTYPE_SYSMOUSE)
	{
	    if (mse->baudrate % 1200 != 0
		|| (mse->baudrate != 0 && mse->baudrate < 1200)
		|| mse->baudrate > 9600)
		return MISC_RET_BADBAUDRATE;
	}
	if ((mse->flags & (MF_CLEAR_DTR|MF_CLEAR_RTS))
		&& (mse->type != MTYPE_MOUSESYS))
	    return MISC_RET_BADFLAGS;

	if (mse->type != MTYPE_OSMOUSE
		&& mse->type != MTYPE_BUSMOUSE)
	{
	    if (mse->samplerate < 0)
		return MISC_RET_BADVAL;
	}

	if (mse->resolution < 0)
	    return MISC_RET_BADVAL;
	if (mse->chordmiddle)
	{
	    if (mse->em3buttons || !(mse->type == MTYPE_MICROSOFT
				    || mse->type == MTYPE_LOGIMAN) )
		return MISC_RET_BADCOMBO;
	}

	/* XXX - This still needs work */

	pInfo = mse->private;
	pMse = pInfo->private;
	oldflags = pMse->mouseFlags;
	
	newProtocol = MapMseMiscToProto(mse->type);
	protoChanged = pMse->protocolID != newProtocol;
	if (protoChanged
		|| pMse->baudRate != mse->baudrate
		|| pMse->sampleRate != mse->samplerate
		|| pMse->resolution != mse->resolution
		|| pMse->mouseFlags != mse->flags)
	    reopen = TRUE;

	if (mse->device)
	    reopen = TRUE;

	if (reopen)
	    (pMse->device->deviceProc)(pMse->device, DEVICE_CLOSE);
	
	pMse->protocolID      = newProtocol;
	pMse->baudRate        = mse->baudrate;
	pMse->sampleRate      = mse->samplerate;
	pMse->resolution      = mse->resolution;
	pMse->buttons         = mse->buttons;
	pMse->emulate3Buttons = mse->em3buttons;
	pMse->emulate3Timeout = mse->em3timeout;
	pMse->chordMiddle     = mse->chordmiddle;
	pMse->mouseFlags      = mse->flags;

	pMse->protocol = ((const char *(*)(MouseProtocolID))
			  xf86MouseProtocolIDToName)(pMse->protocolID);
	
	if (mse->device) {
	    if (MiscExtAuthorizeDevice(pInfo, mse->device)) {
		xf86ReplaceStrOption(pInfo->options, "Device", mse->device);
	    } else {
		return MISC_RET_BADVAL;
	    }
	}
	
	if (reopen) {
	    /* Only if protocol is changed explicitely disable auto detect */
	    if (protoChanged)
		pMse->autoProbe = FALSE;
	    (pMse->device->deviceProc)(pMse->device, DEVICE_ON);
	}
	/* Set pInfo->options too */
	
       if ((oldflags & MF_CLEAR_DTR) != (pMse->mouseFlags & MF_CLEAR_DTR))
	   xf86ReplaceBoolOption(pInfo->options, "ClearDTR",
				 pMse->mouseFlags | MF_CLEAR_DTR);
       if ((oldflags & MF_CLEAR_RTS) != (pMse->mouseFlags & MF_CLEAR_RTS))
	   xf86ReplaceBoolOption(pInfo->options, "ClearRTS",
				 pMse->mouseFlags | MF_CLEAR_RTS);
    }
    return MISC_RET_BADVAL;
}

_X_EXPORT Bool
MiscExtGetFilePaths(const char **configfile, const char **modulepath,
		    const char **logfile)
{
    *configfile = xf86ConfigFile;
    *modulepath = xf86ModulePath;
    *logfile    = xf86LogFile;

    return TRUE;
}

_X_EXPORT int
MiscExtPassMessage(int scrnIndex, const char *msgtype, const char *msgval,
		   char **retstr)
{
    ScrnInfoPtr pScr = xf86Screens[scrnIndex];

    /* should check this in the protocol, but xf86NumScreens isn't exported */
    if (scrnIndex >= xf86NumScreens)
	return BadValue;

    if (*pScr->HandleMessage == NULL)
	    return BadImplementation;
    return (*pScr->HandleMessage)(scrnIndex, msgtype, msgval, retstr);
}

#endif /* XF86MISC */

