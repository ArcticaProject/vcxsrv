/*
 * This file generated automatically from xprint.xml by c-client.xsl using XSLT.
 * Edit at your peril.
 */

#include <string.h>
#include <assert.h>
#include "xcbext.h"
#include "xprint.h"

xcb_extension_t xcb_x_print_id = { "XpExtension" };


/*****************************************************************************
 **
 ** xcb_x_print_string8_t * xcb_x_print_printer_name
 ** 
 ** @param const xcb_x_print_printer_t *R
 ** @returns xcb_x_print_string8_t *
 **
 *****************************************************************************/
 
xcb_x_print_string8_t *
xcb_x_print_printer_name (const xcb_x_print_printer_t *R  /**< */)
{
    return (xcb_x_print_string8_t *) (R + 1);
}


/*****************************************************************************
 **
 ** int xcb_x_print_printer_name_length
 ** 
 ** @param const xcb_x_print_printer_t *R
 ** @returns int
 **
 *****************************************************************************/
 
int
xcb_x_print_printer_name_length (const xcb_x_print_printer_t *R  /**< */)
{
    return R->nameLen;
}


/*****************************************************************************
 **
 ** xcb_x_print_string8_iterator_t xcb_x_print_printer_name_iterator
 ** 
 ** @param const xcb_x_print_printer_t *R
 ** @returns xcb_x_print_string8_iterator_t
 **
 *****************************************************************************/
 
xcb_x_print_string8_iterator_t
xcb_x_print_printer_name_iterator (const xcb_x_print_printer_t *R  /**< */)
{
    xcb_x_print_string8_iterator_t i;
    i.data = (xcb_x_print_string8_t *) (R + 1);
    i.rem = R->nameLen;
    i.index = (char *) i.data - (char *) R;
    return i;
}


/*****************************************************************************
 **
 ** xcb_x_print_string8_t * xcb_x_print_printer_description
 ** 
 ** @param const xcb_x_print_printer_t *R
 ** @returns xcb_x_print_string8_t *
 **
 *****************************************************************************/
 
xcb_x_print_string8_t *
xcb_x_print_printer_description (const xcb_x_print_printer_t *R  /**< */)
{
    xcb_generic_iterator_t prev = xcb_x_print_string8_end(xcb_x_print_printer_name_iterator(R));
    return (xcb_x_print_string8_t *) ((char *) prev.data + XCB_TYPE_PAD(xcb_x_print_string8_t, prev.index));
}


/*****************************************************************************
 **
 ** int xcb_x_print_printer_description_length
 ** 
 ** @param const xcb_x_print_printer_t *R
 ** @returns int
 **
 *****************************************************************************/
 
int
xcb_x_print_printer_description_length (const xcb_x_print_printer_t *R  /**< */)
{
    return R->descLen;
}


/*****************************************************************************
 **
 ** xcb_x_print_string8_iterator_t xcb_x_print_printer_description_iterator
 ** 
 ** @param const xcb_x_print_printer_t *R
 ** @returns xcb_x_print_string8_iterator_t
 **
 *****************************************************************************/
 
xcb_x_print_string8_iterator_t
xcb_x_print_printer_description_iterator (const xcb_x_print_printer_t *R  /**< */)
{
    xcb_x_print_string8_iterator_t i;
    xcb_generic_iterator_t prev = xcb_x_print_string8_end(xcb_x_print_printer_name_iterator(R));
    i.data = (xcb_x_print_string8_t *) ((char *) prev.data + XCB_TYPE_PAD(xcb_x_print_string8_t, prev.index));
    i.rem = R->descLen;
    i.index = (char *) i.data - (char *) R;
    return i;
}


/*****************************************************************************
 **
 ** void xcb_x_print_printer_next
 ** 
 ** @param xcb_x_print_printer_iterator_t *i
 ** @returns void
 **
 *****************************************************************************/
 
void
xcb_x_print_printer_next (xcb_x_print_printer_iterator_t *i  /**< */)
{
    xcb_x_print_printer_t *R = i->data;
    xcb_generic_iterator_t child = xcb_x_print_string8_end(xcb_x_print_printer_description_iterator(R));
    --i->rem;
    i->data = (xcb_x_print_printer_t *) child.data;
    i->index = child.index;
}


/*****************************************************************************
 **
 ** xcb_generic_iterator_t xcb_x_print_printer_end
 ** 
 ** @param xcb_x_print_printer_iterator_t i
 ** @returns xcb_generic_iterator_t
 **
 *****************************************************************************/
 
xcb_generic_iterator_t
xcb_x_print_printer_end (xcb_x_print_printer_iterator_t i  /**< */)
{
    xcb_generic_iterator_t ret;
    while(i.rem > 0)
        xcb_x_print_printer_next(&i);
    ret.data = i.data;
    ret.rem = i.rem;
    ret.index = i.index;
    return ret;
}


/*****************************************************************************
 **
 ** void xcb_x_print_pcontext_next
 ** 
 ** @param xcb_x_print_pcontext_iterator_t *i
 ** @returns void
 **
 *****************************************************************************/
 
void
xcb_x_print_pcontext_next (xcb_x_print_pcontext_iterator_t *i  /**< */)
{
    --i->rem;
    ++i->data;
    i->index += sizeof(xcb_x_print_pcontext_t);
}


/*****************************************************************************
 **
 ** xcb_generic_iterator_t xcb_x_print_pcontext_end
 ** 
 ** @param xcb_x_print_pcontext_iterator_t i
 ** @returns xcb_generic_iterator_t
 **
 *****************************************************************************/
 
xcb_generic_iterator_t
xcb_x_print_pcontext_end (xcb_x_print_pcontext_iterator_t i  /**< */)
{
    xcb_generic_iterator_t ret;
    ret.data = i.data + i.rem;
    ret.index = i.index + ((char *) ret.data - (char *) i.data);
    ret.rem = 0;
    return ret;
}


/*****************************************************************************
 **
 ** void xcb_x_print_string8_next
 ** 
 ** @param xcb_x_print_string8_iterator_t *i
 ** @returns void
 **
 *****************************************************************************/
 
void
xcb_x_print_string8_next (xcb_x_print_string8_iterator_t *i  /**< */)
{
    --i->rem;
    ++i->data;
    i->index += sizeof(xcb_x_print_string8_t);
}


/*****************************************************************************
 **
 ** xcb_generic_iterator_t xcb_x_print_string8_end
 ** 
 ** @param xcb_x_print_string8_iterator_t i
 ** @returns xcb_generic_iterator_t
 **
 *****************************************************************************/
 
xcb_generic_iterator_t
xcb_x_print_string8_end (xcb_x_print_string8_iterator_t i  /**< */)
{
    xcb_generic_iterator_t ret;
    ret.data = i.data + i.rem;
    ret.index = i.index + ((char *) ret.data - (char *) i.data);
    ret.rem = 0;
    return ret;
}


/*****************************************************************************
 **
 ** xcb_x_print_print_query_version_cookie_t xcb_x_print_print_query_version
 ** 
 ** @param xcb_connection_t *c
 ** @returns xcb_x_print_print_query_version_cookie_t
 **
 *****************************************************************************/
 
xcb_x_print_print_query_version_cookie_t
xcb_x_print_print_query_version (xcb_connection_t *c  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_QUERY_VERSION,
        /* isvoid */ 0
    };
    
    struct iovec xcb_parts[4];
    xcb_x_print_print_query_version_cookie_t xcb_ret;
    xcb_x_print_print_query_version_request_t xcb_out;
    
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, XCB_REQUEST_CHECKED, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_x_print_print_query_version_cookie_t xcb_x_print_print_query_version_unchecked
 ** 
 ** @param xcb_connection_t *c
 ** @returns xcb_x_print_print_query_version_cookie_t
 **
 *****************************************************************************/
 
xcb_x_print_print_query_version_cookie_t
xcb_x_print_print_query_version_unchecked (xcb_connection_t *c  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_QUERY_VERSION,
        /* isvoid */ 0
    };
    
    struct iovec xcb_parts[4];
    xcb_x_print_print_query_version_cookie_t xcb_ret;
    xcb_x_print_print_query_version_request_t xcb_out;
    
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, 0, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_x_print_print_query_version_reply_t * xcb_x_print_print_query_version_reply
 ** 
 ** @param xcb_connection_t                          *c
 ** @param xcb_x_print_print_query_version_cookie_t   cookie
 ** @param xcb_generic_error_t                      **e
 ** @returns xcb_x_print_print_query_version_reply_t *
 **
 *****************************************************************************/
 
xcb_x_print_print_query_version_reply_t *
xcb_x_print_print_query_version_reply (xcb_connection_t                          *c  /**< */,
                                       xcb_x_print_print_query_version_cookie_t   cookie  /**< */,
                                       xcb_generic_error_t                      **e  /**< */)
{
    return (xcb_x_print_print_query_version_reply_t *) xcb_wait_for_reply(c, cookie.sequence, e);
}


/*****************************************************************************
 **
 ** xcb_x_print_print_get_printer_list_cookie_t xcb_x_print_print_get_printer_list
 ** 
 ** @param xcb_connection_t            *c
 ** @param uint32_t                     printerNameLen
 ** @param uint32_t                     localeLen
 ** @param const xcb_x_print_string8_t *printer_name
 ** @param const xcb_x_print_string8_t *locale
 ** @returns xcb_x_print_print_get_printer_list_cookie_t
 **
 *****************************************************************************/
 
