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
#include "cards.h"
#include "options.h"
#include "loader.h"
#include "stubs.h"
#include <X11/Xresource.h>
#include <X11/Xos.h>

#ifdef USE_MODULES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(X_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE X_POSIX_C_SOURCE
#include <setjmp.h>
#undef _POSIX_C_SOURCE
#else
#include <setjmp.h>
#endif
#include <signal.h>
#include <ctype.h>

#include <stdarg.h>

#ifndef OPTIONSPATH
#define OPTIONSPATH "/usr/lib/X11"
#endif

#ifndef SIGNALRETURNSINT
void sig_handler(int);
#else
int sig_handler(int);
#endif	/* SIGNALRETURNSINT */

static Bool EnumDatabase(XrmDatabase*, XrmBindingList, XrmQuarkList,
			 XrmRepresentation*, XrmValue*, XPointer);

static sigjmp_buf jmp;
int signal_caught;
int error_level;
char *loaderPath, **loaderList, **ploaderList;
extern XrmDatabase options_xrm;
extern int noverify;
extern ModuleType module_type;
static OptionInfoPtr option;

extern FontModule *font_module;
extern int numFontModules;

char **checkerLegend;
int *checkerErrors;

#ifndef SIGNALRETURNSINT
void
#else
int
#endif
sig_handler(int sig)
{
    char *str;

    switch (sig) {
	case SIGTRAP:
	    str = "TRAP";
	    break;
	case SIGBUS:
	    str = "BUS";
	    break;
	case SIGSEGV:
	    str = "SEGV";
	    break;
	case SIGILL:
	    str = "ILL";
	    break;
	case SIGFPE:
	    str = "FPE";
	    break;
	default:
	    str = "???";
	    break;
    }

    if (signal_caught == 1) {
	ErrorF("  ERROR I am dead.\n");
	exit(1);
    }
    else if (signal_caught == 2)
	abort();
    ++signal_caught;
    ErrorF("  ERROR SIG%s caught!\n", str);
    if (!noverify)
	error_level += 50;
    siglongjmp(jmp, 1);
    /*NOTREACHED*/
}

void
CheckMsg(int code, char *fmt, ...)
{
    va_list ap;

    ++checkerErrors[code];
    ErrorF("%3d ", code);

    va_start(ap, fmt);
    VErrorF(fmt, ap);
    va_end(ap);
}

static Bool
EnumDatabase(XrmDatabase *db, XrmBindingList bindings, XrmQuarkList quarks,
	     XrmRepresentation *type, XrmValue *value, XPointer closure)
{
    char *res = XrmQuarkToString(quarks[1]);

    if (res) {
	option = module_options->option;
	while (option->name) {
	    if (strcasecmp(option->name, res) == 0)
		return (False);
	    ++option;
	}
	CheckMsg(CHECKER_OPTION_UNUSED,
		 "WARNING %s.%s is not used\n",
		 XrmQuarkToString(quarks[0]), res);
	++error_level;
    }

    return (False);
}

