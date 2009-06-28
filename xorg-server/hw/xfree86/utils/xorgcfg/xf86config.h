/*
 * Copyright (c) 2000 by Conectiva S.A. (http://www.conectiva.com)
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
 * CONECTIVA LINUX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * Except as contained in this notice, the name of Conectiva Linux shall
 * not be used in advertising or otherwise to promote the sale, use or other
 * dealings in this Software without prior written authorization from
 * Conectiva Linux.
 *
 * Author: Paulo César Pereira de Andrade <pcpa@conectiva.com.br>
 *
 */

#include "config.h"

#ifndef _xf86cfg_xf86config_h
#define _xf86cfg_xf86config_h

#define xf86addInput(head, ptr)		\
	(XF86ConfInputPtr)xf86addListItem((GenericListPtr)(head), (GenericListPtr)(ptr))
#define xf86addInputref(head, ptr)	\
	(XF86ConfInputrefPtr)xf86addListItem((GenericListPtr)(head), (GenericListPtr)(ptr))
#define xf86addDevice(head, ptr)	\
	(XF86ConfDevicePtr)xf86addListItem((GenericListPtr)(head), (GenericListPtr)(ptr))
#define xf86addDisplayMode(head, ptr)	\
	(XF86ModePtr)xf86addListItem((GenericListPtr)(head), (GenericListPtr)(ptr))
#define xf86addMonitor(head, ptr)	\
	(XF86ConfMonitorPtr)xf86addListItem((GenericListPtr)(head), (GenericListPtr)(ptr))
#define xf86addScreen(head, ptr)	\
	(XF86ConfScreenPtr)xf86addListItem((GenericListPtr)(head), (GenericListPtr)(ptr))
#define xf86addLayout(head, ptr)	\
	(XF86ConfLayoutPtr)xf86addListItem((GenericListPtr)(head), (GenericListPtr)(ptr))
#define xf86addModeLine(head, ptr)	\
	(XF86ConfModeLinePtr)xf86addListItem((GenericListPtr)(head), (GenericListPtr)(ptr))
#define xf86addModes(head, ptr)		\
	(XF86ConfModesPtr)xf86addListItem((GenericListPtr)(head), (GenericListPtr)(ptr))
#define xf86addModesLink(head, ptr)	\
	(XF86ConfModesLinkPtr)xf86addListItem((GenericListPtr)(head), (GenericListPtr)(ptr))
#define xf86addModule(head, ptr)	\
	(XF86LoadPtr)xf86addListItem((GenericListPtr)(head), (GenericListPtr)(ptr))
#define xf86addScreenAdaptor(head, ptr)	\
	(XF86ConfAdaptorLinkPtr)xf86addListItem((GenericListPtr)(head), (GenericListPtr)(ptr))
#define xf86addScreenDisplay(head, ptr)	\
	(XF86ConfDisplayPtr)xf86addListItem((GenericListPtr)(head), (GenericListPtr)(ptr))
#define xf86addVideoAdaptor(head, ptr)	\
	(XF86ConfVideoAdaptorPtr)xf86addListItem((GenericListPtr)(head), (GenericListPtr)(ptr))
#define xf86addVideoPort(head, ptr)	\
	(XF86ConfVideoPortPtr)xf86addListItem((GenericListPtr)(head), (GenericListPtr)(ptr))
#define xf86addAdjacency(head, ptr)	\
	(XF86ConfAdjacencyPtr)xf86addListItem((GenericListPtr)(head), (GenericListPtr)(ptr))
#define xf86addVendor(head, ptr)	\
	(XF86ConfVendorPtr)xf86addListItem((GenericListPtr)(head), (GenericListPtr)(ptr))
#define xf86addVendorSub(head, ptr)	\
	(XF86ConfVendSubPtr)xf86addListItem((GenericListPtr)(head), (GenericListPtr)(ptr))
#define xf86addBuffers(head, ptr)	\
	(XF86ConfBuffersPtr)xf86addListItem((GenericListPtr)(head), (GenericListPtr)(ptr))


int xf86removeOption(XF86OptionPtr*, char*);
int xf86removeInput(XF86ConfigPtr, XF86ConfInputPtr);
int xf86removeInputRef(XF86ConfLayoutPtr, XF86ConfInputPtr);
int xf86removeDevice(XF86ConfigPtr, XF86ConfDevicePtr);
int xf86removeDisplayMode(XF86ConfDisplayPtr, XF86ModePtr);
int xf86removeMonitor(XF86ConfigPtr, XF86ConfMonitorPtr);
int xf86removeScreen(XF86ConfigPtr, XF86ConfScreenPtr);
int xf86removeAdjacency(XF86ConfLayoutPtr, XF86ConfAdjacencyPtr);
int xf86removeInactive(XF86ConfLayoutPtr, XF86ConfInactivePtr);
int xf86removeLayout(XF86ConfigPtr, XF86ConfLayoutPtr);
int xf86removeModule(XF86ConfigPtr, XF86LoadPtr);
int xf86removeModes(XF86ConfigPtr, XF86ConfModesPtr);
int xf86removeModesModeLine(XF86ConfModesPtr, XF86ConfModeLinePtr);
int xf86removeMonitorModeLine(XF86ConfMonitorPtr, XF86ConfModeLinePtr);
int xf86removeMonitorModesLink(XF86ConfMonitorPtr, XF86ConfModesLinkPtr);
int xf86removeScreenAdaptorLink(XF86ConfScreenPtr, XF86ConfAdaptorLinkPtr);
int xf86removeScreenDisplay(XF86ConfScreenPtr, XF86ConfDisplayPtr);
int xf86removeVideoAdaptor(XF86ConfigPtr, XF86ConfVideoAdaptorPtr);
int xf86removeVideoPort(XF86ConfVideoAdaptorPtr, XF86ConfVideoPortPtr);
int xf86removeVendor(XF86ConfigPtr, XF86ConfVendorPtr);
int xf86removeVendorSub(XF86ConfVendorPtr, XF86ConfVendSubPtr);
int xf86removeBuffers(XF86ConfDRIPtr, XF86ConfBuffersPtr);

int xf86renameInput(XF86ConfigPtr, XF86ConfInputPtr, char*);
int xf86renameDevice(XF86ConfigPtr, XF86ConfDevicePtr, char*);
int xf86renameMonitor(XF86ConfigPtr, XF86ConfMonitorPtr, char*);
int xf86renameLayout(XF86ConfigPtr, XF86ConfLayoutPtr, char*);
int xf86renameScreen(XF86ConfigPtr, XF86ConfScreenPtr, char*);

extern void xf86freeAdaptorLinkList(XF86ConfAdaptorLinkPtr);
extern void xf86freeDisplayList(XF86ConfDisplayPtr);
extern void xf86freeModeList(XF86ModePtr);
extern void xf86freeVendorSubList(XF86ConfVendSubPtr);

#endif /* _xf86cfg_xf86config_h */