xcb_x_print_print_get_printer_list_cookie_t
xcb_x_print_print_get_printer_list (xcb_connection_t            *c  /**< */,
                                    uint32_t                     printerNameLen  /**< */,
                                    uint32_t                     localeLen  /**< */,
                                    const xcb_x_print_string8_t *printer_name  /**< */,
                                    const xcb_x_print_string8_t *locale  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 6,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_GET_PRINTER_LIST,
        /* isvoid */ 0
    };
    
    struct iovec xcb_parts[8];
    xcb_x_print_print_get_printer_list_cookie_t xcb_ret;
    xcb_x_print_print_get_printer_list_request_t xcb_out;
    
    xcb_out.printerNameLen = printerNameLen;
    xcb_out.localeLen = localeLen;
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_parts[4].iov_base = (char *) printer_name;
    xcb_parts[4].iov_len = printerNameLen * sizeof(xcb_x_print_string8_t);
    xcb_parts[5].iov_base = 0;
    xcb_parts[5].iov_len = -xcb_parts[4].iov_len & 3;
    xcb_parts[6].iov_base = (char *) locale;
    xcb_parts[6].iov_len = localeLen * sizeof(xcb_x_print_string8_t);
    xcb_parts[7].iov_base = 0;
    xcb_parts[7].iov_len = -xcb_parts[6].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, XCB_REQUEST_CHECKED, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_x_print_print_get_printer_list_cookie_t xcb_x_print_print_get_printer_list_unchecked
 ** 
 ** @param xcb_connection_t            *c
 ** @param uint32_t                     printerNameLen
 ** @param uint32_t                     localeLen
 ** @param const xcb_x_print_string8_t *printer_name
 ** @param const xcb_x_print_string8_t *locale
 ** @returns xcb_x_print_print_get_printer_list_cookie_t
 **
 *****************************************************************************/
 
xcb_x_print_print_get_printer_list_cookie_t
xcb_x_print_print_get_printer_list_unchecked (xcb_connection_t            *c  /**< */,
                                              uint32_t                     printerNameLen  /**< */,
                                              uint32_t                     localeLen  /**< */,
                                              const xcb_x_print_string8_t *printer_name  /**< */,
                                              const xcb_x_print_string8_t *locale  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 6,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_GET_PRINTER_LIST,
        /* isvoid */ 0
    };
    
    struct iovec xcb_parts[8];
    xcb_x_print_print_get_printer_list_cookie_t xcb_ret;
    xcb_x_print_print_get_printer_list_request_t xcb_out;
    
    xcb_out.printerNameLen = printerNameLen;
    xcb_out.localeLen = localeLen;
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_parts[4].iov_base = (char *) printer_name;
    xcb_parts[4].iov_len = printerNameLen * sizeof(xcb_x_print_string8_t);
    xcb_parts[5].iov_base = 0;
    xcb_parts[5].iov_len = -xcb_parts[4].iov_len & 3;
    xcb_parts[6].iov_base = (char *) locale;
    xcb_parts[6].iov_len = localeLen * sizeof(xcb_x_print_string8_t);
    xcb_parts[7].iov_base = 0;
    xcb_parts[7].iov_len = -xcb_parts[6].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, 0, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** int xcb_x_print_print_get_printer_list_printers_length
 ** 
 ** @param const xcb_x_print_print_get_printer_list_reply_t *R
 ** @returns int
 **
 *****************************************************************************/
 
int
xcb_x_print_print_get_printer_list_printers_length (const xcb_x_print_print_get_printer_list_reply_t *R  /**< */)
{
    return R->listCount;
}


/*****************************************************************************
 **
 ** xcb_x_print_printer_iterator_t xcb_x_print_print_get_printer_list_printers_iterator
 ** 
 ** @param const xcb_x_print_print_get_printer_list_reply_t *R
 ** @returns xcb_x_print_printer_iterator_t
 **
 *****************************************************************************/
 
xcb_x_print_printer_iterator_t
xcb_x_print_print_get_printer_list_printers_iterator (const xcb_x_print_print_get_printer_list_reply_t *R  /**< */)
{
    xcb_x_print_printer_iterator_t i;
    i.data = (xcb_x_print_printer_t *) (R + 1);
    i.rem = R->listCount;
    i.index = (char *) i.data - (char *) R;
    return i;
}


/*****************************************************************************
 **
 ** xcb_x_print_print_get_printer_list_reply_t * xcb_x_print_print_get_printer_list_reply
 ** 
 ** @param xcb_connection_t                             *c
 ** @param xcb_x_print_print_get_printer_list_cookie_t   cookie
 ** @param xcb_generic_error_t                         **e
 ** @returns xcb_x_print_print_get_printer_list_reply_t *
 **
 *****************************************************************************/
 
xcb_x_print_print_get_printer_list_reply_t *
xcb_x_print_print_get_printer_list_reply (xcb_connection_t                             *c  /**< */,
                                          xcb_x_print_print_get_printer_list_cookie_t   cookie  /**< */,
                                          xcb_generic_error_t                         **e  /**< */)
{
    return (xcb_x_print_print_get_printer_list_reply_t *) xcb_wait_for_reply(c, cookie.sequence, e);
}


/*****************************************************************************
 **
 ** xcb_void_cookie_t xcb_x_print_print_rehash_printer_list_checked
 ** 
 ** @param xcb_connection_t *c
 ** @returns xcb_void_cookie_t
 **
 *****************************************************************************/
 
xcb_void_cookie_t
xcb_x_print_print_rehash_printer_list_checked (xcb_connection_t *c  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_REHASH_PRINTER_LIST,
        /* isvoid */ 1
    };
    
    struct iovec xcb_parts[4];
    xcb_void_cookie_t xcb_ret;
    xcb_x_print_print_rehash_printer_list_request_t xcb_out;
    
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, XCB_REQUEST_CHECKED, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_void_cookie_t xcb_x_print_print_rehash_printer_list
 ** 
 ** @param xcb_connection_t *c
 ** @returns xcb_void_cookie_t
 **
 *****************************************************************************/
 
xcb_void_cookie_t
xcb_x_print_print_rehash_printer_list (xcb_connection_t *c  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_REHASH_PRINTER_LIST,
        /* isvoid */ 1
    };
    
    struct iovec xcb_parts[4];
    xcb_void_cookie_t xcb_ret;
    xcb_x_print_print_rehash_printer_list_request_t xcb_out;
    
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, 0, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_void_cookie_t xcb_x_print_create_context_checked
 ** 
 ** @param xcb_connection_t            *c
 ** @param uint32_t                     context_id
 ** @param uint32_t                     printerNameLen
 ** @param uint32_t                     localeLen
 ** @param const xcb_x_print_string8_t *printerName
 ** @param const xcb_x_print_string8_t *locale
 ** @returns xcb_void_cookie_t
 **
 *****************************************************************************/
 
xcb_void_cookie_t
xcb_x_print_create_context_checked (xcb_connection_t            *c  /**< */,
                                    uint32_t                     context_id  /**< */,
                                    uint32_t                     printerNameLen  /**< */,
                                    uint32_t                     localeLen  /**< */,
                                    const xcb_x_print_string8_t *printerName  /**< */,
                                    const xcb_x_print_string8_t *locale  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 6,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_CREATE_CONTEXT,
        /* isvoid */ 1
    };
    
    struct iovec xcb_parts[8];
    xcb_void_cookie_t xcb_ret;
    xcb_x_print_create_context_request_t xcb_out;
    
    xcb_out.context_id = context_id;
    xcb_out.printerNameLen = printerNameLen;
    xcb_out.localeLen = localeLen;
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_parts[4].iov_base = (char *) printerName;
    xcb_parts[4].iov_len = printerNameLen * sizeof(xcb_x_print_string8_t);
    xcb_parts[5].iov_base = 0;
    xcb_parts[5].iov_len = -xcb_parts[4].iov_len & 3;
    xcb_parts[6].iov_base = (char *) locale;
    xcb_parts[6].iov_len = localeLen * sizeof(xcb_x_print_string8_t);
    xcb_parts[7].iov_base = 0;
    xcb_parts[7].iov_len = -xcb_parts[6].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, XCB_REQUEST_CHECKED, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_void_cookie_t xcb_x_print_create_context
 ** 
 ** @param xcb_connection_t            *c
 ** @param uint32_t                     context_id
 ** @param uint32_t                     printerNameLen
 ** @param uint32_t                     localeLen
 ** @param const xcb_x_print_string8_t *printerName
 ** @param const xcb_x_print_string8_t *locale
 ** @returns xcb_void_cookie_t
 **
 *****************************************************************************/
 
xcb_void_cookie_t
xcb_x_print_create_context (xcb_connection_t            *c  /**< */,
                            uint32_t                     context_id  /**< */,
                            uint32_t                     printerNameLen  /**< */,
                            uint32_t                     localeLen  /**< */,
                            const xcb_x_print_string8_t *printerName  /**< */,
                            const xcb_x_print_string8_t *locale  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 6,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_CREATE_CONTEXT,
        /* isvoid */ 1
    };
    
    struct iovec xcb_parts[8];
    xcb_void_cookie_t xcb_ret;
    xcb_x_print_create_context_request_t xcb_out;
    
    xcb_out.context_id = context_id;
    xcb_out.printerNameLen = printerNameLen;
    xcb_out.localeLen = localeLen;
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_parts[4].iov_base = (char *) printerName;
    xcb_parts[4].iov_len = printerNameLen * sizeof(xcb_x_print_string8_t);
    xcb_parts[5].iov_base = 0;
    xcb_parts[5].iov_len = -xcb_parts[4].iov_len & 3;
    xcb_parts[6].iov_base = (char *) locale;
    xcb_parts[6].iov_len = localeLen * sizeof(xcb_x_print_string8_t);
    xcb_parts[7].iov_base = 0;
    xcb_parts[7].iov_len = -xcb_parts[6].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, 0, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_void_cookie_t xcb_x_print_print_set_context_checked
 ** 
 ** @param xcb_connection_t *c
 ** @param uint32_t          context
 ** @returns xcb_void_cookie_t
 **
 *****************************************************************************/
 
