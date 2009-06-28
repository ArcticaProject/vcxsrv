/*
(c) Copyright 1996 Hewlett-Packard Company
(c) Copyright 1996 International Business Machines Corp.
(c) Copyright 1996 Sun Microsystems, Inc.
(c) Copyright 1996 Novell, Inc.
(c) Copyright 1996 Digital Equipment Corp.
(c) Copyright 1996 Fujitsu Limited
(c) Copyright 1996 Hitachi, Ltd.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the names of the copyright holders shall
not be used in advertising or otherwise to promote the sale, use or other
dealings in this Software without prior written authorization from said
copyright holders.
*/
/*******************************************************************
**
**    *********************************************************
**    *
**    *  File:		attributes.c
**    *
**    *  Contents:
**    *                 Implementation of the attribute store for Xp.
**    *
**    *  Copyright:	Copyright 1995 Hewlett-Packard Company
**    *
**    *********************************************************
** 
********************************************************************/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/Xproto.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#if (defined(sun) && defined(SVR4)) || defined(__SCO__) || defined(__UNIXWARE__)
#include <wchar.h>
#endif
#include "scrnintstr.h"

#include <X11/extensions/Printstr.h>

#include "attributes.h"

#include <X11/Xlib.h>
#include <X11/Xresource.h>

#include "spooler.h"

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif


static XrmDatabase CopyDb(XrmDatabase inDb);

extern XrmDatabase XpSpoolerGetServerAttributes(void);

static int attrGeneration = 0;

typedef struct {
    XrmDatabase *pDb;
    char *qualifier;
    char *modelId;
} DbEnumStruct;

typedef struct {
    char *stringDb;
    int nextPos;
    int space;
} StringDbStruct;

typedef struct _printerAttrs {
    struct _printerAttrs *next;
    char *name;
    char *qualifier;
    XrmDatabase printerAttrs;
    XrmDatabase docAttrs;
    XrmDatabase jobAttrs;
} PrAttrs, *PrAttrPtr;

static PrAttrPtr attrList = (PrAttrPtr)NULL;

typedef struct _systemAttrs {
    XrmDatabase doc;
    XrmDatabase job;
    XrmDatabase printers;
    XrmDatabase server;
} SysAttrs, *SysAttrsPtr;

SysAttrs systemAttributes;

/*
 * attrCtxtPrivIndex hold the attribute store's context private index.
 * This index is allocated at the time the attribute store is initialized.
 */
static DevPrivateKey attrCtxtPrivKey = &attrCtxtPrivKey;

/*
 * The ContextAttrs structure descibes the context private space reserved
 * by the attribute store.
 */
typedef struct _contextAttrs {
    XrmDatabase printerAttrs;
    XrmDatabase docAttrs;
    XrmDatabase jobAttrs;
    XrmDatabase pageAttrs;
} ContextAttrs, *ContextAttrPtr;

/*
 * XPDIR is relative to (i.e. is a subdir of) XPRINTDIR/$LANG.
 */
static const char XPDIR[] = "/print";
/*
 * The following files/directories define or are within subdirectories of the 
 * above-defined XPDIR.
 */
static const char XPPRINTERATTRFILE[] = "/attributes/printer";
static const char XPJOBATTRFILE[] = "/attributes/job";
static const char XPDOCATTRFILE[] = "/attributes/document";
static const char XPMODELDIR[] = "/models";

static char NULL_STRING[] = "\0";

/*
 * XpGetConfigDirBase returns a string containing the path name of the base
 * where the print server configuration directory is localed.
 */
static
char *XpGetConfigDirBase(void)
{
    char *configDir;

    /*
     * If the XPCONFIGDIR environment variable is not set, then use the
     * compile-time constant XPRINTDIR.  XPRINTDIR is passed in on the
     * compile command line, and is defined in $(TOP)/config/cf/Project.tmpl.
     */
    if((configDir = getenv("XPCONFIGDIR")) == (char *)NULL)
	configDir = XPRINTDIR;

    return configDir;
}

/*
 * XpGetConfigDir returns a string containing the path name of the print
 * server configuration directory.  If the useLocale parameter is False
 * the it returns the path to the "/C" directory.  If the useLocale
 * parameter is True it returns the path to the directory associated with
 * $LANG.  It makes no attempt to ensure that the directory actually exists.
 */
char *
XpGetConfigDir(Bool useLocale)
{ 
    char *dirName, *langName, *langDir, *configDir;
    Bool freeLangDir = False;

    if(useLocale == False) langDir = "/C";
    else 
    {
        langName = getenv("LC_ALL");
        if (langName == NULL) {
            langName = getenv("LANG");
        }
        
	if(langName == (char *)NULL)
	    return (char *)NULL;
	else
	{
	    if(strcmp(langName, "C") == 0)
		return (char *)NULL;
	    langDir = (char *)xalloc(strlen(langName) + 2);
	    sprintf(langDir, "/%s", langName);
	    freeLangDir = True;
	}
    }
    
    configDir = XpGetConfigDirBase();

    dirName = (char *)xalloc(strlen(configDir) + strlen(XPDIR) + 
			      strlen(langDir) + 1);
    sprintf(dirName, "%s%s%s", configDir, langDir, XPDIR);

    if(freeLangDir == True)
	xfree(langDir);

    return dirName;
}

/*
 * GetMergedDatabase reads and merges xrmdb files from the top-level printer
 * config directory, and from the directory associated with the current
 * locale (if other than the top-level).
 */
static XrmDatabase
GetMergedDatabase(const char *attrName)
{
    char *dirName, *fileName;
    XrmDatabase db;

    if((dirName = XpGetConfigDir(False)) == (char *)NULL)
	return (XrmDatabase)NULL;
    if((fileName = (char *)xalloc(strlen(dirName) + strlen(attrName) + 1)) ==
       (char *)NULL)
	return (XrmDatabase)NULL;
    sprintf(fileName, "%s%s", dirName, attrName);
    db = XrmGetFileDatabase(fileName);
    xfree(fileName);
    xfree(dirName);

    if((dirName = XpGetConfigDir(True)) == (char *)NULL) 
	return db;
    if((fileName = (char *)xalloc(strlen(dirName) + strlen(attrName) + 1)) ==
       (char *)NULL)
	return db;
    sprintf(fileName, "%s%s", dirName, attrName);
    (void)XrmCombineFileDatabase(fileName, &db, True);
    xfree(fileName);
    xfree(dirName);

    return db;
}

/*
 * BuildSystemAttributes reads the on-disk configuration files for printers,
 * initial job, and initial document attributes.  The resulting xrm 
 * databases are then dissected as needed for each printer.
 * It also allocates a contextPrivate space for the attributes,
 * reserving space to store pointers to the attribute stores for
 * the context.
 */
