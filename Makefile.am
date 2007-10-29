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
	ps_status.c strlcpy.c recovery.c

pg_md5_SOURCES = pg_md5.c md5.c md5.h

DEFS = @DEFS@ \
	-DDEFAULT_CONFIGDIR=\"$(sysconfdir)\"

sysconf_DATA = pgpool.conf.sample pcp.conf.sample pool_hba.conf.sample
pkgdata_DATA = sql/system_db.sql sample/pgpool.pam

AM_CPPFLAGS = -pg -Wall -Wmissing-prototypes -Wmissing-declarations -D_GNU_SOURCE -I @PGSQL_INCLUDE_DIR@

pgpool_LDADD = -L@PGSQL_LIB_DIR@ -lpq parser/libsql-parser.a pcp/libpcp.la parser/nodes.o
if enable_rpath
  pgpool_LDFLAGS = -pg -rpath @PGSQL_LIB_DIR@ -rpath $(libdir)
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
		sample/pgpool_remote_start sample/pgpool_recovery sample/pgpool_recovery_pitr \
		sample/dist_def_pgbench.sql sql/pgpool-recovery/pgpool-recovery.c \
		sql/pgpool-recovery/pgpool-recovery.sql.in sql/pgpool-recovery/Makefile \
		test/parser/expected/cursor.out test/parser/expected/copy.out \
		test/parser/expected/privileges.out test/parser/expected/delete.out \
		test/parser/expected/insert.out test/parser/expected/misc.out \
		test/parser/expected/prepare.out test/parser/expected/transaction.out \
		test/parser/expected/select.out test/parser/expected/update.out \
		test/parser/expected/var.out test/parser/expected/create.out \
		test/parser/expected/drop.out test/parser/input/cursor.sql \
		test/parser/input/copy.sql test/parser/input/privileges.sql \
		test/parser/input/delete.sql test/parser/input/insert.sql \
		test/parser/input/misc.sql test/parser/input/prepare.sql \
		test/parser/input/transaction.sql test/parser/input/select.sql \
		test/parser/input/update.sql test/parser/input/var.sql \
		test/parser/input/create.sql test/parser/input/drop.sql \
		test/parser/input/alter.sql test/parser/.cvsignore test/parser/Makefile \
		test/parser/README test/parser/main.c \
		test/parser/pool.h test/parser/run-test \
		test/parser/test.diff test/parser/parse_schedule \
		test/jdbc/*.java test/jdbc/pgpool.properties test/jdbc/prepare.sql test/jdbc/run.sh \
		test/jdbc/expected/autocommit test/jdbc/expected/batch \
		test/jdbc/expected/column test/jdbc/expected/lock test/jdbc/expected/select \
		test/jdbc/expected/update \
		redhat/pgpool.init redhat/pgpool.sysconfig

SUBDIRS = parser pcp
