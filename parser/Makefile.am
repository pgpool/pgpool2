noinst_LIBRARIES = libsql-parser.a

libsql_parser_a_SOURCES = gram.y gramparse.h list.c makefuncs.c makefuncs.h memnodes.h nodes.h nodes.c parsenodes.h parser.c parser.h pg_list.h pool_memory.c pool_memory.h pool_string.c pool_string.h primnodes.h  scansup.h value.c value.h keywords.h keywords.c pool_parser.h outfuncs.c copyfuncs.c

EXTRA_DIST = scan.c scan.l

AM_CPPFLAGS = -pg -Wall -Wmissing-prototypes -Wmissing-declarations -D_GNU_SOURCE -I .. -I @PGSQL_INCLUDE_DIR@

AM_YFLAGS = -d

gram.c: gram.y scan.c
scan.c: scan.l
	$(LEX) -o'scan.c' $<