static void
BuildSystemAttributes(void)
{
    if(systemAttributes.printers != (XrmDatabase)NULL)
	XrmDestroyDatabase(systemAttributes.printers);
    systemAttributes.printers = GetMergedDatabase(XPPRINTERATTRFILE);
    if(systemAttributes.job != (XrmDatabase)NULL)
	XrmDestroyDatabase(systemAttributes.job);
    systemAttributes.job = GetMergedDatabase(XPJOBATTRFILE);
    if(systemAttributes.doc != (XrmDatabase)NULL)
	XrmDestroyDatabase(systemAttributes.doc);
    systemAttributes.doc = GetMergedDatabase(XPDOCATTRFILE);
    if(systemAttributes.server != (XrmDatabase)NULL)
	XrmDestroyDatabase(systemAttributes.server);
    systemAttributes.server = XpSpoolerGetServerAttributes();
    return;
}

/*
 * AddDbEntry is called by XrmEnumerateDatabase, and adds the supplied
 * database entry to the database pointed to within the "DbEnumStruct"
 * passed as the client_data (aka "closure").
 */
static Bool
AddDbEntry(
    XrmDatabase *sourceDB,
    XrmBindingList bindings,
    XrmQuarkList quarks,
    XrmRepresentation *type,
    XrmValue *value,
    XPointer client_data)
{
    DbEnumStruct *pEnumStruct = (DbEnumStruct *)client_data;
    XrmName xrm_name[5];
    XrmClass xrm_class[5];
    XrmBinding xrm_bind[3];
    XrmValue realVal;
    XrmRepresentation rep_type;

    xrm_name[0] = XrmStringToQuark (pEnumStruct->qualifier);
    xrm_class[0] = XrmStringToQuark (pEnumStruct->modelId);

    for(;*quarks; quarks++)
	xrm_name[1] = xrm_class[1] = *quarks;

    xrm_name[2] = (XrmQuark)NULL;
    xrm_class[2] = (XrmQuark)NULL;

    if(XrmQGetResource (*sourceDB, xrm_name, xrm_class, &rep_type, &realVal))
    {
        xrm_bind[0] = XrmBindLoosely;

	xrm_name[0] = xrm_name[1];
	xrm_name[1] = NULLQUARK;

        XrmQPutStringResource(pEnumStruct->pDb, xrm_bind, xrm_name, 
			      (char *)realVal.addr);
    }
    
    return FALSE;
}

/*
 * BuildPrinterAttrs - builds and returns an XrmDatabase for the printer
 * of the specified name/qualifier, if we have enough information.
 * If we don't have a model-config
 * file, then just enumerate the systemAttributes->printers database, 
 * otherwise read in the model-config database and merge into it the
 * systemAttributes->printers database.  This database is then enumerated
 * with the printer qualifier (and the model name as class if we have it), and
 * the resulting elements are stored into the database for this particular
 * printer.
 */
static XrmDatabase
BuildPrinterAttrs(
    char *printerName,
    char *qualifierName)
{
    XrmDatabase printerDB = (XrmDatabase)NULL;

    if(systemAttributes.printers != (XrmDatabase)NULL)
    {
        char *fileName;
        XrmDatabase modelDB = (XrmDatabase)NULL;
        XrmName xrm_name[5], xrm_class[2];
        XrmRepresentation rep_type;
        XrmValue value;
        DbEnumStruct enumStruct;
        Bool freeModelDB = False;
        /*
         * Build the initial db based on the model-config files
         */
        xrm_name[0] = XrmStringToQuark (qualifierName);
        xrm_name[1] = XrmStringToQuark ("xp-model-identifier");
        xrm_name[2] = (XrmQuark)NULL;
        XrmQGetResource (systemAttributes.printers, xrm_name, xrm_name, 
			 &rep_type, &value);

        if(value.addr != (XPointer)NULL)
        {
            fileName = (char *)xalloc(strlen(XPMODELDIR) + 
				      strlen((char *)value.addr) + 
				      strlen("model-config") + 3);
	    sprintf(fileName, "%s/%s/%s", XPMODELDIR, value.addr,
		    "model-config");
	    modelDB = GetMergedDatabase(fileName);
            xfree(fileName);
	    if(modelDB != (XrmDatabase)NULL)
	    {
		XrmDatabase tempDB = (XrmDatabase)NULL;
		/*
		 * have to make a temp copy because MergeDatabase destroys
		 * the "source" database. Merge in the printers DB
		 */
		tempDB = CopyDb(systemAttributes.printers);
		XrmMergeDatabases(tempDB, &modelDB);
		freeModelDB = True;
	    }
        }

	/*
	 * Check to see if we knew the name AND found a database file
	 */
	if(modelDB == (XrmDatabase)NULL)
	     modelDB = systemAttributes.printers;

        xrm_name[0] = XrmStringToQuark (qualifierName);
	xrm_name[1] = (XrmQuark)NULL;
	xrm_class[0] = XrmStringToQuark((char *)value.addr);
	xrm_class[1] = (XrmQuark)NULL;
	enumStruct.pDb = &printerDB;
	enumStruct.qualifier = (char *)qualifierName;
	enumStruct.modelId = (char *)value.addr;
	XrmEnumerateDatabase(modelDB, xrm_name, xrm_class, XrmEnumAllLevels,
			     AddDbEntry, (XPointer) &enumStruct);

        if(freeModelDB == True) XrmDestroyDatabase(modelDB);
    }
    XrmPutStringResource(&printerDB, "*printer-name", printerName);
    XrmPutStringResource(&printerDB, "*qualifier", qualifierName);
    return printerDB;
}

/*
 * BuildABase - builds an XrmDatabase by enumerating the supplied sourceBase
 * database for elements relevant for the printer named by printerName,
 * and deriving a class for printerName from the model declared in the
 * systemAttributes.printers database.  If no model is defined for this
 * printer then the printerName is used as the class as well.
 * 
 * This is used to build the initial value document and initial value
 * job attribute databases for each printer by searching the system
 * level doc and job databases.
 */
