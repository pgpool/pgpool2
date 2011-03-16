bin_PROGRAMS = pgpool pg_md5

pgpool_SOURCES = pool.h pool_type.h version.h pgpool.conf.sample \
	pgpool.conf.sample-replication pgpool.conf.sample-master-slave \
	pgpool.conf.sample-stream \
	README.euc_jp README.online-recovery pgpool.spec \
	main.c child.c pool_auth.c pool_config.l pool_config.h pool_error.c \
	pool_process_query.c pool_stream.c pool_stream.h pool_connection_pool.c pool_params.c \
	pool_signal.h pool_signal.c pcp_child.c md5.c md5.h pcp.conf.sample \
	pool_ipc.h pool_shmem.c pool_sema.c pool_system.c \
	pool_rewrite_query.c pool_rewrite_query.h pool_rewrite_outfuncs.c \
	pool_query_cache.c pool_hba.conf.sample sample/pgpool.pam\
	pool_hba.c pool_path.h pool_path.c pool_ip.h pool_ip.c pool_type.h \
	ps_status.c strlcpy.c recovery.c pool_relcache.c pool_relcache.h pool_process_reporting.c \
	pool_ssl.c pool_timestamp.c pool_timestamp.h \
	pool_proto2.c pool_proto_modules.c pool_proto_modules.h \
	pool_lobj.c pool_lobj.h \
	pool_process_context.c pool_process_context.h \
	pool_session_context.c pool_session_context.h \
	pool_query_context.c pool_query_context.h \
	pool_worker_child.c \
	pool_passwd.c pool_passwd.h \
	pool_globals.c \
	pool_select_walker.c pool_select_walker.h \
    getopt_long.c getopt_long.h

pg_md5_SOURCES = pg_md5.c md5.c md5.h \
	pool.h \
	pool_config_md5.c pool_config.h \
	pool_error.c \
	pool_signal.h pool_signal.c \
	pool_passwd.c pool_passwd.h \
	pool_globals.c

DEFS = @DEFS@ \
	-DDEFAULT_CONFIGDIR=\"$(sysconfdir)\"

sysconf_DATA = pgpool.conf.sample pcp.conf.sample pool_hba.conf.sample \
				pgpool.conf.sample-replication pgpool.conf.sample-master-slave \
				pgpool.conf.sample-stream
pkgdata_DATA = sql/system_db.sql sample/pgpool.pam

AM_CPPFLAGS = -D_GNU_SOURCE -I @PGSQL_INCLUDE_DIR@

# suggested by libtoolize
ACLOCAL_AMFLAGS = -I m4

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
		doc/where_to_send_queries.pdf doc/where_to_send_queries.odg doc/pgpool-ja.css \
		doc/basebackup.sh doc/pgpool_remote_start doc/recovery.conf.sample \
		sample/pgpool_remote_start sample/pgpool_recovery sample/pgpool_recovery_pitr \
		sample/dist_def_pgbench.sql sample/replicate_def_pgbench.sql \
		sql/Makefile \
		sql/pgpool-recovery/pgpool-recovery.c \
		sql/pgpool-recovery/pgpool-recovery.sql.in sql/pgpool-recovery/Makefile \
		sql/pgpool-regclass/pgpool-regclass.c \
		sql/pgpool-regclass/pgpool-regclass.sql.in sql/pgpool-regclass/Makefile \
		sql/pgpool-walrecrunning/README sql/pgpool-walrecrunning/pgpool-walrecrunning.c \
		sql/pgpool-walrecrunning/pgpool-walrecrunning.sql.in sql/pgpool-walrecrunning/Makefile \
		test/parser/expected/copy.out test/parser/expected/create.out \
		test/parser/expected/cursor.out test/parser/expected/delete.out \
		test/parser/expected/drop.out test/parser/expected/insert.out \
		test/parser/expected/misc.out test/parser/expected/prepare.out \
		test/parser/expected/privileges.out test/parser/expected/scanner.out \
		test/parser/expected/select.out \
		test/parser/expected/transaction.out test/parser/expected/update.out \
		test/parser/expected/v84.out test/parser/expected/v90.out \
		test/parser/expected/var.out \
		test/parser/input/alter.sql \
		test/parser/input/copy.sql test/parser/input/create.sql \
		test/parser/input/cursor.sql test/parser/input/delete.sql \
		test/parser/input/drop.sql test/parser/input/insert.sql \
		test/parser/input/misc.sql test/parser/input/prepare.sql \
		test/parser/input/privileges.sql test/parser/input/scanner.sql \
		test/parser/input/select.sql \
		test/parser/input/transaction.sql test/parser/input/update.sql \
		test/parser/input/v84.sql test/parser/input/v90.sql \
		test/parser/input/var.sql \
		test/parser/.cvsignore test/parser/Makefile \
		test/parser/README test/parser/main.c \
		test/parser/pool.h test/parser/run-test \
		test/parser/parse_schedule \
		test/C/Makefile test/C/test_extended.c \
		test/jdbc/*.java test/jdbc/README.euc_jp test/jdbc/pgpool.properties test/jdbc/prepare.sql test/jdbc/run.sh \
		test/jdbc/expected/autocommit test/jdbc/expected/batch \
		test/jdbc/expected/column test/jdbc/expected/lock test/jdbc/expected/select \
		test/jdbc/expected/update test/jdbc/expected/insert test/jdbc/expected/CreateTempTable \
		test/pdo-test/README.euc_jp test/pdo-test/collections.inc test/pdo-test/def.inc \
		test/pdo-test/log.txt test/pdo-test/pdotest.php test/pdo-test/regsql.inc \
		test/pdo-test/SQLlist/test1.sql test/pdo-test/SQLlist/test2.sql \
		test/pdo-test/mod/database.inc test/pdo-test/mod/def.inc test/pdo-test/mod/errorhandler.inc \
		test/timestamp/Makefile test/timestamp/input/insert.sql \
		test/timestamp/input/update.sql test/timestamp/input/misc.sql \
		test/timestamp/expected/insert.out test/timestamp/expected/update.out \
		test/timestamp/expected/misc.out test/timestamp/main.c \
		test/timestamp/parse_schedule test/timestamp/run-test \
		redhat/pgpool.init redhat/pgpool.sysconfig redhat/pgpool.conf.sample.patch


SUBDIRS = parser pcp
