#!/usr/bin/bash
#-------------------------------------------------------------------
# test script for client authentication
#

# This test is only valid with PostgreSQL 10 or later.
if [ $PGVERSION -le 9 ];then
    echo "all tests skipped due to PostgreSQL version: $PGVERSION"
    exit 0
fi

# --------------------------------------------------------------------
# PAM authentication tests are not performed by default.  In order to
# test PAM authentication (both frontend and backend), run following
# scripts here (040.client_auth).
#
# Create pam users list.
# $ ./list_pam_user.sh > pam_users.txt
#
# Create input file for newusers command.
# The result file is newusers_input.txt.
# $ ./create_pam_user.sh pam_users.txt
#
# Finally run newusers command as root.
# the users are not allowed to login (login shell is /usr/sbin/nologin)
# /usr/sbin/newusers newusers_input.txt
#
# Note that newusers just ignore users that are already registered in
# the system. So it is harmless to generate new newusers_input.txt and
# run newusers command. It will happily register new users.
#
# Then set the environment variable DO_PAM_TEST to "true".
#
if [ "$DO_PAM_TEST" != "true" ];then
    echo "all pam tests will be skipped"
fi
# --------------------------------------------------------------------
# LDAP authentication tests are not performed by default.  In order to
# test LDAP authentication (both frontend and backend), run following
# scripts here (040.client_auth). The procedure below assumes that
# LDAP server is created on localhost, with LDAP admin name is
# "admin", domain is "nodomain".
#
# To create LDAP ldif file, run:
# $ cat client_auth_2node.csv|./list_ldap_user.sh|./create_ldap_user.sh > ldap_users.ldif
#
# Then run ldapadd command.
# ldapadd -x -D cn=admin,dc=nodomain -W -f ldap_users.ldif
#
# Set the environment variable DO_LDAP_TEST to "true".
#
if [ "$DO_LDAP_TEST" != "true" ];then
    echo "all ldap tests will be skipped"
fi
# --------------------------------------------------------------------
# Function declarations

# create user
# $1: username
# $2: "scram", "md5", "password", "pam" or "ldap".
# $3: port
# If "password" is specified, encryption method becomes 'scram-sha-256'.
function add_user()
{
    createuser -p $3 $1
    if [ $2 = "scram" ];then
	psql -p $3 -c "SET password_encryption = 'scram-sha-256'; ALTER USER $1 WITH ENCRYPTED PASSWORD '$1'"
    elif [ $2 = "md5" ];then
	psql -p $3 -c "SET password_encryption = 'md5'; ALTER USER $1 WITH ENCRYPTED PASSWORD '$1'"
    elif [ $2 = "password" ];then
	psql -p $3 -c "SET password_encryption = 'scram-sha-256'; ALTER USER $1 WITH ENCRYPTED PASSWORD '$1'"
    elif [ "$DO_PAM_TEST" = "true" -a $2 = "pam" ];then
	psql -p $3 -c "SET password_encryption = 'scram-sha-256'; ALTER USER $1 WITH ENCRYPTED PASSWORD '$1'"
    elif [ "$DO_PAM_TEST" != "true" -a $2 = "pam" ];then
	echo "do nothing because DO_PAM_TEST is not true and auth method is pam"
    elif [ "$DO_LDAP_TEST" = "true" -a $2 = "ldap" ];then
	psql -p $3 -c "SET password_encryption = 'scram-sha-256'; ALTER USER $1 WITH ENCRYPTED PASSWORD '$1'"
    elif [ "$DO_LDAP_TEST" != "true" -a $2 = "ldap" ];then
	echo "do nothing because DO_LDAP_TEST is not true and auth method is ldap"
    else
	echo "wrong auth method \"$2\" for add_user $1"
	exit 1
    fi
}

