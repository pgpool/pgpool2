noinst_LIBRARIES = lib-watchdog.a

lib_watchdog_a_SOURCES = \
	watchdog.c \
	watchdog.h \
	wd_child.c \
	wd_ext.h \
	wd_if.c \
	wd_init.c \
	wd_lifecheck.c \
	wd_list.c \
	wd_packet.c \
	wd_ping.c \
	wd_udp.c

AM_CPPFLAGS = -D_GNU_SOURCE -I .. -I @PGSQL_INCLUDE_DIR@
