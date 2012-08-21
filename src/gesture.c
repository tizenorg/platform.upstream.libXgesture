/*
 *
 * libxgesture
 *
 * Contact: Sung-Jin Park <sj76.park@samsung.com>
 *          Sangjin LEE <lsj119@samsung.com>
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved 
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of Red Hat
 * not be used in advertising or publicity pertaining to distribution
 * of the software without specific, written prior permission.  Red
 * Hat makes no representations about the suitability of this software
 * for any purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 * THE AUTHORS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#define NEED_EVENTS
#define NEED_REPLIES
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <X11/Xlibint.h>
#include <X11/extensions/Xext.h>
#include <X11/extensions/extutil.h>
#include <X11/extensions/gesture.h>
#include <X11/extensions/gestureproto.h>

#ifdef __XGESTURE_LIB_DEBUG__
#include <stdio.h>
#define TRACE(msg)  fprintf(stderr, "[X11][GestureExt] %s\n", msg);
#else
#define TRACE(msg)
#endif

static XExtensionInfo _gesture_info_data;
static XExtensionInfo *gesture_info = &_gesture_info_data;
static char *gesture_extension_name = GESTURE_EXT_NAME;

#define GestureCheckExtension(dpy,i,val) \
  XextCheckExtension (dpy, i, gesture_extension_name, val)

//hooking functions
static char    *error_string(Display *dpy, int code, XExtCodes *codes,
			     char *buf, int n);
static int close_display(Display *dpy, XExtCodes *extCodes);
static Bool wire_to_event(Display *dpy, XEvent *event, xEvent *wire);
static Status event_to_wire(Display *dpy, XEvent *event, xEvent *wire);

static /* const */ XExtensionHooks gesture_extension_hooks = {
    NULL,				/* create_gc */
    NULL,				/* copy_gc */
    NULL,				/* flush_gc */
    NULL,				/* free_gc */
    NULL,				/* create_font */
    NULL,				/* free_font */
    close_display,			/* close_display */
    wire_to_event,			/* wire_to_event */
    event_to_wire,			/* event_to_wire */
    NULL,				/* error */
    error_string,		/* error_string */
};

static char    *gesture_error_list[] = {
    "ClientNotLocal",
    "InValidMask",
    "OperationNotSupported",
};

typedef struct _GestureVersionInfoRec {
    short major;
    short minor;
    int num_errors;
} GestureVersionInfo;

#define GET_VERSION(info) ((info) ? (const GestureVersionInfo*)(info)->data : NULL)
#define IS_VERSION_SUPPORTED(info) (!!GET_VERSION(info))

static XEXT_GENERATE_FIND_DISPLAY (find_display, gesture_info,
                                   gesture_extension_name,
                                   &gesture_extension_hooks,
                                   GestureNumberEvents, NULL)

static XEXT_GENERATE_CLOSE_DISPLAY (close_display, gesture_info)

static
XExtDisplayInfo *find_display_create_optional(Display *dpy, Bool create)
{
    XExtDisplayInfo *dpyinfo;

    if (!gesture_info) {
        if (!(gesture_info = XextCreateExtension())) return NULL;
    }

    if (!(dpyinfo = XextFindDisplay (gesture_info, dpy)) && create) {
        dpyinfo = XextAddDisplay(gesture_info, dpy,
                                 GESTURE_EXT_NAME,
                                 &gesture_extension_hooks,
                                 GestureNumberErrors,
                                 (XPointer)GetVersionInfo(dpy));
    }

    return dpyinfo;
}

static
char *error_string(Display *dpy, int code, XExtCodes *codes, char *buf, int n)
{
    XExtDisplayInfo *info = find_display_create_optional(dpy, False);
    int nerr = IS_VERSION_SUPPORTED(info) ? GET_VERSION(info)->num_errors : 0;

    code -= codes->first_error;
    if (code >= 0 && code < nerr) {
	char tmp[256];
	sprintf (tmp, "%s.%d", GESTURE_EXT_NAME, code);
	XGetErrorDatabaseText (dpy, "XProtoError", tmp, gesture_error_list[code], buf, n);
	return buf;
    }

    return (char *)0;
}