xcb_void_cookie_t
xcb_x_print_print_set_context_checked (xcb_connection_t *c  /**< */,
                                       uint32_t          context  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_SET_CONTEXT,
        /* isvoid */ 1
    };
    
    struct iovec xcb_parts[4];
    xcb_void_cookie_t xcb_ret;
    xcb_x_print_print_set_context_request_t xcb_out;
    
    xcb_out.context = context;
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, XCB_REQUEST_CHECKED, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_void_cookie_t xcb_x_print_print_set_context
 ** 
 ** @param xcb_connection_t *c
 ** @param uint32_t          context
 ** @returns xcb_void_cookie_t
 **
 *****************************************************************************/
 
xcb_void_cookie_t
xcb_x_print_print_set_context (xcb_connection_t *c  /**< */,
                               uint32_t          context  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_SET_CONTEXT,
        /* isvoid */ 1
    };
    
    struct iovec xcb_parts[4];
    xcb_void_cookie_t xcb_ret;
    xcb_x_print_print_set_context_request_t xcb_out;
    
    xcb_out.context = context;
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, 0, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_x_print_print_get_context_cookie_t xcb_x_print_print_get_context
 ** 
 ** @param xcb_connection_t *c
 ** @returns xcb_x_print_print_get_context_cookie_t
 **
 *****************************************************************************/
 
xcb_x_print_print_get_context_cookie_t
xcb_x_print_print_get_context (xcb_connection_t *c  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_GET_CONTEXT,
        /* isvoid */ 0
    };
    
    struct iovec xcb_parts[4];
    xcb_x_print_print_get_context_cookie_t xcb_ret;
    xcb_x_print_print_get_context_request_t xcb_out;
    
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, XCB_REQUEST_CHECKED, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_x_print_print_get_context_cookie_t xcb_x_print_print_get_context_unchecked
 ** 
 ** @param xcb_connection_t *c
 ** @returns xcb_x_print_print_get_context_cookie_t
 **
 *****************************************************************************/
 
xcb_x_print_print_get_context_cookie_t
xcb_x_print_print_get_context_unchecked (xcb_connection_t *c  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_GET_CONTEXT,
        /* isvoid */ 0
    };
    
    struct iovec xcb_parts[4];
    xcb_x_print_print_get_context_cookie_t xcb_ret;
    xcb_x_print_print_get_context_request_t xcb_out;
    
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, 0, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_x_print_print_get_context_reply_t * xcb_x_print_print_get_context_reply
 ** 
 ** @param xcb_connection_t                        *c
 ** @param xcb_x_print_print_get_context_cookie_t   cookie
 ** @param xcb_generic_error_t                    **e
 ** @returns xcb_x_print_print_get_context_reply_t *
 **
 *****************************************************************************/
 
xcb_x_print_print_get_context_reply_t *
xcb_x_print_print_get_context_reply (xcb_connection_t                        *c  /**< */,
                                     xcb_x_print_print_get_context_cookie_t   cookie  /**< */,
                                     xcb_generic_error_t                    **e  /**< */)
{
    return (xcb_x_print_print_get_context_reply_t *) xcb_wait_for_reply(c, cookie.sequence, e);
}


/*****************************************************************************
 **
 ** xcb_void_cookie_t xcb_x_print_print_destroy_context_checked
 ** 
 ** @param xcb_connection_t *c
 ** @param uint32_t          context
 ** @returns xcb_void_cookie_t
 **
 *****************************************************************************/
 
xcb_void_cookie_t
xcb_x_print_print_destroy_context_checked (xcb_connection_t *c  /**< */,
                                           uint32_t          context  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_DESTROY_CONTEXT,
        /* isvoid */ 1
    };
    
    struct iovec xcb_parts[4];
    xcb_void_cookie_t xcb_ret;
    xcb_x_print_print_destroy_context_request_t xcb_out;
    
    xcb_out.context = context;
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, XCB_REQUEST_CHECKED, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_void_cookie_t xcb_x_print_print_destroy_context
 ** 
 ** @param xcb_connection_t *c
 ** @param uint32_t          context
 ** @returns xcb_void_cookie_t
 **
 *****************************************************************************/
 
xcb_void_cookie_t
xcb_x_print_print_destroy_context (xcb_connection_t *c  /**< */,
                                   uint32_t          context  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_DESTROY_CONTEXT,
        /* isvoid */ 1
    };
    
    struct iovec xcb_parts[4];
    xcb_void_cookie_t xcb_ret;
    xcb_x_print_print_destroy_context_request_t xcb_out;
    
    xcb_out.context = context;
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, 0, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_x_print_print_get_screen_of_context_cookie_t xcb_x_print_print_get_screen_of_context
 ** 
 ** @param xcb_connection_t *c
 ** @returns xcb_x_print_print_get_screen_of_context_cookie_t
 **
 *****************************************************************************/
 
xcb_x_print_print_get_screen_of_context_cookie_t
xcb_x_print_print_get_screen_of_context (xcb_connection_t *c  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_GET_SCREEN_OF_CONTEXT,
        /* isvoid */ 0
    };
    
    struct iovec xcb_parts[4];
    xcb_x_print_print_get_screen_of_context_cookie_t xcb_ret;
    xcb_x_print_print_get_screen_of_context_request_t xcb_out;
    
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, XCB_REQUEST_CHECKED, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_x_print_print_get_screen_of_context_cookie_t xcb_x_print_print_get_screen_of_context_unchecked
 ** 
 ** @param xcb_connection_t *c
 ** @returns xcb_x_print_print_get_screen_of_context_cookie_t
 **
 *****************************************************************************/
 
xcb_x_print_print_get_screen_of_context_cookie_t
xcb_x_print_print_get_screen_of_context_unchecked (xcb_connection_t *c  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_GET_SCREEN_OF_CONTEXT,
        /* isvoid */ 0
    };
    
    struct iovec xcb_parts[4];
    xcb_x_print_print_get_screen_of_context_cookie_t xcb_ret;
    xcb_x_print_print_get_screen_of_context_request_t xcb_out;
    
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, 0, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_x_print_print_get_screen_of_context_reply_t * xcb_x_print_print_get_screen_of_context_reply
 ** 
 ** @param xcb_connection_t                                  *c
 ** @param xcb_x_print_print_get_screen_of_context_cookie_t   cookie
 ** @param xcb_generic_error_t                              **e
 ** @returns xcb_x_print_print_get_screen_of_context_reply_t *
 **
 *****************************************************************************/
 
xcb_x_print_print_get_screen_of_context_reply_t *
xcb_x_print_print_get_screen_of_context_reply (xcb_connection_t                                  *c  /**< */,
                                               xcb_x_print_print_get_screen_of_context_cookie_t   cookie  /**< */,
                                               xcb_generic_error_t                              **e  /**< */)
{
    return (xcb_x_print_print_get_screen_of_context_reply_t *) xcb_wait_for_reply(c, cookie.sequence, e);
}


/*****************************************************************************
 **
 ** xcb_void_cookie_t xcb_x_print_print_start_job_checked
 ** 
 ** @param xcb_connection_t *c
 ** @param uint8_t           output_mode
 ** @returns xcb_void_cookie_t
 **
 *****************************************************************************/
 
xcb_void_cookie_t
xcb_x_print_print_start_job_checked (xcb_connection_t *c  /**< */,
                                     uint8_t           output_mode  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_START_JOB,
        /* isvoid */ 1
    };
    
    struct iovec xcb_parts[4];
    xcb_void_cookie_t xcb_ret;
    xcb_x_print_print_start_job_request_t xcb_out;
    
    xcb_out.output_mode = output_mode;
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, XCB_REQUEST_CHECKED, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_void_cookie_t xcb_x_print_print_start_job
 ** 
 ** @param xcb_connection_t *c
 ** @param uint8_t           output_mode
 ** @returns xcb_void_cookie_t
 **
 *****************************************************************************/
 
xcb_void_cookie_t
xcb_x_print_print_start_job (xcb_connection_t *c  /**< */,
                             uint8_t           output_mode  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_START_JOB,
        /* isvoid */ 1
    };
    
    struct iovec xcb_parts[4];
    xcb_void_cookie_t xcb_ret;
    xcb_x_print_print_start_job_request_t xcb_out;
    
    xcb_out.output_mode = output_mode;
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, 0, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_void_cookie_t xcb_x_print_print_end_job_checked
 ** 
 ** @param xcb_connection_t *c
 ** @param uint8_t           cancel
 ** @returns xcb_void_cookie_t
 **
 *****************************************************************************/
 
xcb_void_cookie_t
xcb_x_print_print_end_job_checked (xcb_connection_t *c  /**< */,
                                   uint8_t           cancel  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_END_JOB,
        /* isvoid */ 1
    };
    
    struct iovec xcb_parts[4];
    xcb_void_cookie_t xcb_ret;
    xcb_x_print_print_end_job_request_t xcb_out;
    
    xcb_out.cancel = cancel;
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, XCB_REQUEST_CHECKED, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_void_cookie_t xcb_x_print_print_end_job
 ** 
 ** @param xcb_connection_t *c
 ** @param uint8_t           cancel
 ** @returns xcb_void_cookie_t
 **
 *****************************************************************************/
 
