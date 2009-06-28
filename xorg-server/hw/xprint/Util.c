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

/* To get the tempnam() prototype in <stdio.h> */
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#if defined(linux) && defined(__STRICT_ANSI__)
#undef __STRICT_ANSI__
#endif

#include <X11/Xos.h>	/* for unistd.h and string.h */
#include <stdio.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "misc.h"
#include "dixstruct.h"

#include <X11/extensions/Print.h>

#include "attributes.h"

#define IN_FILE_STRING "%(InFile)%"
#define OUT_FILE_STRING          "%(OutFile)%"

/*
 * ReplaceAnyString returns a string combining the input strings.
 * It replaces all occurances of 'target' with the supplied
 * 'replacement'.
 * The original input string will generally be freed, 
 * and the caller is responsible for freeing whatever string is returned.
 */
char *
ReplaceAnyString(
    char *string, 
    char *target, 
    char *replacement)
{
    char *pKeyString;

    if(replacement != (char *)NULL)
    {
        while((pKeyString = strstr(string, target)) != (char *)NULL)
        {
	    char *newString;
    
	    newString = (char *)xalloc(strlen(string) + strlen(replacement) - 
				       strlen(target) + 1);
	    strncpy(newString, string, pKeyString - string);
	    newString[pKeyString - string] = '\0';
	    strcat(newString, replacement);
	    strcat(newString, pKeyString + strlen(target));
	    xfree(string);
	    string = newString;
        }
    }

    return string;
}

/*
 * ReplaceFileString returns a string combining the input strings.
 * It replaces all occurances of IN_FILE_STRING with the supplied
 * inFileName, and all occurances of OUT_FILE_STRING with the
 * supplied outFileName.  The original input string will generally be freed, 
 * and the caller is responsible for freeing whatever string is returned.
 */
char *
ReplaceFileString(
    char *string,
    char *inFileName,
    char *outFileName)
{
    char *pKeyString,
	 *pInFileString = IN_FILE_STRING,
	 *pOutFileString = OUT_FILE_STRING;

    if(inFileName != (char *)NULL)
    {
        while((pKeyString = strstr(string, pInFileString)) != 
	      (char *)NULL)
        {
	    char *newString;
    
	    newString = (char *)xalloc(strlen(string) + 
				        strlen(inFileName) + 1);
	    strncpy(newString, string, pKeyString - string);
	    newString[pKeyString - string] = '\0';
	    strcat(newString, inFileName);
	    strcat(newString, pKeyString + strlen(pInFileString));
	    xfree(string);
	    string = newString;
        }
    }

    if(outFileName != (char *)NULL)
    {
        while((pKeyString = strstr(string, pOutFileString)) != 
	      (char *)NULL)
        {
	    char *newString;
    
	    newString = (char *)xalloc(strlen(string) + 
				        strlen(outFileName) + 1);
	    strncpy(newString, string, pKeyString - string);
	    newString[pKeyString - string] = '\0';
	    strcat(newString, outFileName);
	    strcat(newString, pKeyString + strlen(pOutFileString));
	    xfree(string);
	    string = newString;
        }
    }
    return string;
}


/*
 * TransferBytes reads numBytes of data from pSrcFile and writes them
 * to pDstFile.  It returns the number of bytes actually transfered,
 * which will be numBytes if it's successful.  Neither pSrcFile nor
 * pDstFile are rewound or their pointers otherwise modified prior to
 * beginning the transfer.
 */
int
TransferBytes(
    FILE *pSrcFile,
    FILE *pDstFile,
    int numBytes)
{
    char buf[10240];
#define BUF_SIZE (sizeof(buf)*sizeof(char))
    int bytesWritten = 0;
    unsigned bytesToXfer;

    for(bytesToXfer = min(BUF_SIZE, (unsigned)numBytes);
        bytesToXfer > 0;
	bytesToXfer = min(BUF_SIZE, (unsigned)(numBytes - bytesWritten)))
    {
	if(fread((void *)buf, (size_t) 1, bytesToXfer, pSrcFile) < bytesToXfer)
	    return bytesWritten;
	if(fwrite((void *)buf, (size_t) 1, bytesToXfer, pDstFile) < bytesToXfer)
	    return bytesWritten;
	bytesWritten += bytesToXfer;
    }
    return bytesWritten;
}

/*
 * CopyContentsAndDelete - does the work of copying and deleting the 
 * pre, no, and post raster files as well as the raster file itself.
 */
Bool
CopyContentsAndDelete(
    FILE **ppSrcFile,
    char **pSrcFileName,
    FILE *pDstFile)
{
    struct stat statBuf;

    if(stat(*pSrcFileName, &statBuf) < 0)
        return FALSE;
    rewind(*ppSrcFile);
    if(TransferBytes(*ppSrcFile, pDstFile,
       (int)statBuf.st_size) != (int)statBuf.st_size)
        return FALSE;
    fclose(*ppSrcFile);
    *ppSrcFile = (FILE *)NULL;
    unlink(*pSrcFileName);
    xfree(*pSrcFileName);
    *pSrcFileName = (char *)NULL;

    return TRUE;
}


