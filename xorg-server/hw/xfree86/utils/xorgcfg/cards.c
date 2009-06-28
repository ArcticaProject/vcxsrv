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

#define CARDS_PRIVATE
#include "cards.h"

#undef SERVER	/* defined in config.h, but of no use here */

/* return values from ReadCardsLine. */
#define ERROR		-3
#define UNKNOWN		-2
#define END		-1
#define	NOTUSEFUL	0
#define	NAME		1
#define	CHIPSET		2
#define	SERVER		3
#define	DRIVER		4
#define	RAMDAC		5
#define	CLOCKCHIP	6
#define	DACSPEED	7
#define	NOCLOCKPROBE	8
#define	UNSUPPORTED	9
#define	SEE		10
#define	LINE		11

/*
 * Prototypes
 */
static int ReadCardsLine(FILE*, char*);	/* must have 256 bytes */
static int CompareCards(_Xconst void *left, _Xconst void *right);
static int BCompareCards(_Xconst void *left, _Xconst void *right);
static void DoReadCardsDatabase(void);
static char **DoFilterCardNames(char *pattern, int *result);

#ifdef USE_MODULES

typedef struct {
    int ivendor;
    unsigned short vendor;
    unsigned short valid_vendor;
    char *chipsets;
    int num_chipsets;
} chipset_check;
#endif

/*
 * Initialization
 */
static int linenum = 0;
static char *Cards = "lib/X11/Cards";
CardsEntry **CardsDB;
int NumCardsEntry;

/*
 * Implementation
 */
#ifdef USE_MODULES
const pciVendorInfo *xf86PCIVendorInfo;
#endif

#ifdef USE_MODULES
void
InitializePciInfo(void)
{
    xf86PCIVendorInfo = pciVendorInfoList;
}

void
CheckChipsets(xf86cfgModuleOptions *opts, int *err)
{
    int i, j, ichk, ivnd = 0, vendor = -1, device;
    const pciDeviceInfo **pDev;
    SymTabPtr chips = opts->chipsets;
    chipset_check *check = NULL;
    int num_check = 0;

    if (!chips) {
	CheckMsg(CHECKER_NO_CHIPSETS, "WARNING No chipsets specified.\n");
	++*err;
	return;
    }

    while (chips->name) {
	device = chips->token & 0xffff;
	vendor = (chips->token & 0xffff0000) >> 16;
	if (vendor == 0)
	    vendor = opts->vendor;

	for (ichk = 0; ichk < num_check; ichk++)
	    if (check[ichk].vendor == vendor)
		break;
	if (ichk >= num_check) {
	    check = (chipset_check*)
		XtRealloc((XtPointer)check,
			  sizeof(chipset_check) * (num_check + 1));
	    check[num_check].vendor = vendor;
	    memset(&check[num_check], 0, sizeof(chipset_check));
	    ++num_check;
	}

	/* Search for vendor in xf86PCIVendorInfo */
	if (xf86PCIVendorInfo) {
	    for (ivnd = 0; xf86PCIVendorInfo[ivnd].VendorID; ivnd++)
		if (vendor == xf86PCIVendorInfo[ivnd].VendorID)
		    break;
	}
	if (xf86PCIVendorInfo && xf86PCIVendorInfo[ivnd].VendorID) {
	    check[ichk].valid_vendor = 1;
	    check[ichk].ivendor = ivnd;
	}
	else {
	    CheckMsg(CHECKER_CANNOT_VERIFY_CHIPSET,
		     "WARNING Cannot verify chipset \"%s\" (0x%x)\n",
		      chips->name, device);
	    ++*err;
	    ++chips;
	    continue;
	}

	if (xf86PCIVendorInfo &&
	    (pDev = xf86PCIVendorInfo[ivnd].Device) != NULL) {
	    if (check[ichk].chipsets == NULL) {
		for (j = 0; pDev[j]; j++)
		    ;
		check[ichk].chipsets = (char*)XtCalloc(1, j);
	    }
	    for (j = 0; pDev[j]; j++) {
		if (device == pDev[j]->DeviceID) {
		    if (strcmp(chips->name, pDev[j]->DeviceName)) {
			CheckMsg(CHECKER_NOMATCH_CHIPSET_STRINGS,
			     "WARNING chipset strings don't match: \"%s\" \"%s\" (0x%x)\n",
			     chips->name, xf86PCIVendorInfo[ivnd].Device[j]->DeviceName,
			     device);
			++*err;
		    }
		    break;
		}
	    }
	    if (!pDev[j]) {
		CheckMsg(CHECKER_CHIPSET_NOT_LISTED,
		     "WARNING chipset \"%s\" (0x%x) not in list.\n", chips->name, device);
		++*err;
	    }
	    else
		check[ichk].chipsets[j] = 1;
	}
	++chips;
    }

    for (i = 0; i < num_check; i++) {
	if (!check[i].valid_vendor) {
	    CheckMsg(CHECKER_CHIPSET_NO_VENDOR,
		     "WARNING No such vendor 0x%x\n", vendor);
	    ++*err;
	}
	for (j = 0; j < check[i].num_chipsets; j++) {
	    if (xf86PCIVendorInfo && !check[i].chipsets[j]) {
		CheckMsg(CHECKER_CHIPSET_NOT_SUPPORTED,
			 "NOTICE chipset \"%s\" (0x%x) not listed as supported.\n",
			 xf86PCIVendorInfo[check[i].ivendor].Device[j]->DeviceName,
			 xf86PCIVendorInfo[check[i].ivendor].Device[j]->DeviceID);
	    }
	}
	XtFree(check[i].chipsets);
    }

    XtFree((XtPointer)check);
}
#endif

