/*
 * Xephyr - A kdrive X server thats runs in a host X window.
 *          Authored by Matthew Allum <mallum@o-hand.com>
 * 
 * Copyright © 2004 Nokia 
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Nokia not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission. Nokia makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * NOKIA DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL NOKIA BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include <kdrive-config.h>
#endif
#include "ephyr.h"
#include "ephyrlog.h"
#include "glx_extinit.h"

extern Window EphyrPreExistingHostWin;
extern Bool EphyrWantGrayScale;
extern Bool EphyrWantResize;
extern Bool EphyrWantNoHostGrab;
extern Bool kdHasPointer;
extern Bool kdHasKbd;
extern Bool ephyr_glamor, ephyr_glamor_gles2;

#ifdef GLXEXT
extern Bool ephyrNoDRI;
#endif
extern Bool ephyrNoXV;

#ifdef KDRIVE_EVDEV
extern KdPointerDriver LinuxEvdevMouseDriver;
extern KdKeyboardDriver LinuxEvdevKeyboardDriver;
#endif

void processScreenOrOutputArg(const char *screen_size, const char *output, char *parent_id);
void processOutputArg(const char *output, char *parent_id);
void processScreenArg(const char *screen_size, char *parent_id);

void
InitCard(char *name)
{
    EPHYR_DBG("mark");
    KdCardInfoAdd(&ephyrFuncs, 0);
}

static const ExtensionModule ephyrExtensions[] = {
#ifdef GLXEXT
 { GlxExtensionInit, "GLX", &noGlxExtension },
#endif
};

static
void ephyrExtensionInit(void)
{
    LoadExtensionList(ephyrExtensions, ARRAY_SIZE(ephyrExtensions), TRUE);
}


void
InitOutput(ScreenInfo * pScreenInfo, int argc, char **argv)
{
    if (serverGeneration == 1)
        ephyrExtensionInit();

    KdInitOutput(pScreenInfo, argc, argv);
}

void
InitInput(int argc, char **argv)
{
    KdKeyboardInfo *ki;
    KdPointerInfo *pi;

    KdAddKeyboardDriver(&EphyrKeyboardDriver);
#ifdef KDRIVE_EVDEV
    KdAddKeyboardDriver(&LinuxEvdevKeyboardDriver);
#endif
    KdAddPointerDriver(&EphyrMouseDriver);
#ifdef KDRIVE_EVDEV
    KdAddPointerDriver(&LinuxEvdevMouseDriver);
#endif

    if (!kdHasKbd) {
        ki = KdNewKeyboard();
        if (!ki)
            FatalError("Couldn't create Xephyr keyboard\n");
        ki->driver = &EphyrKeyboardDriver;
        KdAddKeyboard(ki);
    }

    if (!kdHasPointer) {
        pi = KdNewPointer();
        if (!pi)
            FatalError("Couldn't create Xephyr pointer\n");
        pi->driver = &EphyrMouseDriver;
        KdAddPointer(pi);
    }

    KdInitInput();
}

void
CloseInput(void)
{
    KdCloseInput();
}

#ifdef DDXBEFORERESET
void
ddxBeforeReset(void)
{
}
#endif

void
ddxUseMsg(void)
{
    KdUseMsg();

    ErrorF("\nXephyr Option Usage:\n");
    ErrorF("-parent <XID>        Use existing window as Xephyr root win\n");
    ErrorF("-sw-cursor           Render cursors in software in Xephyr\n");
    ErrorF("-fullscreen          Attempt to run Xephyr fullscreen\n");
    ErrorF("-output <NAME>       Attempt to run Xephyr fullscreen (restricted to given output geometry)\n");
    ErrorF("-grayscale           Simulate 8bit grayscale\n");
    ErrorF("-resizeable          Make Xephyr windows resizeable\n");
#ifdef GLAMOR
    ErrorF("-glamor              Enable 2D acceleration using glamor\n");
    ErrorF("-glamor_gles2        Enable 2D acceleration using glamor (with GLES2 only)\n");
#endif
    ErrorF
        ("-fakexa              Simulate acceleration using software rendering\n");
    ErrorF("-verbosity <level>   Set log verbosity level\n");
#ifdef GLXEXT
    ErrorF("-nodri               do not use DRI\n");
#endif
    ErrorF("-noxv                do not use XV\n");
    ErrorF("-name [name]         define the name in the WM_CLASS property\n");
    ErrorF
        ("-title [title]       set the window title in the WM_NAME property\n");
    ErrorF("-no-host-grab        Disable grabbing the keyboard and mouse.\n");
    ErrorF("\n");
}

void
processScreenOrOutputArg(const char *screen_size, const char *output, char *parent_id)
{
    KdCardInfo *card;

    InitCard(0);                /*Put each screen on a separate card */
    card = KdCardInfoLast();

    if (card) {
        KdScreenInfo *screen;
        unsigned long p_id = 0;
        Bool use_geometry;

        screen = KdScreenInfoAdd(card);
        KdParseScreen(screen, screen_size);
        screen->driver = calloc(1, sizeof(EphyrScrPriv));
        if (!screen->driver)
            FatalError("Couldn't alloc screen private\n");

        if (parent_id) {
            p_id = strtol(parent_id, NULL, 0);
        }

        use_geometry = (strchr(screen_size, '+') != NULL);
        EPHYR_DBG("screen number:%d\n", screen->mynum);
        hostx_add_screen(screen, p_id, screen->mynum, use_geometry, output);
    }
    else {
        ErrorF("No matching card found!\n");
    }
}

