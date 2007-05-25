bin_PROGRAMS = pgpool pg_md5

pgpool_SOURCES = pool.h pool_type.h version.h pgpool.conf.sample \
	README.euc_jp pgpool.spec \
	main.c child.c pool_auth.c pool_config.l pool_error.c \
	pool_process_query.c pool_stream.c pool_connection_pool.c pool_params.c \
	pool_signal.h pool_signal.c pcp_child.c md5.c md5.h pcp.conf.sample \
	pool_ipc.h pool_shmem.c pool_sema.c pool_system.c \
	pool_rewrite_query.c pool_rewrite_query.h pool_rewrite_outfuncs.c \
	pool_query_cache.c pool_hba.conf.sample sample/pgpool.pam\
	pool_hba.c pool_path.h pool_path.c pool_ip.h pool_ip.c pool_type.h \
	ps_status.c strlcpy.c

pg_md5_SOURCES = pg_md5.c md5.c md5.h

DEFS = @DEFS@ \
	-DDEFAULT_CONFIGDIR=\"$(sysconfdir)\"

sysconf_DATA = pgpool.conf.sample pcp.conf.sample pool_hba.conf.sample
pkgdata_DATA = sql/system_db.sql sample/pgpool.pam

AM_CPPFLAGS = -Wall -Wmissing-prototypes -Wmissing-declarations -D_GNU_SOURCE -I @PGSQL_INCLUDE_DIR@

pgpool_LDADD = -L@PGSQL_LIB_DIR@ -lpq parser/libsql-parser.a pcp/libpcp.la parser/nodes.o
if enable_rpath
  pgpool_LDFLAGS = -rpath @PGSQL_LIB_DIR@ -rpath $(libdir)
else
  pgpool_LDFLAGS = 
endif

AM_YFLAGS = -d

man_MANS = pgpool.8

pgpool.8: pgpool.8.in
	sed 's,@sysconfdir\@,$(sysconfdir),g' $? >$@

CLEANFILES = pgpool.8
EXTRA_DIST = pgpool.8.in sql/system_db.sql sample/pgpool.pam doc/pgpool-ja.html doc/pgpool-en.html \
		doc/tutorial-ja.html doc/tutorial-en.html doc/pgpool.css \
		sample/dist_def_pgbench.sql

SUBDIRS = parser pcp