void
ReadCardsDatabase(void)
{
#ifdef USE_MODULES
    if (!nomodules) {
	int i, j, ivendor, idevice;
	char name[256];
	_Xconst char *vendor, *device;
	CardsEntry *entry = NULL, *tmp;
	xf86cfgModuleOptions *opts = module_options;
	const pciDeviceInfo **pDev;

	/* Only list cards that have a driver installed */
	while (opts) {
	    if (opts->chipsets) {
		SymTabPtr chips = opts->chipsets;

		while (chips->name) {
		    vendor = opts->name;
		    device = chips->name;
		    ivendor = (chips->token & 0xffff0000) >> 16;
		    idevice = chips->token & 0xffff0;
		    if (ivendor == 0)
			ivendor = opts->vendor;

		    if (xf86PCIVendorInfo) {
			for (i = 0; xf86PCIVendorInfo[i].VendorName; i++)
			    if (ivendor == xf86PCIVendorInfo[i].VendorID) {
				vendor = xf86PCIVendorInfo[i].VendorName;
				break;
			    }
			if (xf86PCIVendorInfo[i].VendorName) {
			    if ((pDev = xf86PCIVendorInfo[i].Device)) {
				for (j = 0; pDev[j]; j++)
				    if (idevice == pDev[j]->DeviceID) {
					device = pDev[j]->DeviceName;
					break;
				    }
			    }
			}
		    }

		    /* Since frequently there is more than one driver for a
		     * single vendor, it is required to avoid duplicates.
		     */
		    XmuSnprintf(name, sizeof(name), "%s %s", vendor, device);
		    tmp = LookupCard(name);

		    if (tmp == NULL || strcmp(tmp->chipset, chips->name) ||
			strcmp(tmp->driver, opts->name)) {
			entry = (CardsEntry*)XtCalloc(1, sizeof(CardsEntry));
			if (NumCardsEntry % 16 == 0) {
			    CardsDB = (CardsEntry**)XtRealloc((XtPointer)CardsDB,
				    sizeof(CardsEntry*) * (NumCardsEntry + 16));
			}
			CardsDB[NumCardsEntry++] = entry;
			entry->name = XtNewString(name);

			/* XXX no private copy of strings */
			entry->chipset = (char*)chips->name;
			entry->driver = opts->name;

			/* better than linear searchs to find duplicates */
			qsort(CardsDB, NumCardsEntry, sizeof(CardsEntry*),
			      CompareCards);
		    }
		    ++chips;
		}
	    }
	    opts = opts->next;
	}

	/* fix entries with the same name */
	for (i = 0; i < NumCardsEntry - 2;) {
	    for (j = i + 1; j < NumCardsEntry - 1 &&
		 strcmp(CardsDB[i]->name, CardsDB[j]->name) == 0; j++)
		    ;

	    if (i + 1 != j) {
		while (i < j) {
		    char *str;

		    if (strcmp(CardsDB[i]->chipset, CardsDB[j]->chipset))
			str = CardsDB[i]->chipset;
		    else
			str = CardsDB[i]->driver;

		    XmuSnprintf(name, sizeof(name), "%s (%s)",
				CardsDB[i]->name, str);
		    XtFree(CardsDB[i]->name);
		    CardsDB[i]->name = XtNewString(name);

		    ++i;
		}
	    }
	    else
		++i;
	}

	/* make sure data is valid to bsearch in */
	qsort(CardsDB, NumCardsEntry, sizeof(CardsEntry*), CompareCards);
    }
    else
#endif
	DoReadCardsDatabase();
}

