noinst_LIBRARIES = libsql-parser.a

libsql_parser_a_SOURCES  = \
	copyfuncs.c \
	gram.y \
	gramparse.h \
	keywords.c \
	keywords.h \
	kwlist.h \
	kwlookup.c \
	list.c \
	makefuncs.c \
	makefuncs.h \
	memnodes.h \
	nodes.c \
	nodes.h \
	outfuncs.c \
	parsenodes.h \
	parser.c \
	parser.h \
	pg_list.h \
	pg_wchar.h \
	pool_memory.c \
	pool_memory.h \
	pool_parser.h \
	pool_string.c \
	pool_string.h \
	primnodes.h \
	scanner.h \
	scansup.c \
	scansup.h \
	value.c \
	value.h \
	wchar.c

EXTRA_DIST = scan.c scan.l

AM_CPPFLAGS = -D_GNU_SOURCE -I .. -I @PGSQL_INCLUDE_DIR@

AM_YFLAGS = -d

gram.c: gram.y scan.c
scan.c: scan.l
	$(LEX) -o'scan.c' $<