static XrmDatabase
BuildABase(
    char *printerName,
    char *qualifierName,
    XrmDatabase sourceBase)
{
    XrmDatabase builtDB = (XrmDatabase)NULL;

    if(sourceBase != (XrmDatabase)NULL)
    {
        XrmName xrm_name[5], xrm_class[2];
        XrmRepresentation rep_type;
        XrmValue value;
        DbEnumStruct enumStruct;

        /*
         * Retrieve the model name for use as the class.
         */
        xrm_name[0] = XrmStringToQuark (printerName);
        xrm_name[1] = XrmStringToQuark ("xp-model-identifier");
        xrm_name[2] = (XrmQuark)NULL;
        XrmQGetResource (systemAttributes.printers, xrm_name, xrm_name, 
			 &rep_type, &value);
	/*
	 * if we have a model name then use it as the class, otherwise
	 * just use the printer name as the class as well as the name.
	 */
        if(value.addr != (XPointer)NULL)
	    xrm_class[0] = XrmStringToQuark((char *)value.addr);
	else
	    xrm_class[0] = xrm_name[0];
	xrm_class[1] = (XrmQuark)NULL;

	xrm_name[1] = (XrmQuark)NULL;

	enumStruct.pDb = &builtDB;
	enumStruct.qualifier = (char *)qualifierName;
	enumStruct.modelId = (char *)value.addr;
	XrmEnumerateDatabase(sourceBase, xrm_name, xrm_class, XrmEnumAllLevels,
			     AddDbEntry, (XPointer) &enumStruct);
    }

    XrmPutStringResource(&builtDB, "*qualifier", qualifierName);

    return builtDB;
}

/*
 * FreeAttrList is called upon server recycle, and frees the printer
 * databases stored in the global attrList.
 */
static void
FreeAttrList(void)
{
    PrAttrPtr pAttr, pNext;

    for(pAttr = attrList, pNext = attrList; 
	pAttr != (PrAttrPtr)NULL; 
	pAttr = pNext)
    {
	pNext = pAttr->next;
	if(pAttr->printerAttrs != (XrmDatabase)NULL)
	    XrmDestroyDatabase(pAttr->printerAttrs);
	if(pAttr->docAttrs != (XrmDatabase)NULL)
	    XrmDestroyDatabase(pAttr->docAttrs);
	if(pAttr->jobAttrs != (XrmDatabase)NULL)
	    XrmDestroyDatabase(pAttr->jobAttrs);
	xfree(pAttr->name);
	xfree(pAttr->qualifier);
	xfree(pAttr);
    }
    attrList = (PrAttrPtr)NULL;
}

/*
 * XpRehashAttributes - frees the per-printer attribute list and
 * calls BuildSystemAttributes to rebuild the overall attribute
 * store.  It is expected that a caller of this will follow it
 * by calling XpBuildAttributeStore for a new list of printers.
 */
int
XpRehashAttributes(void)
{
    if(attrList != (PrAttrPtr)NULL)
        FreeAttrList();
    BuildSystemAttributes();
    return Success;
}

/*
 * XpBuildAttributeStore - creates the attribute database associated
 * with the specified printer.  The first time this is called it
 * calls BuildSystemAttributes to create the system-level databases.
 */
void
XpBuildAttributeStore(
    char *printerName,
    char *qualifierName)
{
    PrAttrPtr pAttr;

    if((pAttr = (PrAttrPtr)xalloc(sizeof(PrAttrs))) == (PrAttrPtr)NULL)
	return;

    if(attrGeneration != serverGeneration)
    {
	if(attrList != (PrAttrPtr)NULL)
	    FreeAttrList();
	dixRequestPrivate(attrCtxtPrivKey, sizeof(ContextAttrs));
	BuildSystemAttributes();

	attrGeneration = serverGeneration;
    }

    if(attrList == (PrAttrPtr)NULL)
    {
	pAttr->next = (PrAttrPtr)NULL;
	attrList = pAttr;
    }
    else
    {
	pAttr->next = attrList;
	attrList = pAttr;
    }

    pAttr->name = strdup(printerName);
    pAttr->qualifier = strdup(qualifierName);
    pAttr->printerAttrs = BuildPrinterAttrs(printerName, qualifierName);
    pAttr->docAttrs = BuildABase(printerName, qualifierName, 
				 systemAttributes.doc);
    pAttr->jobAttrs = BuildABase(printerName, qualifierName,
				 systemAttributes.job);
}


static Bool
StoreEntry(
    XrmDatabase *sourceDB,
    XrmBindingList bindings,
    XrmQuarkList quarks,
    XrmRepresentation *type,
    XrmValue *value,
    XPointer client_data)
{
    XrmDatabase *outDb = (XrmDatabase *)client_data;

    XrmQPutStringResource(outDb, bindings, quarks, (char *)value->addr);
    
    return FALSE;
}

/*
 * XpCopyDb - makes a copy of the specified XrmDatabase and returns
 * the copy.
 */
static XrmDatabase
CopyDb(XrmDatabase inDb)
{
    XrmDatabase outDb = (XrmDatabase)NULL;
    XrmQuark empty = NULLQUARK;

    (void)XrmEnumerateDatabase(inDb, &empty, &empty, XrmEnumAllLevels,
			       StoreEntry, (XPointer) &outDb);
    return outDb;
}

/* 
 * XpInitAttributes - initializes the attribute store for the specified
 * context.  It does this by making copies of the printer, doc, and job
 * attributes databases for the printer associated with the context.
 */
void
XpInitAttributes(XpContextPtr pContext)
{
    ContextAttrPtr pCtxtAttrs;
    PrAttrPtr pPrAttr = attrList;

    /* Initialize all the pointers to NULL */
    pCtxtAttrs = (ContextAttrPtr)dixLookupPrivate(&pContext->devPrivates,
						  attrCtxtPrivKey);
    (void)memset((void *)pCtxtAttrs, 0, (size_t) sizeof(ContextAttrs));

    for(pPrAttr = attrList; pPrAttr != (PrAttrPtr)NULL; pPrAttr = pPrAttr->next)
	if(!strcmp(pPrAttr->name, pContext->printerName)) break;

    if(pPrAttr != (PrAttrPtr)NULL)
    {
	pCtxtAttrs->printerAttrs = CopyDb(pPrAttr->printerAttrs);
	pCtxtAttrs->docAttrs = CopyDb(pPrAttr->docAttrs);
	pCtxtAttrs->jobAttrs = CopyDb(pPrAttr->jobAttrs);
    }
}

void
XpDestroyAttributes(
    XpContextPtr pContext)
{
    ContextAttrPtr pCtxtAttrs;

    pCtxtAttrs = (ContextAttrPtr)dixLookupPrivate(&pContext->devPrivates,
						  attrCtxtPrivKey);
    if(pCtxtAttrs->printerAttrs != (XrmDatabase)NULL)
	XrmDestroyDatabase(pCtxtAttrs->printerAttrs);
    if(pCtxtAttrs->docAttrs != (XrmDatabase)NULL)
	XrmDestroyDatabase(pCtxtAttrs->docAttrs);
    if(pCtxtAttrs->jobAttrs != (XrmDatabase)NULL)
	XrmDestroyDatabase(pCtxtAttrs->jobAttrs);
    if(pCtxtAttrs->pageAttrs != (XrmDatabase)NULL)
	XrmDestroyDatabase(pCtxtAttrs->pageAttrs);
}