static void
DoReadCardsDatabase(void)
{
    char buffer[256];
    FILE *fp = fopen(Cards, "r");
    int i, result;
    CardsEntry *entry = NULL;
    static char *CardsError = "Error reading Cards database, at line %d (%s).\n";

    if (fp == NULL) {
	fprintf(stderr, "Cannot open Cards database.\n");
	exit(1);
    }

    while ((result = ReadCardsLine(fp, buffer)) != END) {
	switch (result) {
	    case ERROR:
		fprintf(stderr, CardsError, linenum, buffer);
		break;
	    case UNKNOWN:
		fprintf(stderr,
			"Unknown field type in Cards database, at line %d (%s).\n",
			linenum, buffer);
		break;
	    case NAME:
		entry = calloc(1, sizeof(CardsEntry));
		if (NumCardsEntry % 16 == 0) {
		    CardsDB = realloc(CardsDB, sizeof(CardsEntry*) *
				      (NumCardsEntry + 16));
		    if (CardsDB == NULL) {
			fprintf(stderr, "Out of memory reading Cards database.\n");
			exit(1);
		    }
		}
		CardsDB[NumCardsEntry++] = entry;
		entry->name = strdup(buffer);
		break;
	    case CHIPSET:
		if (entry == NULL || entry->chipset != NULL) {
		    fprintf(stderr, CardsError, linenum, buffer);
		}
#if 0
		else
		    entry->chipset = strdup(buffer);
#endif
		break;
	    case SERVER:
		if (entry == NULL || entry->server != NULL) {
		    fprintf(stderr, CardsError, linenum, buffer);
		}
		else
		    entry->server = strdup(buffer);
		break;
	    case DRIVER:
		if (entry == NULL || entry->driver != NULL) {
		    fprintf(stderr, CardsError, linenum, buffer);
		}
		else
		    entry->driver = strdup(buffer);
		break;
	    case RAMDAC:
		if (entry == NULL || entry->ramdac != NULL) {
		    fprintf(stderr, CardsError, linenum, buffer);
		}
		else
		    entry->ramdac = strdup(buffer);
		break;
	    case CLOCKCHIP:
		if (entry == NULL || entry->clockchip != NULL) {
		    fprintf(stderr, CardsError, linenum, buffer);
		}
		else
		    entry->clockchip = strdup(buffer);
		break;
	    case DACSPEED:
		if (entry == NULL || entry->dacspeed != NULL) {
		    fprintf(stderr, CardsError, linenum, buffer);
		}
		else
		    entry->dacspeed = strdup(buffer);
		break;
	    case NOCLOCKPROBE:
		if (entry == NULL) {
		    fprintf(stderr, CardsError, linenum, buffer);
		}
		else
		    entry->flags |= F_NOCLOCKPROBE;
		break;
	    case UNSUPPORTED:
		if (entry == NULL) {
		    fprintf(stderr, CardsError, linenum, buffer);
		}
		else
		    entry->flags |= F_UNSUPPORTED;
		break;
	    case SEE:
		if (entry == NULL || entry->see != NULL) {
		    fprintf(stderr, CardsError, linenum, buffer);
		}
		else
		    entry->see = strdup(buffer);
		break;
	    case LINE:
		if (entry == NULL) {
		    fprintf(stderr, CardsError, linenum, buffer);
		}
		else if (entry->lines == NULL)
		    entry->lines = strdup(buffer);
		else {
		    char *str = malloc(strlen(entry->lines) + strlen(buffer) + 2);

		    sprintf(str, "%s\n%s", entry->lines, buffer);
		    free(entry->lines);
		    entry->lines = str;
		}
		break;
	}
    }

    fclose(fp);

    qsort(CardsDB, NumCardsEntry, sizeof(CardsEntry*), CompareCards);

#ifdef DEBUG
    for (i = 0; i < NumCardsEntry - 1; i++) {
	if (strcmp(CardsDB[i]->name, CardsDB[i+1]->name) == 0)
	    fprintf(stderr, "Duplicate entry in Cards database: (%s).\n",
		    CardsDB[i]->name);
    }
#endif

    for (i = 0; i < NumCardsEntry - 1; i++) {
	if (CardsDB[i]->see != NULL) {
	    if ((entry = LookupCard(CardsDB[i]->see)) == NULL) {
		fprintf(stderr, "Cannot find card '%s' for filling defaults.\n",
			CardsDB[i]->see);
		continue;
	    }
	    if (CardsDB[i]->chipset == NULL && entry->chipset != NULL)
		CardsDB[i]->chipset = strdup(entry->chipset);
	    if (CardsDB[i]->server == NULL && entry->server != NULL)
		CardsDB[i]->server = strdup(entry->server);
	    if (CardsDB[i]->driver == NULL && entry->driver != NULL)
		CardsDB[i]->driver = strdup(entry->driver);
	    if (CardsDB[i]->ramdac == NULL && entry->ramdac != NULL)
		CardsDB[i]->ramdac = strdup(entry->ramdac);
	    if (CardsDB[i]->clockchip == NULL && entry->clockchip != NULL)
		CardsDB[i]->clockchip = strdup(entry->clockchip);
	    if (CardsDB[i]->dacspeed == NULL && entry->dacspeed != NULL)
		CardsDB[i]->dacspeed = strdup(entry->dacspeed);
	    if (CardsDB[i]->flags & F_NOCLOCKPROBE)
		CardsDB[i]->flags |= F_NOCLOCKPROBE;
	    if (CardsDB[i]->flags & F_UNSUPPORTED)
		CardsDB[i]->flags |= F_UNSUPPORTED;
	    if (entry->lines != NULL) {
		if (CardsDB[i]->lines == NULL)
		    CardsDB[i]->lines = strdup(entry->lines);
		else {
		    char *str = malloc(strlen(entry->lines) +
					      strlen(CardsDB[i]->lines) + 2);

		    sprintf(str, "%s\n%s", CardsDB[i]->lines, entry->lines);
		    free(CardsDB[i]->lines);
		    CardsDB[i]->lines = str;
		}
	    }
	    if (entry->see != NULL) {
#ifdef DEBUG
		fprintf(stderr, "Nested SEE entry: %s -> %s -> %s\n",
			CardsDB[i]->name, CardsDB[i]->see, entry->see);
#endif
		CardsDB[i]->see = strdup(entry->see);
		--i;
		continue;
	    }
	    free(CardsDB[i]->see);
	    CardsDB[i]->see = NULL;
	}
    }
}

