#!/usr/bin/env bash
#
# pgpool-II regression test driver.
#
# usage: regress.sh [test_name]
# -i install directory of pgpool
# -b pgbench path
# -p installation path of Postgres
# -j JDBC driver path
# -m install (install pgpool-II and use that for tests) / noinstall : Default install

dir=`pwd`
MODE=install
PG_INSTALL_DIR=/usr/local/pgsql/bin
PGPOOL_PATH=/usr/local
JDBC_DRIVER=/usr/local/pgsql/share/postgresql-9.2-1003.jdbc4.jar
log=$dir/log
fail=0
ok=0

CRED=$(tput setaf 1)
CGREEN=$(tput setaf 2)
CBLUE=$(tput setaf 4)
CNORM=$(tput sgr0)


function install_pgpool
{
	echo "creating pgpool-II temporary installation ..."
        PGPOOL_PATH=$dir/temp/installed
        
	make install -C $dir/../../ -e prefix=${PGPOOL_PATH}
	
	echo "moving pgpool_setup to temporary installation path ..."
        cp $dir/../pgpool_setup ${PGPOOL_PATH}/pgpool_setup
	export PGPOOL_SETUP=$PGPOOL_PATH/pgpool_setup
}

function verify_pginstallation
{
	# PostgreSQL bin directory
	PGBIN=`$PG_INSTALL_DIR/pg_config --bindir`
	if [ -z $PGBIN ]; then
		echo "$0: cannot locate pg_config"
		exit 1
	fi
}

function export_env_vars
{
	if [[ -z "$PGPOOL_PATH" ]]; then
		# check if pgpool is in the path
		PGPOOL_PATH=/usr/local
		export PGPOOL_SETUP=$HOME/bin/pgpool_setup
 	fi
	
	if [[ -z "$PGBENCH_PATH" ]]; then
		if [ -x $PGBIN/pgbench ]; then
			PGBENCH_PATH=$PGBIN/pgbench
		else
			PGBENCH_PATH=`which pgbench`
		fi
	fi

	if [ ! -x $PGBENCH_PATH ]; then
		echo "$0] cannot locate pgbench"; exit 1
 	fi
	
	echo "using pgpool-II at "$PGPOOL_PATH
    export PGPOOL_INSTALL_DIR=$PGPOOL_PATH
	
	export TESTLIBS=$dir/libs.sh
	export PGBIN=$PGBIN
	export JDBC_DRIVER=$JDBC_DRIVER
	export PGBENCH_PATH=$PGBENCH_PATH
}
function print_info
{
	echo ${CBLUE}"*************************"${CNORM}

	echo "REGRESSION MODE : "${CBLUE}$MODE${CNORM}
	echo "PGPOOL-II       : "${CBLUE}$PGPOOL_PATH${CNORM}
	echo "PostgreSQL bin  : "${CBLUE}$PGBIN${CNORM}
	echo "pgbench         : "${CBLUE}$PGBENCH_PATH${CNORM}
	echo "PostgreSQL jdbc : "${CBLUE}$JDBC_DRIVER${CNORM}
	echo ${CBLUE}"*************************"${CNORM}
}

function print_usage
{
	printf "Usage:\n"
	printf "  %s: [Options]... [test_name]\n" $(basename $0) >&2
	printf "\nOptions:\n"
	printf "  -p   DIRECTORY           Postgres installed directory\n" >&2
	printf "  -b   PATH                pgbench installed path, if different from Postgres installed directory\n" >&2
	printf "  -i   DIRECTORY           pgpool installed directory, if already installed pgpool is to be used for tests\n" >&2
	printf "  -m   install/noinstall   make install pgpool to temp directory for executing regression tests [Default: install]\n" >&2
	printf "  -j   FILE                Postgres jdbc jar file path\n" >&2
	printf "  -?                       print this help and then exit\n\n" >&2
	printf "Please read the README for details on adding new tests\n" >&2

}

trap "echo ; exit 0" SIGINT SIGQUIT

while getopts "p:m:i:j:b:?" OPTION
do
  case $OPTION in
    p)  PG_INSTALL_DIR="$OPTARG";;
    m)  MODE="$OPTARG";;
    i)  PGPOOL_PATH="$OPTARG";;
    j)  JDBC_DRIVER="$OPTARG";;
    b)  PGBENCH_PATH="$OPTARG";;
    ?)  print_usage
        exit 2;;
  esac
done

shift $(($OPTIND - 1))
if [ "$MODE" = "install" ]; then
	install_pgpool

elif [ "$MODE" = "noinstall" ]; then
	echo not installing pgpool for the tests ...
	if [[ -n "$PGPOOL_INSTALL_PATH" ]]; then
		PGPOOL_PATH=$PGPOOL_INSTALL_PATH
	fi
	export PGPOOL_SETUP=$dir/../pgpool_setup
else
	echo $MODE : Invalid mode
	exit -1
fi 

verify_pginstallation
export_env_vars
print_info

#Start executing tests
rm -fr $log
mkdir $log

cd tests

if [ $# -eq 1 ];then
	dirs=`ls|grep $1`
else
	dirs=`ls`
fi

for i in $dirs
do
	cd $i
	echo -n "testing $i..."
	./test.sh > $log/$i 2>&1
	if [ $? = 0 ];then
		echo ${CGREEN}"ok."${CNORM}
		ok=`expr $ok + 1`
	else
		echo ${CRED}"failed."${CNORM}
		fail=`expr $fail + 1`
	fi

	cd ..

done

total=`expr $ok + $fail`

echo "out of $total ok:$ok failed:$fail"
