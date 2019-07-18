AM_CFLAGS  = -I$(top_builddir)/include -I$(top_builddir)/lib -I$(top_builddir)/libmetrics

if STATIC_BUILD
noinst_LTLIBRARIES    = libmodibmams.la
libmodibmams_la_SOURCES = mod_ibmams.c 
else
pkglib_LTLIBRARIES    = modibmams.la
modibmams_la_SOURCES = mod_ibmams.c 
modibmams_la_LDFLAGS = -module -avoid-version
modibmams_la_LIBADD = $(top_builddir)/libmetrics/libmetrics.la

EXTRA_DIST = ../conf.d/ibmams.conf
endif

INCLUDES = @APR_INCLUDES@

