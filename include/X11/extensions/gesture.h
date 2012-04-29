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

#ifndef _GESTURE_LIB_H_
#define _GESTURE_LIB_H_

#include <X11/Xfuncproto.h>
#include <X11/extensions/gestureconst.h>

#ifndef Bool
#define Bool int
#endif

#ifndef XFixed
typedef int XFixed;
#endif

#ifndef XDouble
typedef double XDouble;
#endif

#define XDoubleToFixed(f)       ((XFixed) ((f) * 65536))
#define XFixedToDouble(f)       (((XDouble) (f)) / 65536)

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;		/* true if this came from a SendEvent request */
	Display *display;		/* Display the event was read from */
	Window window;		/* window on which event was requested in event mask */
	Time time;			/* server timestamp when event happened */
	int kind;				/* subevent type */
	int groupid;
	int num_group;
} XGestureNotifyGroupEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;		/* true if this came from a SendEvent request */
	Display *display;		/* Display the event was read from */
	Window window;		/* window on which event was requested in event mask */
	Time time;			/* server timestamp when event happened */
	int kind;				/* subevent type */
	int num_finger;
	int direction;			/* 8 way direction (0 ~ 7) */
	int distance;			/* distance between first point and last point (pixel) */
	Time duration;		/* time difference between press and release (ms) */
	XFixed angle;			/* angel difference between horizontal line and flick line (radian) */
} XGestureNotifyFlickEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;		/* true if this came from a SendEvent request */
	Display *display;		/* Display the event was read from */
	Window window;		/* window on which event was requested in event mask */
	Time time;			/* server timestamp when event happened */
	int kind;				/* subevent type */
	int num_finger;
	int direction;			/* 8 way direction (0 ~ 7) */
	int distance;			/* distance between first point and last point (pixel) */
	Time duration;		/* time difference between press and release (ms) */
	int dx;				/* x coordinate delta */
	int dy;				/* y coordinate delta */
} XGestureNotifyPanEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;		/* true if this came from a SendEvent request */
	Display *display;		/* Display the event was read from */
	Window window;		/* window on which event was requested in event mask */
	Time time;			/* server timestamp when event happened */
	int kind;				/* subevent type */
	int num_finger;
	int distance;			/* distance from center (pixel) */
	int cx;				/* center x coordinate */
	int cy;				/* center ycoordinate */
	XFixed zoom;			/* zoom factor (base : 1.0) */
	XFixed angle;			/* angel difference between first line and current line (radian) */
} XGestureNotifyPinchRotationEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;		/* true if this came from a SendEvent request */
	Display *display;		/* Display the event was read from */
	Window window;		/* window on which event was requested in event mask */
	Time time;			/* server timestamp when event happened */
	int kind;				/* subevent type */
	int num_finger;
	int cx;				/* center x coordinate */
	int cy;				/* center ycoordinate */
	int tap_repeat;		/* tap repeats such as SINGLE_TAP, DBL_TAP and so on */
	Time interval;		/* time difference between tap and previous tap (ms) */
} XGestureNotifyTapEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;		/* true if this came from a SendEvent request */
	Display *display;		/* Display the event was read from */
	Window window;		/* window on which event was requested in event mask */
	Time time;			/* server timestamp when event happened */
	int kind;				/* subevent type */
	int num_finger;
	int cx;				/* center x coordinate */
	int cy;				/* center ycoordinate */
	Time interval;		/* time difference between tap and hold (ms) */
	Time holdtime;		/* hold time (ms) */
} XGestureNotifyTapNHoldEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;		/* true if this came from a SendEvent request */
	Display *display;		/* Display the event was read from */
	Window window;		/* window on which event was requested in event mask */
	Time time;			/* server timestamp when event happened */
	int kind;				/* subevent type */
	int num_finger;
	int cx;				/* center x coordinate */
	int cy;				/* center ycoordinate */
	Time holdtime;		/* hold time (ms) */
} XGestureNotifyHoldEvent;

union _XGestureCommonEvent {
	struct {
		int type;
		unsigned long serial;
		Bool send_event;
		Display *display;
		Window window;
		Time time;
		int kind;
	} any;
	XGestureNotifyGroupEvent gev;
	XGestureNotifyFlickEvent fev;
	XGestureNotifyPanEvent pev;
	XGestureNotifyPinchRotationEvent pcrev;
	XGestureNotifyTapEvent tev;
	XGestureNotifyTapNHoldEvent thev;
	XGestureNotifyHoldEvent hev;
};

typedef union _XGestureCommonEvent XGestureCommonEvent;

_XFUNCPROTOBEGIN

extern Bool XGestureQueryExtension (Display *dpy, int *event_base, int *error_base);

extern Bool XGestureQueryVersion (Display *dpy, int *majorVersion,
			    int *minorVersion, int *patchVersion);

extern Status XGestureSelectEvents(Display* dpy, Window w, Mask mask);

extern Status XGestureGetSelectedEvents(Display* dpy, Window w, Mask *mask_return);

extern Status XGestureGrabEvent(Display* dpy, Window w, int eventType, int num_finger, Time time);

extern Status XGestureUngrabEvent(Display* dpy, Window w, int eventType, int num_finger, Time time);

_XFUNCPROTOEND

#endif//_GESTURE_LIB_H_

