AM_CPPFLAGS = -D_GNU_SOURCE -DPOOL_PRIVATE -I @PGSQL_INCLUDE_DIR@

lib_LTLIBRARIES = libpcp.la
libpcp_la_SOURCES = pcp.c \
		    pcp_stream.c \
                    pcp_error.c \
                    pcp_timeout.c \
                    md5.c \
                    strlcpy.c

include_HEADERS = ../../../src/include/pcp/pcp.h \
					../../../src/include/pcp/libpcp_ext.h \
					../../../src/include/pool_type.h \
					../../../src/include/utils/pool_process_reporting.h

md5.c: ../../../src/auth/md5.c
	rm -f $@ && ln -s $< .
md5.h: ../../../src/include/auth/md5.h
	rm -f $@ && ln -s $< .
pcp_stream.c: ../../../src/utils/pcp/pcp_stream.c
	rm -f $@ && ln -s $< .
pcp_error.c: ../../../src/utils/pcp/pcp_error.c
	rm -f $@ && ln -s $< .
pcp_timeout.c: ../../../src/utils/pcp/pcp_timeout.c
	rm -f $@ && ln -s $< .
strlcpy.c: ../../../src/utils/strlcpy.c
	rm -f $@ && ln -s $< .