Bool
LoaderInitializeOptions(void)
{
    static int first = 1;
    static char *modules = "lib/modules";
    volatile Bool options_ok = False;
    char *ptr, query[256];
    char *ptr2, query2[256];
    char *type;
    XrmValue value;
    XrmQuark names[2];
    XrmQuark classes[2];
    volatile int i;
    static ModuleType module_types[] = {
	GenericModule, FontRendererModule, InputModule, VideoModule, NullModule
    };

    /* The offset in this vector must match loader.h:enum ModuleType values */
    static char *module_strs[] = {
	"Null Module", "Video Module", "Input Module", "Generic Module", "Font Module"
    };

    if (first) {
	checkerLegend = (char**)
	    XtCalloc(1, sizeof(char*) * (CHECKER_LAST_MESSAGE + 1));
	checkerErrors = (int*)
	    XtCalloc(1, sizeof(int) * (CHECKER_LAST_MESSAGE + 1));
	xf86cfgLoaderInit();
	first = 0;

	checkerLegend[CHECKER_OPTIONS_FILE_MISSING] =
	"The Options file, normally " OPTIONSPATH "/Options was not found.\n";
	checkerLegend[CHECKER_OPTION_DESCRIPTION_MISSING] =
	"No description for the module option. The description should be in\n"
	"in the Options file, and using the sintax:\n"
	"Module.Option:	any text describing the option";
	checkerLegend[CHECKER_LOAD_FAILED] =
	"Failed to load the module. Usually the loader will print a complete\n"
	"description for the reason the module was not loaded. Use the -verbose\n"
	"command line option if it is not printing any messages.";
	checkerLegend[CHECKER_RECOGNIZED_AS] =
	"This message means the module code did not follow what was expected\n"
	"by the checker. For video drivers, it did not call xf86AddDriver,\n"
	"a input module did not call xf86AddInputDriver and a font renderer\n"
	"module did not call LoadFont. This message can also be printed if\n"
	"the module is in the incorrect directory.";
	checkerLegend[CHECKER_NO_OPTIONS_AVAILABLE] =
	"The driver does not have an AvailableOptions function, or that\n"
	"function is returning NULL. If the driver is returning NULL, and\n"
	"really does not need any options from "__XCONFIGFILE__", than the message\n"
	"can be ignored.";
	checkerLegend[CHECKER_NO_VENDOR_CHIPSET] =
	"The checker could not fetch the PCI chipset/vendor information from\n"
	"the module. The checker currently wraps xf86PrintChipsets and\n"
	"xf86MatchPciInstances to read the information from the module.";
	checkerLegend[CHECKER_CANNOT_VERIFY_CHIPSET] =
	"The vendor id was not found, so it is not possible to search the list\n"
	"of chipsets.";
	checkerLegend[CHECKER_OPTION_UNUSED] =
	"The option description is defined in the Options file, but the option\n"
	"was name not retrieved when calling the module AvailableOptions.";
	checkerLegend[CHECKER_NOMATCH_CHIPSET_STRINGS] =
	"The string specified in the module does not match the one in\n"
	"common/xf86PciInfo.h";
	checkerLegend[CHECKER_CHIPSET_NOT_LISTED] =
	"This means that common/xf86PciInfo.h does not have an entry for the\n"
	"given vendor and id.";
	checkerLegend[CHECKER_CHIPSET_NOT_SUPPORTED] =
	"The chipset is listed in common/xf86PciInfo.h, but the driver does\n"
	"not support it, or does not list it in the chipsets fetched by the checker.";
	checkerLegend[CHECKER_CHIPSET_NO_VENDOR] =
	"The vendor id specified to xf86MatchPciInstances is not defined in\n"
	"common/xf86PciInfo.h";
	checkerLegend[CHECKER_NO_CHIPSETS] =
	"No chipsets were passed to xf86MatchPciIntances.";
	checkerLegend[CHECKER_FILE_MODULE_NAME_MISMATCH] =
	"The module name string does not match the the modname field of the\n"
	"XF86ModuleVersionInfo structure. This generally is not an error, but\n"
	"to may be a good idea to use the same string to avoid confusion.";
    }

    if (XF86Module_path == NULL) {
	XF86Module_path = malloc(strlen(XFree86Dir) + strlen(modules) + 2);
	sprintf(XF86Module_path, "%s/%s", XFree86Dir, modules);
    }

    if (loaderPath == NULL || strcmp(XF86Module_path, loaderPath))
	loaderPath = strdup(XF86Module_path);
    else
	/* nothing new */
	return (True);

    if (!noverify) {
	options_ok = InitializeOptionsDatabase();
	InitializePciInfo();
    }

    for (i = 0; module_types[i] != NullModule; i++) {
	xf86cfgLoaderInitList(module_types[i]);
	if (!noverify)
	    ErrorF("================= Checking modules of type \"%s\" =================\n",
		   module_strs[module_types[i]]);

	if (loaderList) {
	    for (ploaderList = loaderList; *ploaderList; ploaderList++) {
		signal_caught = 0;
		signal(SIGTRAP, sig_handler);
		signal(SIGBUS, sig_handler);
		signal(SIGSEGV, sig_handler);
		signal(SIGILL, sig_handler);
		signal(SIGFPE, sig_handler);
		if (sigsetjmp(jmp, 1) == 0) {
		    if (!noverify) {
			int ok, nfont_modules;

			nfont_modules = numFontModules;
			error_level = 0;
			ErrorF("CHECK MODULE %s\n", *ploaderList);
			if ((ok = xf86cfgCheckModule()) == 0) {
			    CheckMsg(CHECKER_LOAD_FAILED,
				     "ERROR Failed to load module.\n");
			    error_level += 50;
			}
			else if (module_type != module_types[i]) {
			    CheckMsg(CHECKER_RECOGNIZED_AS,
				     "WARNING %s recognized as a \"%s\"\n", *ploaderList,
				     module_strs[module_type]);
			    ++error_level;
			}
			if (ok) {
			    if (options_ok) {
				if ((module_options == NULL || module_options->option == NULL) &&
				    module_type != GenericModule) {
				    CheckMsg(CHECKER_NO_OPTIONS_AVAILABLE,
					     "WARNING Not a generic module, but no options available.\n");
				    ++error_level;
				}
				else if (module_options && strcmp(module_options->name, *ploaderList) == 0) {
				    ErrorF("  CHECK OPTIONS\n");
				    option = module_options->option;

				    while (option->name) {
					XmuSnprintf(query, sizeof(query), "%s.%s", *ploaderList, option->name);
					for (ptr = query, ptr2 = query2; *ptr; ptr++) {
					    if (*ptr != '_' && *ptr != ' ' && *ptr != '\t')
						*ptr2 = tolower(*ptr);
					}
					*ptr2 = '\0';
					/* all resources are in lowercase */
					if (!XrmGetResource(options_xrm, query2, "Module.Option", &type, &value) ||
					    value.addr == NULL) {
					    CheckMsg(CHECKER_OPTION_DESCRIPTION_MISSING,
						     "WARNING no description for %s\n", query);
					    ++error_level;
					}
					++option;
				    }

				    /* now do a linear search for Options file entries that are not
				     * in the driver.
				     */
				    names[0] = XrmPermStringToQuark(module_options->name);
				    classes[0] = XrmPermStringToQuark("Option");
				    names[1] = classes[1] = NULLQUARK;
				    (void)XrmEnumerateDatabase(options_xrm, (XrmNameList)&names, (XrmClassList)&classes,
							       XrmEnumOneLevel, EnumDatabase, NULL);
				}
			    }
			    else {
				CheckMsg(CHECKER_OPTIONS_FILE_MISSING,
					 "ERROR Options file missing.\n");
				error_level += 10;
			    }

			    if (module_type == VideoModule &&
				(module_options == NULL || module_options->vendor < 0 ||
				 module_options->chipsets == NULL)) {
				CheckMsg(CHECKER_NO_VENDOR_CHIPSET,
				         "WARNING No vendor/chipset information available.\n");
				++error_level;
			    }
			    else if (module_type == VideoModule) {
				if (module_options == NULL) {
				    /* No description for this, if this happen,
				     * something really strange happened. */
				    ErrorF("  ERROR No module_options!?!\n");
				    error_level += 50;
				}
				else {
				    ErrorF("  CHECK CHIPSETS\n");
				    CheckChipsets(module_options, &error_level);
				}
			    }

			    /* font modules check */
			    if (module_type == FontRendererModule) {
				if (strcmp(*ploaderList, font_module->name)) {
				    /* not an error */
				    ErrorF("  NOTICE FontModule->name specification mismatch: \"%s\" \"%s\"\n",
					   *ploaderList, font_module->name);
				}
				if (nfont_modules + 1 != numFontModules) {
				    /* not an error */
				    ErrorF("  NOTICE font module \"%s\" loaded more than one font renderer.\n",
					   *ploaderList);
				}
			    }
			    else if (nfont_modules != numFontModules) {
				ErrorF("  WARNING number of font modules changed from %d to %d.\n",
				       nfont_modules, numFontModules);
				++error_level;
			    }
			}
			ErrorF("  SUMMARY error_level set to %d.\n\n", error_level);
		    }
		    else
			(void)xf86cfgCheckModule();
		}
		signal(SIGTRAP, SIG_DFL);
		signal(SIGBUS, SIG_DFL);
		signal(SIGSEGV, SIG_DFL);
		signal(SIGILL, SIG_DFL);
		signal(SIGFPE, SIG_DFL);
	    }
	    xf86cfgLoaderFreeList();
	}
	else
	    ErrorF("  ERROR Failed to initialize module list.\n");
    }

    if (!noverify) {
	ErrorF("===================================== LEGEND ===============================\n");
	ErrorF("NOTICE lines are just informative.\n");
	ErrorF("WARNING lines add 1 to error_level.\n");
	ErrorF("ERROR lines add 2 or more (based on the severity of the error) to error_level.\n\n");
	for (i = 0; i <= CHECKER_LAST_MESSAGE; i++)
	    if (checkerErrors[i]) {
		ErrorF("%3d\n%s\n\n", i, checkerLegend[i]);
	    }
    }

    return (True);
}
#endif
