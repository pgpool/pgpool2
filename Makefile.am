#AM_CPPFLAGS = -D_GNU_SOURCE -I @PGSQL_INCLUDE_DIR@
AUTOMAKE_OPTIONS = subdir-objects

# suggested by libtoolize
ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = pgpool_adm/Makefile pgpool_adm/TODO pgpool_adm/pgpool_adm--1.0.sql \
			pgpool_adm/pgpool_adm.c pgpool_adm/pgpool_adm.control \
			pgpool_adm/pgpool_adm.h pgpool_adm/pgpool_adm.sql.in

SUBDIRS = src doc

