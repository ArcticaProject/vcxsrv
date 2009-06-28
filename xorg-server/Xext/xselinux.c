/************************************************************

Author: Eamon Walsh <ewalsh@epoch.ncsc.mil>

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
this permission notice appear in supporting documentation.  This permission
notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

********************************************************/

/*
 * Portions of this code copyright (c) 2005 by Trusted Computer Solutions, Inc.
 * All rights reserved.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <sys/socket.h>
#include <stdio.h>
#include <stdarg.h>

#include <selinux/selinux.h>
#include <selinux/label.h>
#include <selinux/avc.h>

#include <libaudit.h>

#include <X11/Xatom.h>
#include "globals.h"
#include "resource.h"
#include "privates.h"
#include "registry.h"
#include "dixstruct.h"
#include "inputstr.h"
#include "windowstr.h"
#include "propertyst.h"
#include "extnsionst.h"
#include "scrnintstr.h"
#include "selection.h"
#include "xacestr.h"
#include "xselinux.h"
#define XSERV_t
#define TRANS_SERVER
#include <X11/Xtrans/Xtrans.h>
#include "../os/osdep.h"
#include "modinit.h"


/*
 * Globals
 */

/* private state keys */
static DevPrivateKey subjectKey = &subjectKey;
static DevPrivateKey objectKey = &objectKey;
static DevPrivateKey dataKey = &dataKey;

/* subject state (clients and devices only) */
typedef struct {
    security_id_t sid;
    security_id_t dev_create_sid;
    security_id_t win_create_sid;
    security_id_t sel_create_sid;
    security_id_t prp_create_sid;
    security_id_t sel_use_sid;
    security_id_t prp_use_sid;
    struct avc_entry_ref aeref;
    char *command;
    int privileged;
} SELinuxSubjectRec;

/* object state */
typedef struct {
    security_id_t sid;
    int poly;
} SELinuxObjectRec;

/* selection and property atom cache */
typedef struct {
    SELinuxObjectRec prp;
    SELinuxObjectRec sel;
} SELinuxAtomRec;

/* audit file descriptor */
static int audit_fd;

/* structure passed to auditing callback */
typedef struct {
    ClientPtr client;	/* client */
    DeviceIntPtr dev;	/* device */
    char *command;	/* client's executable path */
    unsigned id;	/* resource id, if any */
    int restype;	/* resource type, if any */
    int event;		/* event type, if any */
    Atom property;	/* property name, if any */
    Atom selection;	/* selection name, if any */
    char *extension;	/* extension name, if any */
} SELinuxAuditRec;

/* labeling handle */
static struct selabel_handle *label_hnd;

/* whether AVC is active */
static int avc_active;

/* atoms for window label properties */
static Atom atom_ctx;
static Atom atom_client_ctx;

/* The unlabeled SID */
static security_id_t unlabeled_sid;

/* Array of object classes indexed by resource type */
static security_class_t *knownTypes;
static unsigned numKnownTypes;

/* Array of event SIDs indexed by event type */
static security_id_t *knownEvents;
static unsigned numKnownEvents;

/* Array of property and selection SID structures */
static SELinuxAtomRec *knownAtoms;
static unsigned numKnownAtoms;

