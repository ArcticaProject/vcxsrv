/*
 * Copyright 1995,96 by Metro Link, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Metro Link, Inc. not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Metro Link, Inc. makes no
 * representations about the suitability of this software for any purpose.
 *  It is provided "as is" without express or implied warranty.
 *
 * METRO LINK, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL METRO LINK, INC. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
/*
 * Copyright (c) 1997-2003 by The XFree86 Project, Inc.
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

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <fcntl.h>
#include <setjmp.h>
#include "sym.h"
#include "misc.h"
#include "mi.h"
#include "cursor.h"
#include "mipointer.h"
#include "loaderProcs.h"
#include "xf86Pci.h"
#include "xf86.h"
#include "xf86Resources.h"
#include "xf86_OSproc.h"
#include "xf86Parser.h"
#include "xf86Config.h"
#include "xf86Xinput.h"
#ifdef XV
#include "xf86xv.h"
#include "xf86xvmc.h"
#endif
#include "xf86cmap.h"
#include "xf86fbman.h"
#include "dgaproc.h"
#ifdef DPMSExtension
#include "dpmsproc.h"
#endif
#include "vidmodeproc.h"
#include "loader.h"
#include "xisb.h"
#include "vbe.h"
#ifndef __OpenBSD__
#include "xf86sbusBus.h"
#endif
#include "compiler.h"
#include "xf86Crtc.h"
#include "xf86Modes.h"
#ifdef RANDR
#include "xf86RandR12.h"
#endif
#include "xf86DDC.h"
#include "edid.h"
#include "xf86Cursor.h"
#include "xf86RamDac.h"
#include "BT.h"
#include "IBM.h"
#include "TI.h"

#include "xf86RamDac.h"
#include "BT.h"

#ifndef HAS_GLIBC_SIGSETJMP
#if defined(setjmp) && defined(__GNU_LIBRARY__) && \
    (!defined(__GLIBC__) || (__GLIBC__ < 2) || \
     ((__GLIBC__ == 2) && (__GLIBC_MINOR__ < 3)))
#define HAS_GLIBC_SIGSETJMP 1
#endif
#endif

#if defined(__alpha__)
# ifdef linux
extern unsigned long _bus_base(void);
extern void _outb(char val, unsigned short port);
extern void _outw(short val, unsigned short port);
extern void _outl(int val, unsigned short port);
extern unsigned int _inb(unsigned short port);
extern unsigned int _inw(unsigned short port);
extern unsigned int _inl(unsigned short port);
# endif

# ifdef __FreeBSD__
#  include <sys/types.h>
extern void outb(u_int32_t port, u_int8_t val);
extern void outw(u_int32_t port, u_int16_t val);
extern void outl(u_int32_t port, u_int32_t val);
extern u_int8_t inb(u_int32_t port);
extern u_int16_t inw(u_int32_t port);
extern u_int32_t inl(u_int32_t port);
# endif

extern void *__divl(long, long);
extern void *__reml(long, long);
extern void *__divlu(long, long);
extern void *__remlu(long, long);
extern void *__divq(long, long);
extern void *__divqu(long, long);
extern void *__remq(long, long);
extern void *__remqu(long, long);
#endif

#if defined(__sparc__) && defined(__FreeBSD__)
extern float _Qp_qtos(unsigned int *);
extern double _Qp_qtod(unsigned int *);
extern unsigned long long _Qp_qtoux(unsigned int *);
extern void _Qp_dtoq(unsigned int *, double);
extern void _Qp_uitoq(unsigned int *, unsigned int);
#endif

#if defined(__GNUC__)
extern long __div64(long, long);
extern long __divdf3(long, long);
extern long __divdi3(long, long);
extern long __divsf3(long, long);
extern long __divsi3(long, long);
extern long __moddi3(long, long);
extern long __modsi3(long, long);

extern long __mul64(long, long);
extern long __muldf3(long, long);
extern long __muldi3(long, long);
extern long __mulsf3(long, long);
extern long __mulsi3(long, long);
extern long __udivdi3(long, long);
extern long __udivsi3(long, long);
extern long __umoddi3(long, long);
extern long __umodsi3(long, long);

#pragma weak __div64
#pragma weak __divdf3
#pragma weak __divdi3
#pragma weak __divsf3
#pragma weak __divsi3
#pragma weak __moddi3
#pragma weak __modsi3
#pragma weak __mul64
#pragma weak __muldf3
#pragma weak __muldi3
#pragma weak __mulsf3
#pragma weak __mulsi3
#pragma weak __udivdi3
#pragma weak __udivsi3
#pragma weak __umoddi3
#pragma weak __umodsi3
#endif

#if defined(__arm__) && defined(__linux__)
#include <sys/io.h>
#endif

#if defined(__powerpc__) && defined(linux)
void _restf14();
void _restf17();
void _restf18();
void _restf19();
void _restf20();
void _restf22();
void _restf23();
void _restf24();
void _restf25();
void _restf26();
void _restf27();
void _restf28();
void _restf29();
void _savef14();
void _savef17();
void _savef18();
void _savef19();
void _savef20();
void _savef22();
void _savef23();
void _savef24();
void _savef25();
void _savef26();
void _savef27();
void _savef28();
void _savef29();

/* even if we compile without -DNO_INLINE we still provide
 * the usual port i/o functions for module use
 */

