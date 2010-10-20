MODULES = pgpool-walrecrunning
DATA_built = pgpool-walrecrunning.sql

# if you are using PostgreSQL 8.0 or later,
# using pg_config is recommended.
# if you are not, comment out following line and...
USE_PGXS = true
# set top_builddir to the PostgreSQL build source tree top.
# (for example /usr/local/src/postgresql-8.4) 
top_builddir = ../..

ifdef USE_PGXS
PGXS = $(shell pg_config --pgxs)
include $(PGXS)
else
subdir = contrib/pgpool-walrecrunning
#top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