# create pool_hba.conf entry
# $1: username
# $2: auth method for pool_hba.conf
# "scram", "md5" or "password"
# $3: pgpool major version first digit (e.g. "4")
function add_pool_hba()
{
    # "scram-sha-256" is only supported in pgpool version 4.0 or later
    if [ $3 -gt 3 -a $2 = "scram" ];then
	echo "host	all	$1	127.0.0.1/32	scram-sha-256" >> $POOL_HBA
	echo "host	all	$1	::1/128	scram-sha-256" >> $POOL_HBA
    elif [ $2 = "md5" ];then
	echo "host	all	$1	127.0.0.1/32	md5" >> $POOL_HBA
	echo "host	all	$1	::1/128	md5" >> $POOL_HBA
    elif [ "$DO_PAM_TEST" = "true" -a $2 = "pam" ];then
	echo "host	all	$1	127.0.0.1/32	pam" >> $POOL_HBA
	echo "host	all	$1	::1/128	pam" >> $POOL_HBA
    elif [ "$DO_PAM_TEST" != "true" -a $2 = "pam" ];then
	echo "do nothing because DO_PAM_TEST is not true and auth method is pam"
    elif [ "$DO_LDAP_TEST" = "true" -a $2 = "ldap" ];then
	echo "host	all	$1	127.0.0.1/32	ldap ldapserver=localhost ldapbasedn="dc=nodomain" ldapsearchattribute=uid" >> $POOL_HBA
	echo "host	all	$1	::1/128	ldap ldapserver=localhost ldapbasedn="dc=nodomain" ldapsearchattribute=uid" >> $POOL_HBA
    elif [ "$DO_LDAP_TEST" != "true" -a $2 = "ldap" ];then
	echo "do nothing because DO_LDAP_TEST is not true and auth method is ldap"
    # "password" is only supported in pgpool version 4.0 or later
    elif [ $3 -gt 3 -a $2 = "password" ];then
	echo "host	all	$1	127.0.0.1/32	password" >> $POOL_HBA
	echo "host	all	$1	::1/128	password" >> $POOL_HBA
    else
	echo "skip adding to pool_hba.conf"
    fi
}

# create pool_passwd entry
# $1: username (also password)
# $2: encryption method (AES, md5 or text)
# $3: pgpool major version first digit (e.g. "4")
function add_pool_passwd()
{
    # "AES" is only supported in pgpool version 4.0 or later
    if [ $3 -gt 3 -a $2 = "AES" ];then
	echo $1|$PG_ENC -m -f $PGPOOL_CONF -u $1 $1
    elif [ $2 = "md5" ];then
	echo $1|$PG_MD5 -m -f $PGPOOL_CONF -u $1 $1
    # "text" is only supported in pgpool version 4.0 or later
    elif [ $3 -gt 3 -a $2 = "text" ];then
	if [ $3 -gt 3 ];then
	    echo "$1:TEXT$1" >>  $POOL_PASSWD
	else
	    echo "$1:$1" >>  $POOL_PASSWD
	fi
    else
	echo "skip adding to pool_passwd"
    fi
}

