AM_CPPFLAGS = -D_GNU_SOURCE -DPOOL_PRIVATE -I @PGSQL_INCLUDE_DIR@
bin_PROGRAMS = pg_md5

pg_md5_SOURCES = pg_md5.c \
		md5.c \
		pool_passwd.c \
		pool_signal.c \
		strlcpy.c \
		regex_array.c \
		pool_config.l \
		../fe_port.c \
		fe_memutils.c \
		pool_globals.c

DEFS = @DEFS@ \
    -DDEFAULT_CONFIGDIR=\"$(sysconfdir)\" -DPOOL_TOOLS

md5.c: ../../../src/auth/md5.c
	rm -f $@ && ln -s $< .
md5.h: ../../../src/include/auth/md5.h
	rm -f $@ && ln -s $< .
pool_passwd.c: ../../../src/auth/pool_passwd.c
	rm -f $@ && ln -s $< .
pool_signal.c: ../../../src/utils/pool_signal.c
	rm -f $@ && ln -s $< .
strlcpy.c: ../../../src/utils/strlcpy.c
	rm -f $@ && ln -s $< .
regex_array.c: ../../../src/utils/regex_array.c
	rm -f $@ && ln -s $< .
pool_config.l: ../../../src/config/pool_config.l
	rm -f $@ && ln -s $< .
fe_memutils.c: ../../../src/tools/fe_memutils.c
	rm -f $@ && ln -s $< .
pool_globals.c: ../../../src/main/pool_globals.c
	rm -f $@ && ln -s $< .