/*
 * XpGetOneAttribute returns the string value of the specified attribute
 * in the specified class for the specified print context.  If the attribute
 * doesn't exist in the database for this context, or if the class database
 * doesn't exist for this context, then NULL is returned.  The caller must
 * not free the returned string, as the returned pointer points into the
 * database.  This function can also return a value from the server attributes,
 * in which case the pContext parameter is ignored.
 */
char *
XpGetOneAttribute(
     XpContextPtr pContext,
     XPAttributes class,
     char *attributeName)
{
    ContextAttrPtr pCtxtAttrs;
    XrmDatabase db = (XrmDatabase)NULL;
    XrmName xrm_name[3];
    XrmRepresentation rep_type;
    XrmValue value;

    if(class == XPServerAttr)
    {
        if(systemAttributes.server == (XrmDatabase)NULL) 
	    return NULL_STRING;

        xrm_name[0] = XrmStringToQuark (attributeName);
        xrm_name[1] = (XrmQuark)NULL;
        XrmQGetResource(systemAttributes.server, xrm_name, xrm_name, 
			&rep_type, &value);

        if(value.addr == (char *)NULL) 
	    return NULL_STRING;
        return (char *)value.addr;
    }
    else
    {
        pCtxtAttrs=(ContextAttrPtr)dixLookupPrivate(&pContext->devPrivates,
						    attrCtxtPrivKey);
        switch(class)
        {
	    case XPPrinterAttr:
	        db = pCtxtAttrs->printerAttrs;
	        break;
	    case XPDocAttr:
	        db = pCtxtAttrs->docAttrs;
	        break;
	    case XPJobAttr:
	        db = pCtxtAttrs->jobAttrs;
	        break;
	    case XPPageAttr:
	        db = pCtxtAttrs->pageAttrs;
	        break;
	    default:
	        break;
        }
    }
    if(db == (XrmDatabase)NULL) 
	return NULL_STRING;

    xrm_name[0] = XrmStringToQuark ("qualifier");
    xrm_name[1] = (XrmQuark)NULL;
    XrmQGetResource(db, xrm_name, xrm_name, &rep_type, &value);

    xrm_name[0] = XrmStringToQuark (value.addr);
    xrm_name[1] = XrmStringToQuark (attributeName);
    xrm_name[2] = (XrmQuark)NULL;
    if(XrmQGetResource(db, xrm_name, xrm_name, &rep_type, &value))
	return (char *)value.addr;
    else
        return NULL_STRING;
}

/*
 * XpPutOneAttribute updates one attribute for the specified
 * context and class. This function is intended for use by the attribute
 * validation module which updates the XrmDatabases directly. This
 * function does not recognize XPServerAttr.
 */
void
XpPutOneAttribute(
       XpContextPtr pContext,
       XPAttributes class,
       const char* attributeName,
       const char* value)
{
    ContextAttrPtr pCtxtAttrs;
    XrmDatabase db;
    XrmBinding bindings[1];
    XrmQuark quarks[2];
    
    pCtxtAttrs = (ContextAttrPtr)dixLookupPrivate(&pContext->devPrivates,
						  attrCtxtPrivKey);
    switch(class)
    {
    case XPPrinterAttr:
	db = pCtxtAttrs->printerAttrs;
	break;
    case XPDocAttr:
	db = pCtxtAttrs->docAttrs;
	break;
    case XPJobAttr:
	db = pCtxtAttrs->jobAttrs;
	break;
    case XPPageAttr:
	db =  pCtxtAttrs->pageAttrs;
	break;
    default:
	return;
    }
    bindings[0] = XrmBindLoosely;
    quarks[0] = XrmStringToQuark(attributeName);
    quarks[1] = (XrmQuark)NULL;
    XrmQPutStringResource(&db, bindings, quarks, value ? value : "");
}

    

/*******************************************************************************
 *
 * The following routines: ExpandSpace, PutString, PutByte, and AppendEntry
 * form the functional core of the GetAttributes routine.  Xrm does not
 * supply a routine to form a string database from an XrmDatabase, except
 * by writing the database to a file.  This code avoids the file system
 * overhead, but is a bit clunky in its memory management.
 *
 ******************************************************************************/

/*
 * ExpandSpace expands the memory allocated for the string database in
 * the StringDbStruct passed in, and updates the "space" field of the
 * struct to indicate the new amount of space available.
 */
static Bool
ExpandSpace(
    StringDbStruct *pStr)
{
    char *newSpace;

    if((newSpace = (char *)xrealloc(pStr->stringDb, pStr->nextPos + pStr->space
				    + 1024)) == (char *)NULL)
	return False;
    pStr->space += 1024;
    pStr->stringDb = newSpace;
    return True;
}

/*
 * PutString puts the contents of a null-terminated string into the string
 * database in the StringDbStruct passed in.  If there is insufficient room
 * for the string, ExpandSpace is called, and the nextPos and space fields
 * are updated.
 */
static void
PutString(
    StringDbStruct *pStr,
    char *pString)
{
    int len = strlen(pString);

    if(len >= pStr->space)
	if(!ExpandSpace(pStr))
	    return;
    strcpy(&pStr->stringDb[pStr->nextPos], pString);
    pStr->nextPos += len;
    pStr->space -= len;
}

/*
 * PutByte puts a single byte value in to the string database in the passed-in
 * StringDbStruct.  ExpandSpace is called if there is insufficient room for
 * the byte, and the nextPos and space fields are updated.
 */
static void
PutByte(
    StringDbStruct *pStr,
    char byte)
{
    if(pStr->space <= 1)
	if(!ExpandSpace(pStr))
	    return;
    pStr->stringDb[pStr->nextPos] = byte;
    pStr->nextPos++;
    pStr->space--;
}

#define XrmQString XrmPermStringToQuark("String")

/*
 * AppendEntry is called by XrmEnumerateDatabase, and serves to append
 * a database entry onto a string database.  The passed-in "closure"
 * struct contains a pointer to the string, and a count of the remaining
 * bytes.  If there are insufficient remaining bytes then the struct
 * is realloced, and the count of the space remaining is updated.
 * Database elements of types other than String are ignored!
 * This code is based directly on that in "DumpEntry" in Xrm.c.
 */