xcb_void_cookie_t
xcb_x_print_print_end_job (xcb_connection_t *c  /**< */,
                           uint8_t           cancel  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_END_JOB,
        /* isvoid */ 1
    };
    
    struct iovec xcb_parts[4];
    xcb_void_cookie_t xcb_ret;
    xcb_x_print_print_end_job_request_t xcb_out;
    
    xcb_out.cancel = cancel;
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, 0, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_void_cookie_t xcb_x_print_print_start_doc_checked
 ** 
 ** @param xcb_connection_t *c
 ** @param uint8_t           driver_mode
 ** @returns xcb_void_cookie_t
 **
 *****************************************************************************/
 
xcb_void_cookie_t
xcb_x_print_print_start_doc_checked (xcb_connection_t *c  /**< */,
                                     uint8_t           driver_mode  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_START_DOC,
        /* isvoid */ 1
    };
    
    struct iovec xcb_parts[4];
    xcb_void_cookie_t xcb_ret;
    xcb_x_print_print_start_doc_request_t xcb_out;
    
    xcb_out.driver_mode = driver_mode;
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, XCB_REQUEST_CHECKED, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_void_cookie_t xcb_x_print_print_start_doc
 ** 
 ** @param xcb_connection_t *c
 ** @param uint8_t           driver_mode
 ** @returns xcb_void_cookie_t
 **
 *****************************************************************************/
 
xcb_void_cookie_t
xcb_x_print_print_start_doc (xcb_connection_t *c  /**< */,
                             uint8_t           driver_mode  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_START_DOC,
        /* isvoid */ 1
    };
    
    struct iovec xcb_parts[4];
    xcb_void_cookie_t xcb_ret;
    xcb_x_print_print_start_doc_request_t xcb_out;
    
    xcb_out.driver_mode = driver_mode;
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, 0, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_void_cookie_t xcb_x_print_print_end_doc_checked
 ** 
 ** @param xcb_connection_t *c
 ** @param uint8_t           cancel
 ** @returns xcb_void_cookie_t
 **
 *****************************************************************************/
 
xcb_void_cookie_t
xcb_x_print_print_end_doc_checked (xcb_connection_t *c  /**< */,
                                   uint8_t           cancel  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_END_DOC,
        /* isvoid */ 1
    };
    
    struct iovec xcb_parts[4];
    xcb_void_cookie_t xcb_ret;
    xcb_x_print_print_end_doc_request_t xcb_out;
    
    xcb_out.cancel = cancel;
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, XCB_REQUEST_CHECKED, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_void_cookie_t xcb_x_print_print_end_doc
 ** 
 ** @param xcb_connection_t *c
 ** @param uint8_t           cancel
 ** @returns xcb_void_cookie_t
 **
 *****************************************************************************/
 
xcb_void_cookie_t
xcb_x_print_print_end_doc (xcb_connection_t *c  /**< */,
                           uint8_t           cancel  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_END_DOC,
        /* isvoid */ 1
    };
    
    struct iovec xcb_parts[4];
    xcb_void_cookie_t xcb_ret;
    xcb_x_print_print_end_doc_request_t xcb_out;
    
    xcb_out.cancel = cancel;
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, 0, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_void_cookie_t xcb_x_print_print_put_document_data_checked
 ** 
 ** @param xcb_connection_t            *c
 ** @param xcb_drawable_t               drawable
 ** @param uint32_t                     len_data
 ** @param uint16_t                     len_fmt
 ** @param uint16_t                     len_options
 ** @param const uint8_t               *data
 ** @param uint32_t                     doc_format_len
 ** @param const xcb_x_print_string8_t *doc_format
 ** @param uint32_t                     options_len
 ** @param const xcb_x_print_string8_t *options
 ** @returns xcb_void_cookie_t
 **
 *****************************************************************************/
 
xcb_void_cookie_t
xcb_x_print_print_put_document_data_checked (xcb_connection_t            *c  /**< */,
                                             xcb_drawable_t               drawable  /**< */,
                                             uint32_t                     len_data  /**< */,
                                             uint16_t                     len_fmt  /**< */,
                                             uint16_t                     len_options  /**< */,
                                             const uint8_t               *data  /**< */,
                                             uint32_t                     doc_format_len  /**< */,
                                             const xcb_x_print_string8_t *doc_format  /**< */,
                                             uint32_t                     options_len  /**< */,
                                             const xcb_x_print_string8_t *options  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 8,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_PUT_DOCUMENT_DATA,
        /* isvoid */ 1
    };
    
    struct iovec xcb_parts[10];
    xcb_void_cookie_t xcb_ret;
    xcb_x_print_print_put_document_data_request_t xcb_out;
    
    xcb_out.drawable = drawable;
    xcb_out.len_data = len_data;
    xcb_out.len_fmt = len_fmt;
    xcb_out.len_options = len_options;
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_parts[4].iov_base = (char *) data;
    xcb_parts[4].iov_len = len_data * sizeof(uint8_t);
    xcb_parts[5].iov_base = 0;
    xcb_parts[5].iov_len = -xcb_parts[4].iov_len & 3;
    xcb_parts[6].iov_base = (char *) doc_format;
    xcb_parts[6].iov_len = doc_format_len * sizeof(xcb_x_print_string8_t);
    xcb_parts[7].iov_base = 0;
    xcb_parts[7].iov_len = -xcb_parts[6].iov_len & 3;
    xcb_parts[8].iov_base = (char *) options;
    xcb_parts[8].iov_len = options_len * sizeof(xcb_x_print_string8_t);
    xcb_parts[9].iov_base = 0;
    xcb_parts[9].iov_len = -xcb_parts[8].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, XCB_REQUEST_CHECKED, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_void_cookie_t xcb_x_print_print_put_document_data
 ** 
 ** @param xcb_connection_t            *c
 ** @param xcb_drawable_t               drawable
 ** @param uint32_t                     len_data
 ** @param uint16_t                     len_fmt
 ** @param uint16_t                     len_options
 ** @param const uint8_t               *data
 ** @param uint32_t                     doc_format_len
 ** @param const xcb_x_print_string8_t *doc_format
 ** @param uint32_t                     options_len
 ** @param const xcb_x_print_string8_t *options
 ** @returns xcb_void_cookie_t
 **
 *****************************************************************************/
 
xcb_void_cookie_t
xcb_x_print_print_put_document_data (xcb_connection_t            *c  /**< */,
                                     xcb_drawable_t               drawable  /**< */,
                                     uint32_t                     len_data  /**< */,
                                     uint16_t                     len_fmt  /**< */,
                                     uint16_t                     len_options  /**< */,
                                     const uint8_t               *data  /**< */,
                                     uint32_t                     doc_format_len  /**< */,
                                     const xcb_x_print_string8_t *doc_format  /**< */,
                                     uint32_t                     options_len  /**< */,
                                     const xcb_x_print_string8_t *options  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 8,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_PUT_DOCUMENT_DATA,
        /* isvoid */ 1
    };
    
    struct iovec xcb_parts[10];
    xcb_void_cookie_t xcb_ret;
    xcb_x_print_print_put_document_data_request_t xcb_out;
    
    xcb_out.drawable = drawable;
    xcb_out.len_data = len_data;
    xcb_out.len_fmt = len_fmt;
    xcb_out.len_options = len_options;
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_parts[4].iov_base = (char *) data;
    xcb_parts[4].iov_len = len_data * sizeof(uint8_t);
    xcb_parts[5].iov_base = 0;
    xcb_parts[5].iov_len = -xcb_parts[4].iov_len & 3;
    xcb_parts[6].iov_base = (char *) doc_format;
    xcb_parts[6].iov_len = doc_format_len * sizeof(xcb_x_print_string8_t);
    xcb_parts[7].iov_base = 0;
    xcb_parts[7].iov_len = -xcb_parts[6].iov_len & 3;
    xcb_parts[8].iov_base = (char *) options;
    xcb_parts[8].iov_len = options_len * sizeof(xcb_x_print_string8_t);
    xcb_parts[9].iov_base = 0;
    xcb_parts[9].iov_len = -xcb_parts[8].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, 0, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_x_print_print_get_document_data_cookie_t xcb_x_print_print_get_document_data
 ** 
 ** @param xcb_connection_t       *c
 ** @param xcb_x_print_pcontext_t  context
 ** @param uint32_t                max_bytes
 ** @returns xcb_x_print_print_get_document_data_cookie_t
 **
 *****************************************************************************/
 
xcb_x_print_print_get_document_data_cookie_t
xcb_x_print_print_get_document_data (xcb_connection_t       *c  /**< */,
                                     xcb_x_print_pcontext_t  context  /**< */,
                                     uint32_t                max_bytes  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_GET_DOCUMENT_DATA,
        /* isvoid */ 0
    };
    
    struct iovec xcb_parts[4];
    xcb_x_print_print_get_document_data_cookie_t xcb_ret;
    xcb_x_print_print_get_document_data_request_t xcb_out;
    
    xcb_out.context = context;
    xcb_out.max_bytes = max_bytes;
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, XCB_REQUEST_CHECKED, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_x_print_print_get_document_data_cookie_t xcb_x_print_print_get_document_data_unchecked
 ** 
 ** @param xcb_connection_t       *c
 ** @param xcb_x_print_pcontext_t  context
 ** @param uint32_t                max_bytes
 ** @returns xcb_x_print_print_get_document_data_cookie_t
 **
 *****************************************************************************/
 
