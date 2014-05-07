AM_CPPFLAGS = -D_GNU_SOURCE -DPOOL_PRIVATE -I @PGSQL_INCLUDE_DIR@

lib_LTLIBRARIES = libpcp.la
libpcp_la_SOURCES = pcp.c $(top_srcdir)/src/utils/pcp/pcp_stream.c \
                    $(top_srcdir)/src/utils/pcp/pcp_error.c \
                    $(top_srcdir)/src/utils/pcp/pcp_timeout.c \
                    $(top_srcdir)/src/auth/md5.c \
                    $(top_srcdir)/src/utils/strlcpy.c

include_HEADERS = $(top_srcdir)/src/include/pcp/pcp.h \
					$(top_srcdir)/src/include/pcp/libpcp_ext.h \
					$(top_srcdir)/src/include/pool_type.h \
					$(top_srcdir)/src/include/utils/pool_process_reporting.h

md5.c: $(top_srcdir)/src/auth/md5.c
	rm -f $@ && ln -s $< .
md5.h: $(top_srcdir)/src/include/auth/md5.h
	rm -f $@ && ln -s $< .

