libs_dir = $(top_srcdir)/src/libs
AM_CPPFLAGS = -D_GNU_SOURCE -DPOOL_PRIVATE -I @PGSQL_INCLUDE_DIR@

bin_PROGRAMS =  pcp_stop_pgpool \
				pcp_node_count \
				pcp_node_info \
				pcp_proc_count \
				pcp_proc_info \
				pcp_systemdb_info \
				pcp_detach_node \
				pcp_attach_node \
				pcp_recovery_node \
				pcp_promote_node \
				pcp_pool_status \
				pcp_watchdog_info
pcp_stop_pgpool_SOURCES = pcp_stop_pgpool.c ../fe_memutils.c
pcp_stop_pgpool_LDADD = $(libs_dir)/pcp/libpcp.la
pcp_node_count_SOURCES = pcp_node_count.c  ../fe_memutils.c
pcp_node_count_LDADD = $(libs_dir)/pcp/libpcp.la
pcp_node_info_SOURCES = pcp_node_info.c ../fe_memutils.c
pcp_node_info_LDADD = $(libs_dir)/pcp/libpcp.la
pcp_proc_count_SOURCES = pcp_proc_count.c ../fe_memutils.c
pcp_proc_count_LDADD = $(libs_dir)/pcp/libpcp.la
pcp_proc_info_SOURCES = pcp_proc_info.c ../fe_memutils.c
pcp_proc_info_LDADD = $(libs_dir)/pcp/libpcp.la
pcp_systemdb_info_SOURCES = pcp_systemdb_info.c ../fe_memutils.c
pcp_systemdb_info_LDADD = $(libs_dir)/pcp/libpcp.la
pcp_detach_node_SOURCES = pcp_detach_node.c ../fe_memutils.c
pcp_detach_node_LDADD = $(libs_dir)/pcp/libpcp.la
pcp_attach_node_SOURCES = pcp_attach_node.c ../fe_memutils.c
pcp_attach_node_LDADD = $(libs_dir)/pcp/libpcp.la
pcp_recovery_node_SOURCES = pcp_recovery_node.c ../fe_memutils.c
pcp_recovery_node_LDADD = $(libs_dir)/pcp/libpcp.la
pcp_pool_status_SOURCES = pcp_pool_status.c ../fe_memutils.c
pcp_pool_status_LDADD = $(libs_dir)/pcp/libpcp.la
pcp_promote_node_SOURCES = pcp_promote_node.c ../fe_memutils.c
pcp_promote_node_LDADD = $(libs_dir)/pcp/libpcp.la
pcp_watchdog_info_SOURCES = pcp_watchdog_info.c ../fe_memutils.c
pcp_watchdog_info_LDADD = $(libs_dir)/pcp/libpcp.la