void
processScreenArg(const char *screen_size, char *parent_id)
{
    processScreenOrOutputArg(screen_size, NULL, parent_id);
}

void
processOutputArg(const char *output, char *parent_id)
{
    processScreenOrOutputArg("100x100+0+0", output, parent_id);
}

int
ddxProcessArgument(int argc, char **argv, int i)
{
    static char *parent = NULL;

    EPHYR_DBG("mark argv[%d]='%s'", i, argv[i]);

    if (i == 1) {
        hostx_use_resname(basename(argv[0]), 0);
    }

    if (!strcmp(argv[i], "-parent")) {
        if (i + 1 < argc) {
            int j;

            /* If parent is specified and a screen argument follows, don't do
             * anything, let the -screen handling init the rest */
            for (j = i; j < argc; j++) {
                if (!strcmp(argv[j], "-screen")) {
                    parent = argv[i + 1];
                    return 2;
                }
            }

            processScreenArg("100x100", argv[i + 1]);
            return 2;
        }

        UseMsg();
        exit(1);
    }
    else if (!strcmp(argv[i], "-screen")) {
        if ((i + 1) < argc) {
            processScreenArg(argv[i + 1], parent);
            parent = NULL;
            return 2;
        }

        UseMsg();
        exit(1);
    }
    else if (!strcmp(argv[i], "-output")) {
        if (i + 1 < argc) {
            processOutputArg(argv[i + 1], NULL);
            return 2;
        }

        UseMsg();
        exit(1);
    }
    else if (!strcmp(argv[i], "-sw-cursor")) {
        hostx_use_sw_cursor();
        return 1;
    }
    else if (!strcmp(argv[i], "-host-cursor")) {
        /* Compatibility with the old command line argument, now the default. */
        return 1;
    }
    else if (!strcmp(argv[i], "-fullscreen")) {
        hostx_use_fullscreen();
        return 1;
    }
    else if (!strcmp(argv[i], "-grayscale")) {
        EphyrWantGrayScale = 1;
        return 1;
    }
    else if (!strcmp(argv[i], "-resizeable")) {
        EphyrWantResize = 1;
        return 1;
    }
#ifdef GLAMOR
    else if (!strcmp (argv[i], "-glamor")) {
        ephyr_glamor = TRUE;
        ephyrFuncs.initAccel = ephyr_glamor_init;
        ephyrFuncs.enableAccel = ephyr_glamor_enable;
        ephyrFuncs.disableAccel = ephyr_glamor_disable;
        ephyrFuncs.finiAccel = ephyr_glamor_fini;
        return 1;
    }
    else if (!strcmp (argv[i], "-glamor_gles2")) {
        ephyr_glamor = TRUE;
        ephyr_glamor_gles2 = TRUE;
        ephyrFuncs.initAccel = ephyr_glamor_init;
        ephyrFuncs.enableAccel = ephyr_glamor_enable;
        ephyrFuncs.disableAccel = ephyr_glamor_disable;
        ephyrFuncs.finiAccel = ephyr_glamor_fini;
        return 1;
    }
#endif
    else if (!strcmp(argv[i], "-fakexa")) {
        ephyrFuncs.initAccel = ephyrDrawInit;
        ephyrFuncs.enableAccel = ephyrDrawEnable;
        ephyrFuncs.disableAccel = ephyrDrawDisable;
        ephyrFuncs.finiAccel = ephyrDrawFini;
        return 1;
    }
    else if (!strcmp(argv[i], "-verbosity")) {
        if (i + 1 < argc && argv[i + 1][0] != '-') {
            int verbosity = atoi(argv[i + 1]);

            LogSetParameter(XLOG_VERBOSITY, verbosity);
            EPHYR_LOG("set verbosiry to %d\n", verbosity);
            return 2;
        }
        else {
            UseMsg();
            exit(1);
        }
    }
#ifdef GLXEXT
    else if (!strcmp(argv[i], "-nodri")) {
        ephyrNoDRI = TRUE;
        EPHYR_LOG("no direct rendering enabled\n");
        return 1;
    }
#endif
    else if (!strcmp(argv[i], "-noxv")) {
        ephyrNoXV = TRUE;
        EPHYR_LOG("no XVideo enabled\n");
        return 1;
    }
    else if (!strcmp(argv[i], "-name")) {
        if (i + 1 < argc && argv[i + 1][0] != '-') {
            hostx_use_resname(argv[i + 1], 1);
            return 2;
        }
        else {
            UseMsg();
            return 0;
        }
    }
    else if (!strcmp(argv[i], "-title")) {
        if (i + 1 < argc && argv[i + 1][0] != '-') {
            hostx_set_title(argv[i + 1]);
            return 2;
        }
        else {
            UseMsg();
            return 0;
        }
    }
    else if (argv[i][0] == ':') {
        hostx_set_display_name(argv[i]);
    }
    /* Xnest compatibility */
    else if (!strcmp(argv[i], "-display")) {
        hostx_set_display_name(argv[i + 1]);
        return 2;
    }
    else if (!strcmp(argv[i], "-sync") ||
             !strcmp(argv[i], "-full") ||
             !strcmp(argv[i], "-sss") || !strcmp(argv[i], "-install")) {
        return 1;
    }
    else if (!strcmp(argv[i], "-bw") ||
             !strcmp(argv[i], "-class") ||
             !strcmp(argv[i], "-geometry") || !strcmp(argv[i], "-scrns")) {
        return 2;
    }
    /* end Xnest compat */
    else if (!strcmp(argv[i], "-no-host-grab")) {
        EphyrWantNoHostGrab = 1;
        return 2;
    }

    return KdProcessArgument(argc, argv, i);
}