static Bool
wire_to_event (Display *dpy, XEvent *event, xEvent *wire)
{
    XExtDisplayInfo *info = find_display (dpy);

    XGestureNotifyGroupEvent *gev;
    xGestureNotifyGroupEvent *wgev;
    XGestureNotifyFlickEvent *fev;
    xGestureNotifyFlickEvent *wfev;
    XGestureNotifyPanEvent *pev;
    xGestureNotifyPanEvent *wpev;
    XGestureNotifyPinchRotationEvent *pcrev;
    xGestureNotifyPinchRotationEvent *wpcrev;
    XGestureNotifyTapEvent *tev;
    xGestureNotifyTapEvent *wtev;
    XGestureNotifyTapNHoldEvent *thev;
    xGestureNotifyTapNHoldEvent *wthev;
    XGestureNotifyHoldEvent *hev;
    xGestureNotifyHoldEvent *whev;

    GestureCheckExtension (dpy, info, False);

#ifdef __XGESTURE_LIB_DEBUG__
    xGestureCommonEvent *wxce = (xGestureCommonEvent *)wire;
    fprintf(stderr, "[wire_to_event] wire->u.u.type=%d, info->codes->first_event=%d\n", 
		wire->u.u.type, info->codes->first_event);
    fprintf(stderr, "[wire_to_event] wxce->any.kind=%d\n", wxce->any.kind);
#endif//__XGESTURE_LIB_DEBUG__

    switch (wire->u.u.type - info->codes->first_event) {
	    case GestureNotifyGroup:
			gev = (XGestureNotifyGroupEvent *)event;
			wgev = (xGestureNotifyGroupEvent *)wire;
			gev->type = wgev->type & 0x7f;
			gev->serial = _XSetLastRequestRead(dpy, (xGenericReply *) wire);
			gev->send_event = (wgev->type & 0x80) != 0;
			gev->display = dpy;
			gev->time = wgev->time;
			gev->kind = wgev->kind;
			gev->groupid = wgev->groupid;
			gev->num_group = wgev->num_group;
			gev->window = wgev->window;
#ifdef __XGESTURE_LIB_DEBUG__
			fprintf(stderr, "[wire_to_event]\n");
			fprintf(stderr, "[ == Wire values == ]\n");
			fprintf(stderr, "groupid=%d\n", wgev->groupid);
			fprintf(stderr, "tap num_group=%d\n", wgev->num_group);
			fprintf(stderr, "window=0x%x\n", wgev->window);
			fprintf(stderr, "time=%d\n", wgev->time);
			fprintf(stderr, "[ == Event values]\n");
			fprintf(stderr, "groupid=%d\n", gev->groupid);
			fprintf(stderr, "tap num_group=%d\n", gev->num_group);
			fprintf(stderr, "window=0x%x\n", gev->window);
			fprintf(stderr, "time=%d\n", gev->time);
#endif//__XGESTURE_LIB_DEBUG__
			return True;

	    case GestureNotifyFlick:
			fev = (XGestureNotifyFlickEvent *)event;
			wfev = (xGestureNotifyFlickEvent *)wire;
			fev->type = wfev->type & 0x7f;
			fev->serial = _XSetLastRequestRead(dpy, (xGenericReply *) wire);
			fev->send_event = (wfev->type & 0x80) != 0;
			fev->display = dpy;
			fev->time = wfev->time;
			fev->kind = wfev->kind;
			fev->num_finger = wfev->num_finger;
			fev->distance = wfev->distance;
			fev->duration = wfev->duration;
			fev->angle = wfev->angle;
			fev->direction = wfev->direction;
			fev->window = wfev->window;
#ifdef __XGESTURE_LIB_DEBUG__
			fprintf(stderr, "[wire_to_event]\n");
			fprintf(stderr, "[ == Wire values == ]\n");
			fprintf(stderr, "num of finger=%d\n", wfev->num_finger);
			fprintf(stderr, "distance=%d\n", wfev->distance);
			fprintf(stderr, "duration=%d\n", wfev->duration);
			fprintf(stderr, "angle=%d\n", wfev->angle);
			fprintf(stderr, "direction=%d\n", wfev->direction);
			fprintf(stderr, "window=0x%x\n", wfev->window);
			fprintf(stderr, "time=%d\n", wfev->time);
			fprintf(stderr, "[ == Event values]\n");
			fprintf(stderr, "num of finger=%d\n", fev->num_finger);
			fprintf(stderr, "distance=%d\n", fev->distance);
			fprintf(stderr, "duration=%d\n", fev->duration);
			fprintf(stderr, "angle=%d\n", fev->angle);
			fprintf(stderr, "direction=%d\n", fev->direction);
			fprintf(stderr, "window=0x%x\n", fev->window);
			fprintf(stderr, "time=%d\n", fev->time);
#endif//__XGESTURE_LIB_DEBUG__
			return True;

	    case GestureNotifyPan:
			pev = (XGestureNotifyPanEvent *)event;
			wpev = (xGestureNotifyPanEvent *)wire;
			pev->type = wpev->type & 0x7f;
			pev->serial = _XSetLastRequestRead(dpy, (xGenericReply *) wire);
			pev->send_event = (wpev->type & 0x80) != 0;
			pev->display = dpy;
			pev->time = wpev->time;
			pev->kind = wpev->kind;
			pev->num_finger = wpev->num_finger;
			pev->direction = wpev->direction;
			pev->dx = (short int)wpev->dx;
			pev->dy = (short int)wpev->dy;
			pev->distance = wpev->distance;
			pev->duration = wpev->duration;
			pev->window = wpev->window;
#ifdef __XGESTURE_LIB_DEBUG__
			fprintf(stderr, "[wire_to_event]\n");
			fprintf(stderr, "[ == Wire values == ]\n");
			fprintf(stderr, "num of finger=%d\n", wpev->num_finger);
			fprintf(stderr, "direction=%d\n", wpev->direction);
			fprintf(stderr, "distance=%d\n", wpev->distance);
			fprintf(stderr, "duration=%d\n", wpev->duration);
			fprintf(stderr, "window=0x%x\n", wpev->window);
			fprintf(stderr, "dx:%d, dy:%d\n", wpev->dx, wpev->dy);
			fprintf(stderr, "time=%d\n", wpev->time);
			fprintf(stderr, "[ == Event values]\n");
			fprintf(stderr, "num of finger=%d\n", pev->num_finger);
			fprintf(stderr, "direction=%d\n", pev->direction);
			fprintf(stderr, "distance=%d\n", pev->distance);
			fprintf(stderr, "duration=%d\n", pev->duration);
			fprintf(stderr, "window=0x%x\n", pev->window);
			fprintf(stderr, "dx:%d, dy:%d\n", pev->dx, pev->dy);
			fprintf(stderr, "time=%d\n", pev->time);
#endif//__XGESTURE_LIB_DEBUG__
			return True;

	    case GestureNotifyPinchRotation:
			pcrev = (XGestureNotifyPinchRotationEvent *)event;
			wpcrev = (xGestureNotifyPinchRotationEvent *)wire;
			pcrev->type = wpcrev->type & 0x7f;
			pcrev->serial = _XSetLastRequestRead(dpy, (xGenericReply *) wire);
			pcrev->send_event = (wpcrev->type & 0x80) != 0;
			pcrev->display = dpy;
			pcrev->time = wpcrev->time;
			pcrev->kind = wpcrev->kind;
			pcrev->num_finger = wpcrev->num_finger;
			pcrev->zoom = wpcrev->zoom;
			pcrev->angle = wpcrev->angle;
			pcrev->cx = wpcrev->cx;
			pcrev->cy = wpcrev->cy;
			pcrev->distance = wpcrev->distance;
			pcrev->window = wpcrev->window;
#ifdef __XGESTURE_LIB_DEBUG__
			fprintf(stderr, "[wire_to_event]\n");
			fprintf(stderr, "[ == Wire values == ]\n");
			fprintf(stderr, "num of finger=%d\n", wpcrev->num_finger);
			fprintf(stderr, "zoom=%d\n", wpcrev->zoom);
			fprintf(stderr, "angle=%d\n", wpcrev->angle);
			fprintf(stderr, "distance=%d\n", wpcrev->distance);
			fprintf(stderr, "window=0x%x\n", wpcrev->window);
			fprintf(stderr, "cx:%d, cy:%d\n", wpcrev->cx, wpcrev->cy);
			fprintf(stderr, "time=%d\n", wpcrev->time);
			fprintf(stderr, "[ == Event values]\n");
			fprintf(stderr, "num of finger=%d\n", pcrev->num_finger);
			fprintf(stderr, "zoom=%d\n", pcrev->zoom);
			fprintf(stderr, "angle=%d\n", pcrev->angle);
			fprintf(stderr, "distance=%d\n", pcrev->distance);
			fprintf(stderr, "window=0x%x\n", pcrev->window);
			fprintf(stderr, "cx:%d, cy:%d\n", pcrev->cx, pcrev->cy);
			fprintf(stderr, "time=%d\n", pcrev->time);
#endif//__XGESTURE_LIB_DEBUG__
			return True;

	    case GestureNotifyTap:
			tev = (XGestureNotifyTapEvent *)event;
			wtev = (xGestureNotifyTapEvent *)wire;
			tev->type = wtev->type & 0x7f;
			tev->serial = _XSetLastRequestRead(dpy, (xGenericReply *) wire);
			tev->send_event = (wtev->type & 0x80) != 0;
			tev->display = dpy;
			tev->time = wtev->time;
			tev->kind = wtev->kind;
			tev->num_finger = wtev->num_finger;
			tev->cx = wtev->cx;		
			tev->cy = wtev->cy;
			tev->tap_repeat = wtev->tap_repeat;
			tev->interval = wtev->interval;
			tev->window = wtev->window;
#ifdef __XGESTURE_LIB_DEBUG__
			fprintf(stderr, "[wire_to_event]\n");
			fprintf(stderr, "[ == Wire values == ]\n");
			fprintf(stderr, "num of finger=%d\n", wtev->num_finger);
			fprintf(stderr, "tap repeat=%d\n", wtev->tap_repeat);
			fprintf(stderr, "window=0x%x\n", wtev->window);
			fprintf(stderr, "cx:%d, cy:%d\n", wtev->cx, wtev->cy);
			fprintf(stderr, "time=%d, interval=%d\n", wtev->time, wtev->interval);
			fprintf(stderr, "[ == Event values]\n");
			fprintf(stderr, "num of finger=%d\n", tev->num_finger);
			fprintf(stderr, "tap repeat=%d\n", tev->tap_repeat);
			fprintf(stderr, "window=0x%x\n", tev->window);
			fprintf(stderr, "cx:%d, cy:%d\n", tev->cx, tev->cy);
			fprintf(stderr, "time=%d, interval=%d\n", tev->time, tev->interval);
#endif//__XGESTURE_LIB_DEBUG__
			return True;

	    case GestureNotifyTapNHold:
			thev = (XGestureNotifyTapNHoldEvent *)event;
			wthev = (xGestureNotifyTapNHoldEvent *)wire;
			thev->type = wthev->type & 0x7f;
			thev->serial = _XSetLastRequestRead(dpy, (xGenericReply *) wire);
			thev->send_event = (wthev->type & 0x80) != 0;
			thev->display = dpy;
			thev->time = wthev->time;
			thev->kind = wthev->kind;
			thev->num_finger = wthev->num_finger;
			thev->cx = wthev->cx;
			thev->cy = wthev->cy;
			thev->interval = wthev->interval;
			thev->window = wthev->window;
#ifdef __XGESTURE_LIB_DEBUG__
			fprintf(stderr, "[wire_to_event]\n");
			fprintf(stderr, "[ == Wire values == ]\n");
			fprintf(stderr, "num of finger=%d\n", wthev->num_finger);
			fprintf(stderr, "interval=%d\n", wthev->interval);
			fprintf(stderr, "window=0x%x\n", wthev->window);
			fprintf(stderr, "cx:%d, cy:%d\n", wthev->cx, wthev->cy);
			fprintf(stderr, "time=%d\n", wthev->time);
			fprintf(stderr, "[ == Event values]\n");
			fprintf(stderr, "num of finger=%d\n", thev->num_finger);
			fprintf(stderr, "interval=%d\n", thev->interval);
			fprintf(stderr, "window=0x%x\n", thev->window);
			fprintf(stderr, "cx:%d, cy:%d\n", thev->cx, thev->cy);
			fprintf(stderr, "time=%d\n", thev->time);
#endif//__XGESTURE_LIB_DEBUG__
			return True;

	    case GestureNotifyHold:
			hev = (XGestureNotifyHoldEvent *)event;
			whev = (xGestureNotifyHoldEvent *)wire;
			hev->type = whev->type & 0x7f;
			hev->serial = _XSetLastRequestRead(dpy, (xGenericReply *) wire);
			hev->send_event = (whev->type & 0x80) != 0;
			hev->display = dpy;
			hev->time = whev->time;
			hev->kind = whev->kind;
			hev->num_finger = whev->num_finger;
			hev->cx = whev->cx;
			hev->cy = whev->cy;
			hev->holdtime = whev->holdtime;
			hev->window = whev->window;
#ifdef __XGESTURE_LIB_DEBUG__
			fprintf(stderr, "[wire_to_event]\n");
			fprintf(stderr, "[ == Wire values == ]\n");
			fprintf(stderr, "num of finger=%d\n", whev->num_finger);
			fprintf(stderr, "holdtime=%d\n", whev->holdtime);
			fprintf(stderr, "window=0x%x\n", whev->window);
			fprintf(stderr, "cx:%d, cy:%d\n", whev->cx, whev->cy);
			fprintf(stderr, "time=%d\n", whev->time);
			fprintf(stderr, "[ == Event values]\n");
			fprintf(stderr, "num of finger=%d\n", hev->num_finger);
			fprintf(stderr, "holdtime=%d\n", hev->holdtime);
			fprintf(stderr, "window=0x%x\n", hev->window);
			fprintf(stderr, "cx:%d, cy:%d\n", hev->cx, hev->cy);
			fprintf(stderr, "time=%d\n", hev->time);
#endif//__XGESTURE_LIB_DEBUG__
			return True;
    }

    return False;
}

