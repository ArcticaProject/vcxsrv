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

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "attributes.h"

/*
 * XpOidNotify value strings
 */
#define NOTIFY_EMAIL_STR "{{event-report-job-completed} electronic-mail}"
#define NOTIFY_NONE_STR  "{}"

#define SafeStrLen(s) ((s) ? strlen((s)) : 0)

/*
 * entry type for the object identifier string map
 */
typedef struct _XpOidStringMapEntry
{
    const char* string;
    int length;
    int msg_set;
    int msg_number;
    const char* default_message;
    
} XpOidStringMapEntry;

/*
 * include the auto-generated static XpOidStringMap
 */
#include "OidStrs.h"

/*
 * XpOid static function declarations
 */
static XpOid XpOidParse(const char* value_string,
			const char** ptr_return);
/*
 * XpOidList static function declarations
 */
static XpOidList* XpOidListParse(const char* value_string,
				 const XpOidList* valid_oids,
				 const char** ptr_return, int i);

/*
 * XpOidList static function declarations
 */
static XpOidCardList* XpOidCardListParse(const char* value_string,
					 const XpOidCardList* valid_cards,
					 const char** ptr_return, int i);

/*
 * XpOidMediumSourceSize static function declarations
 */
static XpOidMediumSS* MediumSSParse(const char* value_string,
				    const XpOidList* valid_trays,
				    const XpOidList* valid_medium_sizes,
				    const char** ptr_return, int i);
static XpOidMediumContinuousSize* MediumContinuousSizeParse(const char*,
							     const char**);
static void MediumContinuousSizeDelete(XpOidMediumContinuousSize* me);
static XpOidMediumDiscreteSizeList* MediumDiscreteSizeListParse(const char*,
								const XpOidList*,
								const char**,
								int i);
static void MediumDiscreteSizeListDelete(XpOidMediumDiscreteSizeList* list);

static BOOL ParseArea(const char* value_string,
		      const char** ptr_return,
		      XpOidArea* area_return);
static BOOL ParseRealRange(const char* value_string,
			   const char** ptr_return,
			   XpOidRealRange* range_return);

/*
 * XpOidTrayMediumList static function declarations
 */
static XpOidTrayMediumList* TrayMediumListParse(const char* value_string,
						const XpOidList* valid_trays,
						const char** ptr_return,
						int i);
static void TrayMediumListValidate(XpOidTrayMediumList* me,
				   const XpOidMediumSS* msss);

/*
 * XpOidDocFmt
 */
static BOOL XpOidDocFmtNext(XpOidDocFmt* doc_fmt,
			    const char* value_string,
			    const char** ptr_return);

/*
 * XpOidDocFmtListParse
 */
static XpOidDocFmtList* XpOidDocFmtListParse(const char* value_string,
					     const XpOidDocFmtList* valid_fmts,
					     const char** ptr_return, int i);

/*
 * misc. parsing static function declarations
 */
static BOOL ParseBoolValue(const char* value_string,
			   const char** ptr_return,
			   BOOL* bool_return);
static BOOL ParseRealValue(const char* value_string,
			   const char** ptr_return,
			   float* real_return);
static BOOL ParseSeqEnd(
			const char* value_string,
			const char** ptr_return);
static BOOL ParseSeqStart(
			  const char* value_string,
			  const char** ptr_return);
static BOOL ParseUnspecifiedValue(
				  const char* value_string,
				  const char** ptr_return);
static int SpanToken(
		     const char* string);
static int SpanWhitespace(
			  const char* string);

/*
 * String comparison function.
 */
#ifdef HAVE_STRCASECMP
# define StrnCaseCmp(s1, s2, len) strncasecmp(s1, s2, len)
#else
static int StrnCaseCmp(const char *s1, const char *s2, size_t len);
#endif

/*
 * ------------------------------------------------------------------------
 * Name: XpOidString
 *
 * Description:
 *
 *     Obtain the string representation of an XpOid.
 *
 *     Example: XpOidString(xpoid_copy_count) returns "copy-count".
 *
 * Return value:
 *
 *     A const pointer to the string.
 */