CardsEntry *
LookupCard(char *name)
{
    CardsEntry **ptr;

    if (NumCardsEntry == 0 || CardsDB == 0)
	return NULL;

    ptr = (CardsEntry**)bsearch(name, CardsDB, NumCardsEntry,
				sizeof(CardsEntry*), BCompareCards);

    return (ptr != NULL ? *ptr : NULL);
}

char **
GetCardNames(int *result)
{
    char **cards = NULL;
    int ncards;

    for (ncards = 0; ncards < NumCardsEntry; ncards++) {
	if (ncards % 16 == 0) {
	    if ((cards = (char**)realloc(cards, sizeof(char*) *
					 (ncards + 16))) == NULL) {
		fprintf(stderr, "Out of memory.\n");
		exit(1);
	    }
	}
	cards[ncards] = strdup(CardsDB[ncards]->name);
    }

    *result = ncards;

    return (cards);
}

char **
FilterCardNames(char *pattern, int *result)
{
#ifdef USE_MODULES
    if (!nomodules) {
	char **cards = NULL;
	int i, ncards = 0;

	for (i = 0; i < NumCardsEntry; i++) {
	    if (strstr(CardsDB[i]->name, pattern) == NULL)
		continue;
	    if (ncards % 16 == 0) {
		if ((cards = (char**)realloc(cards, sizeof(char*) *
					     (ncards + 16))) == NULL) {
		    fprintf(stderr, "Out of memory.\n");
		    exit(1);
		}
	    }
	    cards[ncards] = strdup(CardsDB[i]->name);
	    ++ncards;
	}

	*result = ncards;

	return (cards);
    }
#endif
    return (DoFilterCardNames(pattern, result));
}