static Bool
AppendEntry(
    XrmDatabase         *db,
    XrmBindingList      bindings,
    XrmQuarkList        quarks,
    XrmRepresentation   *type,
    XrmValuePtr         value,
    XPointer            data)
{
    StringDbStruct *pEnumStr = (StringDbStruct *)data;
    Bool        firstNameSeen;
    unsigned int i;
    char *s, c;

    if (*type != XrmQString)
	return False;

    for (firstNameSeen = False; *quarks; bindings++, quarks++) {
        if (*bindings == XrmBindLoosely) {
	    PutString(pEnumStr, "*");
        } else if (firstNameSeen) {
	    PutString(pEnumStr, ".");
        }
        firstNameSeen = True;
	PutString(pEnumStr, XrmQuarkToString(*quarks));
    }
    s = value->addr;
    i = value->size;
    PutString(pEnumStr, ":\t");
    if(i) i--;

    if (i && (*s == ' ' || *s == '\t'))
        PutByte(pEnumStr, '\\'); /* preserve leading whitespace */

    while (i--) {
        c = *s++;
        if (c == '\n') {
            if (i)
                PutString(pEnumStr, "\\n\\\n");
            else
                PutString(pEnumStr, "\\n");
        } else if (c == '\\')
            PutString(pEnumStr, "\\\\");
        else if ((c < ' ' && c != '\t') ||
                 ((unsigned char)c >= 0x7f && (unsigned char)c < 0xa0))
	{
	    char temp[4];
            (void) sprintf(temp, "\\%03o", (unsigned char)c);
	    PutString(pEnumStr, temp);
	}
        else
            PutByte(pEnumStr, c);
    }
    PutByte(pEnumStr, '\n');
    pEnumStr->stringDb[pEnumStr->nextPos] = (char)'\0';
    return False;
}

/*
 * XpGetAttributes returns a string database version of the Xrm database
 * for the specified context and class.  This function can also return the
 * contents of the server attributes, in which case the pContext parameter
 * is ignored. 
 *
 * The caller is responsible for freeing the returned string, 
 * unlike XpGetOneAttribute, where the caller must not free the string.
 */
char *
XpGetAttributes(
     XpContextPtr pContext,
     XPAttributes class)
{
    ContextAttrPtr pCtxtAttrs;
    XrmDatabase db = (XrmDatabase)NULL;
    StringDbStruct enumStruct;
    XrmQuark empty = NULLQUARK;

    if(class == XPServerAttr)
	db = systemAttributes.server;
    else
    {
        pCtxtAttrs=(ContextAttrPtr)dixLookupPrivate(&pContext->devPrivates,
						    attrCtxtPrivKey);
        switch(class)
        {
	    case XPServerAttr:
	        db = systemAttributes.server;
	        break;
	    case XPPrinterAttr:
	        db = pCtxtAttrs->printerAttrs;
	        break;
	    case XPDocAttr:
	        db = pCtxtAttrs->docAttrs;
	        break;
	    case XPJobAttr:
	        db = pCtxtAttrs->jobAttrs;
	        break;
	    case XPPageAttr:
	        db = pCtxtAttrs->pageAttrs;
	        break;
	    default:
	        break;
        }
    }
    if(db == (XrmDatabase)NULL) 
    {
	char *retval = (char *)xalloc(1);
	retval[0] = (char)'\0';
	return retval;
    }

    if((enumStruct.stringDb = (char *)xalloc(1024)) == (char *)NULL)
	return (char *)NULL;
    enumStruct.stringDb[0] = (char)'\0';
    enumStruct.nextPos = 0;
    enumStruct.space = 1024;
    (void)XrmEnumerateDatabase(db, &empty, &empty, XrmEnumAllLevels,
			       AppendEntry, (XPointer) &enumStruct);

    return enumStruct.stringDb;
}

int
XpAugmentAttributes(
     XpContextPtr pContext,
     XPAttributes class,
     char *attributes)
{
    XrmDatabase db;
    ContextAttrPtr pCtxtAttrs;

    db = XrmGetStringDatabase(attributes);
    if(db == (XrmDatabase)NULL) return BadAlloc;

    pCtxtAttrs = (ContextAttrPtr)dixLookupPrivate(&pContext->devPrivates,
						  attrCtxtPrivKey);
    switch(class)
    {
	case XPPrinterAttr:
	    XrmMergeDatabases(db, &pCtxtAttrs->printerAttrs);
	    break;
	case XPDocAttr:
	    XrmMergeDatabases(db, &pCtxtAttrs->docAttrs);
	    break;
	case XPJobAttr:
	    XrmMergeDatabases(db, &pCtxtAttrs->jobAttrs);
	    break;
	case XPPageAttr:
	    XrmMergeDatabases(db, &pCtxtAttrs->pageAttrs);
	    break;
	default:
	    break;
    }
    return Success;
}

/*
 * XpSetAttributes - sets the attribute stores for a specified context.
 */
int
XpSetAttributes(
     XpContextPtr pContext,
     XPAttributes class,
     char *attributes)
{
    XrmDatabase db;
    ContextAttrPtr pCtxtAttrs;

    db = XrmGetStringDatabase(attributes);
    if(db == (XrmDatabase)NULL) return BadAlloc;

    pCtxtAttrs=(ContextAttrPtr)dixLookupPrivate(&pContext->devPrivates,
						attrCtxtPrivKey);
    switch(class)
    {
	case XPPrinterAttr:
	    if(pCtxtAttrs->printerAttrs != (XrmDatabase)NULL)
	        XrmDestroyDatabase(pCtxtAttrs->printerAttrs);
	    pCtxtAttrs->printerAttrs = db;
	    break;
	case XPDocAttr:
	    if(pCtxtAttrs->docAttrs != (XrmDatabase)NULL)
	        XrmDestroyDatabase(pCtxtAttrs->docAttrs);
	    pCtxtAttrs->docAttrs = db;
	    break;
	case XPJobAttr:
	    if(pCtxtAttrs->jobAttrs != (XrmDatabase)NULL)
	        XrmDestroyDatabase(pCtxtAttrs->jobAttrs);
	    pCtxtAttrs->jobAttrs = db;
	    break;
	case XPPageAttr:
	    if(pCtxtAttrs->pageAttrs != (XrmDatabase)NULL)
	        XrmDestroyDatabase(pCtxtAttrs->pageAttrs);
	    pCtxtAttrs->pageAttrs = db;
	    break;
	default:
	    break;
    }
    return Success;
}

void
XpAddPrinterAttribute(
    char *printerName,
    char *printerQualifier,
    char *attributeName,
    char *attributeValue)
{
    PrAttrPtr pAttr;

    for(pAttr = attrList; pAttr != (PrAttrPtr)NULL; pAttr = pAttr->next)
    {
	if(!strcmp(printerQualifier, pAttr->qualifier))
	{
            XrmPutStringResource(&pAttr->printerAttrs, attributeName, 
				 attributeValue);
	    break;
	}
    }
}