/* dynamically allocated security classes and permissions */
static struct security_class_mapping map[] = {
    { "x_drawable", { "read", "write", "destroy", "create", "getattr", "setattr", "list_property", "get_property", "set_property", "", "", "list_child", "add_child", "remove_child", "hide", "show", "blend", "override", "", "", "", "", "send", "receive", "", "manage", NULL }},
    { "x_screen", { "", "", "", "", "getattr", "setattr", "saver_getattr", "saver_setattr", "", "", "", "", "", "", "hide_cursor", "show_cursor", "saver_hide", "saver_show", NULL }},
    { "x_gc", { "", "", "destroy", "create", "getattr", "setattr", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "use", NULL }},
    { "x_font", { "", "", "destroy", "create", "getattr", "", "", "", "", "", "", "", "add_glyph", "remove_glyph", "", "", "", "", "", "", "", "", "", "", "use", NULL }},
    { "x_colormap", { "read", "write", "destroy", "create", "getattr", "", "", "", "", "", "", "", "add_color", "remove_color", "", "", "", "", "", "", "install", "uninstall", "", "", "use", NULL }},
    { "x_property", { "read", "write", "destroy", "create", "getattr", "setattr", "", "", "", "", "", "", "", "", "", "", "write", NULL }},
    { "x_selection", { "read", "", "", "setattr", "getattr", "setattr", NULL }},
    { "x_cursor", { "read", "write", "destroy", "create", "getattr", "setattr", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "use", NULL }},
    { "x_client", { "", "", "destroy", "", "getattr", "setattr", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "manage", NULL }},
    { "x_device", { "read", "write", "", "", "getattr", "setattr", "", "", "", "getfocus", "setfocus", "", "", "", "", "", "", "grab", "freeze", "force_cursor", "", "", "", "", "use", "manage", "", "bell", NULL }},
    { "x_server", { "record", "", "", "", "getattr", "setattr", "", "", "", "", "", "", "", "", "", "", "", "grab", "", "", "", "", "", "", "", "manage", "debug", NULL }},
    { "x_extension", { "", "", "", "", "query", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "use", NULL }},
    { "x_event", { "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "send", "receive", NULL }},
    { "x_synthetic_event", { "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "send", "receive", NULL }},
    { "x_resource", { "read", "write", "write", "write", "read", "write", "read", "read", "write", "read", "write", "read", "write", "write", "write", "read", "read", "write", "write", "write", "write", "write", "write", "read", "read", "write", "read", "write", NULL }},
    { NULL }
};

/* x_resource "read" bits from the list above */
#define SELinuxReadMask (DixReadAccess|DixGetAttrAccess|DixListPropAccess| \
			 DixGetPropAccess|DixGetFocusAccess|DixListAccess| \
			 DixShowAccess|DixBlendAccess|DixReceiveAccess| \
			 DixUseAccess|DixDebugAccess)

/* forward declarations */
static void SELinuxScreen(CallbackListPtr *, pointer, pointer);

/* "true" pointer value for use as callback data */
static pointer truep = (pointer)1;


/*
 * Support Routines
 */

/*
 * Looks up a name in the selection or property mappings
 */
static int
SELinuxAtomToSIDLookup(Atom atom, SELinuxObjectRec *obj, int map, int polymap)
{
    const char *name = NameForAtom(atom);
    security_context_t ctx;
    int rc = Success;

    obj->poly = 1;

    /* Look in the mappings of names to contexts */
    if (selabel_lookup(label_hnd, &ctx, name, map) == 0) {
	obj->poly = 0;
    } else if (errno != ENOENT) {
	ErrorF("SELinux: a property label lookup failed!\n");
	return BadValue;
    } else if (selabel_lookup(label_hnd, &ctx, name, polymap) < 0) {
	ErrorF("SELinux: a property label lookup failed!\n");
	return BadValue;
    }

    /* Get a SID for context */
    if (avc_context_to_sid(ctx, &obj->sid) < 0) {
	ErrorF("SELinux: a context_to_SID call failed!\n");
	rc = BadAlloc;
    }

    freecon(ctx);
    return rc;
}

/*
 * Looks up the SID corresponding to the given property or selection atom
 */
static int
SELinuxAtomToSID(Atom atom, int prop, SELinuxObjectRec **obj_rtn)
{
    SELinuxObjectRec *obj;
    int rc, map, polymap;

    if (atom >= numKnownAtoms) {
	/* Need to increase size of atoms array */
	unsigned size = sizeof(SELinuxAtomRec);
	knownAtoms = xrealloc(knownAtoms, (atom + 1) * size);
	if (!knownAtoms)
	    return BadAlloc;
	memset(knownAtoms + numKnownAtoms, 0,
	       (atom - numKnownAtoms + 1) * size);
	numKnownAtoms = atom + 1;
    }

    if (prop) {
	obj = &knownAtoms[atom].prp;
	map = SELABEL_X_PROP;
	polymap = SELABEL_X_POLYPROP;
    } else {
	obj = &knownAtoms[atom].sel;
	map = SELABEL_X_SELN;
	polymap = SELABEL_X_POLYSELN;
    }

    if (!obj->sid) {
	rc = SELinuxAtomToSIDLookup(atom, obj, map, polymap);
	if (rc != Success)
	    goto out;
    }

    *obj_rtn = obj;
    rc = Success;
out:
    return rc;
}

/*
 * Looks up a SID for a selection/subject pair
 */
static int
SELinuxSelectionToSID(Atom selection, SELinuxSubjectRec *subj,
		      security_id_t *sid_rtn, int *poly_rtn)
{
    int rc;
    SELinuxObjectRec *obj;
    security_id_t tsid;

    /* Get the default context and polyinstantiation bit */
    rc = SELinuxAtomToSID(selection, 0, &obj);
    if (rc != Success)
	return rc;

    /* Check for an override context next */
    if (subj->sel_use_sid) {
	sidget(tsid = subj->sel_use_sid);
	goto out;
    }

    sidget(tsid = obj->sid);

    /* Polyinstantiate if necessary to obtain the final SID */
    if (obj->poly) {
	sidput(tsid);
	if (avc_compute_member(subj->sid, obj->sid,
			       SECCLASS_X_SELECTION, &tsid) < 0) {
	    ErrorF("SELinux: a compute_member call failed!\n");
	    return BadValue;
	}
    }
out:
    *sid_rtn = tsid;
    if (poly_rtn)
	*poly_rtn = obj->poly;
    return Success;
}

/*
 * Looks up a SID for a property/subject pair
 */
static int
SELinuxPropertyToSID(Atom property, SELinuxSubjectRec *subj,
		     security_id_t *sid_rtn, int *poly_rtn)
{
    int rc;
    SELinuxObjectRec *obj;
    security_id_t tsid, tsid2;

    /* Get the default context and polyinstantiation bit */
    rc = SELinuxAtomToSID(property, 1, &obj);
    if (rc != Success)
	return rc;

    /* Check for an override context next */
    if (subj->prp_use_sid) {
	sidget(tsid = subj->prp_use_sid);
	goto out;
    }

    /* Perform a transition */
    if (avc_compute_create(subj->sid, obj->sid,
			   SECCLASS_X_PROPERTY, &tsid) < 0) {
	ErrorF("SELinux: a compute_create call failed!\n");
	return BadValue;
    }

    /* Polyinstantiate if necessary to obtain the final SID */
    if (obj->poly) {
	tsid2 = tsid;
	if (avc_compute_member(subj->sid, tsid2,
			       SECCLASS_X_PROPERTY, &tsid) < 0) {
	    ErrorF("SELinux: a compute_member call failed!\n");
	    sidput(tsid2);
	    return BadValue;
	}
	sidput(tsid2);
    }
out:
    *sid_rtn = tsid;
    if (poly_rtn)
	*poly_rtn = obj->poly;
    return Success;
}

/*
 * Looks up the SID corresponding to the given event type
 */
static int
SELinuxEventToSID(unsigned type, security_id_t sid_of_window,
		  SELinuxObjectRec *sid_return)
{
    const char *name = LookupEventName(type);
    security_context_t con;
    type &= 127;

    if (type >= numKnownEvents) {
	/* Need to increase size of classes array */
	unsigned size = sizeof(security_id_t);
	knownEvents = xrealloc(knownEvents, (type + 1) * size);
	if (!knownEvents)
	    return BadAlloc;
	memset(knownEvents + numKnownEvents, 0,
	       (type - numKnownEvents + 1) * size);
	numKnownEvents = type + 1;
    }

    if (!knownEvents[type]) {
	/* Look in the mappings of event names to contexts */
	if (selabel_lookup(label_hnd, &con, name, SELABEL_X_EVENT) < 0) {
	    ErrorF("SELinux: an event label lookup failed!\n");
	    return BadValue;
	}
	/* Get a SID for context */
	if (avc_context_to_sid(con, knownEvents + type) < 0) {
	    ErrorF("SELinux: a context_to_SID call failed!\n");
	    return BadAlloc;
	}
	freecon(con);
    }

    /* Perform a transition to obtain the final SID */
    if (avc_compute_create(sid_of_window, knownEvents[type], SECCLASS_X_EVENT,
			   &sid_return->sid) < 0) {
	ErrorF("SELinux: a compute_create call failed!\n");
	return BadValue;
    }

    return Success;
}

/*
 * Returns the object class corresponding to the given resource type.
 */
static security_class_t
SELinuxTypeToClass(RESTYPE type)
{
    RESTYPE fulltype = type;
    type &= TypeMask;

    if (type >= numKnownTypes) {
	/* Need to increase size of classes array */
	unsigned size = sizeof(security_class_t);
	knownTypes = xrealloc(knownTypes, (type + 1) * size);
	if (!knownTypes)
	    return 0;
	memset(knownTypes + numKnownTypes, 0,
	       (type - numKnownTypes + 1) * size);
	numKnownTypes = type + 1;
    }

    if (!knownTypes[type]) {
	const char *str;
	knownTypes[type] = SECCLASS_X_RESOURCE;

	if (fulltype & RC_DRAWABLE)
	    knownTypes[type] = SECCLASS_X_DRAWABLE;
	if (fulltype == RT_GC)
	    knownTypes[type] = SECCLASS_X_GC;
	if (fulltype == RT_FONT)
	    knownTypes[type] = SECCLASS_X_FONT;
	if (fulltype == RT_CURSOR)
	    knownTypes[type] = SECCLASS_X_CURSOR;
	if (fulltype == RT_COLORMAP)
	    knownTypes[type] = SECCLASS_X_COLORMAP;

	/* Need to do a string lookup */
	str = LookupResourceName(fulltype);
	if (!strcmp(str, "PICTURE"))
	    knownTypes[type] = SECCLASS_X_DRAWABLE;
	if (!strcmp(str, "GLYPHSET"))
	    knownTypes[type] = SECCLASS_X_FONT;
    }

    return knownTypes[type];
}

/*
 * Performs an SELinux permission check.
 */
static int
SELinuxDoCheck(SELinuxSubjectRec *subj, SELinuxObjectRec *obj,
	       security_class_t class, Mask mode, SELinuxAuditRec *auditdata)
{
    /* serverClient requests OK */
    if (subj->privileged)
	return Success;

    auditdata->command = subj->command;
    errno = 0;

    if (avc_has_perm(subj->sid, obj->sid, class, mode, &subj->aeref,
		     auditdata) < 0) {
	if (mode == DixUnknownAccess)
	    return Success; /* DixUnknownAccess requests OK ... for now */
	if (errno == EACCES)
	    return BadAccess;
	ErrorF("SELinux: avc_has_perm: unexpected error %d\n", errno);
	return BadValue;
    }

    return Success;
}

/*
 * Labels a newly connected client.
 */
static void
SELinuxLabelClient(ClientPtr client)
{
    XtransConnInfo ci = ((OsCommPtr)client->osPrivate)->trans_conn;
    int fd = _XSERVTransGetConnectionNumber(ci);
    SELinuxSubjectRec *subj;
    SELinuxObjectRec *obj;
    security_context_t ctx;

    subj = dixLookupPrivate(&client->devPrivates, subjectKey);
    sidput(subj->sid);
    obj = dixLookupPrivate(&client->devPrivates, objectKey);
    sidput(obj->sid);

    /* Try to get a context from the socket */
    if (fd < 0 || getpeercon(fd, &ctx) < 0) {
	/* Otherwise, fall back to a default context */
	if (selabel_lookup(label_hnd, &ctx, NULL, SELABEL_X_CLIENT) < 0)
	    FatalError("SELinux: failed to look up remote-client context\n");
    }

    /* For local clients, try and determine the executable name */
    if (_XSERVTransIsLocal(ci)) {
	struct ucred creds;
	socklen_t len = sizeof(creds);
	char path[PATH_MAX + 1];
	size_t bytes;

	memset(&creds, 0, sizeof(creds));
	if (getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &creds, &len) < 0)
	    goto finish;

	snprintf(path, PATH_MAX + 1, "/proc/%d/cmdline", creds.pid);
	fd = open(path, O_RDONLY);
	if (fd < 0)
	    goto finish;

	bytes = read(fd, path, PATH_MAX + 1);
	close(fd);
	if (bytes <= 0)
	    goto finish;

	subj->command = xalloc(bytes);
	if (!subj->command)
	    goto finish;

	memcpy(subj->command, path, bytes);
	subj->command[bytes - 1] = 0;
    }

finish:
    /* Get a SID from the context */
    if (avc_context_to_sid(ctx, &subj->sid) < 0)
	FatalError("SELinux: client %d: context_to_sid(%s) failed\n",
		   client->index, ctx);

    sidget(obj->sid = subj->sid);
    freecon(ctx);
}

/*
 * Labels initial server objects.
 */
static void
SELinuxLabelInitial(void)
{
    int i;
    XaceScreenAccessRec srec;
    SELinuxSubjectRec *subj;
    SELinuxObjectRec *obj;
    security_context_t ctx;
    pointer unused;

    /* Do the serverClient */
    subj = dixLookupPrivate(&serverClient->devPrivates, subjectKey);
    obj = dixLookupPrivate(&serverClient->devPrivates, objectKey);
    subj->privileged = 1;
    sidput(subj->sid);

    /* Use the context of the X server process for the serverClient */
    if (getcon(&ctx) < 0)
	FatalError("SELinux: couldn't get context of X server process\n");

    /* Get a SID from the context */
    if (avc_context_to_sid(ctx, &subj->sid) < 0)
	FatalError("SELinux: serverClient: context_to_sid(%s) failed\n", ctx);

    sidget(obj->sid = subj->sid);
    freecon(ctx);

    srec.client = serverClient;
    srec.access_mode = DixCreateAccess;
    srec.status = Success;

    for (i = 0; i < screenInfo.numScreens; i++) {
	/* Do the screen object */
	srec.screen = screenInfo.screens[i];
	SELinuxScreen(NULL, NULL, &srec);

	/* Do the default colormap */
	dixLookupResource(&unused, screenInfo.screens[i]->defColormap,
			  RT_COLORMAP, serverClient, DixCreateAccess);
    }
}

/*
 * Labels new resource objects.
 */
static int
SELinuxLabelResource(XaceResourceAccessRec *rec, SELinuxSubjectRec *subj,
		     SELinuxObjectRec *obj, security_class_t class)
{
    int offset;
    security_id_t tsid;

    /* Check for a create context */
    if (rec->rtype == RT_WINDOW && subj->win_create_sid) {
	sidget(obj->sid = subj->win_create_sid);
	return Success;
    }

    if (rec->parent)
	offset = dixLookupPrivateOffset(rec->ptype);

    if (rec->parent && offset >= 0) {
	/* Use the SID of the parent object in the labeling operation */
	PrivateRec **privatePtr = DEVPRIV_AT(rec->parent, offset);
	SELinuxObjectRec *pobj = dixLookupPrivate(privatePtr, objectKey);
	tsid = pobj->sid;
    } else {
	/* Use the SID of the subject */
	tsid = subj->sid;
    }

    /* Perform a transition to obtain the final SID */
    if (avc_compute_create(subj->sid, tsid, class, &obj->sid) < 0) {
	ErrorF("SELinux: a compute_create call failed!\n");
	return BadValue;
    }

    return Success;
}


/*
 * Libselinux Callbacks
 */

static int
SELinuxAudit(void *auditdata,
	     security_class_t class,
	     char *msgbuf,
	     size_t msgbufsize)
{
    SELinuxAuditRec *audit = auditdata;
    ClientPtr client = audit->client;
    char idNum[16], *propertyName, *selectionName;
    int major = -1, minor = -1;

    if (client) {
	REQUEST(xReq);
	if (stuff) {
	    major = stuff->reqType;
	    minor = MinorOpcodeOfRequest(client);
	}
    }
    if (audit->id)
	snprintf(idNum, 16, "%x", audit->id);

    propertyName = audit->property ? NameForAtom(audit->property) : NULL;
    selectionName = audit->selection ? NameForAtom(audit->selection) : NULL;

    return snprintf(msgbuf, msgbufsize,
		    "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		    (major >= 0) ? "request=" : "",
		    (major >= 0) ? LookupRequestName(major, minor) : "",
		    audit->command ? " comm=" : "",
		    audit->command ? audit->command : "",
		    audit->dev ? " xdevice=\"" : "",
		    audit->dev ? audit->dev->name : "",
		    audit->dev ? "\"" : "",
		    audit->id ? " resid=" : "",
		    audit->id ? idNum : "",
		    audit->restype ? " restype=" : "",
		    audit->restype ? LookupResourceName(audit->restype) : "",
		    audit->event ? " event=" : "",
		    audit->event ? LookupEventName(audit->event & 127) : "",
		    audit->property ? " property=" : "",
		    audit->property ? propertyName : "",
		    audit->selection ? " selection=" : "",
		    audit->selection ? selectionName : "",
		    audit->extension ? " extension=" : "",
		    audit->extension ? audit->extension : "");
}

static int
SELinuxLog(int type, const char *fmt, ...)
{
    va_list ap;
    char buf[MAX_AUDIT_MESSAGE_LENGTH];
    int rc, aut = AUDIT_USER_AVC;

    va_start(ap, fmt);
    vsnprintf(buf, MAX_AUDIT_MESSAGE_LENGTH, fmt, ap);
    rc = audit_log_user_avc_message(audit_fd, aut, buf, NULL, NULL, NULL, 0);
    va_end(ap);
    LogMessageVerb(X_WARNING, 0, "%s", buf);
    return 0;
}

/*
 * XACE Callbacks
 */

static void
SELinuxDevice(CallbackListPtr *pcbl, pointer unused, pointer calldata)
{
    XaceDeviceAccessRec *rec = calldata;
    SELinuxSubjectRec *subj;
    SELinuxObjectRec *obj;
    SELinuxAuditRec auditdata = { .client = rec->client, .dev = rec->dev };
    int rc;

    subj = dixLookupPrivate(&rec->client->devPrivates, subjectKey);
    obj = dixLookupPrivate(&rec->dev->devPrivates, objectKey);

    /* If this is a new object that needs labeling, do it now */
    if (rec->access_mode & DixCreateAccess) {
	SELinuxSubjectRec *dsubj;
	dsubj = dixLookupPrivate(&rec->dev->devPrivates, subjectKey);

	sidput(dsubj->sid);
	sidput(obj->sid);

	if (subj->dev_create_sid) {
	    /* Label the device with the create context */
	    sidget(obj->sid = subj->dev_create_sid);
	    sidget(dsubj->sid = subj->dev_create_sid);
	} else {
	    /* Label the device directly with the process SID */
	    sidget(obj->sid = subj->sid);
	    sidget(dsubj->sid = subj->sid);
	}
    }

    /* XXX only check read permission on XQueryKeymap */
    /* This is to allow the numerous apps that call XQueryPointer to work */
    if (rec->access_mode & DixReadAccess) {
	ClientPtr client = rec->client;
	REQUEST(xReq);
	if (stuff && stuff->reqType != X_QueryKeymap) {
	    rec->access_mode &= ~DixReadAccess;
	    rec->access_mode |= DixGetAttrAccess;
	}
    }

    rc = SELinuxDoCheck(subj, obj, SECCLASS_X_DEVICE, rec->access_mode,
			&auditdata);
    if (rc != Success)
	rec->status = rc;
}

static void
SELinuxSend(CallbackListPtr *pcbl, pointer unused, pointer calldata)
{
    XaceSendAccessRec *rec = calldata;
    SELinuxSubjectRec *subj;
    SELinuxObjectRec *obj, ev_sid;
    SELinuxAuditRec auditdata = { .client = rec->client, .dev = rec->dev };
    security_class_t class;
    int rc, i, type;

    if (rec->dev)
	subj = dixLookupPrivate(&rec->dev->devPrivates, subjectKey);
    else
	subj = dixLookupPrivate(&rec->client->devPrivates, subjectKey);

    obj = dixLookupPrivate(&rec->pWin->devPrivates, objectKey);

    /* Check send permission on window */
    rc = SELinuxDoCheck(subj, obj, SECCLASS_X_DRAWABLE, DixSendAccess,
			&auditdata);
    if (rc != Success)
	goto err;

    /* Check send permission on specific event types */
    for (i = 0; i < rec->count; i++) {
	type = rec->events[i].u.u.type;
	class = (type & 128) ? SECCLASS_X_FAKEEVENT : SECCLASS_X_EVENT;

	rc = SELinuxEventToSID(type, obj->sid, &ev_sid);
	if (rc != Success)
	    goto err;

	auditdata.event = type;
	rc = SELinuxDoCheck(subj, &ev_sid, class, DixSendAccess, &auditdata);
	if (rc != Success)
	    goto err;
    }
    return;
err:
    rec->status = rc;
}

static void
SELinuxReceive(CallbackListPtr *pcbl, pointer unused, pointer calldata)
{
    XaceReceiveAccessRec *rec = calldata;
    SELinuxSubjectRec *subj;
    SELinuxObjectRec *obj, ev_sid;
    SELinuxAuditRec auditdata = { .client = NULL };
    security_class_t class;
    int rc, i, type;

    subj = dixLookupPrivate(&rec->client->devPrivates, subjectKey);
    obj = dixLookupPrivate(&rec->pWin->devPrivates, objectKey);

    /* Check receive permission on window */
    rc = SELinuxDoCheck(subj, obj, SECCLASS_X_DRAWABLE, DixReceiveAccess,
			&auditdata);
    if (rc != Success)
	goto err;

    /* Check receive permission on specific event types */
    for (i = 0; i < rec->count; i++) {
	type = rec->events[i].u.u.type;
	class = (type & 128) ? SECCLASS_X_FAKEEVENT : SECCLASS_X_EVENT;

	rc = SELinuxEventToSID(type, obj->sid, &ev_sid);
	if (rc != Success)
	    goto err;

	auditdata.event = type;
	rc = SELinuxDoCheck(subj, &ev_sid, class, DixReceiveAccess, &auditdata);
	if (rc != Success)
	    goto err;
    }
    return;
err:
    rec->status = rc;
}

static void
SELinuxExtension(CallbackListPtr *pcbl, pointer unused, pointer calldata)
{
    XaceExtAccessRec *rec = calldata;
    SELinuxSubjectRec *subj, *serv;
    SELinuxObjectRec *obj;
    SELinuxAuditRec auditdata = { .client = rec->client };
    int rc;

    subj = dixLookupPrivate(&rec->client->devPrivates, subjectKey);
    obj = dixLookupPrivate(&rec->ext->devPrivates, objectKey);

    /* If this is a new object that needs labeling, do it now */
    /* XXX there should be a separate callback for this */
    if (obj->sid == unlabeled_sid) {
	const char *name = rec->ext->name;
	security_context_t con;
	security_id_t sid;

	serv = dixLookupPrivate(&serverClient->devPrivates, subjectKey);

	/* Look in the mappings of extension names to contexts */
	if (selabel_lookup(label_hnd, &con, name, SELABEL_X_EXT) < 0) {
	    ErrorF("SELinux: a property label lookup failed!\n");
	    rec->status = BadValue;
	    return;
	}
	/* Get a SID for context */
	if (avc_context_to_sid(con, &sid) < 0) {
	    ErrorF("SELinux: a context_to_SID call failed!\n");
	    rec->status = BadAlloc;
	    return;
	}

	sidput(obj->sid);

	/* Perform a transition to obtain the final SID */
	if (avc_compute_create(serv->sid, sid, SECCLASS_X_EXTENSION,
			       &obj->sid) < 0) {
	    ErrorF("SELinux: a SID transition call failed!\n");
	    freecon(con);
	    rec->status = BadValue;
	    return;
	}
	freecon(con);
    }

    /* Perform the security check */
    auditdata.extension = rec->ext->name;
    rc = SELinuxDoCheck(subj, obj, SECCLASS_X_EXTENSION, rec->access_mode,
			&auditdata);
    if (rc != Success)
	rec->status = rc;
}

static void
SELinuxSelection(CallbackListPtr *pcbl, pointer unused, pointer calldata)
{
    XaceSelectionAccessRec *rec = calldata;
    SELinuxSubjectRec *subj;
    SELinuxObjectRec *obj, *data;
    Selection *pSel = *rec->ppSel;
    Atom name = pSel->selection;
    Mask access_mode = rec->access_mode;
    SELinuxAuditRec auditdata = { .client = rec->client, .selection = name };
    security_id_t tsid;
    int rc;

    subj = dixLookupPrivate(&rec->client->devPrivates, subjectKey);
    obj = dixLookupPrivate(&pSel->devPrivates, objectKey);

    /* If this is a new object that needs labeling, do it now */
    if (access_mode & DixCreateAccess) {
	sidput(obj->sid);
	rc = SELinuxSelectionToSID(name, subj, &obj->sid, &obj->poly);
	if (rc != Success)
	    obj->sid = unlabeled_sid;
	access_mode = DixSetAttrAccess;
    }
    /* If this is a polyinstantiated object, find the right instance */
    else if (obj->poly) {
	rc = SELinuxSelectionToSID(name, subj, &tsid, NULL);
	if (rc != Success) {
	    rec->status = rc;
	    return;
	}
	while (pSel->selection != name || obj->sid != tsid) {
	    if ((pSel = pSel->next) == NULL)
		break;
	    obj = dixLookupPrivate(&pSel->devPrivates, objectKey);
	}
	sidput(tsid);
	
	if (pSel)
	    *rec->ppSel = pSel;
	else {
	    rec->status = BadMatch;
	    return;
	}
    }

    /* Perform the security check */
    rc = SELinuxDoCheck(subj, obj, SECCLASS_X_SELECTION, access_mode,
			&auditdata);
    if (rc != Success)
	rec->status = rc;

    /* Label the content (advisory only) */
    if (access_mode & DixSetAttrAccess) {
	data = dixLookupPrivate(&pSel->devPrivates, dataKey);
	sidput(data->sid);
	if (subj->sel_create_sid)
	    sidget(data->sid = subj->sel_create_sid);
	else
	    sidget(data->sid = obj->sid);
    }
}

static void
SELinuxProperty(CallbackListPtr *pcbl, pointer unused, pointer calldata)
{
    XacePropertyAccessRec *rec = calldata;
    SELinuxSubjectRec *subj;
    SELinuxObjectRec *obj, *data;
    PropertyPtr pProp = *rec->ppProp;
    Atom name = pProp->propertyName;
    SELinuxAuditRec auditdata = { .client = rec->client, .property = name };
    security_id_t tsid;
    int rc;

    subj = dixLookupPrivate(&rec->client->devPrivates, subjectKey);
    obj = dixLookupPrivate(&pProp->devPrivates, objectKey);

    /* If this is a new object that needs labeling, do it now */
    if (rec->access_mode & DixCreateAccess) {
	sidput(obj->sid);
	rc = SELinuxPropertyToSID(name, subj, &obj->sid, &obj->poly);
	if (rc != Success) {
	    rec->status = rc;
	    return;
	}
    }
    /* If this is a polyinstantiated object, find the right instance */
    else if (obj->poly) {
	rc = SELinuxPropertyToSID(name, subj, &tsid, NULL);
	if (rc != Success) {
	    rec->status = rc;
	    return;
	}
	while (pProp->propertyName != name || obj->sid != tsid) {
	    if ((pProp = pProp->next) == NULL)
		break;
	    obj = dixLookupPrivate(&pProp->devPrivates, objectKey);
	}
	sidput(tsid);

	if (pProp)
	    *rec->ppProp = pProp;
	else {
	    rec->status = BadMatch;
	    return;
	}
    }

    /* Perform the security check */
    rc = SELinuxDoCheck(subj, obj, SECCLASS_X_PROPERTY, rec->access_mode,
			&auditdata);
    if (rc != Success)
	rec->status = rc;

    /* Label the content (advisory only) */
    if (rec->access_mode & DixWriteAccess) {
	data = dixLookupPrivate(&pProp->devPrivates, dataKey);
	sidput(data->sid);
	if (subj->prp_create_sid)
	    sidget(data->sid = subj->prp_create_sid);
	else
	    sidget(data->sid = obj->sid);
    }
}

static void
SELinuxResource(CallbackListPtr *pcbl, pointer unused, pointer calldata)
{
    XaceResourceAccessRec *rec = calldata;
    SELinuxSubjectRec *subj;
    SELinuxObjectRec *obj;
    SELinuxAuditRec auditdata = { .client = rec->client };
    Mask access_mode = rec->access_mode;
    PrivateRec **privatePtr;
    security_class_t class;
    int rc, offset;

    subj = dixLookupPrivate(&rec->client->devPrivates, subjectKey);

    /* Determine if the resource object has a devPrivates field */
    offset = dixLookupPrivateOffset(rec->rtype);
    if (offset < 0) {
	/* No: use the SID of the owning client */
	class = SECCLASS_X_RESOURCE;
	privatePtr = &clients[CLIENT_ID(rec->id)]->devPrivates;
	obj = dixLookupPrivate(privatePtr, objectKey);
    } else {
	/* Yes: use the SID from the resource object itself */
	class = SELinuxTypeToClass(rec->rtype);
	privatePtr = DEVPRIV_AT(rec->res, offset);
	obj = dixLookupPrivate(privatePtr, objectKey);
    }

    /* If this is a new object that needs labeling, do it now */
    if (access_mode & DixCreateAccess && offset >= 0) {
	rc = SELinuxLabelResource(rec, subj, obj, class);
	if (rc != Success) {
	    rec->status = rc;
	    return;
	}
    }

    /* Collapse generic resource permissions down to read/write */
    if (class == SECCLASS_X_RESOURCE) {
	access_mode = !!(rec->access_mode & SELinuxReadMask); /* rd */
	access_mode |= !!(rec->access_mode & ~SELinuxReadMask) << 1; /* wr */
    }

    /* Perform the security check */
    auditdata.restype = rec->rtype;
    auditdata.id = rec->id;
    rc = SELinuxDoCheck(subj, obj, class, access_mode, &auditdata);
    if (rc != Success)
	rec->status = rc;

    /* Perform the background none check on windows */
    if (access_mode & DixCreateAccess && rec->rtype == RT_WINDOW) {
	rc = SELinuxDoCheck(subj, obj, class, DixBlendAccess, &auditdata);
	if (rc != Success)
	    ((WindowPtr)rec->res)->forcedBG = TRUE;
    }
}

static void
SELinuxScreen(CallbackListPtr *pcbl, pointer is_saver, pointer calldata)
{
    XaceScreenAccessRec *rec = calldata;
    SELinuxSubjectRec *subj;
    SELinuxObjectRec *obj;
    SELinuxAuditRec auditdata = { .client = rec->client };
    Mask access_mode = rec->access_mode;
    int rc;

    subj = dixLookupPrivate(&rec->client->devPrivates, subjectKey);
    obj = dixLookupPrivate(&rec->screen->devPrivates, objectKey);

    /* If this is a new object that needs labeling, do it now */
    if (access_mode & DixCreateAccess) {
	sidput(obj->sid);

	/* Perform a transition to obtain the final SID */
	if (avc_compute_create(subj->sid, subj->sid, SECCLASS_X_SCREEN,
			       &obj->sid) < 0) {
	    ErrorF("SELinux: a compute_create call failed!\n");
	    rec->status = BadValue;
	    return;
	}
    }

    if (is_saver)
	access_mode <<= 2;

    rc = SELinuxDoCheck(subj, obj, SECCLASS_X_SCREEN, access_mode, &auditdata);
    if (rc != Success)
	rec->status = rc;
}

static void
SELinuxClient(CallbackListPtr *pcbl, pointer unused, pointer calldata)
{
    XaceClientAccessRec *rec = calldata;
    SELinuxSubjectRec *subj;
    SELinuxObjectRec *obj;
    SELinuxAuditRec auditdata = { .client = rec->client };
    int rc;

    subj = dixLookupPrivate(&rec->client->devPrivates, subjectKey);
    obj = dixLookupPrivate(&rec->target->devPrivates, objectKey);

    rc = SELinuxDoCheck(subj, obj, SECCLASS_X_CLIENT, rec->access_mode,
			&auditdata);
    if (rc != Success)
	rec->status = rc;
}

static void
SELinuxServer(CallbackListPtr *pcbl, pointer unused, pointer calldata)
{
    XaceServerAccessRec *rec = calldata;
    SELinuxSubjectRec *subj;
    SELinuxObjectRec *obj;
    SELinuxAuditRec auditdata = { .client = rec->client };
    int rc;

    subj = dixLookupPrivate(&rec->client->devPrivates, subjectKey);
    obj = dixLookupPrivate(&serverClient->devPrivates, objectKey);

    rc = SELinuxDoCheck(subj, obj, SECCLASS_X_SERVER, rec->access_mode,
			&auditdata);
    if (rc != Success)
	rec->status = rc;
}


/*
 * DIX Callbacks
 */

static void
SELinuxClientState(CallbackListPtr *pcbl, pointer unused, pointer calldata)
{
    NewClientInfoRec *pci = calldata;

    switch (pci->client->clientState) {
    case ClientStateInitial:
	SELinuxLabelClient(pci->client);
	break;

    default:
	break;
    }
}

static void
SELinuxResourceState(CallbackListPtr *pcbl, pointer unused, pointer calldata)
{
    ResourceStateInfoRec *rec = calldata;
    SELinuxSubjectRec *subj;
    SELinuxObjectRec *obj;
    WindowPtr pWin;

    if (rec->type != RT_WINDOW)
	return;

    pWin = (WindowPtr)rec->value;
    subj = dixLookupPrivate(&wClient(pWin)->devPrivates, subjectKey);

    if (subj->sid) {
	security_context_t ctx;
	int rc = avc_sid_to_context(subj->sid, &ctx);
	if (rc < 0)
	    FatalError("SELinux: Failed to get security context!\n");
	rc = dixChangeWindowProperty(serverClient,
				     pWin, atom_client_ctx, XA_STRING, 8,
				     PropModeReplace, strlen(ctx), ctx, FALSE);
	if (rc != Success)
	    FatalError("SELinux: Failed to set label property on window!\n");
	freecon(ctx);
    } else
	FatalError("SELinux: Unexpected unlabeled client found\n");

    obj = dixLookupPrivate(&pWin->devPrivates, objectKey);

    if (obj->sid) {
	security_context_t ctx;
	int rc = avc_sid_to_context(obj->sid, &ctx);
	if (rc < 0)
	    FatalError("SELinux: Failed to get security context!\n");
	rc = dixChangeWindowProperty(serverClient,
				     pWin, atom_ctx, XA_STRING, 8,
				     PropModeReplace, strlen(ctx), ctx, FALSE);
	if (rc != Success)
	    FatalError("SELinux: Failed to set label property on window!\n");
	freecon(ctx);
    } else
	FatalError("SELinux: Unexpected unlabeled window found\n");
}


/*
 * DevPrivates Callbacks
 */

static void
SELinuxSubjectInit(CallbackListPtr *pcbl, pointer unused, pointer calldata)
{
    PrivateCallbackRec *rec = calldata;
    SELinuxSubjectRec *subj = *rec->value;

    sidget(unlabeled_sid);
    subj->sid = unlabeled_sid;

    avc_entry_ref_init(&subj->aeref);
}

static void
SELinuxSubjectFree(CallbackListPtr *pcbl, pointer unused, pointer calldata)
{
    PrivateCallbackRec *rec = calldata;
    SELinuxSubjectRec *subj = *rec->value;

    xfree(subj->command);

    if (avc_active) {
	sidput(subj->sid);
	sidput(subj->dev_create_sid);
	sidput(subj->win_create_sid);
	sidput(subj->sel_create_sid);
	sidput(subj->prp_create_sid);
    }
}

static void
SELinuxObjectInit(CallbackListPtr *pcbl, pointer unused, pointer calldata)
{
    PrivateCallbackRec *rec = calldata;
    SELinuxObjectRec *obj = *rec->value;

    sidget(unlabeled_sid);
    obj->sid = unlabeled_sid;
}

static void
SELinuxObjectFree(CallbackListPtr *pcbl, pointer unused, pointer calldata)
{
    PrivateCallbackRec *rec = calldata;
    SELinuxObjectRec *obj = *rec->value;

    if (avc_active)
	sidput(obj->sid);
}


/*
 * Extension Dispatch
 */

#define CTX_DEV offsetof(SELinuxSubjectRec, dev_create_sid)
#define CTX_WIN offsetof(SELinuxSubjectRec, win_create_sid)
#define CTX_PRP offsetof(SELinuxSubjectRec, prp_create_sid)
#define CTX_SEL offsetof(SELinuxSubjectRec, sel_create_sid)
#define USE_PRP offsetof(SELinuxSubjectRec, prp_use_sid)
#define USE_SEL offsetof(SELinuxSubjectRec, sel_use_sid)

typedef struct {
    security_context_t octx;
    security_context_t dctx;
    CARD32 octx_len;
    CARD32 dctx_len;
    CARD32 id;
} SELinuxListItemRec;

static int
ProcSELinuxQueryVersion(ClientPtr client)
{
    SELinuxQueryVersionReply rep;

    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.server_major = SELINUX_MAJOR_VERSION;
    rep.server_minor = SELINUX_MINOR_VERSION;
    if (client->swapped) {
	int n;
	swaps(&rep.sequenceNumber, n);
	swapl(&rep.length, n);
	swaps(&rep.server_major, n);
	swaps(&rep.server_minor, n);
    }
    WriteToClient(client, sizeof(rep), (char *)&rep);
    return (client->noClientException);
}

static int
SELinuxSendContextReply(ClientPtr client, security_id_t sid)
{
    SELinuxGetContextReply rep;
    security_context_t ctx = NULL;
    int len = 0;

    if (sid) {
	if (avc_sid_to_context(sid, &ctx) < 0)
	    return BadValue;
	len = strlen(ctx) + 1;
    }

    rep.type = X_Reply;
    rep.length = (len + 3) >> 2;
    rep.sequenceNumber = client->sequence;
    rep.context_len = len;

    if (client->swapped) {
	int n;
	swapl(&rep.length, n);
	swaps(&rep.sequenceNumber, n);
	swapl(&rep.context_len, n);
    }

    WriteToClient(client, sizeof(SELinuxGetContextReply), (char *)&rep);
    WriteToClient(client, len, ctx);
    freecon(ctx);
    return client->noClientException;
}

static int
ProcSELinuxSetCreateContext(ClientPtr client, unsigned offset)
{
    PrivateRec **privPtr = &client->devPrivates;
    security_id_t *pSid;
    security_context_t ctx;
    char *ptr;

    REQUEST(SELinuxSetCreateContextReq);
    REQUEST_FIXED_SIZE(SELinuxSetCreateContextReq, stuff->context_len);

    ctx = (char *)(stuff + 1);
    if (stuff->context_len > 0 && ctx[stuff->context_len - 1])
	return BadLength;

    if (offset == CTX_DEV) {
	/* Device create context currently requires manage permission */
	int rc = XaceHook(XACE_SERVER_ACCESS, client, DixManageAccess);
	if (rc != Success)
	    return rc;
	privPtr = &serverClient->devPrivates;
    }
    else if (offset == USE_SEL) {
	/* Selection use context currently requires no selections owned */
	Selection *pSel;
	for (pSel = CurrentSelections; pSel; pSel = pSel->next)
	    if (pSel->client == client)
		return BadMatch;
    }

    ptr = dixLookupPrivate(privPtr, subjectKey);
    pSid = (security_id_t *)(ptr + offset);
    sidput(*pSid);
    *pSid = NULL;

    if (stuff->context_len > 0) {
	if (security_check_context(ctx) < 0)
	    return BadValue;
	if (avc_context_to_sid(ctx, pSid) < 0)
	    return BadValue;
    }
    return Success;
}

static int
ProcSELinuxGetCreateContext(ClientPtr client, unsigned offset)
{
    security_id_t *pSid;
    char *ptr;

    REQUEST_SIZE_MATCH(SELinuxGetCreateContextReq);

    if (offset == CTX_DEV)
	ptr = dixLookupPrivate(&serverClient->devPrivates, subjectKey);
    else
	ptr = dixLookupPrivate(&client->devPrivates, subjectKey);

    pSid = (security_id_t *)(ptr + offset);
    return SELinuxSendContextReply(client, *pSid);
}

static int
ProcSELinuxSetDeviceContext(ClientPtr client)
{
    security_context_t ctx;
    security_id_t sid;
    DeviceIntPtr dev;
    SELinuxSubjectRec *subj;
    SELinuxObjectRec *obj;
    int rc;

    REQUEST(SELinuxSetContextReq);
    REQUEST_FIXED_SIZE(SELinuxSetContextReq, stuff->context_len);

    ctx = (char *)(stuff + 1);
    if (stuff->context_len < 1 || ctx[stuff->context_len - 1])
	return BadLength;

    rc = dixLookupDevice(&dev, stuff->id, client, DixManageAccess);
    if (rc != Success)
	return rc;

    if (security_check_context(ctx) < 0)
	return BadValue;
    if (avc_context_to_sid(ctx, &sid) < 0)
	return BadValue;

    subj = dixLookupPrivate(&dev->devPrivates, subjectKey);
    sidput(subj->sid);
    subj->sid = sid;
    obj = dixLookupPrivate(&dev->devPrivates, objectKey);
    sidput(obj->sid);
    sidget(obj->sid = sid);

    return Success;
}

static int
ProcSELinuxGetDeviceContext(ClientPtr client)
{
    DeviceIntPtr dev;
    SELinuxSubjectRec *subj;
    int rc;

    REQUEST(SELinuxGetContextReq);
    REQUEST_SIZE_MATCH(SELinuxGetContextReq);

    rc = dixLookupDevice(&dev, stuff->id, client, DixGetAttrAccess);
    if (rc != Success)
	return rc;

    subj = dixLookupPrivate(&dev->devPrivates, subjectKey);
    return SELinuxSendContextReply(client, subj->sid);
}

static int
ProcSELinuxGetWindowContext(ClientPtr client)
{
    WindowPtr pWin;
    SELinuxObjectRec *obj;
    int rc;

    REQUEST(SELinuxGetContextReq);
    REQUEST_SIZE_MATCH(SELinuxGetContextReq);

    rc = dixLookupWindow(&pWin, stuff->id, client, DixGetAttrAccess);
    if (rc != Success)
	return rc;

    obj = dixLookupPrivate(&pWin->devPrivates, objectKey);
    return SELinuxSendContextReply(client, obj->sid);
}

static int
ProcSELinuxGetPropertyContext(ClientPtr client, pointer privKey)
{
    WindowPtr pWin;
    PropertyPtr pProp;
    SELinuxObjectRec *obj;
    int rc;

    REQUEST(SELinuxGetPropertyContextReq);
    REQUEST_SIZE_MATCH(SELinuxGetPropertyContextReq);

    rc = dixLookupWindow(&pWin, stuff->window, client, DixGetPropAccess);
    if (rc != Success)
	return rc;

    rc = dixLookupProperty(&pProp, pWin, stuff->property, client,
			   DixGetAttrAccess);
    if (rc != Success)
	return rc;

    obj = dixLookupPrivate(&pProp->devPrivates, privKey);
    return SELinuxSendContextReply(client, obj->sid);
}

static int
ProcSELinuxGetSelectionContext(ClientPtr client, pointer privKey)
{
    Selection *pSel;
    SELinuxObjectRec *obj;
    int rc;

    REQUEST(SELinuxGetContextReq);
    REQUEST_SIZE_MATCH(SELinuxGetContextReq);

    rc = dixLookupSelection(&pSel, stuff->id, client, DixGetAttrAccess);
    if (rc != Success)
	return rc;

    obj = dixLookupPrivate(&pSel->devPrivates, privKey);
    return SELinuxSendContextReply(client, obj->sid);
}

static int
ProcSELinuxGetClientContext(ClientPtr client)
{
    ClientPtr target;
    SELinuxSubjectRec *subj;
    int rc;

    REQUEST(SELinuxGetContextReq);
    REQUEST_SIZE_MATCH(SELinuxGetContextReq);

    rc = dixLookupClient(&target, stuff->id, client, DixGetAttrAccess);
    if (rc != Success)
	return rc;

    subj = dixLookupPrivate(&target->devPrivates, subjectKey);
    return SELinuxSendContextReply(client, subj->sid);
}

static int
SELinuxPopulateItem(SELinuxListItemRec *i, PrivateRec **privPtr, CARD32 id,
		    int *size)
{
    SELinuxObjectRec *obj = dixLookupPrivate(privPtr, objectKey);
    SELinuxObjectRec *data = dixLookupPrivate(privPtr, dataKey);

    if (avc_sid_to_context(obj->sid, &i->octx) < 0)
	return BadValue;
    if (avc_sid_to_context(data->sid, &i->dctx) < 0)
	return BadValue;

    i->id = id;
    i->octx_len = (strlen(i->octx) + 4) >> 2;
    i->dctx_len = (strlen(i->dctx) + 4) >> 2;

    *size += i->octx_len + i->dctx_len + 3;
    return Success;
}

static void
SELinuxFreeItems(SELinuxListItemRec *items, int count)
{
    int k;
    for (k = 0; k < count; k++) {
	freecon(items[k].octx);
	freecon(items[k].dctx);
    }
    xfree(items);
}

static int
SELinuxSendItemsToClient(ClientPtr client, SELinuxListItemRec *items,
			 int size, int count)
{
    int rc, k, n, pos = 0;
    SELinuxListItemsReply rep;
    CARD32 *buf;

    buf = xcalloc(size, sizeof(CARD32));
    if (!buf) {
	rc = BadAlloc;
	goto out;
    }

    /* Fill in the buffer */
    for (k = 0; k < count; k++) {
	buf[pos] = items[k].id;
	if (client->swapped)
	    swapl(buf + pos, n);
	pos++;

	buf[pos] = items[k].octx_len * 4;
	if (client->swapped)
	    swapl(buf + pos, n);
	pos++;

	buf[pos] = items[k].dctx_len * 4;
	if (client->swapped)
	    swapl(buf + pos, n);
	pos++;

	memcpy((char *)(buf + pos), items[k].octx, strlen(items[k].octx) + 1);
	pos += items[k].octx_len;
	memcpy((char *)(buf + pos), items[k].dctx, strlen(items[k].dctx) + 1);
	pos += items[k].dctx_len;
    }

    /* Send reply to client */
    rep.type = X_Reply;
    rep.length = size;
    rep.sequenceNumber = client->sequence;
    rep.count = count;

    if (client->swapped) {
	swapl(&rep.length, n);
	swaps(&rep.sequenceNumber, n);
	swapl(&rep.count, n);
    }

    WriteToClient(client, sizeof(SELinuxListItemsReply), (char *)&rep);
    WriteToClient(client, size * 4, (char *)buf);

    /* Free stuff and return */
    rc = client->noClientException;
    xfree(buf);
out:
    SELinuxFreeItems(items, count);
    return rc;
}

static int
ProcSELinuxListProperties(ClientPtr client)
{
    WindowPtr pWin;
    PropertyPtr pProp;
    SELinuxListItemRec *items;
    int rc, count, size, i;
    CARD32 id;

    REQUEST(SELinuxGetContextReq);
    REQUEST_SIZE_MATCH(SELinuxGetContextReq);

    rc = dixLookupWindow(&pWin, stuff->id, client, DixListPropAccess);
    if (rc != Success)
	return rc;

    /* Count the number of properties and allocate items */
    count = 0;
    for (pProp = wUserProps(pWin); pProp; pProp = pProp->next)
	count++;
    items = xcalloc(count, sizeof(SELinuxListItemRec));
    if (!items)
	return BadAlloc;

    /* Fill in the items and calculate size */
    i = 0;
    size = 0;
    for (pProp = wUserProps(pWin); pProp; pProp = pProp->next) {
	id = pProp->propertyName;
	rc = SELinuxPopulateItem(items + i, &pProp->devPrivates, id, &size);
	if (rc != Success) {
	    SELinuxFreeItems(items, count);
	    return rc;
	}
	i++;
    }

    return SELinuxSendItemsToClient(client, items, size, count);
}

static int
ProcSELinuxListSelections(ClientPtr client)
{
    Selection *pSel;
    SELinuxListItemRec *items;
    int rc, count, size, i;
    CARD32 id;

    REQUEST_SIZE_MATCH(SELinuxGetCreateContextReq);

    /* Count the number of selections and allocate items */
    count = 0;
    for (pSel = CurrentSelections; pSel; pSel = pSel->next)
	count++;
    items = xcalloc(count, sizeof(SELinuxListItemRec));
    if (!items)
	return BadAlloc;

    /* Fill in the items and calculate size */
    i = 0;
    size = 0;
    for (pSel = CurrentSelections; pSel; pSel = pSel->next) {
	id = pSel->selection;
	rc = SELinuxPopulateItem(items + i, &pSel->devPrivates, id, &size);
	if (rc != Success) {
	    SELinuxFreeItems(items, count);
	    return rc;
	}
	i++;
    }

    return SELinuxSendItemsToClient(client, items, size, count);
}

static int
ProcSELinuxDispatch(ClientPtr client)
{
    REQUEST(xReq);
    switch (stuff->data) {
    case X_SELinuxQueryVersion:
	return ProcSELinuxQueryVersion(client);
    case X_SELinuxSetDeviceCreateContext:
	return ProcSELinuxSetCreateContext(client, CTX_DEV);
    case X_SELinuxGetDeviceCreateContext:
	return ProcSELinuxGetCreateContext(client, CTX_DEV);
    case X_SELinuxSetDeviceContext:
	return ProcSELinuxSetDeviceContext(client);
    case X_SELinuxGetDeviceContext:
	return ProcSELinuxGetDeviceContext(client);
    case X_SELinuxSetWindowCreateContext:
	return ProcSELinuxSetCreateContext(client, CTX_WIN);
    case X_SELinuxGetWindowCreateContext:
	return ProcSELinuxGetCreateContext(client, CTX_WIN);
    case X_SELinuxGetWindowContext:
	return ProcSELinuxGetWindowContext(client);
    case X_SELinuxSetPropertyCreateContext:
	return ProcSELinuxSetCreateContext(client, CTX_PRP);
    case X_SELinuxGetPropertyCreateContext:
	return ProcSELinuxGetCreateContext(client, CTX_PRP);
    case X_SELinuxSetPropertyUseContext:
	return ProcSELinuxSetCreateContext(client, USE_PRP);
    case X_SELinuxGetPropertyUseContext:
	return ProcSELinuxGetCreateContext(client, USE_PRP);
    case X_SELinuxGetPropertyContext:
	return ProcSELinuxGetPropertyContext(client, objectKey);
    case X_SELinuxGetPropertyDataContext:
	return ProcSELinuxGetPropertyContext(client, dataKey);
    case X_SELinuxListProperties:
	return ProcSELinuxListProperties(client);
    case X_SELinuxSetSelectionCreateContext:
	return ProcSELinuxSetCreateContext(client, CTX_SEL);
    case X_SELinuxGetSelectionCreateContext:
	return ProcSELinuxGetCreateContext(client, CTX_SEL);
    case X_SELinuxSetSelectionUseContext:
	return ProcSELinuxSetCreateContext(client, USE_SEL);
    case X_SELinuxGetSelectionUseContext:
	return ProcSELinuxGetCreateContext(client, USE_SEL);
    case X_SELinuxGetSelectionContext:
	return ProcSELinuxGetSelectionContext(client, objectKey);
    case X_SELinuxGetSelectionDataContext:
	return ProcSELinuxGetSelectionContext(client, dataKey);
    case X_SELinuxListSelections:
	return ProcSELinuxListSelections(client);
    case X_SELinuxGetClientContext:
	return ProcSELinuxGetClientContext(client);
    default:
	return BadRequest;
    }
}

static int
SProcSELinuxQueryVersion(ClientPtr client)
{
    REQUEST(SELinuxQueryVersionReq);
    int n;

    REQUEST_SIZE_MATCH(SELinuxQueryVersionReq);
    swaps(&stuff->client_major, n);
    swaps(&stuff->client_minor, n);
    return ProcSELinuxQueryVersion(client);
}

static int
SProcSELinuxSetCreateContext(ClientPtr client, unsigned offset)
{
    REQUEST(SELinuxSetCreateContextReq);
    int n;

    REQUEST_AT_LEAST_SIZE(SELinuxSetCreateContextReq);
    swapl(&stuff->context_len, n);
    return ProcSELinuxSetCreateContext(client, offset);
}

static int
SProcSELinuxSetDeviceContext(ClientPtr client)
{
    REQUEST(SELinuxSetContextReq);
    int n;

    REQUEST_AT_LEAST_SIZE(SELinuxSetContextReq);
    swapl(&stuff->id, n);
    swapl(&stuff->context_len, n);
    return ProcSELinuxSetDeviceContext(client);
}

static int
SProcSELinuxGetDeviceContext(ClientPtr client)
{
    REQUEST(SELinuxGetContextReq);
    int n;

    REQUEST_SIZE_MATCH(SELinuxGetContextReq);
    swapl(&stuff->id, n);
    return ProcSELinuxGetDeviceContext(client);
}

static int
SProcSELinuxGetWindowContext(ClientPtr client)
{
    REQUEST(SELinuxGetContextReq);
    int n;

    REQUEST_SIZE_MATCH(SELinuxGetContextReq);
    swapl(&stuff->id, n);
    return ProcSELinuxGetWindowContext(client);
}

static int
SProcSELinuxGetPropertyContext(ClientPtr client, pointer privKey)
{
    REQUEST(SELinuxGetPropertyContextReq);
    int n;

    REQUEST_SIZE_MATCH(SELinuxGetPropertyContextReq);
    swapl(&stuff->window, n);
    swapl(&stuff->property, n);
    return ProcSELinuxGetPropertyContext(client, privKey);
}

static int
SProcSELinuxGetSelectionContext(ClientPtr client, pointer privKey)
{
    REQUEST(SELinuxGetContextReq);
    int n;

    REQUEST_SIZE_MATCH(SELinuxGetContextReq);
    swapl(&stuff->id, n);
    return ProcSELinuxGetSelectionContext(client, privKey);
}

static int
SProcSELinuxListProperties(ClientPtr client)
{
    REQUEST(SELinuxGetContextReq);
    int n;

    REQUEST_SIZE_MATCH(SELinuxGetContextReq);
    swapl(&stuff->id, n);
    return ProcSELinuxListProperties(client);
}

static int
SProcSELinuxGetClientContext(ClientPtr client)
{
    REQUEST(SELinuxGetContextReq);
    int n;

    REQUEST_SIZE_MATCH(SELinuxGetContextReq);
    swapl(&stuff->id, n);
    return ProcSELinuxGetClientContext(client);
}

static int
SProcSELinuxDispatch(ClientPtr client)
{
    REQUEST(xReq);
    int n;

    swaps(&stuff->length, n);

    switch (stuff->data) {
    case X_SELinuxQueryVersion:
	return SProcSELinuxQueryVersion(client);
    case X_SELinuxSetDeviceCreateContext:
	return SProcSELinuxSetCreateContext(client, CTX_DEV);
    case X_SELinuxGetDeviceCreateContext:
	return ProcSELinuxGetCreateContext(client, CTX_DEV);
    case X_SELinuxSetDeviceContext:
	return SProcSELinuxSetDeviceContext(client);
    case X_SELinuxGetDeviceContext:
	return SProcSELinuxGetDeviceContext(client);
    case X_SELinuxSetWindowCreateContext:
	return SProcSELinuxSetCreateContext(client, CTX_WIN);
    case X_SELinuxGetWindowCreateContext:
	return ProcSELinuxGetCreateContext(client, CTX_WIN);
    case X_SELinuxGetWindowContext:
	return SProcSELinuxGetWindowContext(client);
    case X_SELinuxSetPropertyCreateContext:
	return SProcSELinuxSetCreateContext(client, CTX_PRP);
    case X_SELinuxGetPropertyCreateContext:
	return ProcSELinuxGetCreateContext(client, CTX_PRP);
    case X_SELinuxSetPropertyUseContext:
	return SProcSELinuxSetCreateContext(client, USE_PRP);
    case X_SELinuxGetPropertyUseContext:
	return ProcSELinuxGetCreateContext(client, USE_PRP);
    case X_SELinuxGetPropertyContext:
	return SProcSELinuxGetPropertyContext(client, objectKey);
    case X_SELinuxGetPropertyDataContext:
	return SProcSELinuxGetPropertyContext(client, dataKey);
    case X_SELinuxListProperties:
	return SProcSELinuxListProperties(client);
    case X_SELinuxSetSelectionCreateContext:
	return SProcSELinuxSetCreateContext(client, CTX_SEL);
    case X_SELinuxGetSelectionCreateContext:
	return ProcSELinuxGetCreateContext(client, CTX_SEL);
    case X_SELinuxSetSelectionUseContext:
	return SProcSELinuxSetCreateContext(client, USE_SEL);
    case X_SELinuxGetSelectionUseContext:
	return ProcSELinuxGetCreateContext(client, USE_SEL);
    case X_SELinuxGetSelectionContext:
	return SProcSELinuxGetSelectionContext(client, objectKey);
    case X_SELinuxGetSelectionDataContext:
	return SProcSELinuxGetSelectionContext(client, dataKey);
    case X_SELinuxListSelections:
	return ProcSELinuxListSelections(client);
    case X_SELinuxGetClientContext:
	return SProcSELinuxGetClientContext(client);
    default:
	return BadRequest;
    }
}


/*
 * Extension Setup / Teardown
 */

static void
SELinuxResetProc(ExtensionEntry *extEntry)
{
    /* Unregister callbacks */
    DeleteCallback(&ClientStateCallback, SELinuxClientState, NULL);
    DeleteCallback(&ResourceStateCallback, SELinuxResourceState, NULL);

    XaceDeleteCallback(XACE_EXT_DISPATCH, SELinuxExtension, NULL);
    XaceDeleteCallback(XACE_RESOURCE_ACCESS, SELinuxResource, NULL);
    XaceDeleteCallback(XACE_DEVICE_ACCESS, SELinuxDevice, NULL);
    XaceDeleteCallback(XACE_PROPERTY_ACCESS, SELinuxProperty, NULL);
    XaceDeleteCallback(XACE_SEND_ACCESS, SELinuxSend, NULL);
    XaceDeleteCallback(XACE_RECEIVE_ACCESS, SELinuxReceive, NULL);
    XaceDeleteCallback(XACE_CLIENT_ACCESS, SELinuxClient, NULL);
    XaceDeleteCallback(XACE_EXT_ACCESS, SELinuxExtension, NULL);
    XaceDeleteCallback(XACE_SERVER_ACCESS, SELinuxServer, NULL);
    XaceDeleteCallback(XACE_SELECTION_ACCESS, SELinuxSelection, NULL);
    XaceDeleteCallback(XACE_SCREEN_ACCESS, SELinuxScreen, NULL);
    XaceDeleteCallback(XACE_SCREENSAVER_ACCESS, SELinuxScreen, truep);

    /* Tear down SELinux stuff */
    selabel_close(label_hnd);
    label_hnd = NULL;

    audit_close(audit_fd);

    avc_destroy();
    avc_active = 0;

    /* Free local state */
    xfree(knownAtoms);
    knownAtoms = NULL;
    numKnownAtoms = 0;

    xfree(knownEvents);
    knownEvents = NULL;
    numKnownEvents = 0;

    xfree(knownTypes);
    knownTypes = NULL;
    numKnownTypes = 0;
}

void
SELinuxExtensionInit(INITARGS)
{
    ExtensionEntry *extEntry;
    struct selinux_opt selabel_option = { SELABEL_OPT_VALIDATE, (char *)1 };
    struct selinux_opt avc_option = { AVC_OPT_SETENFORCE, (char *)0 };
    security_context_t con;
    int ret = TRUE;

    /* Check SELinux mode on system */
    if (!is_selinux_enabled()) {
	ErrorF("SELinux: Disabled on system, not enabling in X server\n");
	return;
    }

    /* Check SELinux mode in configuration file */
    switch(selinuxEnforcingState) {
    case SELINUX_MODE_DISABLED:
	LogMessage(X_INFO, "SELinux: Disabled in configuration file\n");
	return;
    case SELINUX_MODE_ENFORCING:
	LogMessage(X_INFO, "SELinux: Configured in enforcing mode\n");
	avc_option.value = (char *)1;
	break;
    case SELINUX_MODE_PERMISSIVE:
	LogMessage(X_INFO, "SELinux: Configured in permissive mode\n");
	avc_option.value = (char *)0;
	break;
    default:
	avc_option.type = AVC_OPT_UNUSED;
	break;
    }

    /* Set up SELinux stuff */
    selinux_set_callback(SELINUX_CB_LOG, (union selinux_callback)SELinuxLog);
    selinux_set_callback(SELINUX_CB_AUDIT, (union selinux_callback)SELinuxAudit);

    if (selinux_set_mapping(map) < 0) {
	if (errno == EINVAL) {
	    ErrorF("SELinux: Invalid object class mapping, disabling SELinux support.\n");
	    return;
	}
	FatalError("SELinux: Failed to set up security class mapping\n");
    }

    if (avc_open(&avc_option, 1) < 0)
	FatalError("SELinux: Couldn't initialize SELinux userspace AVC\n");
    avc_active = 1;

    label_hnd = selabel_open(SELABEL_CTX_X, &selabel_option, 1);
    if (!label_hnd)
	FatalError("SELinux: Failed to open x_contexts mapping in policy\n");

    if (security_get_initial_context("unlabeled", &con) < 0)
	FatalError("SELinux: Failed to look up unlabeled context\n");
    if (avc_context_to_sid(con, &unlabeled_sid) < 0)
	FatalError("SELinux: a context_to_SID call failed!\n");
    freecon(con);

    /* Prepare for auditing */
    audit_fd = audit_open();
    if (audit_fd < 0)
	FatalError("SELinux: Failed to open the system audit log\n");

    /* Allocate private storage */
    if (!dixRequestPrivate(subjectKey, sizeof(SELinuxSubjectRec)) ||
	!dixRequestPrivate(objectKey, sizeof(SELinuxObjectRec)) ||
	!dixRequestPrivate(dataKey, sizeof(SELinuxObjectRec)))
	FatalError("SELinux: Failed to allocate private storage.\n");

    /* Create atoms for doing window labeling */
    atom_ctx = MakeAtom("_SELINUX_CONTEXT", 16, TRUE);
    if (atom_ctx == BAD_RESOURCE)
	FatalError("SELinux: Failed to create atom\n");
    atom_client_ctx = MakeAtom("_SELINUX_CLIENT_CONTEXT", 23, TRUE);
    if (atom_client_ctx == BAD_RESOURCE)
	FatalError("SELinux: Failed to create atom\n");

    /* Register callbacks */
    ret &= dixRegisterPrivateInitFunc(subjectKey, SELinuxSubjectInit, NULL);
    ret &= dixRegisterPrivateDeleteFunc(subjectKey, SELinuxSubjectFree, NULL);
    ret &= dixRegisterPrivateInitFunc(objectKey, SELinuxObjectInit, NULL);
    ret &= dixRegisterPrivateDeleteFunc(objectKey, SELinuxObjectFree, NULL);
    ret &= dixRegisterPrivateInitFunc(dataKey, SELinuxObjectInit, NULL);
    ret &= dixRegisterPrivateDeleteFunc(dataKey, SELinuxObjectFree, NULL);

    ret &= AddCallback(&ClientStateCallback, SELinuxClientState, NULL);
    ret &= AddCallback(&ResourceStateCallback, SELinuxResourceState, NULL);

    ret &= XaceRegisterCallback(XACE_EXT_DISPATCH, SELinuxExtension, NULL);
    ret &= XaceRegisterCallback(XACE_RESOURCE_ACCESS, SELinuxResource, NULL);
    ret &= XaceRegisterCallback(XACE_DEVICE_ACCESS, SELinuxDevice, NULL);
    ret &= XaceRegisterCallback(XACE_PROPERTY_ACCESS, SELinuxProperty, NULL);
    ret &= XaceRegisterCallback(XACE_SEND_ACCESS, SELinuxSend, NULL);
    ret &= XaceRegisterCallback(XACE_RECEIVE_ACCESS, SELinuxReceive, NULL);
    ret &= XaceRegisterCallback(XACE_CLIENT_ACCESS, SELinuxClient, NULL);
    ret &= XaceRegisterCallback(XACE_EXT_ACCESS, SELinuxExtension, NULL);
    ret &= XaceRegisterCallback(XACE_SERVER_ACCESS, SELinuxServer, NULL);
    ret &= XaceRegisterCallback(XACE_SELECTION_ACCESS, SELinuxSelection, NULL);
    ret &= XaceRegisterCallback(XACE_SCREEN_ACCESS, SELinuxScreen, NULL);
    ret &= XaceRegisterCallback(XACE_SCREENSAVER_ACCESS, SELinuxScreen, truep);
    if (!ret)
	FatalError("SELinux: Failed to register one or more callbacks\n");

    /* Add extension to server */
    extEntry = AddExtension(SELINUX_EXTENSION_NAME,
			    SELinuxNumberEvents, SELinuxNumberErrors,
			    ProcSELinuxDispatch, SProcSELinuxDispatch,
			    SELinuxResetProc, StandardMinorOpcode);

    AddExtensionAlias("Flask", extEntry);

    /* Label objects that were created before we could register ourself */
    SELinuxLabelInitial();
}
