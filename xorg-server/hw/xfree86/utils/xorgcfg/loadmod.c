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

#ifdef USE_MODULES
#include <setjmp.h>

#ifndef HAS_GLIBC_SIGSETJMP
#if defined(setjmp) && defined(__GNU_LIBRARY__) && \
    (!defined(__GLIBC__) || (__GLIBC__ < 2) || \
     ((__GLIBC__ == 2) && (__GLIBC_MINOR__ < 3)))
#define HAS_GLIBC_SIGSETJMP 1
#endif
#endif

#define LOADER_PRIVATE
#include "loader.h"

#define	True		1
#define False		0
#define XtPointer	char*
#define XtMalloc	malloc
#define XtCalloc	calloc
#define XtRealloc	realloc
#define XtFree		free
#define XtNew(t)	malloc(sizeof(t))
#define XtNewString(s)	((s) ? strdup(s) : NULL)

#define	pointer void*

/* XXX beware (or fix it) libc functions called here are the xf86 ones */

static void AddModuleOptions(char*, const OptionInfoRec*);
#if 0
void xf86AddDriver(DriverPtr, void*, int);
Bool xf86ServerIsOnlyDetecting(void);
void xf86AddInputDriver(InputDriverPtr, pointer, int);
void xf86AddModuleInfo(ModuleInfoPtr, void*);
Bool xf86LoaderCheckSymbol(const char*);
void xf86LoaderRefSymLists(const char **, ...);
void xf86LoaderReqSymLists(const char **, ...);
void xf86Msg(int, const char*, ...);
void xf86MsgVerb(int, int, const char*, ...);
void xf86PrintChipsets(const char*, const char*, SymTabPtr);
void xf86ErrorFVerb(int verb, const char *format, ...);
int xf86MatchDevice(const char*, GDevPtr**);
int xf86MatchPciInstances(const char*, int, SymTabPtr, PciChipsets*, GDevPtr*, int, DriverPtr,int**);
int xf86MatchIsaInstances(const char*, SymTabPtr, pointer*, DriverPtr, pointer, GDevPtr*, int, int**);
void *xf86LoadDrvSubModule(DriverPtr drv, const char*);
void xf86DrvMsg(int, int, const char*, ...);
Bool xf86IsPrimaryPci(pcVideoPtr*);
Bool xf86CheckPciSlot( const struct pci_device * );
#endif

extern char *loaderPath, **loaderList, **ploaderList;
xf86cfgModuleOptions *module_options;
FontModule *font_module;
int numFontModules;

extern int noverify, error_level;

int xf86ShowUnresolved = 1;