xcb_x_print_print_get_document_data_cookie_t
xcb_x_print_print_get_document_data_unchecked (xcb_connection_t       *c  /**< */,
                                               xcb_x_print_pcontext_t  context  /**< */,
                                               uint32_t                max_bytes  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_GET_DOCUMENT_DATA,
        /* isvoid */ 0
    };
    
    struct iovec xcb_parts[4];
    xcb_x_print_print_get_document_data_cookie_t xcb_ret;
    xcb_x_print_print_get_document_data_request_t xcb_out;
    
    xcb_out.context = context;
    xcb_out.max_bytes = max_bytes;
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, 0, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** uint8_t * xcb_x_print_print_get_document_data_data
 ** 
 ** @param const xcb_x_print_print_get_document_data_reply_t *R
 ** @returns uint8_t *
 **
 *****************************************************************************/
 
uint8_t *
xcb_x_print_print_get_document_data_data (const xcb_x_print_print_get_document_data_reply_t *R  /**< */)
{
    return (uint8_t *) (R + 1);
}


/*****************************************************************************
 **
 ** int xcb_x_print_print_get_document_data_data_length
 ** 
 ** @param const xcb_x_print_print_get_document_data_reply_t *R
 ** @returns int
 **
 *****************************************************************************/
 
int
xcb_x_print_print_get_document_data_data_length (const xcb_x_print_print_get_document_data_reply_t *R  /**< */)
{
    return R->dataLen;
}


/*****************************************************************************
 **
 ** xcb_generic_iterator_t xcb_x_print_print_get_document_data_data_end
 ** 
 ** @param const xcb_x_print_print_get_document_data_reply_t *R
 ** @returns xcb_generic_iterator_t
 **
 *****************************************************************************/
 
xcb_generic_iterator_t
xcb_x_print_print_get_document_data_data_end (const xcb_x_print_print_get_document_data_reply_t *R  /**< */)
{
    xcb_generic_iterator_t i;
    i.data = ((uint8_t *) (R + 1)) + (R->dataLen);
    i.rem = 0;
    i.index = (char *) i.data - (char *) R;
    return i;
}


/*****************************************************************************
 **
 ** xcb_x_print_print_get_document_data_reply_t * xcb_x_print_print_get_document_data_reply
 ** 
 ** @param xcb_connection_t                              *c
 ** @param xcb_x_print_print_get_document_data_cookie_t   cookie
 ** @param xcb_generic_error_t                          **e
 ** @returns xcb_x_print_print_get_document_data_reply_t *
 **
 *****************************************************************************/
 
xcb_x_print_print_get_document_data_reply_t *
xcb_x_print_print_get_document_data_reply (xcb_connection_t                              *c  /**< */,
                                           xcb_x_print_print_get_document_data_cookie_t   cookie  /**< */,
                                           xcb_generic_error_t                          **e  /**< */)
{
    return (xcb_x_print_print_get_document_data_reply_t *) xcb_wait_for_reply(c, cookie.sequence, e);
}


/*****************************************************************************
 **
 ** xcb_void_cookie_t xcb_x_print_print_start_page_checked
 ** 
 ** @param xcb_connection_t *c
 ** @param xcb_window_t      window
 ** @returns xcb_void_cookie_t
 **
 *****************************************************************************/
 
xcb_void_cookie_t
xcb_x_print_print_start_page_checked (xcb_connection_t *c  /**< */,
                                      xcb_window_t      window  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_START_PAGE,
        /* isvoid */ 1
    };
    
    struct iovec xcb_parts[4];
    xcb_void_cookie_t xcb_ret;
    xcb_x_print_print_start_page_request_t xcb_out;
    
    xcb_out.window = window;
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, XCB_REQUEST_CHECKED, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_void_cookie_t xcb_x_print_print_start_page
 ** 
 ** @param xcb_connection_t *c
 ** @param xcb_window_t      window
 ** @returns xcb_void_cookie_t
 **
 *****************************************************************************/
 
xcb_void_cookie_t
xcb_x_print_print_start_page (xcb_connection_t *c  /**< */,
                              xcb_window_t      window  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_START_PAGE,
        /* isvoid */ 1
    };
    
    struct iovec xcb_parts[4];
    xcb_void_cookie_t xcb_ret;
    xcb_x_print_print_start_page_request_t xcb_out;
    
    xcb_out.window = window;
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, 0, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_void_cookie_t xcb_x_print_print_end_page_checked
 ** 
 ** @param xcb_connection_t *c
 ** @param uint8_t           cancel
 ** @returns xcb_void_cookie_t
 **
 *****************************************************************************/
 
xcb_void_cookie_t
xcb_x_print_print_end_page_checked (xcb_connection_t *c  /**< */,
                                    uint8_t           cancel  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_END_PAGE,
        /* isvoid */ 1
    };
    
    struct iovec xcb_parts[4];
    xcb_void_cookie_t xcb_ret;
    xcb_x_print_print_end_page_request_t xcb_out;
    
    xcb_out.cancel = cancel;
    memset(xcb_out.pad0, 0, 3);
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, XCB_REQUEST_CHECKED, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_void_cookie_t xcb_x_print_print_end_page
 ** 
 ** @param xcb_connection_t *c
 ** @param uint8_t           cancel
 ** @returns xcb_void_cookie_t
 **
 *****************************************************************************/
 
xcb_void_cookie_t
xcb_x_print_print_end_page (xcb_connection_t *c  /**< */,
                            uint8_t           cancel  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_END_PAGE,
        /* isvoid */ 1
    };
    
    struct iovec xcb_parts[4];
    xcb_void_cookie_t xcb_ret;
    xcb_x_print_print_end_page_request_t xcb_out;
    
    xcb_out.cancel = cancel;
    memset(xcb_out.pad0, 0, 3);
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, 0, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_void_cookie_t xcb_x_print_print_select_input_checked
 ** 
 ** @param xcb_connection_t       *c
 ** @param xcb_x_print_pcontext_t  context
 ** @param uint32_t                event_mask
 ** @param const uint32_t         *event_list
 ** @returns xcb_void_cookie_t
 **
 *****************************************************************************/
 
xcb_void_cookie_t
xcb_x_print_print_select_input_checked (xcb_connection_t       *c  /**< */,
                                        xcb_x_print_pcontext_t  context  /**< */,
                                        uint32_t                event_mask  /**< */,
                                        const uint32_t         *event_list  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 4,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_SELECT_INPUT,
        /* isvoid */ 1
    };
    
    struct iovec xcb_parts[6];
    xcb_void_cookie_t xcb_ret;
    xcb_x_print_print_select_input_request_t xcb_out;
    
    xcb_out.context = context;
    xcb_out.event_mask = event_mask;
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_parts[4].iov_base = (char *) event_list;
    xcb_parts[4].iov_len = xcb_popcount(event_mask) * sizeof(uint32_t);
    xcb_parts[5].iov_base = 0;
    xcb_parts[5].iov_len = -xcb_parts[4].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, XCB_REQUEST_CHECKED, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_void_cookie_t xcb_x_print_print_select_input
 ** 
 ** @param xcb_connection_t       *c
 ** @param xcb_x_print_pcontext_t  context
 ** @param uint32_t                event_mask
 ** @param const uint32_t         *event_list
 ** @returns xcb_void_cookie_t
 **
 *****************************************************************************/
 
xcb_void_cookie_t
xcb_x_print_print_select_input (xcb_connection_t       *c  /**< */,
                                xcb_x_print_pcontext_t  context  /**< */,
                                uint32_t                event_mask  /**< */,
                                const uint32_t         *event_list  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 4,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_SELECT_INPUT,
        /* isvoid */ 1
    };
    
    struct iovec xcb_parts[6];
    xcb_void_cookie_t xcb_ret;
    xcb_x_print_print_select_input_request_t xcb_out;
    
    xcb_out.context = context;
    xcb_out.event_mask = event_mask;
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_parts[4].iov_base = (char *) event_list;
    xcb_parts[4].iov_len = xcb_popcount(event_mask) * sizeof(uint32_t);
    xcb_parts[5].iov_base = 0;
    xcb_parts[5].iov_len = -xcb_parts[4].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, 0, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_x_print_print_input_selected_cookie_t xcb_x_print_print_input_selected
 ** 
 ** @param xcb_connection_t       *c
 ** @param xcb_x_print_pcontext_t  context
 ** @returns xcb_x_print_print_input_selected_cookie_t
 **
 *****************************************************************************/
 
xcb_x_print_print_input_selected_cookie_t
xcb_x_print_print_input_selected (xcb_connection_t       *c  /**< */,
                                  xcb_x_print_pcontext_t  context  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_INPUT_SELECTED,
        /* isvoid */ 0
    };
    
    struct iovec xcb_parts[4];
    xcb_x_print_print_input_selected_cookie_t xcb_ret;
    xcb_x_print_print_input_selected_request_t xcb_out;
    
    xcb_out.context = context;
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, XCB_REQUEST_CHECKED, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_x_print_print_input_selected_cookie_t xcb_x_print_print_input_selected_unchecked
 ** 
 ** @param xcb_connection_t       *c
 ** @param xcb_x_print_pcontext_t  context
 ** @returns xcb_x_print_print_input_selected_cookie_t
 **
 *****************************************************************************/
 