const char *
XpGetPrinterAttribute(const char *printerName,
		      const char *attribute)
{
    PrAttrPtr pAttr;
    XrmValue value;
    char *type;

    for(pAttr = attrList; pAttr != (PrAttrPtr)NULL; pAttr = pAttr->next)
    {
        if(!strcmp(printerName, pAttr->qualifier))
        {
	    char *attrStr;

	    attrStr = (char *)xalloc(strlen(printerName) + strlen(attribute) +
				     2);
	    sprintf(attrStr, "%s.%s", printerName, attribute);
            XrmGetResource(pAttr->printerAttrs, attrStr, attrStr,
                           &type, &value);
	    xfree(attrStr);
            break;
        }
    }
    if(value.addr != (XPointer)NULL && strlen(value.addr) != 0)
	return value.addr;
    else
      return "";
}

/*******************************************************************************
 *
 * The following routines are not attribute routines, but are rather
 * spooler interface functions.  They should presumably move to 
 * a SpoolerIf.c of some similarly named file.
 *
 ******************************************************************************/
#include <locale.h>

static char serverAttrStr[] = "*document-attributes-supported:	copy-count\n\
*job-attributes-supported:	job-name job-owner\
 notification-profile xp-spooler-command-options\n\
*multiple-documents-supported:	False";

XrmDatabase
XpSpoolerGetServerAttributes(void)
{
    char *totalAttrs, *localeName;
    XrmDatabase db;

    localeName = setlocale(LC_CTYPE, (char *)NULL);
    if(!localeName || strlen(localeName) == 0)
	localeName = "C";

    if((totalAttrs = (char *)xalloc(strlen(serverAttrStr) + strlen(localeName)
				    + 11)) == (char *)NULL)
	return (XrmDatabase)NULL;
    sprintf(totalAttrs, "%s\n%s\t%s", serverAttrStr, "*locale:", localeName);

    db =  XrmGetStringDatabase(totalAttrs);
    xfree(totalAttrs);
    return db;
}

/*
 * Tailf() works similar to "/bin/tail -f fd_in >fd_out" until
 * the process |child| terminates (the child status is
 * returned in |child_status|).
 * This function is used to copy the stdout/stderr output of a
 * child to fd_out until the child terminates.
 */
static
void Tailf(int fd_in, int fd_out, pid_t child, int *child_status)
{
    char           b[256];
    ssize_t        sz;
    Bool           childDone = FALSE;
    struct timeval timeout;
    long           fpos = 0; /* XXX: this is not correct for largefile support */

    timeout.tv_sec  = 0;
    timeout.tv_usec = 100000;

    for(;;)
    {
        /* Check whether the child is still alive or not */
        if (waitpid(child, child_status, WNOHANG) == child)
            childDone = TRUE;

        /* Copy traffic from |fd_in| to |fd_out|
         * (Note we have to use |pread()| here to avoid race conditions
         * between a child process writing to the same file using the
         * same file pointer (|dup(2)| and |fork(2)| just duplicate the
         * file handle but not the pointer)).
         */
        while ((sz = pread(fd_in, b, sizeof(b), fpos)) > 0)
        {
            fpos += sz;
            write(fd_out, b, sz);
        }

        if (childDone)
            break;

        (void)select(0, NULL, NULL, NULL, &timeout);
    }
}

/*
 * SendFileToCommand takes three character pointers - the file name,
 * the command to execute,
 * and the "argv" style NULL-terminated vector of arguments for the command.
 * The command is exec'd, and the file contents are sent to the command
 * via stdin.
 *
 * WARNING:  This function will try to adopt the userId of the supplied
 *           user name prior to exec'ing the supplied command.
 */
static void
SendFileToCommand(
    XpContextPtr pContext,
    char *fileName,
    char *pCommand,
    char **argVector,
    char *userName)
{
    pid_t childPid;
    int pipefd[2];
    int status;
    struct stat statBuf;
    FILE *fp, *outPipe;
    FILE *resFp; /* output from launched command */
    int   resfd;
    
    resFp = tmpfile();
    if (resFp == NULL)
    {
        ErrorF("SendFileToCommand: Cannot open temporary file for command output\n");
        return;
    }
    resfd = fileno(resFp);

    if(pipe(pipefd))
    {
        ErrorF("SendFileToCommand: Cannot open pipe\n");
        fclose(resFp);
        return;
    }

    if(stat(fileName, &statBuf) < 0 || (int)statBuf.st_size == 0)
    {
        close(pipefd[0]);
        close(pipefd[1]);
        fclose(resFp);
        return;
    }

    fp = fopen(fileName, "r");
    if(fp == (FILE *)NULL)
    {
        ErrorF("SendFileToCommand: Cannot open scratch spool file '%s'\n", fileName);
        close(pipefd[0]);
        close(pipefd[1]);
        fclose(resFp);
        return;
    }
    
    if((childPid = fork()) == 0)
    {
        close(pipefd[1]);

        /* Replace current stdin with input from the pipe */
	close(STDIN_FILENO);
	dup(pipefd[0]);
	close(pipefd[0]);

        /* Close current stdout and redirect it to resfd */
        close(STDOUT_FILENO);
        dup(resfd);

        /* Close current stderr and redirect it to resfd
         * (valgrind may not like that, in this case simply start it using
         * % valgrind 50>/dev/tty --logfile-fd=50 <more-options> ./Xprt ... #)
         */
        close(STDERR_FILENO);
        dup(resfd);

        fclose(resFp);
        
	/*
	 * If a user name is specified, try to set our uid to match that
	 * user name.  This is to allow e.g. a banner page to show the
	 * name of the printing user rather than the user who started
	 * the print server.
	 */
	if(userName)
	{
	    uid_t myUid;

	    if((myUid = geteuid()) == (uid_t)0)
	    {
	        struct passwd *pPasswd;

	        if((pPasswd = getpwnam(userName)))
	        {
                    if (setgid((gid_t)pPasswd->pw_gid) != 0)
                        perror("SendFileToCommand: setgid() failure.");

                    if (initgroups(userName, (gid_t)pPasswd->pw_gid) != 0)
                        perror("SendFileToCommand: initgroups() failure.");

                    if (setuid((uid_t)pPasswd->pw_uid) != 0)
                        perror("SendFileToCommand: setuid() failure.");
	        }
	    }
	}
	/* return BadAlloc? */
	if (execv(pCommand, argVector) == -1) {
	    FatalError("unable to exec '%s'", pCommand);
	}
    }
    else
    {
	(void) close(pipefd[0]);

 	outPipe = fdopen(pipefd[1], "w");
	(void) TransferBytes(fp, outPipe, (int)statBuf.st_size);
	
	(void) fclose(outPipe);
	(void) fclose(fp);

        /* Wait for spooler child (and send all it's output to stderr) */
        Tailf(resfd, STDERR_FILENO, childPid, &status);
        
        if (status != EXIT_SUCCESS)
        {
            ErrorF("SendFileToCommand: spooler command returned non-zero status %d.\n", status);
        }

        /* Store "xp-spooler-command-results" XPJobAttr that the
         * client can fetch it on demand */
        if ((fstat(resfd, &statBuf) >= 0) && (statBuf.st_size >= 0))
        {
            long  bufSize;
            char *buf;

            bufSize = statBuf.st_size;

            /* Clamp buffer size to 4MB to prevent that we allocate giant 
             * buffers if the spooler goes mad and spams it's stdout/stderr
             * channel. */
            bufSize = MIN(bufSize, 4*1024*1024);

            buf = xalloc(bufSize+1);
            if (buf != NULL)
            {
                bufSize = pread(resfd, buf, bufSize, 0);
                buf[bufSize]='\0';

                /* XXX: This should be converted from local multibyte encoding to
                 * Compound Text encoding first */
                XpPutOneAttribute(pContext, XPJobAttr, "xp-spooler-command-results", buf);

                xfree(buf);
            }
        }
        else
        {
            ErrorF("SendFileToCommand: fstat() failed.\n");
        }

        fclose(resFp);
    }
    return;
}