static Status
event_to_wire (Display *dpy, XEvent *event, xEvent *wire)
{
    XExtDisplayInfo *info = find_display (dpy);

    XGestureNotifyGroupEvent *gev;
    xGestureNotifyGroupEvent *wgev;
    XGestureNotifyFlickEvent *fev;
    xGestureNotifyFlickEvent *wfev;
    XGestureNotifyPanEvent *pev;
    xGestureNotifyPanEvent *wpev;
    XGestureNotifyPinchRotationEvent *pcrev;
    xGestureNotifyPinchRotationEvent *wpcrev;
    XGestureNotifyTapEvent *tev;
    xGestureNotifyTapEvent *wtev;
    XGestureNotifyTapNHoldEvent *thev;
    xGestureNotifyTapNHoldEvent *wthev;
    XGestureNotifyHoldEvent *hev;
    xGestureNotifyHoldEvent *whev;
    GestureCheckExtension (dpy, info, False);


#ifdef __XGESTURE_LIB_DEBUG__
    XGestureCommonEvent *xce = (XGestureCommonEvent *)event;
    fprintf(stderr, "[event_to_wire] event->type=%d, info->codes->first_event=%d\n", 
		event->type, info->codes->first_event);
    fprintf(stderr, "[event_to_wire] xce->any.kind=%d\n", xce->any.kind);
#endif//__XGESTURE_LIB_DEBUG__

    switch (event->type - info->codes->first_event) {
	    case GestureNotifyGroup:
			gev = (XGestureNotifyGroupEvent *)event;
			wgev = (xGestureNotifyGroupEvent *)wire;
			wgev->type = gev->type | (gev->send_event ? 0x80 : 0);
			wgev->sequenceNumber = gev->serial & 0xffff;
			wgev->time = gev->time;
			wgev->kind = gev->kind;
			wgev->groupid = gev->groupid;
			wgev->num_group = gev->num_group;
			wgev->window = gev->window;
#ifdef __XGESTURE_LIB_DEBUG__
			fprintf(stderr, "[wire_to_event]\n");
			fprintf(stderr, "[ == Wire values == ]\n");
			fprintf(stderr, "groupid=%d\n", wgev->groupid);
			fprintf(stderr, "tap num_group=%d\n", wgev->num_group);
			fprintf(stderr, "window=0x%x\n", wgev->window);
			fprintf(stderr, "time=%d\n", wgev->time);
			fprintf(stderr, "[ == Event values]\n");
			fprintf(stderr, "groupid=%d\n", gev->groupid);
			fprintf(stderr, "tap num_group=%d\n", gev->num_group);
			fprintf(stderr, "window=0x%x\n", gev->window);
			fprintf(stderr, "time=%d\n", gev->time);
#endif//__XGESTURE_LIB_DEBUG__
			return True;

	    case GestureNotifyFlick:
			fev = (XGestureNotifyFlickEvent *)event;
			wfev = (xGestureNotifyFlickEvent *)wire;
			wfev->type = fev->type | (fev->send_event ? 0x80 : 0);
			wfev->sequenceNumber = fev->serial & 0xffff;
			wfev->time = fev->time;
			wfev->kind = fev->kind;
			wfev->window = fev->window;
			wfev->num_finger = fev->num_finger;
			wfev->distance = fev->distance;
			wfev->duration = fev->duration;
			wfev->angle = fev->angle;
			wfev->direction = fev->direction;
#ifdef __XGESTURE_LIB_DEBUG__
			fprintf(stderr, "[wire_to_event]\n");
			fprintf(stderr, "[ == Wire values == ]\n");
			fprintf(stderr, "num of finger=%d\n", wfev->num_finger);
			fprintf(stderr, "distance=%d\n", wfev->distance);
			fprintf(stderr, "duration=%d\n", wfev->duration);
			fprintf(stderr, "angle=%d\n", wfev->angle);
			fprintf(stderr, "direction=%d\n", wfev->direction);
			fprintf(stderr, "window=0x%x\n", wfev->window);
			fprintf(stderr, "time=%d\n", wfev->time);
			fprintf(stderr, "[ == Event values]\n");
			fprintf(stderr, "num of finger=%d\n", fev->num_finger);
			fprintf(stderr, "distance=%d\n", fev->distance);
			fprintf(stderr, "duration=%d\n", fev->duration);
			fprintf(stderr, "angle=%d\n", fev->angle);
			fprintf(stderr, "direction=%d\n", fev->direction);
			fprintf(stderr, "window=0x%x\n", fev->window);
			fprintf(stderr, "time=%d\n", fev->time);
#endif//__XGESTURE_LIB_DEBUG__
			return True;

	    case GestureNotifyPan:
			pev = (XGestureNotifyPanEvent *)event;
			wpev = (xGestureNotifyPanEvent *)wire;
			wpev->type = pev->type | (pev->send_event ? 0x80 : 0);
			wpev->sequenceNumber = pev->serial & 0xffff;
			wpev->time = pev->time;
			wpev->kind = pev->kind;
			wpev->window = pev->window;
			wpev->num_finger = pev->num_finger;
			wpev->distance = pev->distance;
			wpev->duration = pev->duration;
			wpev->direction = pev->direction;
			wpev->dx = (short int)pev->dx;
			wpev->dy = (short int)pev->dy;
#ifdef __XGESTURE_LIB_DEBUG__
			fprintf(stderr, "[wire_to_event]\n");
			fprintf(stderr, "[ == Wire values == ]\n");
			fprintf(stderr, "num of finger=%d\n", wpev->num_finger);
			fprintf(stderr, "direction=%d\n", wpev->direction);
			fprintf(stderr, "distance=%d\n", wpev->distance);
			fprintf(stderr, "duration=%d\n", wpev->duration);
			fprintf(stderr, "window=0x%x\n", wpev->window);
			fprintf(stderr, "dx:%d, dy:%d\n", wpev->dx, wpev->dy);
			fprintf(stderr, "time=%d\n", wpev->time);
			fprintf(stderr, "[ == Event values]\n");
			fprintf(stderr, "num of finger=%d\n", pev->num_finger);
			fprintf(stderr, "direction=%d\n", pev->direction);
			fprintf(stderr, "distance=%d\n", pev->distance);
			fprintf(stderr, "duration=%d\n", pev->duration);
			fprintf(stderr, "window=0x%x\n", pev->window);
			fprintf(stderr, "dx:%d, dy:%d\n", pev->dx, pev->dy);
			fprintf(stderr, "time=%d\n", pev->time);
#endif//__XGESTURE_LIB_DEBUG__
			return True;

	    case GestureNotifyPinchRotation:
			pcrev = (XGestureNotifyPinchRotationEvent *)event;
			wpcrev = (xGestureNotifyPinchRotationEvent *)wire;
			wpcrev->type = pcrev->type | (pcrev->send_event ? 0x80 : 0);
			wpcrev->sequenceNumber = pcrev->serial & 0xffff;
			wpcrev->time = pcrev->time;
			wpcrev->kind = pcrev->kind;
			wpcrev->window = pcrev->window;
			wpcrev->num_finger = pcrev->num_finger;
			wpcrev->zoom = pcrev->zoom;
			wpcrev->angle = pcrev->angle;
			wpcrev->cx = pcrev->cx;
			wpcrev->cy = pcrev->cy;
			wpcrev->distance = pcrev->distance;
#ifdef __XGESTURE_LIB_DEBUG__
			fprintf(stderr, "[wire_to_event]\n");
			fprintf(stderr, "[ == Wire values == ]\n");
			fprintf(stderr, "num of finger=%d\n", wpcrev->num_finger);
			fprintf(stderr, "zoom=%d\n", wpcrev->zoom);
			fprintf(stderr, "angle=%d\n", wpcrev->angle);
			fprintf(stderr, "distance=%d\n", wpcrev->distance);
			fprintf(stderr, "window=0x%x\n", wpcrev->window);
			fprintf(stderr, "cx:%d, cy:%d\n", wpcrev->cx, wpcrev->cy);
			fprintf(stderr, "time=%d\n", wpcrev->time);
			fprintf(stderr, "[ == Event values]\n");
			fprintf(stderr, "num of finger=%d\n", pcrev->num_finger);
			fprintf(stderr, "zoom=%d\n", pcrev->zoom);
			fprintf(stderr, "angle=%d\n", pcrev->angle);
			fprintf(stderr, "distance=%d\n", pcrev->distance);
			fprintf(stderr, "window=0x%x\n", pcrev->window);
			fprintf(stderr, "cx:%d, cy:%d\n", pcrev->cx, pcrev->cy);
			fprintf(stderr, "time=%d\n", pcrev->time);
#endif//__XGESTURE_LIB_DEBUG__
			return True;

	    case GestureNotifyTap:
			tev = (XGestureNotifyTapEvent *)event;
			wtev = (xGestureNotifyTapEvent *)wire;
			wtev->type = tev->type | (tev->send_event ? 0x80 : 0);
			wtev->sequenceNumber = tev->serial & 0xffff;
			wtev->time = tev->time;
			wtev->kind = tev->kind;
			wtev->window = tev->window;
			wtev->num_finger = tev->num_finger;
			wtev->cx = tev->cx;
			wtev->cy = tev->cy;
			wtev->tap_repeat = tev->tap_repeat;
			wtev->interval = tev->interval;
#ifdef __XGESTURE_LIB_DEBUG__
			fprintf(stderr, "[event_to_wire]\n");
			fprintf(stderr, "[ == Wire values == ]\n");
			fprintf(stderr, "num of finger=%d\n", wtev->num_finger);
			fprintf(stderr, "tap repeat=%d\n", wtev->tap_repeat);
			fprintf(stderr, "window=0x%x\n", wtev->window);
			fprintf(stderr, "cx:%d, cy:%d\n", wtev->cx, wtev->cy);
			fprintf(stderr, "time=%d, interval=%d\n", wtev->time, wtev->interval);
			fprintf(stderr, "[ == Event values]\n");
			fprintf(stderr, "num of finger=%d\n", tev->num_finger);
			fprintf(stderr, "tap repeat=%d\n", tev->tap_repeat);
			fprintf(stderr, "window=0x%x\n", tev->window);
			fprintf(stderr, "cx:%d, cy:%d\n", tev->cx, tev->cy);
			fprintf(stderr, "time=%d, interval=%d\n", tev->time, tev->interval);
#endif//__XGESTURE_LIB_DEBUG__
	 		return True;

	    case GestureNotifyTapNHold:
			thev = (XGestureNotifyTapNHoldEvent *)event;
			wthev = (xGestureNotifyTapNHoldEvent *)wire;
			wthev->type = thev->type | (thev->send_event ? 0x80 : 0);
			wthev->sequenceNumber = thev->serial & 0xffff;
			wthev->time = thev->time;
			wthev->kind = thev->kind;
			wthev->window = thev->window;
			wthev->num_finger = thev->num_finger;
			wthev->cx = thev->cx;
			wthev->cy = thev->cy;
			wthev->interval = thev->interval;
			wthev->holdtime = thev->holdtime;
#ifdef __XGESTURE_LIB_DEBUG__
			fprintf(stderr, "[wire_to_event]\n");
			fprintf(stderr, "[ == Wire values == ]\n");
			fprintf(stderr, "num of finger=%d\n", wthev->num_finger);
			fprintf(stderr, "interval=%d\n", wthev->interval);
			fprintf(stderr, "window=0x%x\n", wthev->window);
			fprintf(stderr, "cx:%d, cy:%d\n", wthev->cx, wthev->cy);
			fprintf(stderr, "time=%d\n", wthev->time);
			fprintf(stderr, "[ == Event values]\n");
			fprintf(stderr, "num of finger=%d\n", thev->num_finger);
			fprintf(stderr, "interval=%d\n", thev->interval);
			fprintf(stderr, "window=0x%x\n", thev->window);
			fprintf(stderr, "cx:%d, cy:%d\n", thev->cx, thev->cy);
			fprintf(stderr, "time=%d\n", thev->time);
#endif//__XGESTURE_LIB_DEBUG__
			return True;

	    case GestureNotifyHold:
			hev = (XGestureNotifyHoldEvent *)event;
			whev = (xGestureNotifyHoldEvent *)wire;
			whev->type = hev->type | (hev->send_event ? 0x80 : 0);
			whev->sequenceNumber = hev->serial & 0xffff;
			whev->time = hev->time;
			whev->kind = hev->kind;
			whev->window = hev->window;
			whev->num_finger = hev->num_finger;
			whev->cx = hev->cx;
			whev->cy = hev->cy;
			whev->holdtime = hev->holdtime;
#ifdef __XGESTURE_LIB_DEBUG__
			fprintf(stderr, "[wire_to_event]\n");
			fprintf(stderr, "[ == Wire values == ]\n");
			fprintf(stderr, "num of finger=%d\n", whev->num_finger);
			fprintf(stderr, "holdtime=%d\n", whev->holdtime);
			fprintf(stderr, "window=0x%x\n", whev->window);
			fprintf(stderr, "cx:%d, cy:%d\n", whev->cx, whev->cy);
			fprintf(stderr, "time=%d\n", whev->time);
			fprintf(stderr, "[ == Event values]\n");
			fprintf(stderr, "num of finger=%d\n", hev->num_finger);
			fprintf(stderr, "holdtime=%d\n", hev->holdtime);
			fprintf(stderr, "window=0x%x\n", hev->window);
			fprintf(stderr, "cx:%d, cy:%d\n", hev->cx, hev->cy);
			fprintf(stderr, "time=%d\n", hev->time);
#endif//__XGESTURE_LIB_DEBUG__
			return True;
    }

    return 0;
}

