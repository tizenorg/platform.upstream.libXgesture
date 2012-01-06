lib_LTLIBRARIES = libXgesture.la

libXgesture_la_SOURCES = \
	gesture.c

AM_CFLAGS = \
	$(GESTURE_CFLAGS) \
	$(MALLOC_ZERO_CFLAGS) \
	$(CWARNFLAGS) \
	-F/System/Library/Frameworks/ApplicationServices.framework/Frameworks \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/include/X11 \
	-I$(top_srcdir)/include/X11/extensions

libXgesture_la_LIBADD = @GESTURE_LIBS@

libXgesture_la_LDFLAGS = -version-info 7:0:0 -no-undefined -framework ApplicationServices

extincludedir = $(includedir)/X11/extensions
extinclude_HEADERS = $(top_srcdir)/include/X11/extensions/gesture.h