LOOKUP miLookupTab[]      = {{0,0}};
LOOKUP dixLookupTab[]     = {{0,0}};
LOOKUP fontLookupTab[]    = {{0,0}};
LOOKUP extLookupTab[]     = {{0,0}};
LOOKUP xfree86LookupTab[] = {
       /* Loader functions */
   SYMFUNC(LoaderDefaultFunc)
   SYMFUNC(LoadSubModule)
   SYMFUNC(DuplicateModule)
   SYMFUNC(LoaderErrorMsg)
   SYMFUNC(LoaderCheckUnresolved)
   SYMFUNC(LoadExtension)
   SYMFUNC(LoadFont)
   SYMFUNC(LoaderReqSymbols)
   SYMFUNC(LoaderReqSymLists)
   SYMFUNC(LoaderRefSymbols)
   SYMFUNC(LoaderRefSymLists)
   SYMFUNC(UnloadSubModule)
   SYMFUNC(LoaderSymbol)
   SYMFUNC(LoaderListDirs)
   SYMFUNC(LoaderFreeDirList)
   SYMFUNC(LoaderGetOS)

   /*
    * these here are our own interfaces to libc functions
    */
   SYMFUNC(xf86abort)
   SYMFUNC(xf86abs)
   SYMFUNC(xf86acos)
   SYMFUNC(xf86asin)
   SYMFUNC(xf86atan)
   SYMFUNC(xf86atan2)
   SYMFUNC(xf86atof)
   SYMFUNC(xf86atoi)
   SYMFUNC(xf86atol)
   SYMFUNC(xf86bsearch)
   SYMFUNC(xf86ceil)
   SYMFUNC(xf86calloc)
   SYMFUNC(xf86clearerr)
   SYMFUNC(xf86close)
   SYMFUNC(xf86cos)
   SYMFUNC(xf86exit)
   SYMFUNC(xf86exp)
   SYMFUNC(xf86fabs)
   SYMFUNC(xf86fclose)
   SYMFUNC(xf86feof)
   SYMFUNC(xf86ferror)
   SYMFUNC(xf86fflush)
   SYMFUNC(xf86fgetc)
   SYMFUNC(xf86fgetpos)
   SYMFUNC(xf86fgets)
   SYMFUNC(xf86finite)
   SYMFUNC(xf86floor)
   SYMFUNC(xf86fmod)
   SYMFUNC(xf86fopen)
   SYMFUNC(xf86fprintf)
   SYMFUNC(xf86fputc)
   SYMFUNC(xf86fputs)
   SYMFUNC(xf86fread)
   SYMFUNC(xf86free)
   SYMFUNC(xf86freopen)
   SYMFUNC(xf86frexp)
   SYMFUNC(xf86fscanf)
   SYMFUNC(xf86fseek)
   SYMFUNC(xf86fsetpos)
   SYMFUNC(xf86ftell)
   SYMFUNC(xf86fwrite)
   SYMFUNC(xf86getc)
   SYMFUNC(xf86getenv)
   SYMFUNC(xf86getpagesize)
   SYMFUNC(xf86hypot)
   SYMFUNC(xf86ioctl)
   SYMFUNC(xf86isalnum)
   SYMFUNC(xf86isalpha)
   SYMFUNC(xf86iscntrl)
   SYMFUNC(xf86isdigit)
   SYMFUNC(xf86isgraph)
   SYMFUNC(xf86islower)
   SYMFUNC(xf86isprint)
   SYMFUNC(xf86ispunct)
   SYMFUNC(xf86isspace)
   SYMFUNC(xf86isupper)
   SYMFUNC(xf86isxdigit)
   SYMFUNC(xf86labs)
   SYMFUNC(xf86ldexp)
   SYMFUNC(xf86log)
   SYMFUNC(xf86log10)
   SYMFUNC(xf86lseek)
   SYMFUNC(xf86malloc)
   SYMFUNC(xf86memchr)
   SYMFUNC(xf86memcmp)
   SYMFUNC(xf86memcpy)
#if (defined(__powerpc__) && (defined(Lynx) || defined(linux))) || \
    defined(__sparc__) || defined(__sparc) || defined(__ia64__) || \
    defined (__amd64__) || defined(__x86_64__)
   /*
    * Some PPC, SPARC, and IA64 compilers generate calls to memcpy to handle
    * structure copies.  This causes a problem both here and in shared
    * libraries as there is no way to map the name of the call to the
    * correct function.
    */
   SYMFUNC(memcpy)
   /*
    * Some PPC, SPARC, and IA64 compilers generate calls to memset to handle 
    * aggregate initializations.
    */
   SYMFUNC(memset)
#endif
   SYMFUNC(xf86memmove)
   SYMFUNC(xf86memset)
   SYMFUNC(xf86mmap)
   SYMFUNC(xf86modf)
   SYMFUNC(xf86munmap)
   SYMFUNC(xf86open)
   SYMFUNC(xf86perror)
   SYMFUNC(xf86pow)
   SYMFUNC(xf86printf)
   SYMFUNC(xf86qsort)
   SYMFUNC(xf86read)
   SYMFUNC(xf86realloc)
   SYMFUNC(xf86remove)
   SYMFUNC(xf86rename)
   SYMFUNC(xf86rewind)
   SYMFUNC(xf86setbuf)
   SYMFUNC(xf86setvbuf)
   SYMFUNC(xf86sin)
   SYMFUNC(xf86snprintf)
   SYMFUNC(xf86sprintf)
   SYMFUNC(xf86sqrt)
   SYMFUNC(xf86sscanf)
   SYMFUNC(xf86strcat)
   SYMFUNC(xf86strcmp)
   SYMFUNC(xf86strcasecmp)
   SYMFUNC(xf86strcpy)
   SYMFUNC(xf86strcspn)
   SYMFUNC(xf86strerror)
   SYMFUNC(xf86strlen)
   SYMFUNC(xf86strncmp)
   SYMFUNC(xf86strncasecmp)
   SYMFUNC(xf86strncpy)
   SYMFUNC(xf86strpbrk)
   SYMFUNC(xf86strchr)
   SYMFUNC(xf86strrchr)
   SYMFUNC(xf86strspn)
   SYMFUNC(xf86strstr)
   SYMFUNC(xf86strtod)
   SYMFUNC(xf86strtok)
   SYMFUNC(xf86strtol)
   SYMFUNC(xf86strtoul)
   SYMFUNC(xf86tan)
   SYMFUNC(xf86tmpfile)
   SYMFUNC(xf86tolower)
   SYMFUNC(xf86toupper)
   SYMFUNC(xf86ungetc)
   SYMFUNC(xf86vfprintf)
   SYMFUNC(xf86vsnprintf)
   SYMFUNC(xf86vsprintf)
   SYMFUNC(xf86write)
  
/* non-ANSI C functions */
   SYMFUNC(xf86opendir)
   SYMFUNC(xf86closedir)
   SYMFUNC(xf86readdir)
   SYMFUNC(xf86rewinddir)
   SYMFUNC(xf86ffs)
   SYMFUNC(xf86strdup)
   SYMFUNC(xf86bzero)
   SYMFUNC(xf86usleep)
   SYMFUNC(xf86execl)

   SYMFUNC(xf86getsecs)
   SYMFUNC(xf86fpossize)      /* for returning sizeof(fpos_t) */

   SYMFUNC(xf86stat)
   SYMFUNC(xf86fstat)
   SYMFUNC(xf86access)
   SYMFUNC(xf86geteuid)
   SYMFUNC(xf86getegid)
   SYMFUNC(xf86getpid)
   SYMFUNC(xf86mknod)
   SYMFUNC(xf86chmod)
   SYMFUNC(xf86chown)
   SYMFUNC(xf86sleep)
   SYMFUNC(xf86mkdir)
   SYMFUNC(xf86shmget)
   SYMFUNC(xf86shmat)
   SYMFUNC(xf86shmdt)
   SYMFUNC(xf86shmctl)
#ifdef HAS_GLIBC_SIGSETJMP
   SYMFUNC(xf86setjmp)
   SYMFUNC(xf86setjmp0)
#if defined(__GLIBC__) && (__GLIBC__ >= 2)
   SYMFUNCALIAS("xf86setjmp1",__sigsetjmp)
#else
   SYMFUNC(xf86setjmp1)
#endif
#else
   SYMFUNCALIAS("xf86setjmp",setjmp)
   SYMFUNCALIAS("xf86setjmp0",setjmp)
   SYMFUNC(xf86setjmp1)
#endif
   SYMFUNCALIAS("xf86longjmp",longjmp)
   SYMFUNC(xf86getjmptype)
   SYMFUNC(xf86setjmp1_arg2)
   SYMFUNC(xf86setjmperror)

    SYMFUNC(xf86AddDriver)
    SYMFUNC(xf86ServerIsOnlyDetecting)
    SYMFUNC(xf86AddInputDriver)
    SYMFUNC(xf86AddModuleInfo)
    SYMFUNC(xf86LoaderCheckSymbol)

    SYMFUNC(xf86LoaderRefSymLists)
    SYMFUNC(xf86LoaderReqSymLists)
    SYMFUNC(xf86Msg)
    SYMFUNC(xf86MsgVerb)
    SYMFUNC(ErrorF)
    SYMFUNC(xf86PrintChipsets)
    SYMFUNC(xf86ErrorFVerb)
    SYMFUNC(xf86MatchDevice)
    SYMFUNC(xf86MatchPciInstances)
    SYMFUNC(xf86MatchIsaInstances)
    SYMFUNC(Xfree)
    SYMFUNC(xf86LoadDrvSubModule)
    SYMFUNC(xf86DrvMsg)
    SYMFUNC(xf86IsPrimaryPci)
    SYMFUNC(xf86CheckPciSlot)
    SYMFUNC(XNFalloc)
    SYMFUNC(XNFrealloc)
    SYMFUNC(XNFcalloc)
    {0,0}
};

