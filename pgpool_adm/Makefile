# contrib/pcp/Makefile

MODULE_big = pgpool_adm
OBJS = pgpool_adm.o
PG_CPPFLAGS = -I$(libpq_srcdir)

EXTENSION = pgpool_adm
DATA = pgpool_adm--1.0.sql
SHLIB_LINK = -lpcp

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/pgpool_adm
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
