/*
 * This file generated automatically from bigreq.xml by c-client.xsl using XSLT.
 * Edit at your peril.
 */

#include <assert.h>
#include "xcbext.h"
#include "bigreq.h"

xcb_extension_t xcb_big_requests_id = { "BIG-REQUESTS" };


/*****************************************************************************
 **
 ** xcb_big_requests_enable_cookie_t xcb_big_requests_enable
 ** 
 ** @param xcb_connection_t *c
 ** @returns xcb_big_requests_enable_cookie_t
 **
 *****************************************************************************/
 
xcb_big_requests_enable_cookie_t
xcb_big_requests_enable (xcb_connection_t *c  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_big_requests_id,
        /* opcode */ XCB_BIG_REQUESTS_ENABLE,
        /* isvoid */ 0
    };
    
    struct iovec xcb_parts[4];
    xcb_big_requests_enable_cookie_t xcb_ret;
    xcb_big_requests_enable_request_t xcb_out;
    
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, XCB_REQUEST_CHECKED, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_big_requests_enable_cookie_t xcb_big_requests_enable_unchecked
 ** 
 ** @param xcb_connection_t *c
 ** @returns xcb_big_requests_enable_cookie_t
 **
 *****************************************************************************/
 
xcb_big_requests_enable_cookie_t
xcb_big_requests_enable_unchecked (xcb_connection_t *c  /**< */)
{
    static const xcb_protocol_request_t xcb_req = {
        /* count */ 2,
        /* ext */ &xcb_big_requests_id,
        /* opcode */ XCB_BIG_REQUESTS_ENABLE,
        /* isvoid */ 0
    };
    
    struct iovec xcb_parts[4];
    xcb_big_requests_enable_cookie_t xcb_ret;
    xcb_big_requests_enable_request_t xcb_out;
    
    
    xcb_parts[2].iov_base = (char *) &xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;
    xcb_ret.sequence = xcb_send_request(c, 0, xcb_parts + 2, &xcb_req);
    return xcb_ret;
}


/*****************************************************************************
 **
 ** xcb_big_requests_enable_reply_t * xcb_big_requests_enable_reply
 ** 
 ** @param xcb_connection_t                  *c
 ** @param xcb_big_requests_enable_cookie_t   cookie
 ** @param xcb_generic_error_t              **e
 ** @returns xcb_big_requests_enable_reply_t *
 **
 *****************************************************************************/
 
xcb_big_requests_enable_reply_t *
xcb_big_requests_enable_reply (xcb_connection_t                  *c  /**< */,
                               xcb_big_requests_enable_cookie_t   cookie  /**< */,
                               xcb_generic_error_t              **e  /**< */)
{
    return (xcb_big_requests_enable_reply_t *) xcb_wait_for_reply(c, cookie.sequence, e);
}