Bool XGestureQueryExtension (Display *dpy, int *event_basep, int *error_basep)
{
    XExtDisplayInfo *info = find_display (dpy);

    TRACE("QueryExtension...");
    if (XextHasExtension(info)) {
        *event_basep = info->codes->first_event;
        *error_basep = info->codes->first_error;
        TRACE("QueryExtension... return True");
        return True;
    } else {
        TRACE("QueryExtension... return False");
        return False;
    }
}

Bool XGestureQueryVersion(Display* dpy, int* majorVersion, int* minorVersion,
			  int* patchVersion)
{
    XExtDisplayInfo *info = find_display (dpy);
    xGestureQueryVersionReply rep;
    xGestureQueryVersionReq *req;

    TRACE("QueryVersion...");
    GestureCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(GestureQueryVersion, req);
    req->reqType = info->codes->major_opcode;
    req->gestureReqType = X_GestureQueryVersion;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
        UnlockDisplay(dpy);
        SyncHandle();
        TRACE("QueryVersion... return False");
        return False;
    }
    *majorVersion = rep.majorVersion;
    *minorVersion = rep.minorVersion;
    *patchVersion = rep.patchVersion;
    UnlockDisplay(dpy);
    SyncHandle();
    TRACE("QueryVersion... return True");

    return True;
}

