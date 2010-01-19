
#ifndef _XINTCONN_H_
#define _XINTCONN_H_ 1

#include <X11/Xfuncproto.h>

_XFUNCPROTOBEGIN

#if !USE_XCB
/* ConnDis.c */

int _XConnectDisplay (
    char *display_name,
    char **fullnamep,           /* RETURN */
    int *dpynump,           /* RETURN */
    int *screenp,           /* RETURN */
    char **auth_namep,          /* RETURN */
    int *auth_namelenp,         /* RETURN */
    char **auth_datap,          /* RETURN */
    int *auth_datalenp);        /* RETURN */
extern int _XDisconnectDisplay(XtransConnInfo trans_conn);
extern Bool _XSendClientPrefix(Display *dpy, xConnClientPrefix *client,
				char *auth_proto, char *auth_string,
				xConnSetupPrefix *prefix);
extern XtransConnInfo _X11TransConnectDisplay(char *display_name,
				char **fullnamep, int *dpynump,
				int *screenp, char **auth_namep,
				int *auth_namelenp, char **auth_datap,
				int *auth_datalenp);
#endif /* !USE_XCB */

/* OpenDis.c */
extern void _XFreeDisplayStructure(Display *dpy);

_XFUNCPROTOEND

#endif /* _XINTCONN_H_ */
