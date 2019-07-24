parser_incdir = $(top_srcdir)/src/include/parser
AM_CPPFLAGS = -D_GNU_SOURCE -I $(parser_incdir) -I @PGSQL_INCLUDE_DIR@
noinst_LIBRARIES = libsql-parser.a

libsql_parser_a_SOURCES  = \
	copyfuncs.c \
	gram.y \
	keywords.c \
	kwlookup.c \
	list.c \
	makefuncs.c \
	nodes.c \
	outfuncs.c \
	parser.c \
	pool_string.c \
	scansup.c \
	stringinfo.c \
	value.c \
	$(top_srcdir)/src/utils/mmgr/mcxt.c \
	$(top_srcdir)/src/utils/mmgr/aset.c \
	$(top_srcdir)/src/utils/error/elog.c \
	wchar.c scan.c

if use_repl_snprintf
  libsql_parser_a_SOURCES  += snprintf.c
endif

EXTRA_DIST = scan.c scan.l

ARFLAGS = cr

AM_YFLAGS = -d

gram.c: gram.y scan.c
scan.c: scan.l
	$(LEX) -o'scan.c' $<