void
OsVendorInit(void)
{
    EPHYR_DBG("mark");

    if (hostx_want_host_cursor())
        ephyrFuncs.initCursor = &ephyrCursorInit;

    KdOsInit(&EphyrOsFuncs);
}

KdCardFuncs ephyrFuncs = {
    ephyrCardInit,              /* cardinit */
    ephyrScreenInitialize,      /* scrinit */
    ephyrInitScreen,            /* initScreen */
    ephyrFinishInitScreen,      /* finishInitScreen */
    ephyrCreateResources,       /* createRes */
    ephyrPreserve,              /* preserve */
    ephyrEnable,                /* enable */
    ephyrDPMS,                  /* dpms */
    ephyrDisable,               /* disable */
    ephyrRestore,               /* restore */
    ephyrScreenFini,            /* scrfini */
    ephyrCardFini,              /* cardfini */

    0,                          /* initCursor */
    0,                          /* enableCursor */
    0,                          /* disableCursor */
    0,                          /* finiCursor */
    0,                          /* recolorCursor */

    0,                          /* initAccel */
    0,                          /* enableAccel */
    0,                          /* disableAccel */
    0,                          /* finiAccel */

    ephyrGetColors,             /* getColors */
    ephyrPutColors,             /* putColors */

    ephyrCloseScreen,           /* closeScreen */
};