Status XGestureSelectEvents(Display* dpy, Window w, Mask mask)
{
    XExtDisplayInfo *info = find_display (dpy);
    xGestureSelectEventsReq *req;

    TRACE("SelectEvents...");
    GestureCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(GestureSelectEvents, req);
    req->reqType = info->codes->major_opcode;
    req->gestureReqType = X_GestureSelectEvents;
    req->window = w;
    req->mask = mask;
    UnlockDisplay(dpy);
    SyncHandle();
    TRACE("SelectEvents... return True");

    return GestureSuccess;
}

Status XGestureGetSelectedEvents(Display* dpy, Window w, Mask *mask_return)
{
    Mask mask_out = 0;

    XExtDisplayInfo *info = find_display (dpy);
    xGestureGetSelectedEventsReply rep;
    xGestureGetSelectedEventsReq *req;

    TRACE("GetSelectedEvents...");
    GestureCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(GestureGetSelectedEvents, req);
    req->reqType = info->codes->major_opcode;
    req->gestureReqType = X_GestureGetSelectedEvents;
    req->window = w;

    if ( !_XReply (dpy, (xReply *) &rep, 0, xTrue) )
    {
	UnlockDisplay(dpy);
	SyncHandle();
	TRACE("GetSelectedEvents... Failed");
	return GestureInvalidReply;
    }

    mask_out = rep.mask;

    UnlockDisplay(dpy);
    SyncHandle();
    TRACE("GetSelectedEvents... Succeed");

    *mask_return = mask_out;
	
    return GestureSuccess;
}