xcb_x_print_print_input_selected_cookie_t
xcb_x_print_print_input_selected_unchecked (xcb_connection_t       *c  /**< */,
                                            xcb_x_print_pcontext_t  context  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_INPUT_SELECTED,
        /* isvoid */ 0
    };
    
    struct iovec xcb_parts[4];
    xcb_x_print_print_input_selected_cookie_t xcb_ret;
    xcb_x_print_print_input_selected_request_t xcb_out;
    
    xcb_out.context = context;
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, 0, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** uint32_t * xcb_x_print_print_input_selected_event_list
 ** 
 ** @param const xcb_x_print_print_input_selected_reply_t *R
 ** @returns uint32_t *
 **
 *****************************************************************************/
 
uint32_t *
xcb_x_print_print_input_selected_event_list (const xcb_x_print_print_input_selected_reply_t *R  /**< */)
{
    return (uint32_t *) (R + 1);
}


/*****************************************************************************
 **
 ** int xcb_x_print_print_input_selected_event_list_length
 ** 
 ** @param const xcb_x_print_print_input_selected_reply_t *R
 ** @returns int
 **
 *****************************************************************************/
 
int
xcb_x_print_print_input_selected_event_list_length (const xcb_x_print_print_input_selected_reply_t *R  /**< */)
{
    return xcb_popcount(R->event_mask);
}


/*****************************************************************************
 **
 ** xcb_generic_iterator_t xcb_x_print_print_input_selected_event_list_end
 ** 
 ** @param const xcb_x_print_print_input_selected_reply_t *R
 ** @returns xcb_generic_iterator_t
 **
 *****************************************************************************/
 
xcb_generic_iterator_t
xcb_x_print_print_input_selected_event_list_end (const xcb_x_print_print_input_selected_reply_t *R  /**< */)
{
    xcb_generic_iterator_t i;
    i.data = ((uint32_t *) (R + 1)) + (xcb_popcount(R->event_mask));
    i.rem = 0;
    i.index = (char *) i.data - (char *) R;
    return i;
}


/*****************************************************************************
 **
 ** uint32_t * xcb_x_print_print_input_selected_all_events_list
 ** 
 ** @param const xcb_x_print_print_input_selected_reply_t *R
 ** @returns uint32_t *
 **
 *****************************************************************************/
 
uint32_t *
xcb_x_print_print_input_selected_all_events_list (const xcb_x_print_print_input_selected_reply_t *R  /**< */)
{
    xcb_generic_iterator_t prev = xcb_x_print_print_input_selected_event_list_end(R);
    return (uint32_t *) ((char *) prev.data + XCB_TYPE_PAD(uint32_t, prev.index));
}


/*****************************************************************************
 **
 ** int xcb_x_print_print_input_selected_all_events_list_length
 ** 
 ** @param const xcb_x_print_print_input_selected_reply_t *R
 ** @returns int
 **
 *****************************************************************************/
 
int
xcb_x_print_print_input_selected_all_events_list_length (const xcb_x_print_print_input_selected_reply_t *R  /**< */)
{
    return xcb_popcount(R->all_events_mask);
}


/*****************************************************************************
 **
 ** xcb_generic_iterator_t xcb_x_print_print_input_selected_all_events_list_end
 ** 
 ** @param const xcb_x_print_print_input_selected_reply_t *R
 ** @returns xcb_generic_iterator_t
 **
 *****************************************************************************/
 
xcb_generic_iterator_t
xcb_x_print_print_input_selected_all_events_list_end (const xcb_x_print_print_input_selected_reply_t *R  /**< */)
{
    xcb_generic_iterator_t i;
    xcb_generic_iterator_t child = xcb_x_print_print_input_selected_event_list_end(R);
    i.data = ((uint32_t *) child.data) + (xcb_popcount(R->all_events_mask));
    i.rem = 0;
    i.index = (char *) i.data - (char *) R;
    return i;
}


/*****************************************************************************
 **
 ** xcb_x_print_print_input_selected_reply_t * xcb_x_print_print_input_selected_reply
 ** 
 ** @param xcb_connection_t                           *c
 ** @param xcb_x_print_print_input_selected_cookie_t   cookie
 ** @param xcb_generic_error_t                       **e
 ** @returns xcb_x_print_print_input_selected_reply_t *
 **
 *****************************************************************************/
 
xcb_x_print_print_input_selected_reply_t *
xcb_x_print_print_input_selected_reply (xcb_connection_t                           *c  /**< */,
                                        xcb_x_print_print_input_selected_cookie_t   cookie  /**< */,
                                        xcb_generic_error_t                       **e  /**< */)
{
    return (xcb_x_print_print_input_selected_reply_t *) xcb_wait_for_reply(c, cookie.sequence, e);
}


/*****************************************************************************
 **
 ** xcb_x_print_print_get_attributes_cookie_t xcb_x_print_print_get_attributes
 ** 
 ** @param xcb_connection_t       *c
 ** @param xcb_x_print_pcontext_t  context
 ** @param uint8_t                 pool
 ** @returns xcb_x_print_print_get_attributes_cookie_t
 **
 *****************************************************************************/
 
xcb_x_print_print_get_attributes_cookie_t
xcb_x_print_print_get_attributes (xcb_connection_t       *c  /**< */,
                                  xcb_x_print_pcontext_t  context  /**< */,
                                  uint8_t                 pool  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_GET_ATTRIBUTES,
        /* isvoid */ 0
    };
    
    struct iovec xcb_parts[4];
    xcb_x_print_print_get_attributes_cookie_t xcb_ret;
    xcb_x_print_print_get_attributes_request_t xcb_out;
    
    xcb_out.context = context;
    xcb_out.pool = pool;
    memset(xcb_out.pad0, 0, 3);
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, XCB_REQUEST_CHECKED, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_x_print_print_get_attributes_cookie_t xcb_x_print_print_get_attributes_unchecked
 ** 
 ** @param xcb_connection_t       *c
 ** @param xcb_x_print_pcontext_t  context
 ** @param uint8_t                 pool
 ** @returns xcb_x_print_print_get_attributes_cookie_t
 **
 *****************************************************************************/
 
xcb_x_print_print_get_attributes_cookie_t
xcb_x_print_print_get_attributes_unchecked (xcb_connection_t       *c  /**< */,
                                            xcb_x_print_pcontext_t  context  /**< */,
                                            uint8_t                 pool  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_GET_ATTRIBUTES,
        /* isvoid */ 0
    };
    
    struct iovec xcb_parts[4];
    xcb_x_print_print_get_attributes_cookie_t xcb_ret;
    xcb_x_print_print_get_attributes_request_t xcb_out;
    
    xcb_out.context = context;
    xcb_out.pool = pool;
    memset(xcb_out.pad0, 0, 3);
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, 0, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_x_print_print_get_attributes_reply_t * xcb_x_print_print_get_attributes_reply
 ** 
 ** @param xcb_connection_t                           *c
 ** @param xcb_x_print_print_get_attributes_cookie_t   cookie
 ** @param xcb_generic_error_t                       **e
 ** @returns xcb_x_print_print_get_attributes_reply_t *
 **
 *****************************************************************************/
 
xcb_x_print_print_get_attributes_reply_t *
xcb_x_print_print_get_attributes_reply (xcb_connection_t                           *c  /**< */,
                                        xcb_x_print_print_get_attributes_cookie_t   cookie  /**< */,
                                        xcb_generic_error_t                       **e  /**< */)
{
    return (xcb_x_print_print_get_attributes_reply_t *) xcb_wait_for_reply(c, cookie.sequence, e);
}


/*****************************************************************************
 **
 ** xcb_x_print_print_get_one_attributes_cookie_t xcb_x_print_print_get_one_attributes
 ** 
 ** @param xcb_connection_t            *c
 ** @param xcb_x_print_pcontext_t       context
 ** @param uint32_t                     nameLen
 ** @param uint8_t                      pool
 ** @param const xcb_x_print_string8_t *name
 ** @returns xcb_x_print_print_get_one_attributes_cookie_t
 **
 *****************************************************************************/
 
xcb_x_print_print_get_one_attributes_cookie_t
xcb_x_print_print_get_one_attributes (xcb_connection_t            *c  /**< */,
                                      xcb_x_print_pcontext_t       context  /**< */,
                                      uint32_t                     nameLen  /**< */,
                                      uint8_t                      pool  /**< */,
                                      const xcb_x_print_string8_t *name  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 4,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_GET_ONE_ATTRIBUTES,
        /* isvoid */ 0
    };
    
    struct iovec xcb_parts[6];
    xcb_x_print_print_get_one_attributes_cookie_t xcb_ret;
    xcb_x_print_print_get_one_attributes_request_t xcb_out;
    
    xcb_out.context = context;
    xcb_out.nameLen = nameLen;
    xcb_out.pool = pool;
    memset(xcb_out.pad0, 0, 3);
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_parts[4].iov_base = (char *) name;
    xcb_parts[4].iov_len = nameLen * sizeof(xcb_x_print_string8_t);
    xcb_parts[5].iov_base = 0;
    xcb_parts[5].iov_len = -xcb_parts[4].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, XCB_REQUEST_CHECKED, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_x_print_print_get_one_attributes_cookie_t xcb_x_print_print_get_one_attributes_unchecked
 ** 
 ** @param xcb_connection_t            *c
 ** @param xcb_x_print_pcontext_t       context
 ** @param uint32_t                     nameLen
 ** @param uint8_t                      pool
 ** @param const xcb_x_print_string8_t *name
 ** @returns xcb_x_print_print_get_one_attributes_cookie_t
 **
 *****************************************************************************/
 