static char **
DoFilterCardNames(char *pattern, int *result)
{
    FILE *fp;
    char **cards = NULL;
    int len, ncards = 0;
    char *cmd, *ptr, buffer[256];

    cmd = malloc(32 + (strlen(pattern) * 2) + strlen(Cards));

    strcpy(cmd, "egrep -i '^NAME\\ .*");
    len = strlen(cmd);
    ptr = pattern;
    while (*ptr) {
	if (!isalnum(*ptr)) {
	    cmd[len++] = '\\';
	}
	cmd[len++] = *ptr++;
    }
    cmd[len] = '\0';
    strcat(cmd, ".*$' ");
    strcat(cmd, Cards);
    strcat(cmd, " | sort");
    /*sprintf(cmd, "egrep -i '^NAME\\ .*%s.*$' %s | sort", pattern, Cards);*/

    if ((fp = popen(cmd, "r")) == NULL) {
	fprintf(stderr, "Cannot read Cards database.\n");
	exit(1);
    }
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
	ptr = buffer + strlen(buffer) - 1;
	while (isspace(*ptr) && ptr > buffer)
	    --ptr;
	if (!isspace(*ptr) && ptr > buffer)
	    ptr[1] = '\0';
	ptr = buffer;
	while (!isspace(*ptr) && *ptr)	/* skip NAME */
	    ++ptr;
	while (isspace(*ptr) && *ptr)
	    ++ptr;
	if (ncards % 16 == 0) {
	    if ((cards = (char**)realloc(cards, sizeof(char*) *
					 (ncards + 16))) == NULL) {
		fprintf(stderr, "Out of memory.\n");
		exit(1);
	    }
	}
	cards[ncards++] = strdup(ptr);
    }
    free(cmd);

    *result = ncards;

    return (cards);
}

static int
ReadCardsLine(FILE *fp, char *value)
{
    char name[32], buffer[256], *ptr, *end;
    int result = NOTUSEFUL;

    ++linenum;

    if (fgets(buffer, sizeof(buffer), fp) == NULL)
	return (END);

    ptr = buffer;
    /* skip initial spaces; should'nt bother about this.. */
    while (isspace(*ptr) && *ptr)
	++ptr;

    if (*ptr == '#' || *ptr == '\0')
	return (NOTUSEFUL);

    end = ptr;
    while (!isspace(*end) && *end)
	++end;
    if (end - ptr > sizeof(buffer) - 1) {
	strncpy(value, buffer, 255);
	value[255] = '\0';
	return (ERROR);
    }
    strncpy(name, ptr, end - ptr);
    name[end - ptr] = '\0';

    /* read the optional arguments */
    ptr = end;
    while (isspace(*ptr) && *ptr)
	++ptr;

    end = ptr + strlen(ptr) - 1;
    while (isspace(*end) && end > ptr)
	--end;
    if (!isspace(*end))
	++end;
    *end = '\0';

    if (strcmp(name, "NAME") == 0)
	result = NAME;
    else if (strcmp(name, "CHIPSET") == 0)
	result = CHIPSET;
    else if (strcmp(name, "SERVER") == 0)
	result = SERVER;
    else if (strcmp(name, "DRIVER") == 0)
	result = DRIVER;
    else if (strcmp(name, "RAMDAC") == 0)
	result = RAMDAC;
    else if (strcmp(name, "CLOCKCHIP") == 0)
	result = CLOCKCHIP;
    else if (strcmp(name, "DACSPEED") == 0)
	result = DACSPEED;
    else if (strcmp(name, "NOCLOCKPROBE") == 0)
	result = NOCLOCKPROBE;
    else if (strcmp(name, "UNSUPPORTED") == 0)
	result = UNSUPPORTED;
    else if (strcmp(name, "SEE") == 0)
	result = SEE;
    else if (strcmp(name, "LINE") == 0)
	result = LINE;
    else if (strcmp(name, "END") == 0)
	result = END;
    else {
	strcpy(value, name);
	return (UNKNOWN);
    }

    /* value *must* have at least 256 bytes */
    strcpy(value, ptr);

    return (result);
}

static int
CompareCards(_Xconst void *left, _Xconst void *right)
{
    return strcasecmp((*(CardsEntry**)left)->name, (*(CardsEntry**)right)->name);
}

static int
BCompareCards(_Xconst void *name, _Xconst void *card)
{
  return (strcasecmp((char*)name, (*(CardsEntry**)card)->name));
}