Status XGestureGrabEvent(Display* dpy, Window w, int eventType, int num_finger, Time time)
{
    XExtDisplayInfo *info = find_display (dpy);
    xGestureGrabEventReply rep;
    xGestureGrabEventReq *req;
    register int status;

    TRACE("GrabEvent...");
    GestureCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(GestureGrabEvent, req);
    req->reqType = info->codes->major_opcode;
    req->gestureReqType = X_GestureGrabEvent;
    req->window = w;
    req->eventType = eventType;
    req->num_finger = num_finger;
    req->time = time;

    if( eventType >= GestureNumberEvents )
    {
    	TRACE("GrabEvent... return GestureGrabAbnormal");
	return GestureGrabAbnormal;
    }

    /* if we ever return, suppress the error */
    if ( !_XReply (dpy, (xReply *) &rep, 0, xTrue) )
    {
	UnlockDisplay(dpy);
	SyncHandle();
	TRACE("GrabEvent... return GestureInvalidReply");
	return GestureInvalidReply;
    }

    status = rep.status;

    UnlockDisplay(dpy);
    SyncHandle();
    TRACE("GrabEvent... return success");

    return status;
}

Status XGestureUngrabEvent(Display* dpy, Window w, int eventType, int num_finger, Time time)
{
    XExtDisplayInfo *info = find_display (dpy);
    xGestureUngrabEventReply rep;
    xGestureUngrabEventReq *req;
    register int status;

    TRACE("UnrabEvent...");
    GestureCheckExtension (dpy, info, False);

    if( !w )
    {
    	TRACE("UnrabEvent...w is invalid");
	return GestureUngrabAbnormal;
    }

    LockDisplay(dpy);
    GetReq(GestureUngrabEvent, req);
    req->reqType = info->codes->major_opcode;
    req->gestureReqType = X_GestureUngrabEvent;
    req->window = w;
    req->eventType = eventType;
    req->num_finger = num_finger;
    req->time = time;

    if( eventType >= GestureNumberEvents )
    {
    	TRACE("UnrabEvent... return GestureUngrabAbnormal");
	return GestureUngrabAbnormal;
    }

    /* if we ever return, suppress the error */
    if ( !_XReply (dpy, (xReply *) &rep, 0, xTrue) )
    {
	UnlockDisplay(dpy);
	SyncHandle();
	TRACE("UnrabEvent... return failed");
	return GestureInvalidReply;
    }

    status = rep.status;

    UnlockDisplay(dpy);
    SyncHandle();
    TRACE("UnrabEvent... return success");

    return status;
}