static DriverPtr driver;
static ModuleInfoPtr info;
static SymTabPtr chips;
static int vendor;
ModuleType module_type = GenericModule;

static void
AddModuleOptions(char *name, const OptionInfoRec *option)
{
    xf86cfgModuleOptions *ptr;
    const OptionInfoRec *tmp;
    SymTabPtr ctmp;
    int count;

    /* XXX If the module is already in the list, then it means that
     * it is now being properly loaded by xf86cfg and the "fake" entry
     * added in xf86cfgLoaderInitList() isn't required anymore.
     * Currently:
     *	ati and vmware are known to fail. */
    for (ptr = module_options; ptr; ptr = ptr->next)
	if (strcmp(name, ptr->name) == 0) {
	    fprintf(stderr, "Module %s already in list!\n", name);
	    return;
	}

    ptr = XtNew(xf86cfgModuleOptions);
    ptr->name = XtNewString(name);
    ptr->type = module_type;
    if (option) {
	for (count = 0, tmp = option; tmp->name != NULL; tmp++, count++)
	    ;
	++count;
	ptr->option = XtCalloc(1, count * sizeof(OptionInfoRec));
	for (count = 0, tmp = option; tmp->name != NULL; count++, tmp++) {
	    memcpy(&ptr->option[count], tmp, sizeof(OptionInfoRec));
	    ptr->option[count].name = XtNewString(tmp->name);
	    if (tmp->type == OPTV_STRING || tmp->type == OPTV_ANYSTR)
		ptr->option[count].value.str = XtNewString(tmp->value.str);
	}
    }
    else
	ptr->option = NULL;
    if (vendor != -1 && chips) {
	ptr->vendor = vendor;
	for (count = 0, ctmp = chips; ctmp->name; ctmp++, count++)
	    ;
	++count;
	ptr->chipsets = XtCalloc(1, count * sizeof(SymTabRec));
	for (count = 0, ctmp = chips; ctmp->name != NULL; count++, ctmp++) {
	    memcpy(&ptr->chipsets[count], ctmp, sizeof(SymTabRec));
	    ptr->chipsets[count].name = XtNewString(ctmp->name);
	}
    }
    else
	ptr->chipsets = NULL;

    ptr->next = module_options;
    module_options = ptr;
}