/*
 * ReplaceAllKeywords causes all the predefined keywords (e.g. %options%)
 * to be replaced with the appropriate values derived from the attribute
 * store for the supplied print context.  The ReplaceAnyString utility
 * routine is used to perform the actual replacements.
 */

static char *
ReplaceAllKeywords(
    XpContextPtr pContext,
    char *command)
{
    char *cmdOpt;

    cmdOpt = XpGetOneAttribute(pContext, XPPrinterAttr, 
			       "xp-spooler-printer-name");
    if(cmdOpt != (char *)NULL && strlen(cmdOpt) != 0)
        command = ReplaceAnyString(command, "%printer-name%", cmdOpt);
    else
        command = ReplaceAnyString(command, "%printer-name%", 
			           pContext->printerName);

    cmdOpt = XpGetOneAttribute(pContext, XPDocAttr, "copy-count");
    if(cmdOpt != (char *)NULL && strlen(cmdOpt) != 0)
        command = ReplaceAnyString(command, "%copy-count%", cmdOpt);
    else
        command = ReplaceAnyString(command, "%copy-count%", "1");

    cmdOpt = XpGetOneAttribute(pContext, XPJobAttr, "job-name");
    if(cmdOpt != (char *)NULL && strlen(cmdOpt) != 0)
        command = ReplaceAnyString(command, "%job-name%", cmdOpt);
    else
        command = ReplaceAnyString(command, "%job-name%", "");

    cmdOpt = XpGetOneAttribute(pContext, XPJobAttr, "job-owner");
    if(cmdOpt != (char *)NULL && strlen(cmdOpt) != 0)
        command = ReplaceAnyString(command, "%job-owner%", cmdOpt);
    else
        command = ReplaceAnyString(command, "%job-owner%", "");

    cmdOpt = XpGetOneAttribute(pContext, XPJobAttr, 
			       "xp-spooler-command-options");
    if(cmdOpt != (char *)NULL && strlen(cmdOpt) != 0)
        command = ReplaceAnyString(command, "%options%", cmdOpt);
    else
        command = ReplaceAnyString(command, "%options%", "");

    /* New in xprint.mozdev.org release 007 - replace "%xpconfigdir%" with
     * location of $XPCONFIGDIR */
    command = ReplaceAnyString(command, "%xpconfigdir%", XpGetConfigDirBase());

    return command;
}

#ifdef __QNX__
#define toascii( c ) ((unsigned)(c) & 0x007f)
#endif

#if defined(CSRG_BASED) || \
    defined(linux) || \
    defined(__CYGWIN__) || \
    (defined(sun) && !defined(SVR4)) || \
    (defined(SVR4) && !defined(sun) && !defined(__UNIXWARE__)) || \
    defined(ISC) || \
    defined(Lynx) || \
    defined(__QNX__) || \
    defined(__APPLE__)
#define iswspace(c) (isascii(c) && isspace(toascii(c)))
#endif

/*
 * GetToken - takes in a string and returns a malloc'd copy of the
 * first non-white-space sequence of characters in the string.
 * It returns the number of _bytes_ (NOT characters) parsed through 
 * the inStr to get to the end of the returned token.
 */
static int
GetToken(
    char *inStr,
    char **outStr)
{
    size_t mbCurMax = MB_CUR_MAX;
    wchar_t curChar;
    int i, numBytes, byteLen = strlen(inStr);
    char *tok;

    /*
     * read through any leading white space.
     */
    for(i = 0, numBytes = 0; i < byteLen; i += numBytes)
    {
        numBytes = mbtowc(&curChar, &inStr[i], mbCurMax);
        if(!iswspace(curChar))
	    break;
    }
    tok = inStr + i;

    /*
     * find the end of the token.
     */
    byteLen = strlen(tok);
    for(i = 0, numBytes = 0; i < byteLen; i += numBytes)
    {
        numBytes = mbtowc(&curChar, &tok[i], mbCurMax);
        if(iswspace(curChar))
	    break;
    }

    if((*outStr = (char *)xalloc(i + 1)) == (char *)NULL)
	return 0;
    strncpy(*outStr, tok, i);
    (*outStr)[i] = (char)'\0';
    return (tok + i) - inStr;
}

static void
FreeVector(
    char **vector)
{
    int i;

    if(vector == (char **)NULL) return;

    for(i = 0; vector[i] != (char *)NULL; i++)
	xfree(vector[i]);
    xfree(vector);
}


/*
 * AddVector appends the pAddition arg vector to the pTarget arg vector.
 * If the pTarget cannot be realloc'd, then pTarget is set to NULL.
 */
static void
AddVector(
    char ***pTarget,
    char **pAddition)
{
    int numTarget, numAdd, i;

    for(numTarget = 0; (*pTarget)[numTarget] != (char *)NULL; numTarget++)
	;
    for(numAdd = 0; pAddition[numAdd] != (char *)NULL; numAdd++)
	;

    *pTarget = (char **)xrealloc((void *)*pTarget, (numTarget + numAdd + 1) * 
	       sizeof(char *));
    if(*pTarget == (char **)NULL)
	return;
    for(i = 0; i < numAdd; i++)
	(*pTarget)[numTarget + i] = pAddition[i];

    (*pTarget)[numTarget + numAdd] = (char *)NULL;
}