xcb_x_print_print_get_one_attributes_cookie_t
xcb_x_print_print_get_one_attributes_unchecked (xcb_connection_t            *c  /**< */,
                                                xcb_x_print_pcontext_t       context  /**< */,
                                                uint32_t                     nameLen  /**< */,
                                                uint8_t                      pool  /**< */,
                                                const xcb_x_print_string8_t *name  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 4,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_GET_ONE_ATTRIBUTES,
        /* isvoid */ 0
    };
    
    struct iovec xcb_parts[6];
    xcb_x_print_print_get_one_attributes_cookie_t xcb_ret;
    xcb_x_print_print_get_one_attributes_request_t xcb_out;
    
    xcb_out.context = context;
    xcb_out.nameLen = nameLen;
    xcb_out.pool = pool;
    memset(xcb_out.pad0, 0, 3);
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_parts[4].iov_base = (char *) name;
    xcb_parts[4].iov_len = nameLen * sizeof(xcb_x_print_string8_t);
    xcb_parts[5].iov_base = 0;
    xcb_parts[5].iov_len = -xcb_parts[4].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, 0, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_x_print_string8_t * xcb_x_print_print_get_one_attributes_value
 ** 
 ** @param const xcb_x_print_print_get_one_attributes_reply_t *R
 ** @returns xcb_x_print_string8_t *
 **
 *****************************************************************************/
 
xcb_x_print_string8_t *
xcb_x_print_print_get_one_attributes_value (const xcb_x_print_print_get_one_attributes_reply_t *R  /**< */)
{
    return (xcb_x_print_string8_t *) (R + 1);
}


/*****************************************************************************
 **
 ** int xcb_x_print_print_get_one_attributes_value_length
 ** 
 ** @param const xcb_x_print_print_get_one_attributes_reply_t *R
 ** @returns int
 **
 *****************************************************************************/
 
int
xcb_x_print_print_get_one_attributes_value_length (const xcb_x_print_print_get_one_attributes_reply_t *R  /**< */)
{
    return R->valueLen;
}


/*****************************************************************************
 **
 ** xcb_x_print_string8_iterator_t xcb_x_print_print_get_one_attributes_value_iterator
 ** 
 ** @param const xcb_x_print_print_get_one_attributes_reply_t *R
 ** @returns xcb_x_print_string8_iterator_t
 **
 *****************************************************************************/
 
xcb_x_print_string8_iterator_t
xcb_x_print_print_get_one_attributes_value_iterator (const xcb_x_print_print_get_one_attributes_reply_t *R  /**< */)
{
    xcb_x_print_string8_iterator_t i;
    i.data = (xcb_x_print_string8_t *) (R + 1);
    i.rem = R->valueLen;
    i.index = (char *) i.data - (char *) R;
    return i;
}


/*****************************************************************************
 **
 ** xcb_x_print_print_get_one_attributes_reply_t * xcb_x_print_print_get_one_attributes_reply
 ** 
 ** @param xcb_connection_t                               *c
 ** @param xcb_x_print_print_get_one_attributes_cookie_t   cookie
 ** @param xcb_generic_error_t                           **e
 ** @returns xcb_x_print_print_get_one_attributes_reply_t *
 **
 *****************************************************************************/
 
xcb_x_print_print_get_one_attributes_reply_t *
xcb_x_print_print_get_one_attributes_reply (xcb_connection_t                               *c  /**< */,
                                            xcb_x_print_print_get_one_attributes_cookie_t   cookie  /**< */,
                                            xcb_generic_error_t                           **e  /**< */)
{
    return (xcb_x_print_print_get_one_attributes_reply_t *) xcb_wait_for_reply(c, cookie.sequence, e);
}


/*****************************************************************************
 **
 ** xcb_void_cookie_t xcb_x_print_print_set_attributes_checked
 ** 
 ** @param xcb_connection_t            *c
 ** @param xcb_x_print_pcontext_t       context
 ** @param uint32_t                     stringLen
 ** @param uint8_t                      pool
 ** @param uint8_t                      rule
 ** @param uint32_t                     attributes_len
 ** @param const xcb_x_print_string8_t *attributes
 ** @returns xcb_void_cookie_t
 **
 *****************************************************************************/
 
xcb_void_cookie_t
xcb_x_print_print_set_attributes_checked (xcb_connection_t            *c  /**< */,
                                          xcb_x_print_pcontext_t       context  /**< */,
                                          uint32_t                     stringLen  /**< */,
                                          uint8_t                      pool  /**< */,
                                          uint8_t                      rule  /**< */,
                                          uint32_t                     attributes_len  /**< */,
                                          const xcb_x_print_string8_t *attributes  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 4,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_SET_ATTRIBUTES,
        /* isvoid */ 1
    };
    
    struct iovec xcb_parts[6];
    xcb_void_cookie_t xcb_ret;
    xcb_x_print_print_set_attributes_request_t xcb_out;
    
    xcb_out.context = context;
    xcb_out.stringLen = stringLen;
    xcb_out.pool = pool;
    xcb_out.rule = rule;
    memset(xcb_out.pad0, 0, 2);
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_parts[4].iov_base = (char *) attributes;
    xcb_parts[4].iov_len = attributes_len * sizeof(xcb_x_print_string8_t);
    xcb_parts[5].iov_base = 0;
    xcb_parts[5].iov_len = -xcb_parts[4].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, XCB_REQUEST_CHECKED, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_void_cookie_t xcb_x_print_print_set_attributes
 ** 
 ** @param xcb_connection_t            *c
 ** @param xcb_x_print_pcontext_t       context
 ** @param uint32_t                     stringLen
 ** @param uint8_t                      pool
 ** @param uint8_t                      rule
 ** @param uint32_t                     attributes_len
 ** @param const xcb_x_print_string8_t *attributes
 ** @returns xcb_void_cookie_t
 **
 *****************************************************************************/
 
xcb_void_cookie_t
xcb_x_print_print_set_attributes (xcb_connection_t            *c  /**< */,
                                  xcb_x_print_pcontext_t       context  /**< */,
                                  uint32_t                     stringLen  /**< */,
                                  uint8_t                      pool  /**< */,
                                  uint8_t                      rule  /**< */,
                                  uint32_t                     attributes_len  /**< */,
                                  const xcb_x_print_string8_t *attributes  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 4,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_SET_ATTRIBUTES,
        /* isvoid */ 1
    };
    
    struct iovec xcb_parts[6];
    xcb_void_cookie_t xcb_ret;
    xcb_x_print_print_set_attributes_request_t xcb_out;
    
    xcb_out.context = context;
    xcb_out.stringLen = stringLen;
    xcb_out.pool = pool;
    xcb_out.rule = rule;
    memset(xcb_out.pad0, 0, 2);
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_parts[4].iov_base = (char *) attributes;
    xcb_parts[4].iov_len = attributes_len * sizeof(xcb_x_print_string8_t);
    xcb_parts[5].iov_base = 0;
    xcb_parts[5].iov_len = -xcb_parts[4].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, 0, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_x_print_print_get_page_dimensions_cookie_t xcb_x_print_print_get_page_dimensions
 ** 
 ** @param xcb_connection_t       *c
 ** @param xcb_x_print_pcontext_t  context
 ** @returns xcb_x_print_print_get_page_dimensions_cookie_t
 **
 *****************************************************************************/
 
xcb_x_print_print_get_page_dimensions_cookie_t
xcb_x_print_print_get_page_dimensions (xcb_connection_t       *c  /**< */,
                                       xcb_x_print_pcontext_t  context  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_GET_PAGE_DIMENSIONS,
        /* isvoid */ 0
    };
    
    struct iovec xcb_parts[4];
    xcb_x_print_print_get_page_dimensions_cookie_t xcb_ret;
    xcb_x_print_print_get_page_dimensions_request_t xcb_out;
    
    xcb_out.context = context;
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, XCB_REQUEST_CHECKED, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_x_print_print_get_page_dimensions_cookie_t xcb_x_print_print_get_page_dimensions_unchecked
 ** 
 ** @param xcb_connection_t       *c
 ** @param xcb_x_print_pcontext_t  context
 ** @returns xcb_x_print_print_get_page_dimensions_cookie_t
 **
 *****************************************************************************/
 
xcb_x_print_print_get_page_dimensions_cookie_t
xcb_x_print_print_get_page_dimensions_unchecked (xcb_connection_t       *c  /**< */,
                                                 xcb_x_print_pcontext_t  context  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_GET_PAGE_DIMENSIONS,
        /* isvoid */ 0
    };
    
    struct iovec xcb_parts[4];
    xcb_x_print_print_get_page_dimensions_cookie_t xcb_ret;
    xcb_x_print_print_get_page_dimensions_request_t xcb_out;
    
    xcb_out.context = context;
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, 0, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_x_print_print_get_page_dimensions_reply_t * xcb_x_print_print_get_page_dimensions_reply
 ** 
 ** @param xcb_connection_t                                *c
 ** @param xcb_x_print_print_get_page_dimensions_cookie_t   cookie
 ** @param xcb_generic_error_t                            **e
 ** @returns xcb_x_print_print_get_page_dimensions_reply_t *
 **
 *****************************************************************************/
 
xcb_x_print_print_get_page_dimensions_reply_t *
xcb_x_print_print_get_page_dimensions_reply (xcb_connection_t                                *c  /**< */,
                                             xcb_x_print_print_get_page_dimensions_cookie_t   cookie  /**< */,
                                             xcb_generic_error_t                            **e  /**< */)
{
    return (xcb_x_print_print_get_page_dimensions_reply_t *) xcb_wait_for_reply(c, cookie.sequence, e);
}


