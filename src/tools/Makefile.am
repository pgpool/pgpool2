SUBDIRS = pcp pgmd5

bin_SCRIPTS =  pgpool_setup watchdog_setup

pgpool_setup:
	ln -s ../test/pgpool_setup .

watchdog_setup:
	ln -s ../test/watchdog_setup .

clean-local:
	-rm -f $(bin_SCRIPTS)