static char **
BuildArgVector(
    char *argString,
    XpContextPtr pContext)
{
    char **pVector;
    char *curTok;
    int numChars, i;
    static int beenHere = 0; /* prevent recursion on embedded %options%
			     */

    pVector = (char **)xalloc(sizeof(char *));
    pVector[0] = (char *)NULL;
    for(i = 0; (numChars = GetToken(argString, &curTok)) != 0; 
	i++, argString += numChars)
    {
	if(beenHere || strcmp(curTok, "%options%"))
	{
	    if(curTok[0] == (char)'\0')
	    {
		xfree(curTok);
	    }
	    else
	    {
	        pVector = (char **)xrealloc((void *)pVector,
					    (i + 2)*sizeof(char *));
	        if(pVector == (char **)NULL)
	            return (char **)NULL;
	        pVector[i] = curTok;
	        pVector[i + 1] = (char *)NULL;
	    }
	}
	else if(!beenHere)
	{
	    char **optionsVec;

	    curTok = ReplaceAllKeywords(pContext, curTok);
	    beenHere = 1;
	    optionsVec = BuildArgVector(curTok, pContext);
	    xfree(curTok);
	    beenHere = 0;
	    AddVector(&pVector, optionsVec);
	    xfree(optionsVec);
	}
    }
    if(numChars == 0 && curTok != (char *)NULL)
	xfree(curTok);
    return pVector;
}

/*
 * VectorizeCommand takes a string and breaks it into a command name and
 * an array of character pointers suitable for handing to execv.  The
 * array is NULL-terminated.
 * The returned char * is the command name, and should be freed when no
 * longer needed.  The array elements returned in the pVector parameter 
 * should be individually freed, and the array itself should also be
 * freed when no longer needed.
 */
static char *
VectorizeCommand(
    char *command,
    char ***pVector,
    XpContextPtr pContext)
{
    char *cmdName;
    int numChars;

    if(command == (char *)NULL)
	return (char *)NULL;
    
    numChars = GetToken(command, &cmdName);

    if(cmdName == (char *)NULL)
	return (char *)NULL;

    /* Mangle the command name, too... */
    cmdName = ReplaceAllKeywords(pContext, cmdName);

    if(cmdName == (char *)NULL)
	return (char *)NULL;

    *pVector = BuildArgVector(command, pContext);
    
    return cmdName;
}

int
XpSubmitJob(fileName, pContext)
     char *fileName;
     XpContextPtr pContext;
{
    char **vector, *cmdNam, *command, *userName;
    int i;

    command = XpGetOneAttribute(pContext, XPPrinterAttr, "xp-spooler-command");
    if(command == (char *)NULL || strlen(command) == 0)
    {
        if( spooler_type )
        {
	    command = strdup(spooler_type->spool_command);
        }
        else
        {
            ErrorF("XpSubmitJob: No default spool command defined.\n");
        }
    }
    else
    {
	command = strdup(command);
    }
    if(command == (char *)NULL)
    {
        ErrorF("XpSubmitJob: No spooler command found, cannot submit job.\n");
	return BadAlloc;
    }
    
    cmdNam = VectorizeCommand(command, &vector, pContext);
    xfree(command);

    if(cmdNam == (char *)NULL)
	return BadAlloc;

    for(i = 0; vector[i] != (char *)NULL; i++)
    {
        vector[i] = ReplaceAllKeywords(pContext, vector[i]);
	if(vector[i] == (char *)NULL)
	{
	    xfree(cmdNam);
	    for(i = 0; vector[i] != (char *)NULL; i++)
		xfree(vector[i]);
	    xfree(vector);
	    return BadAlloc;
	}
    }

    userName = XpGetOneAttribute(pContext, XPJobAttr, "job-owner");
    if(userName != (char *)NULL && strlen(userName) == 0)
	userName = (char *)NULL;

    SendFileToCommand(pContext, fileName, cmdNam, vector, userName);

    FreeVector(vector);
    xfree(cmdNam);
    
    return Success;
}

/*
 * SearchInputTrays()
 *
 * Given a tray, return the medium in the tray.  Conversely, given a
 * medium, return a tray in which it can be found.  In either case,
 * return NULL if the given tray or medium cannot be found.
 */
#define TRAY 0
#define MEDIUM 1

static char *
SearchInputTrays(XpContextPtr pCon,
		 int which,
		 char *val)
{
    char *inputTraysMedium, tray[80], medium[80], *copy;
    char *pS, *pE, *pLast;
    
    inputTraysMedium = XpGetOneAttribute( pCon, XPPrinterAttr,
					 "input-trays-medium" );
    
    copy = strdup( inputTraysMedium );
    pS = copy;
    pLast = copy + strlen( copy );
    
    while( pS < pLast )
      {
	  while( *pS && *pS != '{' )
	    pS++;
	  
	  pE = ++pS;
	  while( *pE && *pE != '}' )
	    pE++;
	  *pE = '\0';

	  sscanf( pS, "%s %s", tray, medium );

	  if( which == MEDIUM && !strcmp( val, medium ) )
	    {
		xfree( copy );
		return strdup( tray );
	    }

	  if( which == TRAY && !strcmp( val, tray ) )
	    {
		xfree( copy );
		return strdup( medium );
	    }
	  
	  pS = pE + 1;
      }

    xfree( copy );
    return strdup( NULL_STRING );
}

/*
 * XpGetTrayMediumFromContext()
 *
 * Given a print context, hit the input-trays-medium,
 * default-input-tray and default-medium attributes to find the
 * appropriate tray to use, and the medium in that tray.
 */
void
XpGetTrayMediumFromContext(XpContextPtr pCon,
			   char **medium,
			   char **tray)
{
    char *defMedium, *defTray;
    char *t, *m;
    
    defMedium = XpGetOneAttribute( pCon, XPPageAttr, 
				  "default-medium" );
    if( *defMedium == '\0' )
      defMedium = XpGetOneAttribute( pCon, XPDocAttr,
				    "default-medium" );

    defTray = XpGetOneAttribute( pCon, XPPageAttr,
				"default-input-tray" );
    if( *defTray == '\0' )
      defTray = XpGetOneAttribute( pCon, XPDocAttr,
				  "default-input-tray" );

    /*
     * First, check to see if the default tray has the default medium
     * installed.  This is the ideal case.
     */
    m = SearchInputTrays( pCon, TRAY, defTray );
    if( !strcmp( m, defMedium ) )
      {
	  xfree( m );
	  *tray = strdup( defTray );
	  *medium = strdup( defMedium );
	  return;
      }

    /*
     * If the default tray doesn't have the default medium, search for
     * a tray which has the default medium.
     */
    t = SearchInputTrays( pCon, MEDIUM, defMedium );
    if( t )
      {
	  *tray = t;
	  *medium = strdup( defMedium );
	  return;
      }
    
    /*
     * If all else fails, just return the default tray, and whatever
     * medium happens to be there.  Note that we simply return
     * whatever is in the attribute store.  Any further correction is
     * left up to the DDX driver.
     */
    *tray = strdup( defTray );
    *medium = m;
    xfree( t );
}