# create pg_hba.conf entry
# $1: username
# $2: "scram", "md5", "password" or "pam".
function add_pg_hba()
{
    for i in data0 data1
    do
	if [ ! -d $i ];then
	    continue
	fi

	PG_HBA=$i/pg_hba.conf
	if [ $2 = "scram" ];then
	    echo "host	all	$1	127.0.0.1/32	scram-sha-256" >> $PG_HBA
	    echo "host	all	$1	::1/128	scram-sha-256" >> $PG_HBA
	    echo "local	all	$1	scram-sha-256" >> $PG_HBA
	elif [ $2 = "md5" ];then
	    echo "host	all	$1	127.0.0.1/32	md5" >> $PG_HBA
	    echo "host	all	$1	::1/128	md5" >> $PG_HBA
	    echo "local	all	$1	md5" >> $PG_HBA
	elif [ $2 = "password" ];then
	    echo "host	all	$1	127.0.0.1/32	password" >> $PG_HBA
	    echo "host	all	$1	::1/128	password" >> $PG_HBA
	    echo "local	all	$1	password" >> $PG_HBA
	elif [ "$DO_PAM_TEST" = "true" -a $2 = "pam" ];then
	    echo "host	all	$1	127.0.0.1/32	pam" >> $PG_HBA
	    echo "host	all	$1	::1/128	pam" >> $PG_HBA
	    echo "local	all	$1	pam" >> $PG_HBA
	elif [ "$DO_PAM_TEST" != "true" -a $2 = "pam" ];then
	    echo "do nothing because DO_PAM_TEST is not true and auth method is pam"
	elif [ "$DO_LDAP_TEST" = "true" -a $2 = "ldap" ];then
	    echo "host	all	$1	127.0.0.1/32	ldap ldapserver=localhost ldapbasedn="dc=nodomain" ldapsearchattribute=uid" >> $PG_HBA
	    echo "host	all	$1	::1/128	ldap ldapserver=localhost ldapbasedn="dc=nodomain" ldapsearchattribute=uid" >> $PG_HBA
	    echo "local	all	$1	ldap ldapserver=localhost ldapbasedn="dc=nodomain" ldapsearchattribute=uid" >> $PG_HBA
	elif [ "$DO_LDAP_TEST" != "true" -a $2 = "ldap" ];then
	    echo "do nothing because DO_LDAP_TEST is not true and auth method is ldap"
	else
	    echo "wrong auth method \"$2\" for add_pg_hba $1"
	    exit 1
	fi
    done
}

# create pgpass entry
# $1: username
function add_pgpass()
{
    echo "127.0.0.1:$PGPORT:$PGDATABASE:$1:$1" >> pgpass
    echo "127.0.0.1:$PGPORT:$PGDATABASE:$1:$1foo" >> pgpasswrong
}

