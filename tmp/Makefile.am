
AM_CFLAGS  = -Wall
AM_CFLAGS += -Werror

AM_CPPFLAGS  = -D_GNU_SOURCE
AM_CPPFLAGS += -I$(top_srcdir)/include

lib_LTLIBRARIES = libtapdiskthin.la

libtapdiskthin_la_SOURCES = payload.c

libtapdiskthin_la_LDFLAGS = -version-info 1:1:1