const char*
XpOidString(XpOid xp_oid)
{
    /*
     * XpOid enum values are index values into the string map
     */
    return XpOidStringMap[xp_oid].string;
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidStringLength
 *
 * Description:
 *
 *     Obtain the length of the string representation for a given
 *     XpOid.
 *
 * Return value:
 *
 *     The string length in bytes.
 *
 */
int
XpOidStringLength(XpOid xp_oid)
{
    /*
     * XpOid enum values are index values into the string map
     */
    return XpOidStringMap[xp_oid].length;
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidFromString
 *
 * Description:
 *
 *     Obtains the XpOid given a string representation of an XpOid.
 *
 *     Example: XpOidFromString("copy-count") returns 'xpoid_copy_count'.
 *
 * Return value:
 *
 *     The XpOid if successful. 'xpoid_none' if the string pointed to by
 *     'value is not recognized or if 'value' is NULL.
 */
XpOid
XpOidFromString(const char* value)
{
    if(value == (const char*)NULL)
	return xpoid_none;
    else
	return XpOidParse(value, (const char**)NULL);
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidParse
 *
 * Description:
 *
 *     Parse the next whitespace-delimited string from 'value_string'
 *     updating 'ptr_return' to point to the next unparsed location in
 *     'value_string'. 'ptr_return' can be NULL.
 *
 * Return value:
 *
 *     The corresponding XpOid for the parsed name string.
 *     A return value of xpoid_none is returned if the parsed name
 *     was not a valid oid or if no name was found.
 *
 */
static XpOid
XpOidParse(const char* value_string,
	   const char** ptr_return)
{
    const char* ptr;
    int length;
    int i;
    /*
     * skip leading whitespace
     */
    ptr = value_string + SpanWhitespace(value_string);
    /*
     * get the whitespace-delimited token length
     */
    length = SpanToken(ptr);
    /*
     * match the oid string in the map
     */
    for(i = 0; i < XpOidStringMapCount; i++)
	if(length == XpOidStringMap[i].length)
	    if(strncmp(ptr, XpOidStringMap[i].string, length) == 0)
		break;
    if(i == XpOidStringMapCount)
	i =  xpoid_none;
    /*
     * update the return pointer and return
     */
    if(ptr_return != (const char**)NULL)
	*ptr_return = ptr+length;
    return i;
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidListNew
 *
 * Description:
 *
 *     Creates a new XpOidList initialized from a whitespace-delimited
 *     list of recognized string representations of oids. The returned
 *     list will contain only oids found within the passed 'valid_oids'
 *     XpOidList.
 *
 *     Note: One may notice that in order to create an XpOidList with
 * 	  this function, an XpOidList is needed; the 'valid_oids' list
 * 	  is often an statically initialized structure. XpOidListInit
 * 	  can also be used.
 *
 * Return value:
 *
 *     NULL if the passed 'value_string' is NULL.
 *     
 *     If the list indicated by 'value_string' is empty or contains only
 *     unrecognized oid string representations, a new XpOidList
 *     containing zero elements is returned.
 *
 *     If 'valid_oids' is NULL all oids are considered valid.
 *
 */
XpOidList*
XpOidListNew(const char* value_string,
	     const XpOidList* valid_oids)
{
    if(value_string == (const char*)NULL)
	return (XpOidList*)NULL;
    else
    {
	const char* ptr;
	return XpOidListParse(value_string, valid_oids, &ptr, 0);
    }
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidListDelete
 *
 * Description:
 *
 *     Frees the memory allocated for 'list'.
 *
 * Return value:
 *
 *     None.
 *
 */
void
XpOidListDelete(XpOidList* list)
{
    if(list != (XpOidList*)NULL)
    {
	XpOidFree((char*)list->list);
	XpOidFree((char*)list);
    }
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidListParse
 *
 * Description:
 *
 *     This function recursively parses the whitespace-delimited list of
 *     oid string representations passed via 'value_string'. Oids are
 *     only added to the resulting list if they are found within the
 *     passed 'valid_oids' XpOidList.
 *
 *     'ptr_return' points to a char* variable allocated by the
 *     caller, and is really only of use during recursion (upon return to
 *     the original caller, it will point to the end of value_string).
 *
 *     'value_string' and 'ptr_return' *cannot* be NULL.
 *
 * Return value:
 *
 *     A newly allocated and initialized XpOidList.
 *
 *     If the list indicated by 'value_string' is empty or contains only
 *     unrecognized oid string representations, a new XpOidList
 *     containing zero elements is returned.
 *
 *     If 'valid_oids' is NULL all oids are considered valid.
 *
 */
static XpOidList*
XpOidListParse(const char* value_string,
	       const XpOidList* valid_oids,
	       const char** ptr_return,
	       int i)
{
    XpOid oid;
    XpOidList* list;
    /*
     * parse the next valid oid out of the value string
     */
    ptr_return = &value_string;
    while(1)
    {
	if(**ptr_return == '\0')
	{
	    /*
	     * end of value string; stop parsing
	     */
	    oid = xpoid_none;
	    break;
	}
	/*
	 * parse the next oid from the value
	 */
	oid = XpOidParse(*ptr_return, ptr_return);
	if(xpoid_none == oid)
	{
	    /*
	     * unrecognized oid; keep parsing
	     */
	    continue;
	}
	if((const XpOidList*)NULL == valid_oids
	   ||
	   XpOidListHasOid(valid_oids, oid))
	{
	    /*
	     * valid oid found; stop parsing
	     */
	    break;
	}
    }
    
    if(oid == xpoid_none)
    {
	/*
	 * end of value string; allocate the list structure
	 */
	list = (XpOidList*)XpOidCalloc(1, sizeof(XpOidList));
	list->count = i;
	list->list = (XpOid*)XpOidCalloc(i, sizeof(XpOid));
    }
    else
    {
	/*
	 * recurse
	 */
	list = XpOidListParse(*ptr_return, valid_oids, ptr_return, i+1);
	/*
	 * set the oid in the list
	 */
	list->list[i] = oid;
    }
    /*
     * return
     */
    return list;
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidListHasOid
 *
 * Description:
 *
 *     Determines if 'oid' is an element of 'list'.        
 *
 * Return value:
 *
 *     xTrue if the oid is found in the list.
 *
 *     xFalse if the oid is not in the list, or if 'list' is NULL.
 *
 */
BOOL
XpOidListHasOid(const XpOidList* list, XpOid oid)
{
    int i;
    if(list != (XpOidList*)NULL)
	for(i = 0; i < list->count; i++)
	    if(list->list[i] == oid)
		return xTrue;
    return xFalse;
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidListGetIndex
 *
 * Description:
 *
 *     Returns the array index of 'oid' in 'list'    
 *
 * Return value:
 *
 *     The index of 'oid' in list.
 *
 *     -1 if the oid is not in the list, or if 'list' is NULL.
 *
 */
int
XpOidListGetIndex(const XpOidList* list, XpOid oid)
{
    int i;
    if(list != (XpOidList*)NULL)
	for(i = 0; i < list->count; i++)
	    if(list->list[i] == oid)
		return i;
    return -1;
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidListString
 *
 * Description:
 *
 *     Creates a string representation of an XpOidList structure.
 *
 * Return value:
 *
 *     A newly allocated     
 *
 */
char*
XpOidListString(const XpOidList* me)
{
    int i;
    int length;
    char* str;
    char* ptr;
    /*
     * allocate enough memory for the oid string representations,
     * including intervening whitespace
     */
    for(i = 0, length = 0; i < XpOidListCount(me); i++)
	length += XpOidStringLength(XpOidListGetOid(me, i)) + 1;
    str = XpOidMalloc(length+1);
    /*
     * format the list
     */
    for(i = 0, ptr = str; i < XpOidListCount(me); i++)
#if defined(sun) && !defined(SVR4)
    {
	sprintf(ptr, "%s ", XpOidString(XpOidListGetOid(me, i)));
	ptr += strlen(ptr);
    }
#else
	ptr += sprintf(ptr, "%s ", XpOidString(XpOidListGetOid(me, i)));
#endif
    /*
     * chop trailing whitespace or terminate empty string
     */
    str[length] = '\0';
    /*
     * return
     */
    return str;
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidLinkedListNew
 *
 * Description:
 *
 *     Creates a new instance of an empty XpOidLinkedList.
 *
 * Return value:
 *
 *     The new XpOidLinkedList.
 *
 */
XpOidLinkedList*
XpOidLinkedListNew()
{
    return (XpOidLinkedList*)XpOidCalloc(1, sizeof(XpOidLinkedList));
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidLinkedListDelete
 *
 * Description:
 *
 *     Frees the memory allocated for a XpOidLinkedList.
 *
 * Return value:
 *
 *     None.
 *
 */
void
XpOidLinkedListDelete(XpOidLinkedList* me)
{
    if(me != (XpOidLinkedList*)NULL)
    {
	while(me->head)
	{
	    me->current = me->head;
	    me->head = me->current->next;
	    XpOidFree((char*)me->current);
	}
	XpOidFree((char*)me);
    }
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidLinkedListGetOid
 *
 * Description:
 *
 *     Retrieves the oid at position 'i' (zero-based) in the
 *     XpOidLinkedList 'me'.
 *
 * Return value:
 *
 *     The oid at position 'i'.
 *
 *     xpoid_none if the oid was not found, or the list is empty (or if
 *     the list contains xpoid_none at position 'i').
 */
XpOid
XpOidLinkedListGetOid(XpOidLinkedList* me, int i)
{
    if(me == (XpOidLinkedList*)NULL || i < 0 || i >= me->count)
    {
	return xpoid_none;
    }
    else
    {
	me->current = me->head;
	while(i--) me->current = me->current->next;
	return me->current->oid;
    }
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidLinkedListAddOid
 *
 * Description:
 *
 *     Adds an oid to the end of an XpOidLinkedList.
 *
 * Return value:
 *
 *     None.
 *
 */
void
XpOidLinkedListAddOid(XpOidLinkedList* me, XpOid oid)
{
    me->current = (XpOidNode)XpOidCalloc(1, sizeof(struct XpOidNodeStruct));
    me->current->oid = oid;
    ++me->count;
    if(me->tail)
    {
	me->tail->next = me->current;
	me->tail = me->current;
    }
    else
	me->head = me->tail = me->current;
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidLinkedListGetIndex
 *
 * Description:
 *
 *     Returns the position of an oid in a XpOidLinkedList.
 *
 * Return value:
 *
 *     The zero-based position of 'oid' in the list.
 *
 *     -1 if the oid is not in the list, or if 'me' is NULL.
 *
 */
int
XpOidLinkedListGetIndex(XpOidLinkedList* me, XpOid oid)
{
    if((XpOidLinkedList*)NULL != me)
    {
	int i = 0;
	me->current = me->head;
	while(me->current)
	    if(me->current->oid == oid)
	    {
		return i;
	    }
	    else
	    {
		++i;
		me->current = me->current->next;
	    }
    }
    return -1;
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidLinkedListHasOid
 *
 * Description:
 *
 *     Determines if an oid is an element of a XpOidLinkedList.
 *
 * Return value:
 *
 *     xTrue if the oid is found in the list.
 *
 *     xFalse if the oid is not in the list, or if 'me' is NULL.
 */
BOOL
XpOidLinkedListHasOid(XpOidLinkedList* me,
		      XpOid oid)
{
    if((XpOidLinkedList*)NULL != me)
    {
	me->current = me->head;
	while(me->current)
	    if(me->current->oid == oid)
		return xTrue;
	    else
		me->current = me->current->next;
    }
    return xFalse;
}
		       
/*
 * ------------------------------------------------------------------------
 * Name: XpOidLinkedListFirstOid
 *
 * Description:
 *
 *     Positions the XpOidLinkedList 'current' pointer to the first entry
 *     in the list.
 *
 * Return value:
 *
 *     The first oid in the list, or xpoid_none if the list NULL or
 *     empty.
 */
XpOid
XpOidLinkedListFirstOid(XpOidLinkedList* me)
{
    if((XpOidLinkedList*)NULL != me && (me->current = me->head))
	return me->current->oid;
    else
	return xpoid_none;
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidLinkedListNextOid
 *
 * Description:
 *
 *     Positions the XpOidLinkedList 'current' pointer to the next entry
 *     in the list.
 *
 * Return value:
 *
 *     The next oid, or xpoid_none if the end of the list has been
 *     reached.
 */
XpOid
XpOidLinkedListNextOid(XpOidLinkedList* me)
{
    if(me->current ? (me->current = me->current->next) : xFalse)
	return me->current->oid;
    else
	return xpoid_none;
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidMediumSSNew
 *
 * Description:
 *
 *     Creates a new XpOidMediumSS initialized from a string value
 *     specified using the medium-source-sizes syntax. See
 *     MediumSSParse() below for parsing details.
 *
 * Return value:
 *
 *     NULL if the passed 'value_string' is NULL, or if a syntax error is
 *     encountered while parsing the medium-source-sizes value.
 *     
 */
XpOidMediumSS*
XpOidMediumSSNew(const char* value_string,
		 const XpOidList* valid_trays,
		 const XpOidList* valid_medium_sizes)
{
    if(value_string == (const char*)NULL)
	return (XpOidMediumSS*)NULL;
    else
    {
	const char* ptr = value_string + SpanWhitespace(value_string);
	if(*ptr == '\0')
	    return (XpOidMediumSS*)NULL;
	else
	    return MediumSSParse(ptr, valid_trays, valid_medium_sizes,
				 &ptr, 0);
    }
}

/*
 * ------------------------------------------------------------------------
 * Name: MediumSSParse
 *
 * Description:
 *
 *     'ptr_return' *cannot* be NULL.
 *     
 *
 * Return value:
 *
 *     
 *
 */
static XpOidMediumSS*
MediumSSParse(const char* value_string,
	      const XpOidList* valid_trays,
	      const XpOidList* valid_medium_sizes,
	      const char** ptr_return,
	      int i)
{
    XpOidMediumSS* medium_ss;
    XpOidMediumSourceSize mss;
    /*
     * check for the start of a new MediumSourceSize sequence
     */
    if(ParseSeqStart(value_string, ptr_return))
    {
	/*
	 * check for an unspecified tray value
	 */
	if(ParseUnspecifiedValue(*ptr_return, ptr_return))
	    mss.input_tray = xpoid_unspecified;
	else
	{
	    const char* tray_str;
	    *ptr_return += SpanWhitespace(*ptr_return);
	    tray_str = *ptr_return;
	    /*
	     * parse out the input tray
	     */
	    mss.input_tray = XpOidParse(*ptr_return, ptr_return);
	    if((const XpOidList*)NULL != valid_trays
	       &&
	       !XpOidListHasOid(valid_trays, mss.input_tray)
	       )
		mss.input_tray = xpoid_none;
	    if(xpoid_none == mss.input_tray)
	    {
		char* invalid_tray_str;
		int len = *ptr_return - tray_str;
		if(len > 0)
		{
		    invalid_tray_str = XpOidMalloc(len+1);
		    strncpy(invalid_tray_str, tray_str, len);
		    invalid_tray_str[len] = '\0';
		    ErrorF("%s\nInvalid tray (%s) found. Will attempt to continue parsing.\n",
			   XPMSG_WARN_MSS, invalid_tray_str);
		    XpOidFree(invalid_tray_str);
		}
	    }
	}
	/*
	 * attempt to parse a Continuous MediumSize sequence
	 */
	mss.ms.continuous_size =
	    MediumContinuousSizeParse(*ptr_return, ptr_return);
	if(mss.ms.continuous_size != (XpOidMediumContinuousSize*)NULL)
	{
	    mss.mstag = XpOidMediumSS_CONTINUOUS;
	}
	else
	{
	    /*
	     * not continuous, try Discrete MediumSize
	     */
	    mss.ms.discrete =
		MediumDiscreteSizeListParse(*ptr_return, valid_medium_sizes,
					    ptr_return, 0);
	    if(mss.ms.discrete == (XpOidMediumDiscreteSizeList*)NULL)
	    {
		const char* tray_str;
		/*
		 * syntax error (MediumDiscreteSizeListParse reports error)
		 */
		switch(mss.input_tray)
		{
		case xpoid_none:
		    tray_str = "an invalid";
		    break;
		case xpoid_unspecified:
		    tray_str = "default (tray specifier omitted)";
		    break;
		default:
		    tray_str = XpOidString(mss.input_tray);
		    break;
		}
		ErrorF("%s\nError occurred while parsing medium sizes for %s tray.\n",
		       XPMSG_WARN_MSS, tray_str);
		return NULL;
	    }
	    mss.mstag = XpOidMediumSS_DISCRETE;
	}
	/*
	 * parse out the MediumSourceSize sequence end
	 */
	if(!ParseSeqEnd(*ptr_return, ptr_return))
	{
	    /*
	     * syntax error
	     */
	    ErrorF("%s\nSequence End expected. Unparsed data: %s\n",
		   XPMSG_WARN_MSS, *ptr_return);
	    return NULL;
	}
	/*
	 * recurse to parse the next MediumSourceSize sequence
	 */
	medium_ss = MediumSSParse(*ptr_return,
				  valid_trays, valid_medium_sizes,
				  ptr_return,
				  xpoid_none == mss.input_tray ? i : i+1);
	if(medium_ss == (XpOidMediumSS*)NULL
	   ||
	   xpoid_none == mss.input_tray)
	{
	    /*
	     * syntax error or invalid tray - clean up
	     */
	    switch(mss.mstag)
	    {
	    case XpOidMediumSS_CONTINUOUS:
		MediumContinuousSizeDelete(mss.ms.continuous_size);
		break;
	    case XpOidMediumSS_DISCRETE:
		MediumDiscreteSizeListDelete(mss.ms.discrete);
		break;
	    }
	    if(medium_ss == (XpOidMediumSS*)NULL)
		/*
		 * syntax error - return
		 */
		return NULL;
	}
	if(xpoid_none != mss.input_tray)
	{
	    /*
	     * copy the current MediumSourceSize into the array
	     */
	    memmove((medium_ss->mss)+i, &mss, sizeof(XpOidMediumSourceSize));
	}
    }
    else
    {
	/*
	 * MediumSourceSize sequence start not found
	 */
	if(**ptr_return == '\0')
	{
	    if(0 == i)
	    {
		ErrorF("%s\nNo valid trays found.\n", XPMSG_WARN_MSS);
		return NULL;
	    }
	    /*
	     * end of value string; allocate the MediumSS structure
	     */
	    medium_ss = (XpOidMediumSS*)XpOidCalloc(1, sizeof(XpOidMediumSS));
	    medium_ss->count = i;
	    medium_ss->mss = (XpOidMediumSourceSize*)
		XpOidCalloc(i, sizeof(XpOidMediumSourceSize));
	}
	else
	{
	    /*
	     * syntax error
	     */
	    ErrorF("%s\nSequence Start expected.\nunparsed data: %s\n",
		   XPMSG_WARN_MSS, *ptr_return);
	    return NULL;
	}
    }
    return medium_ss;
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidMediumSSDelete
 *
 * Description:
 *
 *     
 *
 * Return value:
 *
 *     
 *
 */
void
XpOidMediumSSDelete(XpOidMediumSS* me)
{
    if(me != (XpOidMediumSS*)NULL)
    {
	int i;
	for(i = 0; i < me->count; i++)
	{
	    switch((me->mss)[i].mstag)
	    {
	    case XpOidMediumSS_CONTINUOUS:
		MediumContinuousSizeDelete((me->mss)[i].ms.continuous_size);
		break;
	    case XpOidMediumSS_DISCRETE:
		MediumDiscreteSizeListDelete((me->mss)[i].ms.discrete);
		break;
	    }
	}
	XpOidFree((char*)me);
    }
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidMediumSSHasSize
 *
 * Description:
 *
 *     
 *
 * Return value:
 *
 *     
 *
 */
BOOL
XpOidMediumSSHasSize(XpOidMediumSS* me, XpOid page_size)
{
    int i_mss, i_ds;
    XpOidMediumDiscreteSizeList* ds_list;

    if(me != (XpOidMediumSS*)NULL && page_size != xpoid_none)
	for(i_mss = 0; i_mss < me->count; i_mss++)
	{
	    switch((me->mss)[i_mss].mstag)
	    {
	    case XpOidMediumSS_DISCRETE:
		ds_list =  (me->mss)[i_mss].ms.discrete;
		for(i_ds = 0; i_ds < ds_list->count; i_ds++)
		    if(page_size == (ds_list->list)[i_ds].page_size)
			return xTrue;
		break;

	    case XpOidMediumSS_CONTINUOUS:
		/*
		 * unsupported
		 */
		break;
	    }
	}
    /*
     * return
     */
    return xFalse;
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidMediumSSString
 *
 * Description:
 *
 *     Creates a string representation of an XpOidMediumSS structure.
 *
 * Return value:
 *
 *     A newly allocated     
 *
 */
char* XpOidMediumSSString(const XpOidMediumSS* me)
{
    int itray, isize;
    int valid_size_count;
    int length;
    char* str;
    char* ptr;
    XpOidMediumDiscreteSize* ds;
    char buf[128];
    /*
     * determine the size of the string representation
     */
    for(itray = 0, length = 0; itray < XpOidMediumSSCount(me); itray++)
    {
	if(xpoid_none == me->mss[itray].input_tray
	   ||
	   XpOidMediumSS_CONTINUOUS == me->mss[itray].mstag)
	{
	    /*
	     * skip invalid tray or unsupported continuous size spec
	     */
	    continue;
	}
	for(isize = 0, valid_size_count = 0;
	    isize < me->mss[itray].ms.discrete->count;
	    isize++)
	{
	    ds = me->mss[itray].ms.discrete->list+isize;
	    if(ds->page_size == xpoid_none)
		continue;
	    ++valid_size_count;
	    length += XpOidStringLength(ds->page_size);
	    length += ds->long_edge_feeds ? 4 : 5; /* "True" or "False" */
#if defined(sun) && !defined(SVR4)
	    sprintf(buf, "{%.4f %.4f %.4f %.4f}",
			      ds->assured_reproduction_area.minimum_x,
			      ds->assured_reproduction_area.maximum_x,
			      ds->assured_reproduction_area.minimum_y,
			      ds->assured_reproduction_area.maximum_y);
	    length += strlen(buf);
#else
	    length += sprintf(buf, "{%.4f %.4f %.4f %.4f}",
			      ds->assured_reproduction_area.minimum_x,
			      ds->assured_reproduction_area.maximum_x,
			      ds->assured_reproduction_area.minimum_y,
			      ds->assured_reproduction_area.maximum_y);
#endif
	    length += 5; /* "{<size> <feed> <area>} " */
	}
	if(valid_size_count == 0)
	{
	    /*
	     * no valid sizes, skip
	     */
	    continue;
	}
	if(xpoid_unspecified == me->mss[itray].input_tray)
	    length += 2;	 /* "''" */
	else
	    length += XpOidStringLength(me->mss[itray].input_tray);
	length += 4; /* "{<tray> <sizes>} " */
    }
    /*
     * allocate
     */
    str = XpOidMalloc(length+1);
    /*
     * format
     */
    for(itray = 0, ptr = str; itray < XpOidMediumSSCount(me); itray++)
    {
	if(xpoid_none == me->mss[itray].input_tray
	   ||
	   XpOidMediumSS_CONTINUOUS == me->mss[itray].mstag)
	{
	    /*
	     * skip invalid tray or unsupported continuous size spec
	     */
	    continue;
	}
	/*
	 * check to ensure all of the specified sizes are valid
	 */
	for(isize = 0, valid_size_count = 0;
	    isize < me->mss[itray].ms.discrete->count;
	    isize++)
	{
	    ds = me->mss[itray].ms.discrete->list+isize;
	    if(ds->page_size != xpoid_none)
		++valid_size_count;
	}
	if(valid_size_count == 0)
	{
	    /*
	     * no valid sizes, skip
	     */
	    continue;
	}

	if(xpoid_unspecified == me->mss[itray].input_tray)
	{
#if defined(sun) && !defined(SVR4)
	    sprintf(ptr, "{'' ");
	    ptr += strlen(ptr);
#else
	    ptr += sprintf(ptr, "{'' ");
#endif
	}
	else
	{
#if defined(sun) && !defined(SVR4)
	    sprintf(ptr, "{%s ", XpOidString(me->mss[itray].input_tray));
	    ptr += strlen(ptr);
#else
	    ptr += sprintf(ptr, "{%s ",
			   XpOidString(me->mss[itray].input_tray));
#endif
	}
	for(isize = 0; isize < me->mss[itray].ms.discrete->count; isize++)
	{
	    ds = me->mss[itray].ms.discrete->list+isize;
	    if(ds->page_size != xpoid_none)
#if defined(sun) && !defined(SVR4)
	    {
		sprintf(ptr, "{%s %s {%.4f %.4f %.4f %.4f}} ",
			       XpOidString(ds->page_size),
			       ds->long_edge_feeds ? "True" : "False",
			       ds->assured_reproduction_area.minimum_x,
			       ds->assured_reproduction_area.maximum_x,
			       ds->assured_reproduction_area.minimum_y,
			       ds->assured_reproduction_area.maximum_y);
		ptr += strlen(ptr);
	    }
#else
		ptr += sprintf(ptr, "{%s %s {%.4f %.4f %.4f %.4f}} ",
			       XpOidString(ds->page_size),
			       ds->long_edge_feeds ? "True" : "False",
			       ds->assured_reproduction_area.minimum_x,
			       ds->assured_reproduction_area.maximum_x,
			       ds->assured_reproduction_area.minimum_y,
			       ds->assured_reproduction_area.maximum_y);
#endif
	}
#if defined(sun) && !defined(SVR4)
	sprintf(ptr, "} ");
	ptr += strlen(ptr);
#else
	ptr += sprintf(ptr, "} ");
#endif
    }
    /*
     * chop trailing whitespace or terminate empty string
     */
    str[length] = '\0';
    /*
     * return
     */
    return str;
}

/*
 * ------------------------------------------------------------------------
 * Name: MediumContinuousSizeParse
 *
 * Description:
 *
 *     'ptr_return' *cannot* be NULL.
 *     
 *
 * Return value:
 *
 *     
 *
 */
static XpOidMediumContinuousSize*
MediumContinuousSizeParse(const char* value_string,
			  const char** ptr_return)
{
    const char* first_nonws_ptr;
    XpOidMediumContinuousSize* mcs = NULL;
    /*
     * skip leading whitespace
     */
    first_nonws_ptr = value_string + SpanWhitespace(value_string);
    /*
     * parse out the MediumSize sequence start char
     */
    if(!ParseSeqStart(first_nonws_ptr, ptr_return))
	goto MediumContinuousSizeParse_error;
    /*
     * peek ahead to see if it looks like we actually have a continuous
     * size spec (looking for the sequence start char on the 1st range spec)
     */
    if(!ParseSeqStart(*ptr_return, (const char**)NULL))
	goto MediumContinuousSizeParse_error;
    /*
     * Ok, let's go for it
     */
    mcs = (XpOidMediumContinuousSize*)
	XpOidCalloc(1, sizeof(XpOidMediumContinuousSize));
    /*
     * "range across the feed direction"
     */
    if(!ParseRealRange(*ptr_return, ptr_return, &mcs->range_across_feed))
	goto MediumContinuousSizeParse_error;
    /*
     * "increment across the feed direction" (optional, default 0)
     */
    if(!ParseUnspecifiedValue(*ptr_return, ptr_return))
	if(!ParseRealValue(*ptr_return, ptr_return,
			   &mcs->increment_across_feed))
	    goto MediumContinuousSizeParse_error;
    /*
     * "range in the feed direction"
     */
    if(!ParseRealRange(*ptr_return, ptr_return, &mcs->range_in_feed))
	goto MediumContinuousSizeParse_error;
    /*
     * "increment in the feed direction" (optional, default 0)
     */
    if(!ParseUnspecifiedValue(*ptr_return, ptr_return))
	if(!ParseRealValue(*ptr_return, ptr_return,
			       &mcs->increment_in_feed))
	    goto MediumContinuousSizeParse_error;
    /*
     * "long edge feeds" flag (default TRUE)
     */
    if(ParseUnspecifiedValue(*ptr_return, ptr_return))
	mcs->long_edge_feeds = xTrue;
    else
	if(!ParseBoolValue(*ptr_return, ptr_return, &mcs->long_edge_feeds))
	    goto MediumContinuousSizeParse_error;
    /*
     * "generic assured reproduction area"
     */
    if(!ParseArea(*ptr_return, ptr_return, &mcs->assured_reproduction_area))
	goto MediumContinuousSizeParse_error;
    /*
     * parse out the MediumSize sequence end character
     */
    if(!ParseSeqEnd(*ptr_return, ptr_return))
	goto MediumContinuousSizeParse_error;
    /*
     * return
     */
    return mcs;
    

 MediumContinuousSizeParse_error:
    /*
     * syntax error - don't log since this function may be called
     * as a lookahead
     */
    *ptr_return = first_nonws_ptr;
    XpOidFree((char*)mcs);
    return NULL;
}

/*
 * ------------------------------------------------------------------------
 * Name: MediumContinuousSizeDelete
 *
 * Description:
 *
 *     'ptr_return' *cannot* be NULL.
 *     
 *
 * Return value:
 *
 *     
 *
 */
static void
MediumContinuousSizeDelete(XpOidMediumContinuousSize* me)
{
    XpOidFree((char*)me);
}

/*
 * ------------------------------------------------------------------------
 * Name: MediumDiscreteSizeListParse
 *
 * Description:
 *
 *     'ptr_return' *cannot* be NULL.
 *
 * Return value:
 *
 *     
 *
 */
static XpOidMediumDiscreteSizeList*
MediumDiscreteSizeListParse(const char* value_string,
			    const XpOidList* valid_medium_sizes,
			    const char** ptr_return,
			    int i)
{
    XpOidMediumDiscreteSizeList* list;
    XpOidMediumDiscreteSize mds;
    /*
     * check for the start of a new MediumSize sequence
     */
    if(ParseSeqStart(value_string, ptr_return))
    {
	/*
	 * "page size"
	 */
	mds.page_size = XpOidParse(*ptr_return, ptr_return);
	if((const XpOidList*)NULL != valid_medium_sizes
	   &&
	   !XpOidListHasOid(valid_medium_sizes, mds.page_size)
	   )
	    mds.page_size = xpoid_none;
	/*
	 * "long edge feeds" flag (default TRUE)
	 */
	if(ParseUnspecifiedValue(*ptr_return, ptr_return))
	    mds.long_edge_feeds = xTrue;
	else
	    if(!ParseBoolValue(*ptr_return, ptr_return,
				  &mds.long_edge_feeds))
	    {
		/*
		 * syntax error
		 */
		ErrorF("%s\nBoolean expected.\nunparsed data: %s\n",
		       XPMSG_WARN_MSS, *ptr_return);
		return (XpOidMediumDiscreteSizeList*)NULL;
	    }
	/*
	 * "assured reproduction area"
	 */
	if(!ParseArea(*ptr_return, ptr_return,
		      &mds.assured_reproduction_area))
	{
	    /*
	     * syntax error
	     */
	    ErrorF("%s\nArea specification error.\nunparsed data: %s\n",
		   XPMSG_WARN_MSS, *ptr_return);
	    return (XpOidMediumDiscreteSizeList*)NULL;
	}
	/*
	 * parse out the MediumSize sequence end character
	 */
	if(!ParseSeqEnd(*ptr_return, ptr_return))
	{
	    ErrorF("%s\nSequence End expected. Unparsed data: %s\n",
		   XPMSG_WARN_MSS, *ptr_return);
	    return (XpOidMediumDiscreteSizeList*)NULL;
	}
	/*
	 * recurse to parse the next Discrete MediumSize sequence
	 */
	if(mds.page_size == xpoid_none)
	{
	    list = MediumDiscreteSizeListParse(*ptr_return, valid_medium_sizes,
					       ptr_return, i);
	}
	else
	{
	    list = MediumDiscreteSizeListParse(*ptr_return, valid_medium_sizes,
					       ptr_return, i+1);
	    if(list != (XpOidMediumDiscreteSizeList*)NULL)
	    {
		/*
		 * copy the current discrete MediumSize into the list
		 */
		memmove((list->list)+i, &mds, sizeof(XpOidMediumDiscreteSize));
	    }
	}
    }
    else
    {
	/*
	 * MediumSize sequence start not found; end of the discrete sizes
	 * list
	 */
	if(0 == i)
	{
	    ErrorF("%s\nNo valid medium sizes found for tray.\n",
		   XPMSG_WARN_MSS);
	    return (XpOidMediumDiscreteSizeList*)NULL;
	}
	list = (XpOidMediumDiscreteSizeList*)
	    XpOidCalloc(1, sizeof(XpOidMediumDiscreteSizeList));
	list->count = i;
	list->list = (XpOidMediumDiscreteSize*)
	    XpOidCalloc(i, sizeof(XpOidMediumDiscreteSize));
    }
    return list;
}

/*
 * ------------------------------------------------------------------------
 * Name: MediumDiscreteSizeListDelete
 *
 * Description:
 *
 *     
 *
 * Return value:
 *
 *     
 *
 */
static void
MediumDiscreteSizeListDelete(XpOidMediumDiscreteSizeList* list)
{
    if(list != (XpOidMediumDiscreteSizeList*)NULL)
    {
	XpOidFree((char*)list->list);
	XpOidFree((char*)list);
    }
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidTrayMediumListNew
 *
 * Description:
 *
 *     Only need the valid trays; validation requires bumping up against
 *     msss using TrayMediumListValidate; this needs valid trays
 *     because of unspecified trays ion msss, but
 *     TrayMediumListValidate will take care of invalid sizes...
 *
 * Return value:
 *
 *     
 *
 */
XpOidTrayMediumList*
XpOidTrayMediumListNew(const char* value_string,
		       const XpOidList* valid_trays,
		       const XpOidMediumSS* msss)
{
    if(value_string == (const char*)NULL)
	return (XpOidTrayMediumList*)NULL;
    else
    {
	const char* ptr;
	XpOidTrayMediumList* me;
	me = TrayMediumListParse(value_string, valid_trays, &ptr, 0);
	if((XpOidTrayMediumList*)NULL != me)
	    TrayMediumListValidate(me, msss);
	return me;
    }
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidTrayMediumListDelete
 *
 * Description:
 *
 *     
 *
 * Return value:
 *
 *     
 *
 */
void
XpOidTrayMediumListDelete(XpOidTrayMediumList* list)
{
    if(list != (XpOidTrayMediumList*)NULL)
    {
	XpOidFree((char*)list->list);
	XpOidFree((char*)list);
    }
}

/*
 * ------------------------------------------------------------------------
 * Name: TrayMediumListParse
 *
 * Description:
 *
 *     'ptr_return' *cannot* be NULL.
 *
 * Return value:
 *
 *     
 *
 */
static XpOidTrayMediumList*
TrayMediumListParse(const char* value_string,
		    const XpOidList* valid_trays,
		    const char** ptr_return, int i)
{
    XpOidTrayMedium tm;
    XpOidTrayMediumList* list;
    /*
     * check for the start of a new InputTrayMedium sequence
     */
    if(ParseSeqStart(value_string, ptr_return))
    {
	/*
	 * "input tray"
	 */
	tm.input_tray = XpOidParse(*ptr_return, ptr_return);
	if((XpOidList*)NULL != valid_trays
	   &&
	   !XpOidListHasOid(valid_trays, tm.input_tray)
	   )
	    tm.input_tray = xpoid_none;
	/*
	 * "medium"
	 */
	tm.medium = XpOidParse(*ptr_return, ptr_return);
	/*
	 * parse out the InputTrayMedium sequence end character
	 */
	if(!ParseSeqEnd(*ptr_return, ptr_return))
	{
	    ErrorF("%s\n", XPMSG_WARN_ITM);
	    return NULL;
	}
	/*
	 * recurse to parse the next InputTrayMedium sequence
	 */
	list = TrayMediumListParse(*ptr_return, valid_trays, ptr_return, i+1);
	if(list != (XpOidTrayMediumList*)NULL)
	{
	    /*
	     * copy the current InputTrayMedium into the list
	     */
	    memmove((list->list)+i, &tm, sizeof(XpOidTrayMedium));
	}
    }
    else
    {
	/*
	 * InputTrayMedium sequence start not found
	 */
	if(**ptr_return == '\0')
	{
	    /*
	     * end of the list
	     */
	    list = (XpOidTrayMediumList*)
		XpOidCalloc(1, sizeof(XpOidTrayMediumList));
	    list->count = i;
	    list->list = (XpOidTrayMedium*)
		XpOidCalloc(i, sizeof(XpOidTrayMedium));
	}
	else
	{
	    /*
	     * syntax error
	     */
	    ErrorF("%s\n", XPMSG_WARN_ITM);
	    return NULL;
	}
    }
    /*
     * return
     */
    return list;
}

/*
 * ------------------------------------------------------------------------
 * Name: TrayMediumListValidate
 *
 * Description:
 *
 *     Validate the input-trays-medium list based on a passed
 *     medium-source-sizes-supported structure. The validated
 *     input-trays-medium list will have the same number of entries upon
 *     return from this function. Invalid entries are indicated by
 *     setting the tray specification to xpoid_none.
 *
 * Return value:
 *
 *     None.
 *
 */
static void
TrayMediumListValidate(XpOidTrayMediumList* me,
		       const XpOidMediumSS* msss)
{
    int i_mss, i_ds, i_itm;
    XpOid current_tray, current_medium;
    XpOidMediumDiscreteSizeList* unspecified_tray_ds;
    XpOidMediumDiscreteSizeList* tray_ds;

    if(msss == (XpOidMediumSS*)NULL
       ||
       me == (XpOidTrayMediumList*)NULL)
    {
	return;
    }
    /*
     * loop through the input trays medium list
     */
    for(i_itm = 0; i_itm < XpOidTrayMediumListCount(me); i_itm++)
    {
	current_tray = XpOidTrayMediumListTray(me, i_itm);
	if(current_tray == xpoid_none)
	    continue;
	current_medium = XpOidTrayMediumListMedium(me, i_itm);
	if(current_medium == xpoid_none)
	{
	    /*
	     * no medium; invalidate this entry
	     */
	    me->list[i_itm].input_tray = xpoid_none;
	    continue;
	}
	/*
	 * loop through the MediumSourceSizes, looking for an appropriate
	 * discrete sizes spec for the current tray
	 */
	unspecified_tray_ds = (XpOidMediumDiscreteSizeList*)NULL;
	tray_ds = (XpOidMediumDiscreteSizeList*)NULL;
	for(i_mss = 0;
	    i_mss < msss->count &&
	    tray_ds == (XpOidMediumDiscreteSizeList*)NULL;
	    i_mss++)
	{
	    switch((msss->mss)[i_mss].mstag)
	    {
	    case XpOidMediumSS_DISCRETE:
		if((msss->mss)[i_mss].input_tray == current_tray)
		    tray_ds = (msss->mss)[i_mss].ms.discrete;
		else if((msss->mss)[i_mss].input_tray == xpoid_unspecified)
		    unspecified_tray_ds = (msss->mss)[i_mss].ms.discrete;
		break;
		   
	    case XpOidMediumSS_CONTINUOUS:
		/*
		 * unsupported
		 */
		break;
	    }
	}
	/*
	 * if the tray was not matched, use the unspecified tray size
	 * list
	 */
	if(tray_ds == (XpOidMediumDiscreteSizeList*)NULL)
	{
	    if(unspecified_tray_ds == (XpOidMediumDiscreteSizeList*)NULL)
	    {
		/*
		 * not even an unspecified tray, invalidate this
		 * input-trays-medium entry.
		 */
		me->list[i_itm].input_tray = xpoid_none;
		continue;
	    }
	    else
		tray_ds = unspecified_tray_ds;
	}
	/*
	 * loop through the discrete sizes list, looking for a size that
	 * matches the medium for the current input tray
	 */
	for(i_ds = 0; i_ds < tray_ds->count; i_ds++)
	{
	    /*
	     * check to see if the current input tray's medium size
	     * matches the current discrete size
	     *
	     * Note: in the CDEnext SI, medium identifiers coincide with
	     *       medium-size identifiers. If the DP-Medium object is
	     *       ever implemented, this check would need to be
	     *       changed so that the input tray's medium size is
	     *       obtained from the indicated Medium object, and not
	     *       inferred from the medium identifier itself.
	     */
	    if((tray_ds->list)[i_ds].page_size == current_medium)
	    {
		/*
		 * The current input tray's medium size matches the
		 * current discrete medium size.
		 */
		break;
	    }
	}
	if(i_ds == tray_ds->count)
	{
	    /*
	     * The current input tray's medium size was not found in the
	     * discrete size list; mark the input tray medium entry
	     * invalid
	     */
	    me->list[i_itm].input_tray = xpoid_none;
	}
	
    }
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidTrayMediumListString
 *
 * Description:
 *
 *     Creates a string representation of an XpOidTrayMediumList structure.
 *
 * Return value:
 *
 *     A newly allocated     
 *
 */
char* XpOidTrayMediumListString(const XpOidTrayMediumList* me)
{
    int i;
    int length;
    char* str;
    char* ptr;
    XpOid tray;
    /*
     * allocate enough memory for the string representation,
     * including intervening delimiters and whitespace
     */
    for(i = 0, length = 0; i < XpOidTrayMediumListCount(me); i++)
    {
	tray = XpOidTrayMediumListTray(me, i);
	if(xpoid_none != tray)
	{
	    length += XpOidStringLength(tray);
	    length += XpOidStringLength(XpOidTrayMediumListMedium(me, i));
	    length += 4;
	}
    }
    str = XpOidMalloc(length+1);
    /*
     * format the list
     */
    for(i = 0, ptr = str; i < XpOidTrayMediumListCount(me); i++)
    {
	tray = XpOidTrayMediumListTray(me, i);
	if(xpoid_none != tray)
	{
#if defined(sun) && !defined(SVR4)
	    sprintf(ptr, "{%s %s} ",
			   XpOidString(tray),
			   XpOidString(XpOidTrayMediumListMedium(me, i)));
	    ptr += strlen(ptr);
#else
	    ptr += sprintf(ptr, "{%s %s} ",
			   XpOidString(tray),
			   XpOidString(XpOidTrayMediumListMedium(me, i)));
#endif
	}
    }
    /*
     * chop trailing whitespace or terminate empty string
     */
    str[length] = '\0';
    /*
     * return
     */
    return str;
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidTrayMediumListHasTray
 *
 * Description:
 *
 *     Determines if 'tray' is found in 'list'.
 *
 * Return value:
 *
 *     xTrue if the tray is found in the list.
 *
 *     xFalse if the tray is not in the list, or if 'list' is NULL.
 *
 */
BOOL
XpOidTrayMediumListHasTray(const XpOidTrayMediumList* list, XpOid tray)
{
    int i;
    if(list != (XpOidTrayMediumList*)NULL && tray != xpoid_none)
	for(i = 0; i < list->count; i++)
	    if(XpOidTrayMediumListTray(list, i) == tray)
		return xTrue;
    return xFalse;
}

/*
 * ------------------------------------------------------------------------
 * Name: ParseArea
 *
 * Description:
 *
 *     Skips leading whitespace and parses out and returns a XpOidArea.
 *
 * Return value:
 *
 *     xTrue if the XpOidArea was successfully parsed. ptr_return is
 *     updated to point to location where the parsing ended.
 *
 *     xFalse if a XpOidArea was not found; ptr_return is updated
 *     to point to the first non-whitespace char in value_string.
 *
 */
static BOOL
ParseArea(const char* value_string,
	  const char** ptr_return,
	  XpOidArea* area_return)
{
    const char* first_nonws_ptr;
    const char* ptr;
    /*
     * skip leading whitespace
     */
    first_nonws_ptr = value_string + SpanWhitespace(value_string);
    /*
     * parse out the area sequence start
     */
    if(!ParseSeqStart(first_nonws_ptr, &ptr))
	goto ParseArea_error;
    /*
     * parse the minimum x value
     */
    if(!ParseRealValue(ptr, &ptr,
		       area_return ? &area_return->minimum_x : NULL))
	goto ParseArea_error;
    /*
     * parse the maximum x value
     */
    if(!ParseRealValue(ptr, &ptr,
		       area_return ? &area_return->maximum_x : NULL))
	goto ParseArea_error;
    /*
     * parse the minimum y value
     */
    if(!ParseRealValue(ptr, &ptr,
		       area_return ? &area_return->minimum_y : NULL))
	goto ParseArea_error;
    /*
     * parse the maximum y value
     */
    if(!ParseRealValue(ptr, &ptr,
		       area_return ? &area_return->maximum_y : NULL))
	goto ParseArea_error;
    /*
     * parse out the area sequence end
     */
    if(!ParseSeqEnd(ptr, &ptr))
	goto ParseArea_error;
    /*
     * update the return pointer
     */
    if(ptr_return != (const char**)NULL)
	*ptr_return = ptr;
    /*
     * return
     */
    return xTrue;
    

 ParseArea_error:
    /*
     * syntax error
     */
    if(ptr_return != (const char**)NULL)
	*ptr_return = first_nonws_ptr;
    return xFalse;
}

/*
 * ------------------------------------------------------------------------
 * Name: ParseRealRange
 *
 * Description:
 *
 *     Skips leading whitespace and parses out and returns a
 *     XpOidRealRange.
 *
 * Return value:
 *
 *     xTrue if the XpOidRealRange was successfully
 *     parsed. ptr_return is updated to point to location where the
 *     parsing ended.
 *
 *     xFalse if a XpOidRealRange was not found; ptr_return is
 *     updated to point to the first non-whitespace char in value_string.
 *
 */
static BOOL
ParseRealRange(const char* value_string,
	       const char** ptr_return,
	       XpOidRealRange* range_return)
{
    const char* first_nonws_ptr;
    const char* ptr;
    /*
     * skip leading whitespace
     */
    first_nonws_ptr = value_string + SpanWhitespace(value_string);
    /*
     * parse out the range sequence start
     */
    if(!ParseSeqStart(first_nonws_ptr, &ptr))
	goto ParseRealRange_error;
    /*
     * parse the lower bound
     */
    if(!ParseRealValue(ptr, &ptr,
		       range_return ? &range_return->lower_bound : NULL))
	goto ParseRealRange_error;
    /*
     * parse the upper bound
     */
    if(!ParseRealValue(ptr, &ptr,
		       range_return ? &range_return->upper_bound : NULL))
	goto ParseRealRange_error;
    /*
     * parse out the range sequence end
     */
    if(!ParseSeqEnd(ptr, &ptr))
	goto ParseRealRange_error;
    /*
     * update the return pointer
     */
    if(ptr_return != (const char**)NULL)
	*ptr_return = ptr;
    /*
     * return
     */
    return xTrue;
    

 ParseRealRange_error:
    /*
     * syntax error
     */
    if(ptr_return != (const char**)NULL)
	*ptr_return = first_nonws_ptr;
    return xFalse;
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidNotifyParse
 *
 * Description:
 *
 *
 * Return value:
 *
 *
 */
XpOidNotify XpOidNotifyParse(const char* value_string)
{
    const char* ptr = value_string;

    if(value_string == (const char*)NULL)
	return XPOID_NOTIFY_NONE;
    /*
     * look for an event handling profile sequence start
     */
    if(!ParseSeqStart(value_string, &ptr))
    {
	if('\0' == *ptr)
	    /*
	     * empty value is valid
	     */
	    return XPOID_NOTIFY_NONE;
	else
	    return XPOID_NOTIFY_UNSUPPORTED;
    }
    /*
     * look for an event set sequence start
     */
    if(!ParseSeqStart(ptr, &ptr))
    {
	/*
	 * check for an empty event handling profile
	 */
	if(ParseSeqEnd(ptr, &ptr))
	{
	    ptr += SpanWhitespace(ptr);
	    if(*ptr == '\0')
		/*
		 * valid empty event handling profile sequence
		 */
		return XPOID_NOTIFY_NONE;
	}
	return XPOID_NOTIFY_UNSUPPORTED;
    }
    /*
     * the only event in the set should be report job completed
     */
    if(xpoid_val_event_report_job_completed != XpOidParse(ptr, &ptr))
	return XPOID_NOTIFY_UNSUPPORTED;
    /*
     * event set sequence end
     */
    if(!ParseSeqEnd(ptr, &ptr))
	return XPOID_NOTIFY_UNSUPPORTED;
    /*
     * delivery method of electronic mail
     */
    if(xpoid_val_delivery_method_electronic_mail != XpOidParse(ptr, &ptr))
	return XPOID_NOTIFY_UNSUPPORTED;
    /*
     * event handling profile sequence end
     */
    if(!ParseSeqEnd(ptr, &ptr))
	return XPOID_NOTIFY_UNSUPPORTED;
    /*
     * end of value
     */
    ptr += SpanWhitespace(ptr);
    if('\0' == *ptr)
	/*
	 * valid supported notification profile
	 */
	return XPOID_NOTIFY_EMAIL;
    else
	return XPOID_NOTIFY_UNSUPPORTED;
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidNotifyString
 *
 * Description:
 *
 *
 * Return value:
 *
 *
 */
const char* XpOidNotifyString(XpOidNotify notify)
{
    switch(notify)
    {
       case XPOID_NOTIFY_NONE:
           return NOTIFY_NONE_STR;
       case XPOID_NOTIFY_EMAIL:
           return NOTIFY_EMAIL_STR;
       case XPOID_NOTIFY_UNSUPPORTED:
           return (const char *)NULL;
    }

    ErrorF("XpOidNotifyString: Unsupported notify=%ld\n", (long)notify);
    return (const char *)NULL;
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidDocFmtNew
 *
 * Description:
 *
 *
 * Return value:
 *
 *
 */
XpOidDocFmt*
XpOidDocFmtNew(const char* value_string)
{
    XpOidDocFmt* doc_fmt;
    const char* ptr;
    
    if((const char*)NULL == value_string)
	return (XpOidDocFmt*)NULL;
    ptr = value_string + SpanWhitespace(value_string);
    if('\0' == *ptr)
	return (XpOidDocFmt*)NULL;
    /*
     * get the document format from the value string
     */
    doc_fmt = (XpOidDocFmt*)XpOidCalloc(1, sizeof(XpOidDocFmt));
    if(xTrue == XpOidDocFmtNext(doc_fmt, ptr, &ptr))
    {
	/*
	 * verify that the document format is the only value specified
	 */
	ptr += SpanWhitespace(ptr);
	if('\0' == *ptr)
	    /*
	     * valid document-format value
	     */
	    return doc_fmt;
    }
    /*
     * invalid
     */
    XpOidDocFmtDelete(doc_fmt);
    ErrorF("%s\n", XPMSG_WARN_DOC_FMT);
    return (XpOidDocFmt*)NULL;
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidDocFmtDelete
 *
 * Description:
 *
 *
 * Return value:
 *
 *
 */
void
XpOidDocFmtDelete(XpOidDocFmt* doc_fmt)
{
    if((XpOidDocFmt*)NULL != doc_fmt)
    {
	XpOidFree(doc_fmt->format);
	XpOidFree(doc_fmt->variant);
	XpOidFree(doc_fmt->version);
	XpOidFree(doc_fmt);
    }
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidDocFmtString
 *
 * Description:
 *
 *
 * Return value:
 *
 *
 */
char*
XpOidDocFmtString(XpOidDocFmt* doc_fmt)
{
    if((XpOidDocFmt*)NULL != doc_fmt)
    {
	if((char*)NULL != doc_fmt->format)
	{
	    char* str = XpOidMalloc(1+SafeStrLen(doc_fmt->format)+
				    1+SafeStrLen(doc_fmt->variant)+
				    1+SafeStrLen(doc_fmt->version)+
				    1+1);
	    sprintf(str, "{%s %s %s}", doc_fmt->format,
		    (char*)NULL != doc_fmt->variant ? doc_fmt->variant : "",
		    (char*)NULL != doc_fmt->version ? doc_fmt->version : "");
	    return str;
	}
    }
    return (char*)NULL;
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidDocFmtNext
 *
 * Description:
 *
 *     Assumes non-NULL value string.
 *
 * Return value:
 *
 *
 */
static BOOL
XpOidDocFmtNext(XpOidDocFmt* doc_fmt,
		const char* value_string,
		const char** ptr_return)
{
    const char* ptr;
    const char* first_nonws_ptr;
    const char* format;
    const char* variant;
    const char* version;
    int format_len;
    int variant_len;
    int version_len;
    /*
     * skip leading whitespace
     */
    ptr = value_string + SpanWhitespace(value_string);
    first_nonws_ptr = ptr;
    /*
     * sequence start
     */
    if(!ParseSeqStart(ptr, &ptr))
	goto XpOidDocFmtNext_error;
    /*
     * skip whitepace to the start of the document format, and save the
     * location
     */
    ptr += SpanWhitespace(ptr);
    format = ptr;
    /*
     * document format
     */
    if(0 == (format_len = SpanToken(ptr)))
	goto XpOidDocFmtNext_error;
    ptr += format_len;
    /*
     * optional variant
     */
    ptr += SpanWhitespace(ptr);
    variant = ptr;
    if(0 != (variant_len = SpanToken(ptr)))
    {
	ptr += variant_len;
	/*
	 * optional version
	 */
	ptr += SpanWhitespace(ptr);
	version = ptr;
	version_len = SpanToken(ptr);
	ptr += version_len;
    }
    else
	version_len = 0;
    /*
     * sequence end
     */
    if(!ParseSeqEnd(ptr, &ptr))
	goto XpOidDocFmtNext_error;
    /*
     * update return pointer
     */
    if((const char**)NULL != ptr_return)
	*ptr_return = ptr;
    /*
     * update the passed document format struct
     */
    memset(doc_fmt, 0, sizeof(XpOidDocFmt));
    doc_fmt->format = XpOidMalloc(format_len+1);
    strncpy(doc_fmt->format, format, format_len);
    doc_fmt->format[format_len] = '\0';
    if(0 < variant_len)
    {
	doc_fmt->variant = XpOidMalloc(variant_len+1);
	strncpy(doc_fmt->variant, variant, variant_len);
	doc_fmt->variant[variant_len] = '\0';
	if(0 < version_len)
	{
	    doc_fmt->version = XpOidMalloc(version_len+1);
	    strncpy(doc_fmt->version, version, version_len);
	    doc_fmt->version[version_len] = '\0';
	}
    }
    return xTrue;

 XpOidDocFmtNext_error:
    if((const char**)NULL != ptr_return)
	*ptr_return = first_nonws_ptr;
    return xFalse;
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidDocFmtListNew
 *
 * Description:
 *
 *
 * Return value:
 *
 *
 */
XpOidDocFmtList*
XpOidDocFmtListNew(const char* value_string,
		   const XpOidDocFmtList* valid_fmts)
{
    if((char*)NULL != value_string)
    {
	const char* ptr;
	return XpOidDocFmtListParse(value_string, valid_fmts, &ptr, 0);
    }
    return (XpOidDocFmtList*)NULL;
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidDocFmtListDelete
 *
 * Description:
 *
 *
 * Return value:
 *
 *
 */
void
XpOidDocFmtListDelete(XpOidDocFmtList* list)
{
    if((XpOidDocFmtList*)NULL != list)
    {
	int i;
	for(i = 0; i < list->count; i++)
	{
	    XpOidFree(list->list[i].format);
	    XpOidFree(list->list[i].variant);
	    XpOidFree(list->list[i].version);
	}
	XpOidFree(list->list);
	XpOidFree(list);
    }
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidDocFmtListString
 *
 * Description:
 *
 *     Assumes the passed structure is valid.
 *
 * Return value:
 *
 *
 */
char*
XpOidDocFmtListString(const XpOidDocFmtList* list)
{
    if((XpOidDocFmtList*)NULL != list)
    {
	if(0 < list->count)
	{
	    int i;
	    int str_len;
	    char* str;
	    char* ptr;
	    /*
	     * allocate the return string
	     */
	    for(i = 0, str_len = 0; i < list->count; i++)
	    {
		str_len +=
		    1 + SafeStrLen(list->list[i].format) +
		    1 + SafeStrLen(list->list[i].variant) +
		    1 + SafeStrLen(list->list[i].version) + 2;
	    }
	    str = XpOidMalloc(str_len+1);
	    /*
	     * print the list into the string and return it
	     */
	    ptr = str;
	    for(i = 0; i < list->count; i++)
	    {
		XpOidDocFmt* df = &list->list[i];
		
#if defined(sun) && !defined(SVR4)
		sprintf(ptr, "{%s %s %s} ",
			    df->format,
			    (char*)NULL != df->variant ? df->variant : "",
			    (char*)NULL != df->version ? df->version : "");
		ptr += strlen(ptr);
#else
		ptr +=
		    sprintf(ptr, "{%s %s %s} ",
			    df->format,
			    (char*)NULL != df->variant ? df->variant : "",
			    (char*)NULL != df->version ? df->version : "");
#endif
	    }
	    return str;
	}
    }
    return (char*)NULL;
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidDocFmtListParse
 *
 * Description:
 *
 *     Assumes the passed value_string and ptr_return are non-NULL.
 *
 * Return value:
 *
 *
 */
static XpOidDocFmtList*
XpOidDocFmtListParse(const char* value_string,
		     const XpOidDocFmtList* valid_fmts,
		     const char** ptr_return,
		     int i)
{
    XpOidDocFmt doc_fmt;
    XpOidDocFmtList* list;
    BOOL status;
    /*
     * get the next document-format from the value string, skipping
     * values not found in the passed list of valid formats
     */
    *ptr_return = value_string;
    while((status = XpOidDocFmtNext(&doc_fmt, *ptr_return, ptr_return))
	  &&
	  (const XpOidDocFmtList*)NULL != valid_fmts
	  &&
	  !XpOidDocFmtListHasFmt(valid_fmts, &doc_fmt)
	  );
    
    if(xFalse == status)
    {
	if('\0' == **ptr_return)
	{
	    if(0 == i)
	    {
		/*
		 * empty value string
		 */
		return (XpOidDocFmtList*)NULL;
	    }
	    else
	    {
		/*
		 * done parsing; allocate the list and return
		 */
		list =
		    (XpOidDocFmtList*)XpOidCalloc(1, sizeof(XpOidDocFmtList));
		list->count = i;
		list->list = (XpOidDocFmt*)XpOidCalloc(i, sizeof(XpOidDocFmt));
		return list;
	    }
	}
	else
	{
	    /*
	     * invalid document format
	     */
	    ErrorF("%s\n", XPMSG_WARN_DOCFMT_LIST);
	    return (XpOidDocFmtList*)NULL;
	}
    }
    else
    {
	/*
	 * recurse to parse remaining document formats
	 */
	list = XpOidDocFmtListParse(*ptr_return, valid_fmts, ptr_return, i+1);
	if((XpOidDocFmtList*)NULL != list)
	{
	    /*
	     * add this doc fmt to the list
	     */
	    list->list[i].format = doc_fmt.format;
	    list->list[i].variant = doc_fmt.variant;
	    list->list[i].version = doc_fmt.version;
	}
	return list;
    }
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidDocFmtListHasFmt
 *
 * Description:
 *
 *     Assumes the passed structure is valid.
 *
 * Return value:
 *
 *
 */
BOOL
XpOidDocFmtListHasFmt(const XpOidDocFmtList* list,
		      const XpOidDocFmt* fmt)
{
    int i;
    if(list != (XpOidDocFmtList*)NULL
       &&
       fmt != (XpOidDocFmt*)NULL
       &&
       fmt->format != (char*)NULL
       )
    {
	for(i = 0; i < list->count; i++)
	{
	    /*
	     * formats must match
	     */
	    if(strcmp(fmt->format, list->list[i].format) != 0)
		continue;
	    /*
	     * variants must both be NULL or match
	     */
	    if(fmt->variant == (char*)NULL)
	    {
		if(list->list[i].variant == (char*)NULL)
		    return xTrue;
		else
		    continue;
	    }
	    if(list->list[i].variant == (char*)NULL)
		continue;
	    if(strcmp(fmt->variant, list->list[i].variant) != 0)
		continue;
	    /*
	     * versions must both be NULL or match
	     */
	    if(fmt->version == (char*)NULL)
	    {
		if(list->list[i].version == (char*)NULL)
		    return xTrue;
		else
		    continue;
	    }
	    if(list->list[i].version == (char*)NULL)
		continue;
	    if(strcmp(fmt->version, list->list[i].version) == 0)
		return xTrue;
	}
    }
    return xFalse;
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidCardListNew
 *
 * Description:
 *
 *
 * Return value:
 *
 *
 */
XpOidCardList*
XpOidCardListNew(const char* value_string, const XpOidCardList* valid_cards)
{
    if((const char*)NULL != value_string)
    {
	const char* ptr;
    
	return XpOidCardListParse(value_string, valid_cards, &ptr, 0);
    }
    else
	return (XpOidCardList*)NULL;
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidCardListDelete
 *
 * Description:
 *
 *
 * Return value:
 *
 *
 */
void
XpOidCardListDelete(XpOidCardList* list)
{
    if((XpOidCardList*)NULL != list)
    {
	XpOidFree(list->list);
	XpOidFree(list);
    }
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidCardListString
 *
 * Description:
 *
 *
 * Return value:
 *
 *
 */
char*
XpOidCardListString(const XpOidCardList* list)
{
    if((XpOidCardList*)NULL != list)
    {
	char buf[48];
	int str_len;
	char* str;
	int i;
	char* ptr;
	/*
	 * allocate the output string
	 */
	for(i = 0, str_len = 0; i < list->count; i++)
#if defined(sun) && !defined(SVR4)
	{
	    sprintf(buf, "%lu", list->list[i]) + 1;
	    str_len += strlen(buf);
	}
#else
	    str_len += sprintf(buf, "%lu", list->list[i]) + 1;
#endif
	str = XpOidMalloc(str_len+1);
	/*
	 * write the list to the string
	 */
	for(i = 0, ptr = str; i < list->count; i++)
#if defined(sun) && !defined(SVR4)
	{
	    sprintf(ptr, "%lu ", list->list[i]);
	    ptr += strlen(ptr);
	}
#else
	    ptr += sprintf(ptr, "%lu ", list->list[i]);
#endif
	return str;
    }
    else
	return (char*)NULL;
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidCardListHasCard
 *
 * Description:
 *
 *     Determines if 'card' is an element of 'list'.
 *
 * Return value:
 *
 *     xTrue if the card is found in the list.
 *
 *     xFalse if the card is not in the list, or if 'list' is NULL.
 *
 */
BOOL
XpOidCardListHasCard(const XpOidCardList* list, unsigned long card)
{
    int i;
    if(list != (XpOidCardList*)NULL)
	for(i = 0; i < list->count; i++)
	    if(list->list[i] == card)
		return xTrue;
    return xFalse;
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidCardListParse
 *
 * Description:
 *
 *     Assumes the passed value_string and ptr_return are non-NULL.
 *
 * Return value:
 *
 *
 */
static XpOidCardList*
XpOidCardListParse(const char* value_string,
		   const XpOidCardList* valid_cards,
		   const char** ptr_return,
		   int i)
{
    unsigned long card;
    XpOidCardList* list;
    BOOL status;
    
    /*
     * get the next card from the value string, skipping values not
     * found in the passed list of valid cards
     */
    *ptr_return = value_string;
    while((status = XpOidParseUnsignedValue(*ptr_return, ptr_return, &card))
	  &&
	  (const XpOidCardList*)NULL != valid_cards
	  &&
	  !XpOidCardListHasCard(valid_cards, card)
	  );
    
    if(xFalse == status)
    {
	if('\0' == **ptr_return)
	{
	    if(0 == i)
	    {
		/*
		 * empty value string
		 */
		return (XpOidCardList*)NULL;
	    }
	    else
	    {
		/*
		 * done parsing; allocate the list and return
		 */
		list = (XpOidCardList*)XpOidCalloc(1, sizeof(XpOidCardList));
		list->count = i;
		list->list =
		    (unsigned long*)XpOidCalloc(i, sizeof(unsigned long));
		return list;
	    }
	}
	else
	{
	    /*
	     * parsing error
	     */
	    ErrorF("%s\n", XPMSG_WARN_CARD_LIST);
	    return (XpOidCardList*)NULL;
	}
    }
    else
    {
	/*
	 * recurse to parse remaining cardinal values
	 */
	list = XpOidCardListParse(*ptr_return, valid_cards, ptr_return, i+1);
	if((XpOidCardList*)NULL != list)
	{
	    /*
	     * add this value to the list
	     */
	    list->list[i] = card;
	}
	return list;
    }
}

/*
 * ------------------------------------------------------------------------
 * Name: ParseBoolValue
 *
 * Description:
 *
 *
 * Return value:
 *
 *
 */
static BOOL
ParseBoolValue(const char* value_string,
		  const char** ptr_return,
		  BOOL* bool_return)
{
    const char* ptr;
    int length;
    BOOL status;
    /*
     * skip leading whitespace
     */
    ptr = value_string + SpanWhitespace(value_string);
    /*
     * get the whitespace-delimited token length
     */
    length = SpanToken(ptr);
    /*
     * determine if true or false or bad
     */
    if(StrnCaseCmp(ptr, "TRUE", length) == 0)
    {
	if(bool_return != (BOOL*)NULL)
	    *bool_return = xTrue;
	status = xTrue;
    }
    else if(StrnCaseCmp(ptr, "FALSE", length) == 0)
    {
	if(bool_return != (BOOL*)NULL)
	    *bool_return = xFalse;
	status = xTrue;
    }
    else
    {
	/*
	 * syntax error
	 */
	status = xFalse;
    }
    /*
     * update the return pointer and return
     */
    if(ptr_return != (const char**)NULL)
	*ptr_return = status ? ptr+length : ptr;
    return status;
}

/*
 * ------------------------------------------------------------------------
 * Name: XpOidParseUnsignedValue
 *
 * Description:
 *
 *     Skips leading whitespace and parses out and returns a unsigned number.
 *
 * Return value:
 *
 *     xTrue if a unsigned number was successfully parsed. ptr_return is
 *     updated to point to location where the unsigned number parsing
 *     ended.
 *
 *     xFalse if a unsigned number was not found; ptr_return is updated
 *     to point to the first non-whitespace char in value_string.
 *
 */
BOOL
XpOidParseUnsignedValue(const char* value_string,
			const char** ptr_return,
			unsigned long* unsigned_return)
{
    long value;
    BOOL status;
    const char* first_nonws_ptr;
    const char* ptr;
    /*
     * skip leading whitespace
     */
    first_nonws_ptr = value_string + SpanWhitespace(value_string);
    value = strtol(first_nonws_ptr, (char**)(&ptr), 0);
    if(ptr == first_nonws_ptr || value < 0)
	status = xFalse;
    else
	status = xTrue;
    /*
     * update return parms
     */
    if(ptr_return != (const char**)NULL)
	*ptr_return = ptr;
    if(unsigned_return != (unsigned long*)NULL)
	*unsigned_return = (unsigned long)value;
    /*
     * return
     */
    return status;
}

/*
 * ------------------------------------------------------------------------
 * Name: ParseRealValue
 *
 * Description:
 *
 *     Skips leading whitespace and parses out and returns a real number.
 *
 * Return value:
 *
 *     xTrue if a real number was successfully parsed. ptr_return is
 *     updated to point to location where the real number parsing
 *     ended.
 *
 *     xFalse if a real number was not found; ptr_return is updated
 *     to point to the first non-whitespace char in value_string.
 *
 */
static BOOL
ParseRealValue(const char* value_string,
	       const char** ptr_return,
	       float* real_return)
{
    float real_value;
    BOOL status;
    const char* first_nonws_ptr;
    const char* ptr;
    /*
     * skip leading whitespace
     */
    first_nonws_ptr = value_string + SpanWhitespace(value_string);
    real_value = (float)strtod(first_nonws_ptr, (char**)(&ptr));
    if(ptr == first_nonws_ptr)
	status = xFalse;
    else
	status = xTrue;
    /*
     * update return parms
     */
    if(ptr_return != (const char**)NULL)
	*ptr_return = ptr;
    if(real_return != (float*)NULL)
	*real_return = real_value;
    /*
     * return
     */
    return status;
}

/*
 * ------------------------------------------------------------------------
 * Name: ParseSeqEnd
 *
 * Description:
 *
 * Description:
 *
 *     Skips leading whitespace and parses out the sequence end
 *     character '}'.
 *
 * Return value:
 *
 *     xTrue if the sequence end character was parsed; ptr_return is
 *     updated to point to the first char following the sequence end
 *     character.
 *
 *     xFalse if the sequence end character was not found; ptr_return is
 *     updated to point to the first non-whitespace char in value_string.
 *
 */
static BOOL
ParseSeqEnd(const char* value_string,
	    const char** ptr_return)
{
    const char* ptr;
    BOOL status;
    /*
     * skip leading whitespace
     */
    ptr = value_string + SpanWhitespace(value_string);
    /*
     * parse out the sequence end character
     */
    if(*ptr == '}')
    {
	status = xTrue;
	++ptr;
    }
    else
	status = xFalse;
    /*
     * update the return pointer
     */
    if(ptr_return != (const char**)NULL)
	*ptr_return = ptr;
    /*
     * return
     */
    return status;
}

/*
 * ------------------------------------------------------------------------
 * Name: ParseSeqStart
 *
 * Description:
 *
 *     Skips leading whitespace and parses out the sequence start
 *     character '{'.
 *
 * Return value:
 *
 *     xTrue if the sequence start character was parsed; ptr_return is
 *     updated to point to the first char following the sequence start
 *     character.
 *
 *     xFalse if the sequence start character was not found; ptr_return is
 *     updated to point to the first non-whitespace char in value_string.
 *
 */
static BOOL
ParseSeqStart(const char* value_string,
	      const char** ptr_return)
{
    const char* ptr;
    BOOL status;
    /*
     * skip leading whitespace
     */
    ptr = value_string + SpanWhitespace(value_string);
    /*
     * parse out the sequence start character
     */
    if(*ptr == '{')
    {
	status = xTrue;
	++ptr;
    }
    else
	status = xFalse;
    /*
     * update the return pointer
     */
    if(ptr_return != (const char**)NULL)
	*ptr_return = ptr;
    /*
     * return
     */
    return status;
}

/*
 * ------------------------------------------------------------------------
 * Name: ParseUnspecifiedValue
 *
 * Description:
 *
 *     Skips leading whitespace and parses out an unspecified optional
 *     value (i.e. matching '' or "" - skips all data between the set of
 *     quotes).
 *
 * Return value:
 *
 *     xTrue if an unspecified value was parsed; ptr_return is updated to
 *     point to the first char following the trailing quote.
 *
 *     xFalse if an unspecified value was not found; ptr_return is updated
 *     to point to the first non-whitespace char in value_string.
 *
 */
static BOOL
ParseUnspecifiedValue(const char* value_string,
		      const char** ptr_return)
{
    BOOL status;
    const char* ptr;
    /*
     * skip leading whitespace
     */
    ptr = value_string + SpanWhitespace(value_string);
    /*
     * parse out an unspecified optional value ('' or "")
     */
    if(*ptr == '\'' || *ptr == '"')
    {
	char delim[2];

	if(ptr_return != (const char**)NULL)
	{
	    delim[0] = *ptr;
	    delim[1] = '\0';
	    /*
	     * skip over the matching delimiter
	     */
	    ++ptr;
	    ptr += strcspn(ptr, delim);
	    if(*ptr != '\0')
		++ptr;
	}
	status = xTrue;
    }
    else
	status = xFalse;
    /*
     * update the return pointer
     */
    if(ptr_return != (const char**)NULL)
	*ptr_return = ptr;
    /*
     * return
     */
    return status;
}

/*
 * ------------------------------------------------------------------------
 * Name: SpanToken
 *
 * Description:
 *
 *     Returns the length of the initial segment of the passed string
 *     that consists entirely of non-whitespace and non-sequence
 *     delimiter characters.
 *
 *
 */
static int
SpanToken(const char* string)
{
    const char* ptr;
    for(ptr = string;
	*ptr != '\0' && !isspace(*ptr) && *ptr != '{' && *ptr != '}';
	++ptr);
    return ptr - string;
}

/*
 * ------------------------------------------------------------------------
 * Name: SpanWhitespace
 *
 * Description:
 *
 *     Returns the length of the initial segment of the passed string
 *     that consists entirely of whitespace characters.
 *
 *
 */
static int
SpanWhitespace(const char* string)
{
    const char* ptr;
    for(ptr = string; *ptr != '\0' && isspace(*ptr); ++ptr);
    return ptr - string;
}

#ifndef HAVE_STRCASECMP
/*
 * ------------------------------------------------------------------------
 * Name: StrnCaseCmp
 *
 * Description:
 *
 *	Implements strncasecmp() for those platforms that need it.
 *
 *
 */
static int
StrnCaseCmp(const char *s1, const char *s2, size_t len)
{
    char c1, c2;
    int result;

    while (len--)
    {
	c1 = *s1++;
	c2 = *s2++;
	result = tolower(c1) - tolower(c2);

	if (result != 0)
	    return result;
    }

    return 0;
}
#endif