# Perform actual tests
# $1: test spec csv file
# $2: number of PostgreSQL nodes
# $3: clustering mode
function do_auth
{
    rm -fr $TESTDIR
    mkdir $TESTDIR
    cd $TESTDIR

    cp /dev/null $failed_usernames
    mode=$3

    # create test environment
    echo -n "creating test environment..."
    $PGPOOL_SETUP -m $mode -n $2 || exit 1
    #$PGPOOL_SETUP -m $mode -n 1 || exit 1
    echo "done."

    source ./bashrc.ports
    export PGPORT=$PGPOOL_PORT
    export PGDATABASE=test

    # Set max_init_children to 1 to make sure we reuse the
    # connection to test wrong password rejection
    echo "num_init_children = 1" >> etc/pgpool.conf

    PGPOOL_CONF=etc/pgpool.conf
    POOL_HBA=etc/pool_hba.conf
    if [ ! -f $POOL_HBA ];then
	cp ../../../../../sample/pool_hba.conf.sample $POOL_HBA
    fi
    POOL_PASSWD=etc/pool_passwd

    #
    # replace trust auth for all users with superuser in pg_hba.conf
    for i in data0 data1
    do
	if [ ! -d $i ];then
	    continue
	fi

	sed -i "s/local *all *all *trust/local	all	$superuser	trust/" $i/pg_hba.conf
	sed -i "s/host *all *all  *all *trust/host      all   $superuser       all    trust/" $i/pg_hba.conf
    done

    #
    # replace trust auth for all users with superuser in pool_hba.conf
    sed -i "s@host    all         all         127.0.0.1/32          trust@host      all   $superuser       all    trust@" $POOL_HBA
    sed -i "s@host    all         all         ::1/128               trust@host    all         $superuser         ::1/128               trust@" $POOL_HBA

    # set up pgpass
    cp /dev/null pgpass
    chmod 0600 pgpass
    cp /dev/null pgpasswrong
    chmod 0600 pgpasswrong

    # pgpool.conf opt
    cp /dev/null $PGPOOL_CONF_OPT
#    echo "include pgpool.conf.opt" >> etc/pgpool.conf
    # add last line marker to pgpool.conf
    echo "#last" >> etc/pgpool.conf

    # start pgpool
    ./startall
    wait_for_pgpool_startup

    IFS=","

    #
    # setup each user
    #
    cat $spec|while read line
    do
	set $line
	if [ $1 = 'username' ];then
	    echo "skip title row"
	else
	    username=$1
	    echo "==== $username ===="
	    pool_hba=$2
	    allow_clear=$3
	    pool_passwd=$4
	    pg_hba=$5
	    expected=$6

	    add_user $username $pg_hba $PGPORT

	    # for logical replication, we need to manually
	    # add PostgreSQL user to node 1.
	    if [ $mode = 'l' ];then
		add_user $username $pg_hba "11003"
	    fi
	    add_pool_hba $username $pool_hba $PGPOOL_VERSION_DIGIT
	    add_pool_passwd $username $pool_passwd $PGPOOL_VERSION_DIGIT
	    add_pg_hba $username $pg_hba
	    add_pgpass $username
	fi
    done
    ./shutdownall

    #
    # do auth tests
    #
    cat $spec|while read line
    do
	set $line
	username=$1
	echo "==== $username ===="
	pool_hba=$2
	allow_clear=$3
	pool_passwd=$4
	pg_hba=$5
	expected=$6

	if [ $1 = 'username' ];then
	    echo "skip tile row"
	elif [ "$DO_PAM_TEST" != "true" ] && [ $pool_hba = "pam" -o $pg_hba = "pam" ];then
	     echo "skip pam test"
	elif [ "$DO_LDAP_TEST" != "true" ] && [ $pool_hba = "ldap" -o $pg_hba = "ldap" ];then
	     echo "skip ldap test"
	else
	    cp $PGPOOL_CONF_OPT $PGPOOL_CONF_OPT.old
	    if [ $pool_hba = "scram" -o $pool_hba = "md5" -o $pool_hba = "password" -o $pool_hba = "pam" -o $pool_hba = "ldap" ];then
		echo "enable_pool_hba = on" > $PGPOOL_CONF_OPT
		last=`tail -1 etc/pgpool.conf`
		if [ "$last" != "#last" ];then
		    # remove the last line if it's not a marker
		    echo "remove the last line in pgpool.conf because \"$last\""
		    sed -i '$d' etc/pgpool.conf
		fi
		cat $PGPOOL_CONF_OPT >> etc/pgpool.conf
		echo "add \"`cat $PGPOOL_CONF_OPT`\""
	    else
		if [ $allow_clear = "off" ];then
		    cp /dev/null $PGPOOL_CONF_OPT
		else
		    echo "allow_clear_text_frontend_auth = on" > $PGPOOL_CONF_OPT
		fi
		last=`tail -1 etc/pgpool.conf`
		if [ $last != "#last" ];then
		    # remove the last line if it's not a marker
		    echo "remove the last line in pgpool.conf because \"$last\""
		    sed -i '$d' etc/pgpool.conf
		fi
		cat $PGPOOL_CONF_OPT >> etc/pgpool.conf
		echo "add \"`cat $PGPOOL_CONF_OPT`\""
	    fi

	    # if pgpool.conf.opt was changed, restart pgpool
	    cmp $PGPOOL_CONF_OPT $PGPOOL_CONF_OPT.old
	    if [ $? != 0 ];then
		./shutdownall
		./startall
		wait_for_pgpool_startup
	    fi

	    result=""
	    PGPASSFILE=pgpass $PSQL -h 127.0.0.1 -U $username -c "SELECT user" >/dev/null 2>&1
	    rtn=$?
	    if [ $expected = "ok" ];then
		echo -n "checking $username auth expecting success..."
		if [ $rtn = 0 ];then
		    echo "ok."
		    echo -n "checking $username auth expecting success in reauth..."
		    PGPASSFILE=pgpass $PSQL -h 127.0.0.1 -U $username -c "SELECT user" >/dev/null 2>&1
		    if [ $? != 0 ];then
			echo "$username: password verification on reauth failed."
		    else
			# reauth was Ok.
			echo "ok."
			echo -n "$username: try with wrong password expecting rejected..."
			PGPASSFILE=pgpasswrong $PSQL -h 127.0.0.1 -U $username -c "SELECT user">/dev/null 2>&1
			if [ $? = 0 ];then
			    echo "$username: wrong password verification failed."
			    echo "$mode $num_backends $username" >> $unexpected_passwd_verifi
			else
			    echo "ok."
			    result=ok
			fi
		    fi
		else
		    echo "$username: password verification failed."
		fi
	    else	# expecting fail
		echo -n "checking $username auth expecting failure..."
		if [ $rtn != 0 ];then
		    echo "ok."
		    result=ok
		else
		    # make sure that wrong password is rejected
		    PGPASSFILE=pgpasswrong $PSQL -h 127.0.0.1 -U $username -c "SELECT user">/dev/null 2>&1
		    if [ $? = 0 ];then
			echo "$username: unexpected successfull password verification"
			echo "$mode $num_backends $username" >> $unexpected_passwd_verifi
		    else
			# maybe this test case should have been "ok", not "fail"?
			echo "$username: while expecting fail, wrong password was rejected but proper password was accepted"
		    fi
		fi
	    fi

	    if [ "$result" != "ok" ];then
		echo -n " $username" >> $failed_usernames
	    fi
	fi
    done

    ./shutdownall
    cd ..

    if [ -s $failed_usernames ];then
	echo "failed tests:"
	cat $failed_usernames
	echo
    fi
}