/*****************************************************************************
 **
 ** xcb_x_print_print_query_screens_cookie_t xcb_x_print_print_query_screens
 ** 
 ** @param xcb_connection_t *c
 ** @returns xcb_x_print_print_query_screens_cookie_t
 **
 *****************************************************************************/
 
xcb_x_print_print_query_screens_cookie_t
xcb_x_print_print_query_screens (xcb_connection_t *c  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_QUERY_SCREENS,
        /* isvoid */ 0
    };
    
    struct iovec xcb_parts[4];
    xcb_x_print_print_query_screens_cookie_t xcb_ret;
    xcb_x_print_print_query_screens_request_t xcb_out;
    
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, XCB_REQUEST_CHECKED, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_x_print_print_query_screens_cookie_t xcb_x_print_print_query_screens_unchecked
 ** 
 ** @param xcb_connection_t *c
 ** @returns xcb_x_print_print_query_screens_cookie_t
 **
 *****************************************************************************/
 
xcb_x_print_print_query_screens_cookie_t
xcb_x_print_print_query_screens_unchecked (xcb_connection_t *c  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_QUERY_SCREENS,
        /* isvoid */ 0
    };
    
    struct iovec xcb_parts[4];
    xcb_x_print_print_query_screens_cookie_t xcb_ret;
    xcb_x_print_print_query_screens_request_t xcb_out;
    
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, 0, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_window_t * xcb_x_print_print_query_screens_roots
 ** 
 ** @param const xcb_x_print_print_query_screens_reply_t *R
 ** @returns xcb_window_t *
 **
 *****************************************************************************/
 
xcb_window_t *
xcb_x_print_print_query_screens_roots (const xcb_x_print_print_query_screens_reply_t *R  /**< */)
{
    return (xcb_window_t *) (R + 1);
}


/*****************************************************************************
 **
 ** int xcb_x_print_print_query_screens_roots_length
 ** 
 ** @param const xcb_x_print_print_query_screens_reply_t *R
 ** @returns int
 **
 *****************************************************************************/
 
int
xcb_x_print_print_query_screens_roots_length (const xcb_x_print_print_query_screens_reply_t *R  /**< */)
{
    return R->listCount;
}


/*****************************************************************************
 **
 ** xcb_window_iterator_t xcb_x_print_print_query_screens_roots_iterator
 ** 
 ** @param const xcb_x_print_print_query_screens_reply_t *R
 ** @returns xcb_window_iterator_t
 **
 *****************************************************************************/
 
xcb_window_iterator_t
xcb_x_print_print_query_screens_roots_iterator (const xcb_x_print_print_query_screens_reply_t *R  /**< */)
{
    xcb_window_iterator_t i;
    i.data = (xcb_window_t *) (R + 1);
    i.rem = R->listCount;
    i.index = (char *) i.data - (char *) R;
    return i;
}


/*****************************************************************************
 **
 ** xcb_x_print_print_query_screens_reply_t * xcb_x_print_print_query_screens_reply
 ** 
 ** @param xcb_connection_t                          *c
 ** @param xcb_x_print_print_query_screens_cookie_t   cookie
 ** @param xcb_generic_error_t                      **e
 ** @returns xcb_x_print_print_query_screens_reply_t *
 **
 *****************************************************************************/
 
xcb_x_print_print_query_screens_reply_t *
xcb_x_print_print_query_screens_reply (xcb_connection_t                          *c  /**< */,
                                       xcb_x_print_print_query_screens_cookie_t   cookie  /**< */,
                                       xcb_generic_error_t                      **e  /**< */)
{
    return (xcb_x_print_print_query_screens_reply_t *) xcb_wait_for_reply(c, cookie.sequence, e);
}


/*****************************************************************************
 **
 ** xcb_x_print_print_set_image_resolution_cookie_t xcb_x_print_print_set_image_resolution
 ** 
 ** @param xcb_connection_t       *c
 ** @param xcb_x_print_pcontext_t  context
 ** @param uint16_t                image_resolution
 ** @returns xcb_x_print_print_set_image_resolution_cookie_t
 **
 *****************************************************************************/
 
xcb_x_print_print_set_image_resolution_cookie_t
xcb_x_print_print_set_image_resolution (xcb_connection_t       *c  /**< */,
                                        xcb_x_print_pcontext_t  context  /**< */,
                                        uint16_t                image_resolution  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_SET_IMAGE_RESOLUTION,
        /* isvoid */ 0
    };
    
    struct iovec xcb_parts[4];
    xcb_x_print_print_set_image_resolution_cookie_t xcb_ret;
    xcb_x_print_print_set_image_resolution_request_t xcb_out;
    
    xcb_out.context = context;
    xcb_out.image_resolution = image_resolution;
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, XCB_REQUEST_CHECKED, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_x_print_print_set_image_resolution_cookie_t xcb_x_print_print_set_image_resolution_unchecked
 ** 
 ** @param xcb_connection_t       *c
 ** @param xcb_x_print_pcontext_t  context
 ** @param uint16_t                image_resolution
 ** @returns xcb_x_print_print_set_image_resolution_cookie_t
 **
 *****************************************************************************/
 
xcb_x_print_print_set_image_resolution_cookie_t
xcb_x_print_print_set_image_resolution_unchecked (xcb_connection_t       *c  /**< */,
                                                  xcb_x_print_pcontext_t  context  /**< */,
                                                  uint16_t                image_resolution  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_SET_IMAGE_RESOLUTION,
        /* isvoid */ 0
    };
    
    struct iovec xcb_parts[4];
    xcb_x_print_print_set_image_resolution_cookie_t xcb_ret;
    xcb_x_print_print_set_image_resolution_request_t xcb_out;
    
    xcb_out.context = context;
    xcb_out.image_resolution = image_resolution;
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, 0, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_x_print_print_set_image_resolution_reply_t * xcb_x_print_print_set_image_resolution_reply
 ** 
 ** @param xcb_connection_t                                 *c
 ** @param xcb_x_print_print_set_image_resolution_cookie_t   cookie
 ** @param xcb_generic_error_t                             **e
 ** @returns xcb_x_print_print_set_image_resolution_reply_t *
 **
 *****************************************************************************/
 
xcb_x_print_print_set_image_resolution_reply_t *
xcb_x_print_print_set_image_resolution_reply (xcb_connection_t                                 *c  /**< */,
                                              xcb_x_print_print_set_image_resolution_cookie_t   cookie  /**< */,
                                              xcb_generic_error_t                             **e  /**< */)
{
    return (xcb_x_print_print_set_image_resolution_reply_t *) xcb_wait_for_reply(c, cookie.sequence, e);
}


/*****************************************************************************
 **
 ** xcb_x_print_print_get_image_resolution_cookie_t xcb_x_print_print_get_image_resolution
 ** 
 ** @param xcb_connection_t       *c
 ** @param xcb_x_print_pcontext_t  context
 ** @returns xcb_x_print_print_get_image_resolution_cookie_t
 **
 *****************************************************************************/
 
xcb_x_print_print_get_image_resolution_cookie_t
xcb_x_print_print_get_image_resolution (xcb_connection_t       *c  /**< */,
                                        xcb_x_print_pcontext_t  context  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_GET_IMAGE_RESOLUTION,
        /* isvoid */ 0
    };
    
    struct iovec xcb_parts[4];
    xcb_x_print_print_get_image_resolution_cookie_t xcb_ret;
    xcb_x_print_print_get_image_resolution_request_t xcb_out;
    
    xcb_out.context = context;
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, XCB_REQUEST_CHECKED, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_x_print_print_get_image_resolution_cookie_t xcb_x_print_print_get_image_resolution_unchecked
 ** 
 ** @param xcb_connection_t       *c
 ** @param xcb_x_print_pcontext_t  context
 ** @returns xcb_x_print_print_get_image_resolution_cookie_t
 **
 *****************************************************************************/
 
xcb_x_print_print_get_image_resolution_cookie_t
xcb_x_print_print_get_image_resolution_unchecked (xcb_connection_t       *c  /**< */,
                                                  xcb_x_print_pcontext_t  context  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_x_print_id,
        /* opcode */ XCB_X_PRINT_PRINT_GET_IMAGE_RESOLUTION,
        /* isvoid */ 0
    };
    
    struct iovec xcb_parts[4];
    xcb_x_print_print_get_image_resolution_cookie_t xcb_ret;
    xcb_x_print_print_get_image_resolution_request_t xcb_out;
    
    xcb_out.context = context;
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, 0, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_x_print_print_get_image_resolution_reply_t * xcb_x_print_print_get_image_resolution_reply
 ** 
 ** @param xcb_connection_t                                 *c
 ** @param xcb_x_print_print_get_image_resolution_cookie_t   cookie
 ** @param xcb_generic_error_t                             **e
 ** @returns xcb_x_print_print_get_image_resolution_reply_t *
 **
 *****************************************************************************/
 
xcb_x_print_print_get_image_resolution_reply_t *
xcb_x_print_print_get_image_resolution_reply (xcb_connection_t                                 *c  /**< */,
                                              xcb_x_print_print_get_image_resolution_cookie_t   cookie  /**< */,
                                              xcb_generic_error_t                             **e  /**< */)
{
    return (xcb_x_print_print_get_image_resolution_reply_t *) xcb_wait_for_reply(c, cookie.sequence, e);
}