extern void xf86WrapperInit(void);

void
xf86cfgLoaderInit(void)
{
    LoaderInit();
    xf86WrapperInit();
}

void
xf86cfgLoaderInitList(int type)
{
    static const char *generic[] = {
	".",
	NULL
    };
    static const char *video[] = {
	"drivers",
	NULL
    };
    static const char *input[] = {
	"input",
	NULL
    };
    static const char *font[] = {
	"fonts",
	NULL
    };
    const char **subdirs;

    switch (type) {
	case GenericModule:
	    subdirs = generic;
	    break;
	case VideoModule:
	    subdirs = video;
	    break;
	case InputModule:
	    subdirs = input;
	    break;
	case FontRendererModule:
	    subdirs = font;
	    break;
	default:
	    fprintf(stderr, "Invalid value passed to xf86cfgLoaderInitList.\n");
	    subdirs = generic;
	    break;
    }
    LoaderSetPath(loaderPath);
    loaderList = LoaderListDirs(subdirs, NULL);

    /* XXX Xf86cfg isn't able to provide enough wrapper functions
     * to these drivers. Maybe the drivers could also be changed
     * to work better when being loaded "just for testing" */
    if (type == VideoModule) {
	module_type = VideoModule;
	AddModuleOptions("vmware", NULL);
	AddModuleOptions("ati", NULL);
	module_type = NullModule;
    }
}

void
xf86cfgLoaderFreeList(void)
{
    LoaderFreeDirList(loaderList);
}