#
#----------------------------------------
# Misc preparations

# pgpool major version first digit (e.g. "4")
PGPOOL_VERSION_DIGIT=`echo $PGPOOL_VERSION|awk '{print $3}'|sed 's/\([1-9]\).*$/\1/'`
failed_usernames=/tmp/failed_usernames
failed_modes=/tmp/failed_modes
unexpected_passwd_verifi=/tmp/unexpected_passwd_verificatio
total_failed_usernames=/tmp/total_failed_usernames
trap "rm $failed_usernames $failed_modes $unexpected_passwd_verifi $total_failed_usernames" EXIT
cp /dev/null $failed_usernames
cp /dev/null $failed_modes
cp /dev/null $unexpected_passwd_verifi
cp /dev/null $total_failed_usernames

source $TESTLIBS
TESTDIR=testdir
PSQL=$PGBIN/psql
PG_ENC=$PGPOOL_INSTALL_DIR/bin/pg_enc
PG_MD5=$PGPOOL_INSTALL_DIR/bin/pg_md5
export CREATEUSER=$PGBIN/createuser
superuser=`whoami`
PGPOOL_CONF_OPT=etc/pgpool.conf.opt
# .pgpoolkey
export PGPOOLKEYFILE=`pwd`/pgpoolkey
echo "secret" > $PGPOOLKEYFILE
chmod 0600 $PGPOOLKEYFILE
#
#----------------------------------------
# Test execution starts here

# Full test (6 clustering modes) takes too long time and it causes
# timeout.  For now, we skip logical replication and slony mode.
#for mode in s r i n l y
for mode in s r i n
do
    echo "==== testing mode: $mode ==="
    anyfail=""
    spec=../client_auth_2node.csv
    num_backends=2
    do_auth $spec $num_backends $mode
    if [ -s $failed_usernames ];then
	anyfail="yes"
	echo "$mode $num_backends `cat $failed_usernames`" >> $total_failed_usernames
    fi

    if [ -n "$anyfail" ];then
	echo -n "$mode " >> $failed_modes
    fi
done

if [ -s $unexpected_passwd_verifi ];then
    echo
    echo "IMPORTANT: unexpected successful password verfication found:"
    cat $unexpected_passwd_verifi
fi

if [ -s $failed_modes ];then
    echo
    echo "failed mode: `cat $failed_modes`"
    echo
    echo "failed tests:"
    cat $total_failed_usernames
    exit 1
fi

exit 0