#define QUADPAD(x) ((((x)+3)>>2)<<2)

int
XpSendDocumentData(
    ClientPtr client,
    FILE *fp,
    int fileLen,
    int maxBufSize)
{
    xPrintGetDocumentDataReply *pRep;
    int bytesWritten;
    unsigned bytesToWrite;
    int result = Success;

    if(client->clientGone)
	return Success;

    pRep = (xPrintGetDocumentDataReply *)xalloc(sz_xPrintGetDocumentDataReply+ 
	   QUADPAD(maxBufSize));

    for(bytesToWrite = min(maxBufSize, fileLen),
	bytesWritten = 0;
	bytesToWrite > 0;
        bytesToWrite = min(maxBufSize, fileLen - bytesWritten))
    {
        pRep->type = X_Reply;
        pRep->sequenceNumber = client->sequence;
        pRep->length = (QUADPAD(bytesToWrite)) >> 2;
	pRep->dataLen = bytesToWrite;

	if(fread((void *)(pRep + 1), 1, bytesToWrite, fp) < bytesToWrite)
	{
	    result = BadAlloc; /* XXX poor error choice? */
	    pRep->statusCode = 2; /* XXX Is this the right value??? */
	}
	else
	    pRep->statusCode = 0; /* XXX Ignored??? */

        pRep->finishedFlag = FALSE;

        if (client->swapped) {
	    int n;
	    long l;

            swaps(&pRep->sequenceNumber, n);
            swapl(&pRep->length, l);
            swapl(&pRep->statusCode, l); /* XXX Why are these longs??? */
            swapl(&pRep->finishedFlag, l); /* XXX Why are these longs??? */
            swapl(&pRep->dataLen, l);
	}

	(void)WriteToClient(client,
			    sz_xPrintGetDocumentDataReply + bytesToWrite, 
			    (char *)pRep);
	bytesWritten += bytesToWrite;
    }

    xfree(pRep);
    return result;
}

/*
 * XpFinishDocData - send a DocumentData reply with the "finishedFlag"
 * field set to TRUE.  This routine should be called from the EndJob
 * function of a driver after the driver has sent all required
 * document data (presumably via XpSendDocumentData).
 */
int
XpFinishDocData(
    ClientPtr client)
{
    xPrintGetDocumentDataReply rep;

    if(client->clientGone)
	return Success;

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = 0;
    rep.dataLen = 0;
    rep.finishedFlag = TRUE;
    rep.statusCode = 0;

    if (client->swapped) {
        int n;
        long l;

	swaps(&rep.sequenceNumber, n);
	swapl(&rep.length, l);
	swapl(&rep.statusCode, l); /* XXX Why are these longs??? */
	swapl(&rep.finishedFlag, l); /* XXX Why are these longs??? */
	swapl(&rep.dataLen, l);
    }

    (void)WriteToClient(client, sz_xPrintGetDocumentDataReply, (char *)&rep);
    return Success;
}

#ifndef HAVE_MKSTEMP
static
char *XpDirName(char *fname)
{
    char *fn, *ptr;

    fn = (char *)xalloc(strlen(fname) + 1);
    if (fn) {
	strcpy(fn, fname);
	ptr = strrchr(fn, '/');
	if (!ptr) {
	    ptr = fn;
	    *ptr++ = '.';
	} else if (ptr == fn)
	    ptr++;
	*ptr = '\0';
    }
    return fn;
}
#endif

Bool
XpOpenTmpFile(
    char *mode,
    char **fname,
    FILE **stream)
{
#ifndef HAVE_MKSTEMP
    char *fn = NULL;

    /* note that there is a small race condition here... */
    if (!(*fname = tempnam(NULL, NULL)) || 
	!(fn = XpDirName(*fname)) ||
	access(fn, W_OK) ||
	!(*stream = fopen(*fname, mode)))
	
    {
	xfree(fn);
	xfree(*fname);
	*fname = NULL;
	*stream = NULL;
	return FALSE;
    }
    xfree(fn);
#else
    int fd;
    
    *stream = NULL;
    *fname = (char *)xalloc(14);
    if (*fname == NULL) 
	return FALSE;
    strcpy(*fname, "/tmp/xpXXXXXX");
    fd = mkstemp(*fname);
    if (fd < 0) {
	xfree(*fname);
	*fname = NULL;
	return FALSE;
    }
    *stream = fdopen(fd, mode);
    if (stream == NULL) {
	xfree(*fname);
	*fname = NULL;
	return FALSE;
    }
#endif
    return TRUE;
}
