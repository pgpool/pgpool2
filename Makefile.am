#AM_CPPFLAGS = -D_GNU_SOURCE -I @PGSQL_INCLUDE_DIR@
AUTOMAKE_OPTIONS = subdir-objects

# suggested by libtoolize
ACLOCAL_AMFLAGS = -I m4

if can_build_docs
DOC_DIR = doc
endif

SUBDIRS = src $(DOC_DIR)
DIST_SUBDIRS = src doc

