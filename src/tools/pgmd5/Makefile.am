AM_CPPFLAGS = -D_GNU_SOURCE -DPOOL_PRIVATE -I @PGSQL_INCLUDE_DIR@
bin_PROGRAMS = pg_md5

pg_md5_SOURCES = pg_md5.c \
			$(top_srcdir)/src/auth/md5.c \
			$(top_srcdir)/src/auth/pool_passwd.c \
			$(top_srcdir)/src/utils/pool_signal.c \
			$(top_srcdir)/src/utils/strlcpy.c \
			$(top_srcdir)/src/utils/regex_array.c \
			$(top_srcdir)/src/config/pool_config.l \
			$(top_srcdir)/src/tools/fe_port.c \
			$(top_srcdir)/src/tools/fe_memutils.c \
			$(top_srcdir)/src/main/pool_globals.c

DEFS = @DEFS@ \
    -DDEFAULT_CONFIGDIR=\"$(sysconfdir)\" -DPOOL_TOOLS