extern volatile unsigned char *ioBase;

/* XXX Should get all of these from elsewhere */
#ifndef linux
extern void outb(IOADDRESS, unsigned char);
extern void outw(IOADDRESS, unsigned short);
extern void outl(IOADDRESS, unsigned int);
extern unsigned int inb(IOADDRESS);
extern unsigned int inw(IOADDRESS);
extern unsigned int inl(IOADDRESS);
#endif
extern void stl_brx(unsigned long, volatile unsigned char *, int);
extern void stw_brx(unsigned short, volatile unsigned char *, int);
extern unsigned long ldl_brx(volatile unsigned char *, int);
extern unsigned short ldw_brx(volatile unsigned char *, int);
#endif

/* XFree86 things */

_X_HIDDEN void *xfree86LookupTab[] = {

    /* Public OSlib functions */
    SYMFUNC(xf86ReadBIOS)
    SYMFUNC(xf86EnableIO)
    SYMFUNC(xf86DisableIO)
    SYMFUNC(xf86LinearVidMem)
    SYMFUNC(xf86CheckMTRR)
    SYMFUNC(xf86MapVidMem)
    SYMFUNC(xf86UnMapVidMem)
    SYMFUNC(xf86MapReadSideEffects)
    SYMFUNC(xf86MapDomainMemory)
    SYMFUNC(xf86UDelay)
    SYMFUNC(xf86SlowBcopy)
    SYMFUNC(xf86SetReallySlowBcopy)
#ifdef __alpha__
    SYMFUNC(xf86SlowBCopyToBus)
    SYMFUNC(xf86SlowBCopyFromBus)
#endif
    SYMFUNC(xf86BusToMem)
    SYMFUNC(xf86MemToBus)
    SYMFUNC(xf86OpenSerial)
    SYMFUNC(xf86SetSerial)
    SYMFUNC(xf86SetSerialSpeed)
    SYMFUNC(xf86ReadSerial)
    SYMFUNC(xf86WriteSerial)
    SYMFUNC(xf86CloseSerial)
    SYMFUNC(xf86WaitForInput)
    SYMFUNC(xf86SerialSendBreak)
    SYMFUNC(xf86FlushInput)
    SYMFUNC(xf86SetSerialModemState)
    SYMFUNC(xf86GetSerialModemState)
    SYMFUNC(xf86SerialModemSetBits)
    SYMFUNC(xf86SerialModemClearBits)
    SYMFUNC(xf86LoadKernelModule)
    SYMFUNC(xf86AgpGARTSupported)
    SYMFUNC(xf86GetAGPInfo)
    SYMFUNC(xf86AcquireGART)
    SYMFUNC(xf86ReleaseGART)
    SYMFUNC(xf86AllocateGARTMemory)
    SYMFUNC(xf86DeallocateGARTMemory)
    SYMFUNC(xf86BindGARTMemory)
    SYMFUNC(xf86UnbindGARTMemory)
    SYMFUNC(xf86EnableAGP)
    SYMFUNC(xf86GARTCloseScreen)
    SYMFUNC(XisbNew)
    SYMFUNC(XisbFree)
    SYMFUNC(XisbRead)
    SYMFUNC(XisbWrite)
    SYMFUNC(XisbTrace)
    SYMFUNC(XisbBlockDuration)

    /* xf86Bus.c */
    SYMFUNC(xf86CheckPciSlot)
    SYMFUNC(xf86ClaimPciSlot)
    SYMFUNC(xf86ClaimFbSlot)
    SYMFUNC(xf86ClaimNoSlot)
    SYMFUNC(xf86ParsePciBusString)
    SYMFUNC(xf86ComparePciBusString)
    SYMFUNC(xf86FormatPciBusNumber)
    SYMFUNC(xf86EnableAccess)
    SYMFUNC(xf86SetCurrentAccess)
    SYMFUNC(xf86IsPrimaryPci)
    SYMFUNC(xf86FreeResList)
    SYMFUNC(xf86ClaimFixedResources)
    SYMFUNC(xf86AddEntityToScreen)
    SYMFUNC(xf86SetEntityInstanceForScreen)
    SYMFUNC(xf86RemoveEntityFromScreen)
    SYMFUNC(xf86GetEntityInfo)
    SYMFUNC(xf86GetNumEntityInstances)
    SYMFUNC(xf86GetDevFromEntity)
    SYMFUNC(xf86GetPciInfoForEntity)
    SYMFUNC(xf86RegisterResources)
    SYMFUNC(xf86CheckPciMemBase)
    SYMFUNC(xf86SetAccessFuncs)
    SYMFUNC(xf86IsEntityPrimary)
    SYMFUNC(xf86SetOperatingState)
    SYMFUNC(xf86FindScreenForEntity)
    SYMFUNC(xf86RegisterStateChangeNotificationCallback)
    SYMFUNC(xf86DeregisterStateChangeNotificationCallback)
    /* Shared Accel Accessor Functions */
    SYMFUNC(xf86GetLastScrnFlag)
    SYMFUNC(xf86SetLastScrnFlag)
    SYMFUNC(xf86IsEntityShared)
    SYMFUNC(xf86SetEntityShared)
    SYMFUNC(xf86IsEntitySharable)
    SYMFUNC(xf86SetEntitySharable)
    SYMFUNC(xf86IsPrimInitDone)
    SYMFUNC(xf86SetPrimInitDone)
    SYMFUNC(xf86ClearPrimInitDone)
    SYMFUNC(xf86AllocateEntityPrivateIndex)
    SYMFUNC(xf86GetEntityPrivate)

    /* xf86Cursor.c */
    SYMFUNC(xf86GetPointerScreenFuncs)

    /* xf86DGA.c */
    /* For drivers */
    SYMFUNC(DGAInit)
    SYMFUNC(DGAReInitModes)
    /* For extmod */
    SYMFUNC(DGAAvailable)
    SYMFUNC(DGAActive)
    SYMFUNC(DGASetMode)
    SYMFUNC(DGASetInputMode)
    SYMFUNC(DGASelectInput)
    SYMFUNC(DGAGetViewportStatus)
    SYMFUNC(DGASetViewport)
    SYMFUNC(DGAInstallCmap)
    SYMFUNC(DGASync)
    SYMFUNC(DGAFillRect)
    SYMFUNC(DGABlitRect)
    SYMFUNC(DGABlitTransRect)
    SYMFUNC(DGAGetModes)
    SYMFUNC(DGAGetOldDGAMode)
    SYMFUNC(DGAGetModeInfo)
    SYMFUNC(DGAChangePixmapMode)
    SYMFUNC(DGACreateColormap)
    SYMFUNC(DGAOpenFramebuffer)
    SYMFUNC(DGACloseFramebuffer)

    /* xf86DPMS.c */
    SYMFUNC(xf86DPMSInit)

    /* xf86Events.c */
    SYMFUNC(SetTimeSinceLastInputEvent)
    SYMFUNC(xf86AddInputHandler)
    SYMFUNC(xf86RemoveInputHandler)
    SYMFUNC(xf86DisableInputHandler)
    SYMFUNC(xf86EnableInputHandler)
    SYMFUNC(xf86AddEnabledDevice)
    SYMFUNC(xf86RemoveEnabledDevice)
    SYMFUNC(xf86InterceptSignals)
    SYMFUNC(xf86InterceptSigIll)
    SYMFUNC(xf86EnableVTSwitch)

    /* xf86Helper.c */
    SYMFUNC(xf86AddDriver)
    SYMFUNC(xf86AddInputDriver)
    SYMFUNC(xf86DeleteDriver)
    SYMFUNC(xf86DeleteInput)
    SYMFUNC(xf86AllocateInput)
    SYMFUNC(xf86AllocateScreen)
    SYMFUNC(xf86DeleteScreen)
    SYMFUNC(xf86AllocateScrnInfoPrivateIndex)
    SYMFUNC(xf86AddPixFormat)
    SYMFUNC(xf86SetDepthBpp)
    SYMFUNC(xf86PrintDepthBpp)
    SYMFUNC(xf86SetWeight)
    SYMFUNC(xf86SetDefaultVisual)
    SYMFUNC(xf86SetGamma)
    SYMFUNC(xf86SetDpi)
    SYMFUNC(xf86SetBlackWhitePixels)
    SYMFUNC(xf86EnableDisableFBAccess)
    SYMFUNC(xf86VDrvMsgVerb)
    SYMFUNC(xf86DrvMsgVerb)
    SYMFUNC(xf86DrvMsg)
    SYMFUNC(xf86MsgVerb)
    SYMFUNC(xf86Msg)
    SYMFUNC(xf86ErrorFVerb)
    SYMFUNC(xf86ErrorF)
    SYMFUNC(xf86TokenToString)
    SYMFUNC(xf86StringToToken)
    SYMFUNC(xf86ShowClocks)
    SYMFUNC(xf86PrintChipsets)
    SYMFUNC(xf86MatchDevice)
    SYMFUNC(xf86MatchPciInstances)
    SYMFUNC(xf86GetVerbosity)
    SYMFUNC(xf86GetVisualName)
    SYMFUNC(xf86GetPix24)
    SYMFUNC(xf86GetDepth)
    SYMFUNC(xf86GetWeight)
    SYMFUNC(xf86GetGamma)
    SYMFUNC(xf86GetFlipPixels)
    SYMFUNC(xf86GetServerName)
    SYMFUNC(xf86ServerIsExiting)
    SYMFUNC(xf86ServerIsOnlyDetecting)
    SYMFUNC(xf86ServerIsOnlyProbing)
    SYMFUNC(xf86ServerIsResetting)
    SYMFUNC(xf86CaughtSignal)
    SYMFUNC(xf86GetVidModeAllowNonLocal)
    SYMFUNC(xf86GetVidModeEnabled)
    SYMFUNC(xf86GetModInDevAllowNonLocal)
    SYMFUNC(xf86GetModInDevEnabled)
    SYMFUNC(xf86GetAllowMouseOpenFail)
    SYMFUNC(xf86IsPc98)
    SYMFUNC(xf86DisableRandR)
    SYMFUNC(xf86GetRotation)
    SYMFUNC(xf86GetModuleVersion)
    SYMFUNC(xf86GetClocks)
    SYMFUNC(xf86SetPriority)
    SYMFUNC(xf86LoadDrvSubModule)
    SYMFUNC(xf86LoadSubModule)
    SYMFUNC(xf86LoadOneModule)
    SYMFUNC(xf86UnloadSubModule)
    SYMFUNC(xf86LoaderCheckSymbol)
    SYMFUNC(xf86LoaderRefSymLists)
    SYMFUNC(xf86LoaderRefSymbols)
    SYMFUNC(xf86LoaderReqSymLists)
    SYMFUNC(xf86LoaderReqSymbols)
    SYMFUNC(xf86SetBackingStore)
    SYMFUNC(xf86SetSilkenMouse)
    /* SYMFUNC(xf86NewSerialNumber) */
    SYMFUNC(xf86FindXvOptions)
    SYMFUNC(xf86GetOS)
    SYMFUNC(xf86ConfigPciEntity)
    SYMFUNC(xf86ConfigFbEntity)
    SYMFUNC(xf86ConfigActivePciEntity)
    SYMFUNC(xf86ConfigPciEntityInactive)
    SYMFUNC(xf86IsScreenPrimary)
    SYMFUNC(xf86RegisterRootWindowProperty)
    SYMFUNC(xf86IsUnblank)

#if (defined(__sparc__) || defined(__sparc)) && !defined(__OpenBSD__)
    /* xf86sbusBus.c */
    SYMFUNC(xf86MatchSbusInstances)
    SYMFUNC(xf86GetSbusInfoForEntity)
    SYMFUNC(xf86GetEntityForSbusInfo)
    SYMFUNC(xf86SbusUseBuiltinMode)
    SYMFUNC(xf86MapSbusMem)
    SYMFUNC(xf86UnmapSbusMem)
    SYMFUNC(xf86SbusHideOsHwCursor)
    SYMFUNC(xf86SbusSetOsHwCursorCmap)
    SYMFUNC(xf86SbusHandleColormaps)
    SYMFUNC(sparcPromInit)
    SYMFUNC(sparcPromClose)
    SYMFUNC(sparcPromGetProperty)
    SYMFUNC(sparcPromGetBool)
#endif

    /* xf86Init.c */
    SYMFUNC(xf86GetPixFormat)
    SYMFUNC(xf86GetBppFromDepth)

    /* xf86Mode.c */
    SYMFUNC(xf86GetNearestClock)
    SYMFUNC(xf86ModeStatusToString)
    SYMFUNC(xf86LookupMode)
    SYMFUNC(xf86CheckModeForMonitor)
    SYMFUNC(xf86InitialCheckModeForDriver)
    SYMFUNC(xf86CheckModeForDriver)
    SYMFUNC(xf86ValidateModes)
    SYMFUNC(xf86DeleteMode)
    SYMFUNC(xf86PruneDriverModes)
    SYMFUNC(xf86SetCrtcForModes)
    SYMFUNC(xf86PrintModes)
    SYMFUNC(xf86ShowClockRanges)

    /* xf86Option.c */
    SYMFUNC(xf86CollectOptions)
    SYMFUNC(xf86CollectInputOptions)
    /* Merging of XInput stuff   */
    SYMFUNC(xf86AddNewOption)
    SYMFUNC(xf86NewOption)
    SYMFUNC(xf86NextOption)
    SYMFUNC(xf86OptionListCreate)
    SYMFUNC(xf86OptionListMerge)
    SYMFUNC(xf86OptionListFree)
    SYMFUNC(xf86OptionName)
    SYMFUNC(xf86OptionValue)
    SYMFUNC(xf86OptionListReport)
    SYMFUNC(xf86SetIntOption)
    SYMFUNC(xf86SetRealOption)
    SYMFUNC(xf86SetStrOption)
    SYMFUNC(xf86SetBoolOption)
    SYMFUNC(xf86CheckIntOption)
    SYMFUNC(xf86CheckRealOption)
    SYMFUNC(xf86CheckStrOption)
    SYMFUNC(xf86CheckBoolOption)
    SYMFUNC(xf86ReplaceIntOption)
    SYMFUNC(xf86ReplaceRealOption)
    SYMFUNC(xf86ReplaceStrOption)
    SYMFUNC(xf86ReplaceBoolOption)
    SYMFUNC(xf86FindOption)
    SYMFUNC(xf86FindOptionValue)
    SYMFUNC(xf86MarkOptionUsed)
    SYMFUNC(xf86MarkOptionUsedByName)
    SYMFUNC(xf86CheckIfOptionUsed)
    SYMFUNC(xf86CheckIfOptionUsedByName)
    SYMFUNC(xf86ShowUnusedOptions)
    SYMFUNC(xf86ProcessOptions)
    SYMFUNC(xf86TokenToOptinfo)
    SYMFUNC(xf86TokenToOptName)
    SYMFUNC(xf86IsOptionSet)
    SYMFUNC(xf86GetOptValString)
    SYMFUNC(xf86GetOptValInteger)
    SYMFUNC(xf86GetOptValULong)
    SYMFUNC(xf86GetOptValReal)
    SYMFUNC(xf86GetOptValFreq)
    SYMFUNC(xf86GetOptValBool)
    SYMFUNC(xf86ReturnOptValBool)
    SYMFUNC(xf86NameCmp)
    SYMFUNC(xf86InitValuatorAxisStruct)
    SYMFUNC(xf86InitValuatorDefaults)

    /* xf86fbman.c */
    SYMFUNC(xf86InitFBManager)
    SYMFUNC(xf86InitFBManagerArea)
    SYMFUNC(xf86InitFBManagerRegion)
    SYMFUNC(xf86InitFBManagerLinear)
    SYMFUNC(xf86RegisterFreeBoxCallback)
    SYMFUNC(xf86FreeOffscreenArea)
    SYMFUNC(xf86AllocateOffscreenArea)
    SYMFUNC(xf86AllocateLinearOffscreenArea)
    SYMFUNC(xf86ResizeOffscreenArea)
    SYMFUNC(xf86FBManagerRunning)
    SYMFUNC(xf86QueryLargestOffscreenArea)
    SYMFUNC(xf86PurgeUnlockedOffscreenAreas)
    SYMFUNC(xf86RegisterOffscreenManager)
    SYMFUNC(xf86AllocateOffscreenLinear)
    SYMFUNC(xf86ResizeOffscreenLinear)
    SYMFUNC(xf86QueryLargestOffscreenLinear)
    SYMFUNC(xf86FreeOffscreenLinear)

    /* xf86cmap.c */
    SYMFUNC(xf86HandleColormaps)
    SYMFUNC(xf86GetGammaRampSize)
    SYMFUNC(xf86GetGammaRamp)
    SYMFUNC(xf86ChangeGammaRamp)

    /* xf86RandR.c */
#ifdef RANDR
    SYMFUNC(xf86RandRSetNewVirtualAndDimensions)
#endif

    /* xf86xv.c */
#ifdef XV
    SYMFUNC(xf86XVScreenInit)
    SYMFUNC(xf86XVRegisterGenericAdaptorDriver)
    SYMFUNC(xf86XVListGenericAdaptors)
    SYMFUNC(xf86XVRegisterOffscreenImages)
    SYMFUNC(xf86XVQueryOffscreenImages)
    SYMFUNC(xf86XVAllocateVideoAdaptorRec)
    SYMFUNC(xf86XVFreeVideoAdaptorRec)
    SYMFUNC(xf86XVFillKeyHelper)
    SYMFUNC(xf86XVFillKeyHelperDrawable)
    SYMFUNC(xf86XVClipVideoHelper)
    SYMFUNC(xf86XVCopyYUV12ToPacked)
    SYMFUNC(xf86XVCopyPacked)

    /* xf86xvmc.c */
    SYMFUNC(xf86XvMCScreenInit)
    SYMFUNC(xf86XvMCCreateAdaptorRec)
    SYMFUNC(xf86XvMCDestroyAdaptorRec)
#endif

    /* xf86VidMode.c */
    SYMFUNC(VidModeExtensionInit)
#ifdef XF86VIDMODE
    SYMFUNC(VidModeGetCurrentModeline)
    SYMFUNC(VidModeGetFirstModeline)
    SYMFUNC(VidModeGetNextModeline)
    SYMFUNC(VidModeDeleteModeline)
    SYMFUNC(VidModeZoomViewport)
    SYMFUNC(VidModeGetViewPort)
    SYMFUNC(VidModeSetViewPort)
    SYMFUNC(VidModeSwitchMode)
    SYMFUNC(VidModeLockZoom)
    SYMFUNC(VidModeGetMonitor)
    SYMFUNC(VidModeGetNumOfClocks)
    SYMFUNC(VidModeGetClocks)
    SYMFUNC(VidModeCheckModeForMonitor)
    SYMFUNC(VidModeCheckModeForDriver)
    SYMFUNC(VidModeSetCrtcForMode)
    SYMFUNC(VidModeAddModeline)
    SYMFUNC(VidModeGetDotClock)
    SYMFUNC(VidModeGetNumOfModes)
    SYMFUNC(VidModeSetGamma)
    SYMFUNC(VidModeGetGamma)
    SYMFUNC(VidModeCreateMode)
    SYMFUNC(VidModeCopyMode)
    SYMFUNC(VidModeGetModeValue)
    SYMFUNC(VidModeSetModeValue)
    SYMFUNC(VidModeGetMonitorValue)
    SYMFUNC(VidModeSetGammaRamp)
    SYMFUNC(VidModeGetGammaRamp)
    SYMFUNC(VidModeGetGammaRampSize)
#endif

    /* Misc */
    SYMFUNC(GetTimeInMillis)

    /* xf86Xinput.c */
    SYMFUNC(xf86ProcessCommonOptions)
    SYMFUNC(xf86PostMotionEvent)
    SYMFUNC(xf86PostProximityEvent)
    SYMFUNC(xf86PostButtonEvent)
    SYMFUNC(xf86PostKeyEvent)
    SYMFUNC(xf86PostKeyboardEvent)
    SYMFUNC(xf86FirstLocalDevice)
    SYMFUNC(xf86ActivateDevice)
    SYMFUNC(xf86XInputSetScreen)
    SYMFUNC(xf86ScaleAxis)
    SYMFUNC(NewInputDeviceRequest)
    SYMFUNC(DeleteInputDeviceRequest)
#ifdef DPMSExtension
    SYMFUNC(DPMSGet)
    SYMFUNC(DPMSSet)
    SYMFUNC(DPMSSupported)
#endif

    SYMFUNC(pciTag)
    SYMFUNC(pciBusAddrToHostAddr)

    /* Loader functions */
    SYMFUNC(LoadSubModule)
    SYMFUNC(DuplicateModule)
    SYMFUNC(LoaderErrorMsg)
    SYMFUNC(LoaderCheckUnresolved)
    SYMFUNC(LoadExtension)
    SYMFUNC(LoaderReqSymbols)
    SYMFUNC(LoaderReqSymLists)
    SYMFUNC(LoaderRefSymbols)
    SYMFUNC(LoaderRefSymLists)
    SYMFUNC(UnloadSubModule)
    SYMFUNC(LoaderSymbol)
    SYMFUNC(LoaderListDirs)
    SYMFUNC(LoaderFreeDirList)
    SYMFUNC(LoaderGetOS)
    SYMFUNC(LoaderShouldIgnoreABI)
    SYMFUNC(LoaderGetABIVersion)

#ifdef XF86DRI
    /*
     * These may have more general uses, but for now, they are only used
     * by the DRI.  Loading them only when the DRI is built may make porting
     * (the non-DRI portions of the X server) easier.
     */
    SYMFUNC(xf86InstallSIGIOHandler)
    SYMFUNC(xf86RemoveSIGIOHandler)
# if defined(__alpha__) && defined(linux)
    SYMFUNC(_bus_base)
# endif
#endif
    SYMFUNC(xf86BlockSIGIO)
    SYMFUNC(xf86UnblockSIGIO)

#if defined(__alpha__)
    SYMFUNC(__divl)
    SYMFUNC(__reml)
    SYMFUNC(__divlu)
    SYMFUNC(__remlu)
    SYMFUNC(__divq)
    SYMFUNC(__divqu)
    SYMFUNC(__remq)
    SYMFUNC(__remqu)

# ifdef linux
    SYMFUNC(_outw)
    SYMFUNC(_outb)
    SYMFUNC(_outl)
    SYMFUNC(_inb)
    SYMFUNC(_inw)
    SYMFUNC(_inl)
    SYMFUNC(_alpha_outw)
    SYMFUNC(_alpha_outb)
    SYMFUNC(_alpha_outl)
    SYMFUNC(_alpha_inb)
    SYMFUNC(_alpha_inw)
    SYMFUNC(_alpha_inl)
# else
    SYMFUNC(outw)
    SYMFUNC(outb)
    SYMFUNC(outl)
    SYMFUNC(inb)
    SYMFUNC(inw)
    SYMFUNC(inl)
# endif
    SYMFUNC(xf86ReadMmio32)
    SYMFUNC(xf86ReadMmio16)
    SYMFUNC(xf86ReadMmio8)
    SYMFUNC(xf86WriteMmio32)
    SYMFUNC(xf86WriteMmio16)
    SYMFUNC(xf86WriteMmio8)
    SYMFUNC(xf86WriteMmioNB32)
    SYMFUNC(xf86WriteMmioNB16)
    SYMFUNC(xf86WriteMmioNB8)
#endif
#if defined(sun) && defined(SVR4)
    SYMFUNC(inb)
    SYMFUNC(inw)
    SYMFUNC(inl)
    SYMFUNC(outb)
    SYMFUNC(outw)
    SYMFUNC(outl)
#endif
#if defined(__powerpc__) && !defined(__OpenBSD__)
    SYMFUNC(inb)
    SYMFUNC(inw)
    SYMFUNC(inl)
    SYMFUNC(outb)
    SYMFUNC(outw)
    SYMFUNC(outl)
# if defined(NO_INLINE)
    SYMFUNC(mem_barrier)
    SYMFUNC(ldl_u)
    SYMFUNC(eieio)
    SYMFUNC(ldl_brx)
    SYMFUNC(ldw_brx)
    SYMFUNC(stl_brx)
    SYMFUNC(stw_brx)
    SYMFUNC(ldq_u)
    SYMFUNC(ldw_u)
    SYMFUNC(stl_u)
    SYMFUNC(stq_u)
    SYMFUNC(stw_u)
    SYMFUNC(write_mem_barrier)
# endif
# if PPCIO_DEBUG
    SYMFUNC(debug_inb)
    SYMFUNC(debug_inw)
    SYMFUNC(debug_inl)
    SYMFUNC(debug_outb)
    SYMFUNC(debug_outw)
    SYMFUNC(debug_outl)
# endif
#endif
#if defined(__GNUC__)
    SYMFUNC(__div64)
    SYMFUNC(__divdf3)
    SYMFUNC(__divdi3)
    SYMFUNC(__divsf3)
    SYMFUNC(__divsi3)
    SYMFUNC(__moddi3)
    SYMFUNC(__modsi3)
    SYMFUNC(__mul64)
    SYMFUNC(__muldf3)
    SYMFUNC(__muldi3)
    SYMFUNC(__mulsf3)
    SYMFUNC(__mulsi3)
    SYMFUNC(__udivdi3)
    SYMFUNC(__udivsi3)
    SYMFUNC(__umoddi3)
    SYMFUNC(__umodsi3)
#endif
#if defined(__ia64__)
    SYMFUNC(outw)
    SYMFUNC(outb)
    SYMFUNC(outl)
    SYMFUNC(inb)
    SYMFUNC(inw)
    SYMFUNC(inl)
#endif
#if defined(__arm__)
    SYMFUNC(outw)
    SYMFUNC(outb)
    SYMFUNC(outl)
    SYMFUNC(inb)
    SYMFUNC(inw)
    SYMFUNC(inl)
#endif

#ifdef __FreeBSD__
#if defined(__sparc__)
    SYMFUNC(_Qp_qtos)
    SYMFUNC(_Qp_qtod)
    SYMFUNC(_Qp_qtoux)
    SYMFUNC(_Qp_uitoq)
    SYMFUNC(_Qp_dtoq)
#endif
#endif

    /* General variables (from xf86.h) */
    SYMVAR(xf86ScreenKey)
    SYMVAR(xf86PixmapKey)
    SYMVAR(xf86Screens)
    SYMVAR(byte_reversed)
    SYMVAR(xf86inSuspend)

    /* predefined resource lists from xf86Bus.h */
    SYMVAR(resVgaExclusive)
    SYMVAR(resVgaShared)
    SYMVAR(resVgaMemShared)
    SYMVAR(resVgaIoShared)
    SYMVAR(resVgaUnusedExclusive)
    SYMVAR(resVgaUnusedShared)
    SYMVAR(resVgaSparseExclusive)
    SYMVAR(resVgaSparseShared)
    SYMVAR(res8514Exclusive)
    SYMVAR(res8514Shared)

#if defined(__powerpc__) && !defined(NO_INLINE)
    SYMVAR(ioBase)
#endif

    /* Globals from xf86Globals.c and xf86Priv.h */
    SYMVAR(xf86ConfigDRI)

    /* Globals from xf86Configure.c */
    SYMVAR(ConfiguredMonitor)

    /* modes */
    SYMVAR(xf86CrtcConfigPrivateIndex)
    SYMFUNC(xf86CrtcConfigInit)
    SYMFUNC(xf86CrtcConfigPrivateIndex)
    SYMFUNC(xf86CrtcCreate)
    SYMFUNC(xf86CrtcDestroy)
    SYMFUNC(xf86CrtcInUse)
    SYMFUNC(xf86CrtcSetScreenSubpixelOrder)
    SYMFUNC(xf86RotateFreeShadow)
    SYMFUNC(xf86RotateCloseScreen)
    SYMFUNC(xf86CrtcRotate)
    SYMFUNC(xf86CrtcSetMode)
    SYMFUNC(xf86CrtcSetSizeRange)
    SYMFUNC(xf86CrtcScreenInit)
    SYMFUNC(xf86CVTMode)
    SYMFUNC(xf86GTFMode)
    SYMFUNC(xf86DisableUnusedFunctions)
    SYMFUNC(xf86DPMSSet)
    SYMFUNC(xf86DuplicateMode)
    SYMFUNC(xf86DuplicateModes)
    SYMFUNC(xf86GetDefaultModes)
    SYMFUNC(xf86GetMonitorModes)
    SYMFUNC(xf86InitialConfiguration)
    SYMFUNC(xf86ModeHSync)
    SYMFUNC(xf86ModesAdd)
    SYMFUNC(xf86ModesEqual)
    SYMFUNC(xf86ModeVRefresh)
    SYMFUNC(xf86ModeWidth)
    SYMFUNC(xf86ModeHeight)
    SYMFUNC(xf86OutputCreate)
    SYMFUNC(xf86OutputDestroy)
    SYMFUNC(xf86OutputGetEDID)
    SYMFUNC(xf86ConnectorGetName)
    SYMFUNC(xf86OutputGetEDIDModes)
    SYMFUNC(xf86OutputRename)
    SYMFUNC(xf86OutputUseScreenMonitor)
    SYMFUNC(xf86OutputSetEDID)
    SYMFUNC(xf86OutputFindClosestMode)
    SYMFUNC(xf86PrintModeline)
    SYMFUNC(xf86ProbeOutputModes)
    SYMFUNC(xf86PruneInvalidModes)
    SYMFUNC(xf86SetModeCrtc)
    SYMFUNC(xf86SetModeDefaultName)
    SYMFUNC(xf86SetScrnInfoModes)
    SYMFUNC(xf86SetDesiredModes)
    SYMFUNC(xf86SetSingleMode)
    SYMFUNC(xf86ValidateModesClocks)
    SYMFUNC(xf86ValidateModesFlags)
    SYMFUNC(xf86ValidateModesSize)
    SYMFUNC(xf86ValidateModesSync)
    SYMFUNC(xf86ValidateModesUserConfig)
    SYMFUNC(xf86DiDGAInit)
    SYMFUNC(xf86DiDGAReInit)
    SYMFUNC(xf86DDCGetModes)
    SYMFUNC(xf86SaveScreen)
#ifdef RANDR
    SYMFUNC(xf86RandR12CreateScreenResources)
    SYMFUNC(xf86RandR12GetOriginalVirtualSize)
    SYMFUNC(xf86RandR12GetRotation)
    SYMFUNC(xf86RandR12Init)
    SYMFUNC(xf86RandR12PreInit)
    SYMFUNC(xf86RandR12SetConfig)
    SYMFUNC(xf86RandR12SetRotations)
    SYMFUNC(xf86RandR12TellChanged)
#endif
    SYMFUNC(xf86_cursors_init)
    SYMFUNC(xf86_reload_cursors)
    SYMFUNC(xf86_show_cursors)
    SYMFUNC(xf86_hide_cursors)
    SYMFUNC(xf86_cursors_fini)
    SYMFUNC(xf86_crtc_clip_video_helper)
    SYMFUNC(xf86_wrap_crtc_notify)
    SYMFUNC(xf86_unwrap_crtc_notify)
    SYMFUNC(xf86_crtc_notify)

    SYMFUNC(xf86DoEDID_DDC1)
    SYMFUNC(xf86DoEDID_DDC2)
    SYMFUNC(xf86InterpretEDID)
    SYMFUNC(xf86PrintEDID)
    SYMFUNC(xf86DoEEDID)
    SYMFUNC(xf86DDCMonitorSet)
    SYMFUNC(xf86SetDDCproperties)
    SYMFUNC(xf86MonitorIsHDMI)

    SYMFUNC(xf86CreateI2CBusRec)
    SYMFUNC(xf86CreateI2CDevRec)
    SYMFUNC(xf86DestroyI2CBusRec)
    SYMFUNC(xf86DestroyI2CDevRec)
    SYMFUNC(xf86I2CBusInit)
    SYMFUNC(xf86I2CDevInit)
    SYMFUNC(xf86I2CFindBus)
    SYMFUNC(xf86I2CFindDev)
    SYMFUNC(xf86I2CGetScreenBuses)
    SYMFUNC(xf86I2CProbeAddress)
    SYMFUNC(xf86I2CReadByte)
    SYMFUNC(xf86I2CReadBytes)
    SYMFUNC(xf86I2CReadStatus)
    SYMFUNC(xf86I2CReadWord)
    SYMFUNC(xf86I2CWriteByte)
    SYMFUNC(xf86I2CWriteBytes)
    SYMFUNC(xf86I2CWriteRead)
    SYMFUNC(xf86I2CWriteVec)
    SYMFUNC(xf86I2CWriteWord)

    /* ramdac/xf86RamDac.c */
    SYMFUNC(RamDacCreateInfoRec)
    SYMFUNC(RamDacHelperCreateInfoRec)
    SYMFUNC(RamDacDestroyInfoRec)
    SYMFUNC(RamDacHelperDestroyInfoRec)
    SYMFUNC(RamDacInit)
    SYMFUNC(RamDacHandleColormaps)
    SYMFUNC(RamDacFreeRec)
    SYMFUNC(RamDacGetHWIndex)
    SYMVAR(RamDacHWPrivateIndex)
    SYMVAR(RamDacScreenPrivateIndex)

    /* ramdac/xf86Cursor.c */
    SYMFUNC(xf86InitCursor)
    SYMFUNC(xf86CreateCursorInfoRec)
    SYMFUNC(xf86DestroyCursorInfoRec)
    SYMFUNC(xf86ForceHWCursor)

    /* ramdac/BT.c */
    SYMFUNC(BTramdacProbe)
    SYMFUNC(BTramdacSave)
    SYMFUNC(BTramdacRestore)
    SYMFUNC(BTramdacSetBpp)

    /* ramdac/IBM.c */
    SYMFUNC(IBMramdacProbe)
    SYMFUNC(IBMramdacSave)
    SYMFUNC(IBMramdacRestore)
    SYMFUNC(IBMramdac526SetBpp)
    SYMFUNC(IBMramdac640SetBpp)
    SYMFUNC(IBMramdac526CalculateMNPCForClock)
    SYMFUNC(IBMramdac640CalculateMNPCForClock)
    SYMFUNC(IBMramdac526HWCursorInit)
    SYMFUNC(IBMramdac640HWCursorInit)
    SYMFUNC(IBMramdac526SetBppWeak)

    /* ramdac/TI.c */
    SYMFUNC(TIramdacCalculateMNPForClock)
    SYMFUNC(TIramdacProbe)
    SYMFUNC(TIramdacSave)
    SYMFUNC(TIramdacRestore)
    SYMFUNC(TIramdac3026SetBpp)
    SYMFUNC(TIramdac3030SetBpp)
    SYMFUNC(TIramdacHWCursorInit)
    SYMFUNC(TIramdacLoadPalette)
};