int
xf86cfgCheckModule(void)
{
    int errmaj, errmin;
    ModuleDescPtr module;
    int nfonts;
    FontModule *fonts, *pfont_module;

    driver = NULL;
    chips = NULL;
    info = NULL;
    pfont_module = NULL;
    vendor = -1;
    module_type = GenericModule;

    if ((module = LoadModule(*ploaderList, NULL, NULL, NULL, NULL,
			     NULL, &errmaj, &errmin)) == NULL) {
	LoaderErrorMsg(NULL, *ploaderList, errmaj, errmin);
	return (0);
    }
    else if (driver && driver->AvailableOptions) {
	/* at least fbdev does not call xf86MatchPciInstances in Probe */
	if (driver->Identify)
	    (*driver->Identify)(-1);
	if (driver->Probe)
	    (*driver->Probe)(driver, PROBE_DETECT);
	AddModuleOptions(*ploaderList, (*driver->AvailableOptions)(-1, -1));
    }
    else if (info && info->AvailableOptions)
	AddModuleOptions(*ploaderList, (*info->AvailableOptions)(NULL));

    if (!noverify) {
	XF86ModuleData *initdata = NULL;
	char *p;

	p = XtMalloc(strlen(*ploaderList) + strlen("ModuleData") + 1);
	strcpy(p, *ploaderList);
	strcat(p, "ModuleData");
	initdata = LoaderSymbol(p);
	if (initdata) {
	    XF86ModuleVersionInfo *vers;

	    vers = initdata->vers;
	    if (vers && strcmp(*ploaderList, vers->modname)) {
		/* This was a problem at some time for some video drivers */
		CheckMsg(CHECKER_FILE_MODULE_NAME_MISMATCH,
			 "WARNING file/module name mismatch: \"%s\" \"%s\"\n",
			 *ploaderList, vers->modname);
		++error_level;
	    }
	}
	XtFree(p);
    }

    nfonts = numFontModules;
    numFontModules = 0;
    fonts = FontModuleList;
    if (fonts) {
	Bool dup = FALSE;
	while (fonts->name) {
	    if (strcasecmp(fonts->name, *ploaderList) == 0) {
		pfont_module = fonts;
		/* HACK:
		 * fonts->names points into modules.
		 * Duplicate string of all remaining names to survive
		 * unloading. Since new fonts are appended to list
		 * this will only happen once per renderer.
		 */
		dup = TRUE;
	    }
	    if (dup)
		fonts->name = strdup(fonts->name);
	    ++numFontModules;
	    ++fonts;
	}
    }
    if (pfont_module)
	module_type = FontRendererModule;
    else if (nfonts + 1 <= numFontModules) {
	/* loader.c will flag a warning if -noverify is not set */
	pfont_module = &FontModuleList[nfonts];
	module_type = FontRendererModule;
    }

    if (font_module) {
	XtFree((XtPointer)font_module->name);
	XtFree((XtPointer)font_module);
	font_module = NULL;
    }
    if (pfont_module) {
	font_module = XtNew(FontModule);
	memcpy(font_module, pfont_module, sizeof(FontModule));
	font_module->name = XtNewString(pfont_module->name);
    }

    UnloadModule(module);

    return (1);
}

_X_EXPORT void
xf86AddDriver(DriverPtr drv, void *module, int flags)
{
    driver = drv;
    if (driver)
	driver->module = module;
    module_type = VideoModule;
}

_X_EXPORT Bool
xf86ServerIsOnlyDetecting(void)
{
    return (True);
}

_X_EXPORT void
xf86AddInputDriver(InputDriverPtr inp, void *module, int flags)
{
    module_type = InputModule;
}

_X_EXPORT void
xf86AddModuleInfo(ModuleInfoPtr inf, void *module)
{
    info = inf;
}

_X_EXPORT Bool
xf86LoaderCheckSymbol(const char *symbol)
{
    return LoaderSymbol(symbol) != NULL;
}

_X_EXPORT void
xf86LoaderRefSymLists(const char **list0, ...)
{
}

_X_EXPORT void
xf86LoaderReqSymLists(const char **list0, ...)
{
}

#if 0
void xf86Msg(int type, const char *format, ...)
{
}
#endif

/*ARGSUSED*/
_X_EXPORT void
xf86PrintChipsets(const char *name, const char *msg, SymTabPtr chipsets)
{
    vendor = 0;
    chips = chipsets;
}

_X_EXPORT int
xf86MatchDevice(const char *name, GDevPtr **gdev)
{
    *gdev = NULL;

    return (1);
}

_X_EXPORT int
xf86MatchPciInstances(const char *name, int VendorID, SymTabPtr chipsets, PciChipsets *PCIchipsets,
		      GDevPtr *devList, int numDevs, DriverPtr drvp, int **foundEntities)
{
    vendor = VendorID;
    if (chips == NULL)
	chips = chipsets;
    *foundEntities = NULL;

    return (0);
}

_X_EXPORT int
xf86MatchIsaInstances(const char *name, SymTabPtr chipsets, IsaChipsets *ISAchipsets, DriverPtr drvp,
		      FindIsaDevProc FindIsaDevice, GDevPtr *devList, int numDevs, int **foundEntities)
{
    *foundEntities = NULL;

    return (0);
}

/*ARGSUSED*/
_X_EXPORT void *
xf86LoadDrvSubModule(DriverPtr drv, const char *name)
{
    pointer ret;
    int errmaj = 0, errmin = 0;

    ret = LoadSubModule(drv->module, name, NULL, NULL, NULL, NULL,
			&errmaj, &errmin);
    if (!ret)
	LoaderErrorMsg(NULL, name, errmaj, errmin);
    return (ret);
}

_X_EXPORT Bool
xf86IsPrimaryPci(pciVideoPtr pPci)
{
    return (True);
}

_X_EXPORT Bool 
xf86CheckPciSlot( const struct pci_device * d )
{
    (void) d;
    return (False);
}
#endif
