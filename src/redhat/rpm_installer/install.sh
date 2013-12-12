#! /bin/bash
# install.sh: Install script for pgpool-II

PATH=/bin:/sbin:/usr/bin:/usr/sbin

MAJOR_VERSION=3.3
DIST=pgdg

# debug mode of this script
SH_DEBUG=0

# PostgreSQL
PG_MAJOR_VERSION=9.3

# pgpool-II
PGPOOL_SOFTWARE_NAME=pgpool-II
P_VERSION=3.3.0
P_RELEASE=1
PGPOOL_CONF_DIR=/etc/pgpool-II

# pgpoolAdmin
ADMIN_SOFTWARE_NAME=pgpoolAdmin
A_VERSION=3.3.0
A_RELEASE=1
ADMIN_DIR=/var/www/html/pgpoolAdmin
APACHE_USER=apache

# packages
PG_VER=${PG_MAJOR_VERSION/./}
ARCHITECTURE=$(uname -i)
PACKAGE_FILES=(
    ${PGPOOL_SOFTWARE_NAME}-pg${PG_VER}-${P_VERSION}-${P_RELEASE}.${DIST}.${ARCHITECTURE}.rpm
    ${ADMIN_SOFTWARE_NAME}-${A_VERSION}-${A_RELEASE}.${DIST}.noarch.rpm
)

# pgpool
PGPOOL_BIN_DIR=/usr/bin
PGPOOL_PORT=9999
PCP_PORT=9898
WATCHDOG_PORT=9000
MODE=stream                    # This will be editted in script.
NODE0_HOST=""                  # This will be editted in script.
NODE1_HOST=""                  # This will be editted in script.
NETMASK="255.255.255.0"        # This will be editted in script.

# postgres
PGHOME=/usr/pgsql-$PG_MAJOR_VERSION
CONTRIB_DIR=$PGHOME/share/contrib
PG_SUPER_USER=postgres
PG_SUPER_USER_PASSWD=$PG_SUPER_USER
PG_SUPER_USER_HOME=`eval echo ~$PG_SUPER_USER`
PG_ADMIN_USER=admin                        # This will be editted in script.
PG_ADMIN_USER_PASSWD=pgpool                # This will be editted in script.
PGPORT=5432                                # This will be editted in script.
PGDATA=$PG_SUPER_USER_HOME/data            # This will be editted in script.
ARCHIVE_DIR=$PG_SUPER_USER_HOME/archivedir # This will be editted in script.
INITDB_OPTION="--no-locale -E UTF8"

# other
USE_WATCHDOG=0                 # This will be editted in script.
NOBODY_SBIN=/var/private/nobody/sbin
PID_FILE_DIR=/var/run/pgpool/
PGPOOL_LOG_DIR=/var/log/pgpool
INSTALL_ONLY=0
SKIPPED=0

NODE_NO=0
THIS_HOST=""
DEST_HOST=""

TEMP_FILE_RPM=/tmp/rpmcheck
TEMP_CONF=/tmp/pgpool_conf_template
BOLD=$'\e[0;30;1m'
SPAN_END=$'\e[m'
PROMPT="[input] "

# -------------------------------------------------------------------
# config
# -------------------------------------------------------------------

function decho()
{
    if [ $SH_DEBUG -eq 1 ]; then
        echo $1
    fi
}

function writeValList()
{
    cat <<EOT > editted/install_val_list
MODE=$MODE
PG_ADMIN_USER=$PG_ADMIN_USER
PG_ADMIN_USER_PASSWD=$PG_ADMIN_USER_PASSWD
PGPORT=$PGPORT
PGDATA=$PGDATA
ARCHIVE_DIR=$ARCHIVE_DIR
USE_WATCHDOG=$USE_WATCHDOG
EOT
}

function readValList()
{
    source editted/install_val_list
}

# -------------------------------------------------------------------
# check
# -------------------------------------------------------------------

function ynQuestion()
{
    local _QUESTION=$1

    while :; do
        echo -n $PROMPT "$_QUESTION (yes/no): "
        read REPLY
        case $REPLY in
            [yY] | [yY][eE][sS])
                return 0
                ;;
            [nN] | [nN][oO])
                return 1
                ;;
        esac
    done
}

function hasPackage()
{
    PACKAGE=$1
    PACKAGE_NAME_SHOWN=$2
    SILENT=0
    if [ $# -eq 3 ]; then
        SILENT=$3
    fi

    egrep -q $PACKAGE $TEMP_FILE_RPM
    if [ $? -ne 0 ]; then
        if [ $SILENT -eq 0 ]; then
            echo
            echo "Please install $PACKAGE_NAME_SHOWN."
        fi
        return 1
    fi

    return 0
}

function checkEnv()
{
    # OS
    if [ -f /etc/redhat-release ]; then
        if grep -q "release 6" /etc/redhat-release; then
        distribution=rhel6
        else
           echo "Your platform is not supported."
           return 1
        fi
    fi

    # pgpool-II
    hasPackage $PGPOOL_SOFTWARE_NAME $PGPOOL_SOFTWARE_NAME 1
    if [ $? -eq 0 ]; then
        echo
        echo "pgpool-II $MAJOR_VERSION is already installed."
        return 1
    fi

    # other
    hasPackage "postgresql${PG_VER}-server" "PostgreSQL (postgresql${PG_VER}-server)"
    if [ $? -ne 0 ]; then return 1; fi
    hasPackage "postgresql${PG_VER}" "PostgreSQL (postgresql${PG_VER})"
    if [ $? -ne 0 ]; then return 1; fi
    hasPackage "httpd" "Apache (httpd)"
    if [ $? -ne 0 ]; then return 1; fi
    hasPackage "php-pgsql" "PHP (php-pgsql)"
    if [ $? -ne 0 ]; then return 1; fi
    hasPackage "php-mbstring" "PHP (php-mbstring)"
    if [ $? -ne 0 ]; then return 1; fi
    hasPackage "php-[45]" "PHP"
    if [ $? -ne 0 ]; then return 1; fi

    # root
    if [ $(id -un) != root ]; then
        echo
        echo "Must be installed as root."
        return 1
    fi

    return 0
}

# -------------------------------------------------------------------
# Nodes
# -------------------------------------------------------------------

function fixNodes()
{
    # node 0
    while :; do
        echo -n $PROMPT "Specify node 0's hostname or IP address : "
        read REPLY
        if [ "$REPLY" != "" ]; then
            if [ $REPLY = localhost ]; then
                echo "NG. Please input actual hostname or IP address."
            else
                NODE0_HOST=$REPLY
                break
            fi
        fi
    done

    # node 1
    while :; do
        echo -n $PROMPT "Specify node 1's hostname or IP address : "
        read REPLY
        if [ "$REPLY" != "" ]; then
            if [ $REPLY = localhost ]; then
                echo "NG. Please input actual hostname or IP address."
            else
                NODE1_HOST=$REPLY
                break
            fi
        fi
    done

}

function fixNetmask()
{
    echo -n $PROMPT "Specify netmask (default: $NETMASK) : "
    read REPLY
    if [ "$REPLY" != "" ]; then
        NETMASK=$REPLY
    fi
}

# -------------------------------------------------------------------
# pgpool.conf
# -------------------------------------------------------------------

# get input from console and check the value if needed.
# input values is set to $RTN
function checkInputParam()
{
    local _PARAM=$1
    local _DESCRIPTION=$2
    local _DEFAULT=$3

    while :; do
        echo -n $PROMPT $_DESCRIPTION
        if [ "$_DEFAULT" != "" ]; then
            echo -n " (default: $_DEFAULT)"
        fi
        echo -n " : "

        read _INPUT_VAL
        if [ "$_INPUT_VAL" = "" ]; then
            if [ "$_DEFAULT" != "" ]; then
                RTN=$_DEFAULT
                return 0
            fi
        else
            case $_PARAM in
                backend_hostname*)
                    if [ $_INPUT_VAL = "localhost" ]; then
                        echo "NG. Please specify the host name."
                    else
                        RTN=$_INPUT_VAL
                        return 0
                    fi
                    ;;
                *)
                    RTN=$_INPUT_VAL
                    return 0
                    ;;
            esac
        fi
    done

    return 0
}

# get input from console and write it to pgpool.conf
function setPgpoolParam()
{
    local _PARAM=$1
    local _DESCRIPTION=$2
    local _DEFAULT=""

    if [ $# -eq 3 ]; then
        _DEFAULT=$3
    fi

    checkInputParam $_PARAM "$_DESCRIPTION" $_DEFAULT
    _NEWVAL=$RTN

    case $_PARAM in
        delegate_IP|backend_data_directory*|heartbeat_*)
            _NEWVAL="'$_NEWVAL'"
            ;;
    esac

    writePgpoolParam $_PARAM $_NEWVAL
}

function getPgpoolParam()
{
    local _PARAM=$1

    RTN=`grep $_PARAM editted/pgpool.conf | sed -e "s/$_PARAM \+= \+\(.*\)/\1/" \
         | sed -e "s/^'\(.*\)'$/\1/"`
}

# rewrite param in pgpool.conf to new values
function writePgpoolParam()
{
    local _PARAM=$1
    local _NEWVAL=$2

    sed -i "s|^[#]*$_PARAM[ ]*=.*$|$_PARAM = $_NEWVAL|" editted/pgpool.conf

    decho "    [$_PARAM] $_NEWVAL"
}

function setBackend()
{
    local _NODE_NUM=$1

    echo "* backend $_NODE_NUM"

    if [ $_NODE_NUM -eq 0 ]; then
        writePgpoolParam "backend_hostname$_NODE_NUM" "'$NODE0_HOST'"

        setPgpoolParam "backend_port$_NODE_NUM"           "Port number"    $PGPORT
        PGPORT=$RTN
        setPgpoolParam "backend_data_directory$_NODE_NUM" "Data directory" $PGDATA
        PGDATA=$RTN

    else
        echo "    Set the same values as backend 0."
        writePgpoolParam "backend_hostname$_NODE_NUM"       "'$NODE1_HOST'"
        writePgpoolParam "backend_port$_NODE_NUM"           $PGPORT
        writePgpoolParam "backend_data_directory$_NODE_NUM" "'$PGDATA'"
    fi
    writePgpoolParam "backend_weight$_NODE_NUM" 1
}

function rewriteWatchdog()
{
    echo "Rewrite watchdog configuration."
    echo
    writePgpoolParam wd_hostname            "'$THIS_HOST'"
    writePgpoolParam other_pgpool_hostname0 "'$DEST_HOST'"

    getPgpoolParam wd_lifecheck_method
    if [ $RTN = "heartbeat" ]; then
        writePgpoolParam heartbeat_destination0 "'$DEST_HOST'"
    fi
}

function setWatchdog()
{
    writePgpoolParam use_watchdog on
    setPgpoolParam   delegate_IP  "delegate IP address"

    # config of this watchdog
    decho
    decho "[[ config of this watchdog ]]"
    writePgpoolParam wd_hostname            "'$THIS_HOST'"
    writePgpoolParam wd_port                $WATCHDOG_PORT

    # config of another pgpool with watchdog
    decho
    decho "[[ config of another pgpool with watchdog ]]"
    writePgpoolParam other_pgpool_hostname0 "'$DEST_HOST'"
    writePgpoolParam other_pgpool_port0     $PGPOOL_PORT
    writePgpoolParam other_wd_port0         $WATCHDOG_PORT

    # lifecheck
    decho
    decho "[[ lifecheck ]]"
    while :; do
        echo $PROMPT "method of watchdog lifecheck (heartbeat / query)"
        read REPLY
        case $REPLY in
            heartbeat|h)
                REPLY=heartbeat
                break
                ;;
            query|q)
                REPLY=query
                break
                ;;
        esac
    done
    writePgpoolParam wd_lifecheck_method $REPLY
    WATCHDOG_METHOD=$REPLY

    case $WATCHDOG_METHOD in
        heartbeat)
            writePgpoolParam heartbeat_device0 "''"
            writePgpoolParam heartbeat_destination0 "'$DEST_HOST'"
            writePgpoolParam heartbeat_destination_port0 "9694"
        ;;

        query)
            writePgpoolParam wd_lifecheck_user     "'$PG_SUPER_USER'"
            writePgpoolParam wd_lifecheck_password "'$PG_SUPER_USER_PASSWD'"
            ;;
    esac

    # command path (to use commands with setuid bit)
    writePgpoolParam ifconfig_path "'$NOBODY_SBIN'"
    writePgpoolParam arping_path   "'$NOBODY_SBIN'"

    # configure netmask for VIP
    getPgpoolParam if_up_cmd
    local _VAL=`echo $RTN | sed -e "s/255.255.255.0/$NETMASK/"`
    writePgpoolParam if_up_cmd "'$_VAL'"

    echo
}

function doConfigPgpool()
{
    cp templates/pgpool.conf.sample editted/pgpool.conf
    ynQuestion "Do you edit pgpool.conf now?"
    if [ $? -ne 0 ]; then
        SKIPPED=1
        return
    fi

    local _STEPS=6

    echo
    echo $BOLD"----------------------------------------------------------------------"$SPAN_END
    echo $BOLD"Configuration for pgpool-II ... "$SPAN_END
    echo $BOLD"----------------------------------------------------------------------"$SPAN_END
    echo

    # -------------------------------------------------------------------
    # [1] general
    # -------------------------------------------------------------------

    decho "[1/$_STEPS] general"
    decho

    writePgpoolParam listen_addresses "'*'"
    writePgpoolParam port 9999
    writePgpoolParam pcp_port 9898

    # -------------------------------------------------------------------
    # [2] mode
    # -------------------------------------------------------------------

    decho
    decho "[2/$_STEPS] replication"
    while :; do
        echo $PROMPT "Which replication mode do you use? (native/ stream)"
        echo

        echo "    native: native replication mode"
        echo "    stream: master slave mode with streaming replication"

        read REPLY
        case $REPLY in
        native)
            writePgpoolParam replication_mode on
            MODE="replication"
            break
            ;;
        stream)
            writePgpoolParam master_slave_mode     on
            writePgpoolParam master_slave_sub_mode "'stream'"
            MODE="stream"
            break
            ;;
        esac
    done

    ynQuestion "Do you use load balancing?"
    if [ $? -eq 0 ]; then
        writePgpoolParam load_balance_mode on
    fi

    ynQuestion "Do you use on memory query cache with shared memory?"
    if [ $? -eq 0 ]; then
        writePgpoolParam memory_cache_enabled on
        writePgpoolParam memqcache_method     "'shmem'"
        writePgpoolParam memqcache_oiddir     "'$PGPOOL_LOG_DIR/oiddir'"
    fi

    # -------------------------------------------------------------------
    # [3] watchdog
    # -------------------------------------------------------------------

    decho
    decho "[3/$_STEPS] watchdog"

    ynQuestion "Do you use watchdog?"
    if [ $? -eq 0 ]; then
        USE_WATCHDOG=1
        setWatchdog
    fi

    # -------------------------------------------------------------------
    # [4] backend
    # -------------------------------------------------------------------

    decho
    decho "[4/$_STEPS] backend"
    decho

    setBackend 0
    echo
    setBackend 1

    # -------------------------------------------------------------------
    # [5] health check user
    # -------------------------------------------------------------------

    decho
    decho "[5/$_STEPS] health check"
    writePgpoolParam health_check_user     "'$PG_SUPER_USER'"
    writePgpoolParam health_check_password "'$PG_SUPER_USER_PASSWD'"
    writePgpoolParam health_check_period   10

    # -------------------------------------------------------------------
    # [6] fail over
    # -------------------------------------------------------------------

    decho
    decho "[6/$_STEPS] fail over & online recovery"
    writePgpoolParam recovery_user "'$PG_SUPER_USER'"
    writePgpoolParam recovery_password "'$PG_SUPER_USER_PASSWD'"
    if [ $MODE = "stream" ]; then
        writePgpoolParam recovery_1st_stage_command "'basebackup-stream.sh'"
        writePgpoolParam failover_command  "'$PGPOOL_CONF_DIR/failover.sh %d %h %p %D %m %M %H %P %r %R'"
        writePgpoolParam sr_check_user     "'$PG_SUPER_USER'"
        writePgpoolParam sr_check_password "'$PG_SUPER_USER_PASSWD'"
    else
        writePgpoolParam recovery_1st_stage_command "'basebackup-replication.sh'"
        writePgpoolParam recovery_2nd_stage_command "'pgpool_recovery_pitr'"
    fi

    echo
    echo $BOLD"----------------------------------------------------------------------"$SPAN_END
    echo "                                                              ... end."
}

# -------------------------------------------------------------------
# pgpoolAdmin's conf
# -------------------------------------------------------------------

function writeAdminParam()
{
    local _PARAM=$1
    local _NEWVAL=$2

    sed -i "s|define('$_PARAM',[ ]*'.*');|define('$_PARAM', '$_NEWVAL');|" editted/pgmgt.conf.php
}

function doConfigAdmin()
{
    cp templates/pgmgt.conf.php editted/
    ynQuestion "Do you edit pgmgt.conf.php now?"
    if [ $? -ne 0 ]; then
        return
    fi

    echo
    echo $BOLD"----------------------------------------------------------------------"$SPAN_END
    echo $BOLD"Configuration for pgpoolAdmin ..."$SPAN_END
    echo $BOLD"----------------------------------------------------------------------"$SPAN_END
    echo

    while :; do
        echo $PROMPT "Which language do you use? (en/fr/ja/zh_cn)"

        read REPLY
        case $REPLY in
        en | fr | ja | zh_cn )
            writeAdminParam "_PGPOOL2_LANG" $REPLY
            break
            ;;
        esac
    done

    writeAdminParam _PGPOOL2_VERSION             $MAJOR_VERSION
    writeAdminParam _PGPOOL2_CONFIG_FILE         $PGPOOL_CONF_DIR/pgpool.conf
    writeAdminParam _PGPOOL2_PASSWORD_FILE       $PGPOOL_CONF_DIR/pcp.conf
    writeAdminParam _PGPOOL2_COMMAND             $PGPOOL_BIN_DIR/pgpool
    writeAdminParam _PGPOOL2_LOG_FILE            $PGPOOL_LOG_DIR/pgpool.log
    writeAdminParam _PGPOOL2_CMD_OPTION_N        1
    writeAdminParam _PGPOOL2_PCP_DIR             $PGPOOL_BIN_DIR
    writeAdminParam _PGPOOL2_STATUS_REFRESH_TIME 5

    echo
    echo $BOLD"----------------------------------------------------------------------"$SPAN_END
    echo "                                                              ... end."
}

# -------------------------------------------------------------------
# pcp.conf
# -------------------------------------------------------------------

function doConfigPcp()
{
    cp templates/pcp.conf.sample editted/pcp.conf
    ynQuestion "Do you edit pcp.conf now?"
    if [ $? -ne 0 ]; then
        return
    fi

    echo
    echo $BOLD"----------------------------------------------------------------------"$SPAN_END
    echo $BOLD"Configuration for PCP ..."$SPAN_END
    echo $BOLD"----------------------------------------------------------------------"$SPAN_END
    echo

    while :; do
        echo -n $PROMPT "username for pgpoolAdmin: "
        read PG_ADMIN_USER
        if [ "$PG_ADMIN_USER" != "" ]; then
            break;
        fi
    done

    while :; do
        echo -n $PROMPT "this user's password: "
        read PG_ADMIN_USER_PASSWD
        if [ "$PG_ADMIN_USER_PASSWD" != "" ]; then
            break;
        fi
    done

    echo
    echo $BOLD"----------------------------------------------------------------------"$SPAN_END
    echo "                                                              ... end."
}

# -------------------------------------------------------------------
# postgresql.conf
# -------------------------------------------------------------------

function doConfigPostgres()
{
    local _STEPS=5

    cp templates/postgresql.conf editted/postgresql.conf
    ynQuestion "Do you edit postgresql.conf now?"
    if [ $? -ne 0 ]; then
        SKIPPED=1
        return
    fi

    echo
    echo $BOLD"----------------------------------------------------------------------"$SPAN_END
    echo $BOLD"Configuration for PostgreSQL ..."$SPAN_END
    echo $BOLD"----------------------------------------------------------------------"$SPAN_END
    echo

    # [1] hot standby
    decho
    decho "[1/$_STEPS] WAL archive"
    setPostgresParam "archive_command" "the directory where to archive a logfile segment" $ARCHIVE_DIR
    writePostgresParam listen_addresses "'*'"
    writePostgresParam archive_mode     on
    #writePostgresParam archive_command  "'cp %p $ARCHIVE_DIR/%f </dev/null'"

    if [ $MODE = "stream" ]; then
        writePostgresParam wal_level       hot_standby
        writePostgresParam max_wal_senders 2
        writePostgresParam hot_standby     on
    else
        writePostgresParam wal_level archive
    fi

    # [2] log
    decho
    decho "[2/$_STEPS] log"
    writePostgresParam logging_collector        on
    writePostgresParam log_filename             "'%A.log'"
    writePostgresParam log_line_prefix          "'%t [%p-%l] '"
    writePostgresParam log_truncate_on_rotation on

    # -------------------------------------------------------------------
    # [3] custom vartiable
    # -------------------------------------------------------------------

    decho "[4/$_STEPS] custom variable for pgpool_recovery extension"
    writePostgresParam pgpool.pg_ctl "'$PGHOME/bin/pg_ctl'"

    # -------------------------------------------------------------------
    # [4] pg_hba.conf
    # -------------------------------------------------------------------

    NODE0_MASK=""
    NODE1_MASK=""

    # If hostname, netmask isn't necessary.
    if expr "$NODE0_HOST" : "^[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}$" > /dev/null; then
        NODE0_MASK=$NETMASK
    fi
    if expr "$NODE1_HOST" : "^[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}$" > /dev/null; then
        NODE1_MASK=$NETMASK
    fi

    decho
    decho "[5/$_STEPS] authorization"
    cp templates/pg_hba.conf editted/pg_hba.conf
    ed -s editted/pg_hba.conf > /dev/null 2>&1 <<EOT
/^#local *replication/s/^#//p
/^#host *replication/s/^#//p
/^#host *replication/s/^#//p
w
q
EOT
    echo "host    replication     $PG_SUPER_USER     $NODE0_HOST    $NODE0_MASK    trust" >> editted/pg_hba.conf
    echo "host    replication     $PG_SUPER_USER     $NODE1_HOST    $NODE1_MASK    trust" >> editted/pg_hba.conf
    echo "host    all             $PG_SUPER_USER     $NODE0_HOST    $NODE0_MASK    trust" >> editted/pg_hba.conf
    echo "host    all             $PG_SUPER_USER     $NODE1_HOST    $NODE1_MASK    trust" >> editted/pg_hba.conf
    echo "host    all             $PG_ADMIN_USER     $NODE0_HOST    $NODE0_MASK    trust" >> editted/pg_hba.conf
    echo "host    all             $PG_ADMIN_USER     $NODE1_HOST    $NODE1_MASK    trust" >> editted/pg_hba.conf

    echo
    echo $BOLD"----------------------------------------------------------------------"$SPAN_END
    echo "                                                              ... end."
}

function setPostgresParam()
{
    local _PARAM=$1
    local _DESCRIPTION=$2
    local _DEFAULT=""

    if [ $# -eq 3 ]; then
        _DEFAULT=$3
    fi

    checkInputParam $_PARAM "$_DESCRIPTION" $_DEFAULT
    _NEWVAL=$RTN

    if [ "$_PARAM" = "archive_command" ]; then
        writePostgresParam archive_command  "'cp %p $ARCHIVE_DIR/%f </dev/null'"
    else
        writePostgresParam $_PARAM $_NEWVAL
    fi
}

function writePostgresParam()
{
    local _PARAM=$1
    local _NEWVAL=$2

    echo "$_PARAM = $_NEWVAL" >> editted/postgresql.conf

    decho "    [$_PARAM] $_NEWVAL"
}

# -------------------------------------------------------------------
# scripts
# -------------------------------------------------------------------

function createConfForScript()
{
    local _SCRIPT=config_for_script
    echo
    echo -n "Create config for failover and online recovery ... "
    cp -f templates/$_SCRIPT editted/
    ed -s editted/$_SCRIPT <<EOT
/__PGHOME__/s@__PGHOME__@$PGHOME@
/__NODE0_HOST__/s@__NODE0_HOST__@$NODE0_HOST@
/__NODE1_HOST__/s@__NODE1_HOST__@$NODE1_HOST@
/__NODE0_PORT__/s@__NODE0_PORT__@$PGPORT@
/__NODE1_PORT__/s@__NODE1_PORT__@$PGPORT@
/__NODE0_DIR__/s@__NODE0_DIR__@$PGDATA@
/__NODE1_DIR__/s@__NODE1_DIR__@$PGDATA@
/__PGSUPERUSER__/s@__PGSUPERUSER__@$PG_SUPER_USER@
/__ARCHIVE_DIR__/s@__ARCHIVE_DIR__@$ARCHIVE_DIR@
/__PGPOOL_LOG_DIR__/s@__PGPOOL_LOG_DIR__@$PGPOOL_LOG_DIR@
w
q
EOT
    echo "OK."
}

function createRecoveryConf()
{
    local _SCRIPT=recovery.conf

    echo -n "create recovery.conf ... ".
    cp -f templates/$_SCRIPT editted/
    ed -s editted/$_SCRIPT <<EOT
/__PGPOOL_PORT__/s@__PGPOOL_PORT__@$PGPOOL_PORT@
/__REPLI_USER__/s@__REPLI_USER__@$PG_SUPER_USER@
/__ARCHIVE_DIR__/s@__ARCHIVE_DIR__@$ARCHIVE_DIR@
w
q
EOT
    echo "OK."
}

function chownToApache()
{
    local _TARGET=$1
    chown $APACHE_USER:$APACHE_USER $_TARGET
}

function copySbin()
{
    if [ ! -e $NOBODY_SBIN ]; then
        mkdir -p $NOBODY_SBIN
    fi
    chownToApache $NOBODY_SBIN
    chmod 700 $NOBODY_SBIN

    cp /sbin/ifconfig $NOBODY_SBIN
    cp /sbin/arping $NOBODY_SBIN
    chmod 4755 $NOBODY_SBIN/ifconfig
    chmod 4755 $NOBODY_SBIN/arping
}

function sshWithoutPass()
{
    local _THIS_USER=$1
    local _REMOTE_HOST=$2
    local _HOME=`eval echo ~$_THIS_USER`
    local _SSH_DIR=$_HOME/.ssh

    if [ ! -e $_SSH_DIR/id_rsa ]; then
        rm $_SSH_DIR/id_rsa* >/dev/null 2>&1
        su - $_THIS_USER -c "ssh-keygen -q -t rsa -P '' -f $_SSH_DIR/id_rsa << EOF

EOF"
    fi

    if [ $? -ne 0 ]; then return 1; fi

    ssh-copy-id -i $_SSH_DIR/id_rsa.pub $PG_SUPER_USER@$_REMOTE_HOST > /dev/null 2>&1
    if [ $? -ne 0 ]; then return 1; fi

    su - $_THIS_USER -c "ssh -o StrictHostKeyChecking=no $PG_SUPER_USER@$_REMOTE_HOST exit" > /dev/null 2>&1
    if [ $? -ne 0 ]; then return 1; fi

    return 0
}

function makeApacheLoginable()
{
    local _APACHE_HOME="/home/$APACHE_USER"

    if [ ! -e $_APACHE_HOME ]; then
        mkdir $_APACHE_HOME -m 700
    fi
    chownToApache $_APACHE_HOME

    su - $APACHE_USER -c "exit" > /dev/null
    if [ $? -ne 0 ]; then
        usermod -d $_APACHE_HOME -s /bin/bash $APACHE_USER > /dev/null 2>&1
    fi
}

# -------------------------------------------------------------------
# Install
# -------------------------------------------------------------------

function doInstall()
{
    echo "* Install packages ... "
    rpm -ivh ${PACKAGE_FILES[@]}
    if [ $? -ne 0 ]; then
        echo "Failed."
        return 1
    fi
    echo
    echo "OK."
    return 0
}

# -------------------------------------------------------------------
# Regist functions
# -------------------------------------------------------------------

function doQueries()
{
    su - $PG_SUPER_USER -c "$PGHOME/bin/pg_ctl -D $PGDATA -w start" > /dev/null 2>&1

    echo -n "- create user: admin ... "
    $PGHOME/bin/psql -p $PGPORT -U $PG_SUPER_USER postgres \
        -c "CREATE USER $PG_ADMIN_USER PASSWORD '$PG_ADMIN_USER_PASSWD' SUPERUSER" >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo "OK."
    else
        echo "Failed."
        echo "    Please create the user \"$PG_ADMIN_USER\" manually. Continuing anyway."
    fi

    echo -n "- create extension: pgpool_regclass ... "
    $PGHOME/bin/psql -p $PGPORT -U $PG_SUPER_USER template1 \
        -c "CREATE EXTENSION pgpool_regclass; " > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo "OK."
    else
        echo "Failed."
        echo "    Please install pgpool_regclass() manually . Continuing anyway."
    fi

    echo -n "- create extension: pgpool_recovery ... "
    $PGHOME/bin/psql -p $PGPORT -U $PG_SUPER_USER template1 \
        -c "CREATE EXTENSION pgpool_recovery;" > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo "OK."
    else
        echo "Failed."
        echo "    Please install pgpool_recovery() manually . Continuing anyway."
    fi

    su - $PG_SUPER_USER -c "$PGHOME/bin/pg_ctl stop -D $PGDATA" > /dev/null 2>&1
}

# ===================================================================
# main
# ===================================================================

# -------------------------------------------------------------------
# [1] check
# -------------------------------------------------------------------

# 1. check environment
echo -n "check for installation ..."

rpm -qa | grep -E "${PGPOOL_SOFTWARE_NAME}|postgresql${PG_VER}|httpd|php|php-mbstring|php-pgsql" > $TEMP_FILE_RPM
checkEnv
if [ $? -ne 0 ]; then
    rm -f $TEMP_FILE_RPM
    exit 1
fi
rm -f $TEMP_FILE_RPM

echo "OK."
echo

# -------------------------------------------------------------------

# 2. licence agreement
echo $BOLD"================================================================="$SPAN_END
cat COPYING
echo $BOLD"================================================================="$SPAN_END
ynQuestion "Do you accept the end user software license agreement?"
if [ $? -ne 0 ]; then exit 1; fi

# -------------------------------------------------------------------

# 3. editing config?
ynQuestion "Do you edit configs? If no, install will start right now without configuration."
if [ $? -ne 0 ]; then
    doInstall
    if [ $? -eq 0 ]; then
        echo "Completed!"
        echo "All configuration should be done manually."
        exit 0
    else
        exit 1
    fi
fi

# -------------------------------------------------------------------
# [2] Node information
# -------------------------------------------------------------------

echo
echo $BOLD"================================================================="$SPAN_END
echo Configuration
echo $BOLD"================================================================="$SPAN_END

echo
echo $BOLD"----------------------------------------------------------------------"$SPAN_END
echo $BOLD"Configuring Host, User, SSH ..."$SPAN_END
echo $BOLD"----------------------------------------------------------------------"$SPAN_END

echo
echo "* Node information"

# 1. IP addresses og both nodes
echo
echo "Two-node cluster (node 0 and node 1) is assumed."
fixNodes

# -------------------------------------------------------------------

# 2. which node is this, node 0 or 1?
echo
echo "Which node is this?"
echo "    If this is node 0, a database cluster is created by initdb after installation."
echo "    Otherwise if this is node 1, the configurations on node 0 is reused."
ynQuestion "Is this node 0?"
if [ $? -eq 0 ]; then
    NODE_NO=0
    THIS_HOST=$NODE0_HOST
    DEST_HOST=$NODE1_HOST
    echo "OK. This is node 0."
else
    echo "This is node 1."
    ynQuestion "Installation on node 0 has finished already?"
    if [ $? -eq 0 ]; then
        NODE_NO=1
        THIS_HOST=$NODE1_HOST
        DEST_HOST=$NODE0_HOST
        echo "OK. "
    else
        echo "Please install pgpool-II to node0 at first."
        exit 1
    fi
fi

# -------------------------------------------------------------------

# 3. The user of PostgreSQL
echo
echo "* Check the user $PG_SUPER_USER"
echo

id $PG_SUPER_USER >/dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "The user $PG_SUPER_USER doesn't exist on $THIS_HOST. Creating ..."
    useradd $PG_SUPER_USER > /dev/null 2>&1
    echo $PG_SUPER_USER:$PG_SUPER_USER_PASSWD | chpasswd
    if [ $? -eq 0 ]; then
        echo "OK."
        echo "    Created \"$PG_SUPER_USER\" user with password \"$PG_SUPER_USER_PASSWD\"."
        echo "    You must change the password after install."
    else
        echo "Failed."
        exit 1
    fi
else
    echo "The user $PG_SUPER_USER exists on $THIS_HOST (this host) ... OK."
fi

ynQuestion "The user $PG_SUPER_USER exists on $DEST_HOST (another host)?"
if [ $? -ne 0 ]; then
    echo "Please create $PG_SUPER_USER on $DEST_HOST before the installation."
    exit 1
fi

# -------------------------------------------------------------------

# 3. password-less access over ssh
echo
echo "* Setup password-less access over ssh"
echo
echo "Try ssh: $PG_SUPER_USER@$THIS_HOST (this host) -> $PG_SUPER_USER@$DEST_HOST (another host)"
sshWithoutPass $PG_SUPER_USER $DEST_HOST
if [ $? -ne 0 ]; then
    echo "Failed to ssh $PG_SUPER_USER@$DEST_HOST (another host)."
    exit 1
else
    echo "OK."
fi

echo
echo "Try ssh: $APACHE_USER@$THIS_HOST (this host) -> $PG_SUPER_USER@$DEST_HOST (another host)"
makeApacheLoginable
if [ $? -ne 0 ]; then
    echo "Failed to make apache loginable. For configuring apache user, httpd must be stopped."
    exit 1
fi
sshWithoutPass $APACHE_USER $DEST_HOST
if [ $? -ne 0 ]; then
    echo "Failed to ssh $PG_SUPER_USER@$DEST_HOST."
    exit 1
else
    echo "OK."
fi

echo
echo "Try ssh: $APACHE_USER@$THIS_HOST (this host) -> $PG_SUPER_USER@$THIS_HOST (this host)"
makeApacheLoginable
if [ $? -ne 0 ]; then
    echo "Failed to make apache loginable. For configuring apache user, httpd must be stopped."
    exit 1
fi
sshWithoutPass $APACHE_USER $THIS_HOST
if [ $? -ne 0 ]; then
    echo "Failed to ssh $PG_SUPER_USER@$THIS_HOST."
    exit 1
else
    echo "OK."
fi

# -------------------------------------------------------------------

# 4. netmask (in only node 0)
if [ $NODE_NO -eq 0 ]; then
    echo

    fixNetmask
fi

echo
echo $BOLD"----------------------------------------------------------------------"$SPAN_END
echo "                                                              ... end."

# -------------------------------------------------------------------
# [3] Editting conf files
# -------------------------------------------------------------------

# create temporary config files in editted directory
rm -rf editted/
mkdir editted/

# node 0
if [ $NODE_NO -eq 0 ]; then
    doConfigPcp
    doConfigPgpool
    doConfigAdmin
    doConfigPostgres

    if [ $SKIPPED -eq 0 ]; then
        createConfForScript
    fi

    if [ $MODE = "stream" ]; then
        createRecoveryConf
        cp templates/basebackup-stream.sh editted/
        cp templates/failover.sh editted/
    else
        cp templates/basebackup-replication.sh editted/
        cp templates/pgpool_recovery_pitr editted/
    fi
    cp templates/pgpool_remote_start editted/

    echo
    echo "Save configuration information for installation on node 1."
    writeValList
    rm -rf $TEMP_CONF
    mkdir $TEMP_CONF
    cp editted/* $TEMP_CONF
    chown -R postgres:postgres $TEMP_CONF
    chmod 700 $TEMP_CONF
    chmod 600 $TEMP_CONF/*

# node 1
else
    echo
    echo "Copy configuration information from node 0."
    su $PG_SUPER_USER -c "rm -rf $TEMP_CONF; mkdir $TEMP_CONF; scp $PG_SUPER_USER@$DEST_HOST:$TEMP_CONF/* $TEMP_CONF"
    cp $TEMP_CONF/* editted
    rm -rf $TEMP_CONF
    readValList
    if [ $USE_WATCHDOG -eq 1 ]; then
        rewriteWatchdog
    fi
fi

# -------------------------------------------------------------------
# [3-1] install RPMs
# -------------------------------------------------------------------

echo
echo $BOLD"================================================================="$SPAN_END
echo "* Installation"
echo $BOLD"================================================================="$SPAN_END

echo
echo "* Setup pgpool-II"

# 1. install pgpool-II and pgpoolAdmin
ynQuestion "Do you install pgpool really?"
if [ $? -ne 0 ]; then
    exit 1
fi

echo
doInstall
if [ $? -ne 0 ]; then exit 1; fi

# -------------------------------------------------------------------

# 2. rewrite pgpool.conf
echo
echo -n "- rewrite pgpool.conf ... "
cp editted/pgpool.conf $PGPOOL_CONF_DIR
if [ $? -ne 0 ]; then
    echo "Failed."
    echo "    Please put pgpool.conf in the current directory to $PGPOOL_CONF_DIR manually. Continuing anyway."
else
    echo "OK."
fi

# -------------------------------------------------------------------

# 3. rewrite pcp.conf
echo -n "- rewrite pcp.conf ... "
if [ "$PG_ADMIN_USER_PASSWD" = "" ]; then
    echo "Skipped."
else
    MD5_PASSWD=`$PGPOOL_BIN_DIR/pg_md5 $PG_ADMIN_USER_PASSWD`
    echo "${PG_ADMIN_USER}:${MD5_PASSWD}" >> editted/pcp.conf

    cp editted/pcp.conf $PGPOOL_CONF_DIR
    if [ $? -ne 0 ]; then
        echo "Failed."
        echo "    Please put pgpool.conf in the current directory to $PGPOOL_CONF_DIR manually. Continuing anyway."
    else
        echo "OK."
    fi
fi

# -------------------------------------------------------------------

# 4. setuid for watchdog
echo -n "- setup watchdog ... "
if [ $USE_WATCHDOG -eq 1 ]; then
    copySbin
    echo "OK."
else
    echo "Skipped."
fi

# -------------------------------------------------------------------
# [3-3] Setup pgpoolAdmin
# -------------------------------------------------------------------

echo
echo "* Setup pgpoolAdmin"

# 1. put conf file
echo -n "- rewrite pgmgt.conf.php ... "
cp editted/pgmgt.conf.php $ADMIN_DIR/conf/
if [ $? -ne 0 ]; then
    echo "Failed."
    echo "    Please put pgmgt.conf.php in the current directory to $ADMIN_DIR/conf manually. Continuing anyway."
else
    echo "OK."
    chmod 666 $ADMIN_DIR/conf/pgmgt.conf.php
fi

# -------------------------------------------------------------------

# 2. create log directories
echo "- create log directries ... OK."
if [ ! -d $PID_FILE_DIR ]; then
    mkdir $PID_FILE_DIR
fi
if [ ! -d $PGPOOL_LOG_DIR ]; then
    mkdir $PGPOOL_LOG_DIR
fi

# -------------------------------------------------------------------

# 3. set permission
chownToApache $PID_FILE_DIR
chownToApache $PGPOOL_LOG_DIR
chmod 777 $PGPOOL_LOG_DIR
chmod 777 $ADMIN_DIR/templates_c/

# -------------------------------------------------------------------
# [4-1] initdb and put config files
# -------------------------------------------------------------------

INITDB_OK=0
if [ $NODE_NO -eq 0 ]; then
    echo
    echo "* Create node 0 (localhost) 's database cluster"
    echo

    echo -n "- initdb ... "

    # 1. stop existing PostgreSQL
    chown $PG_SUPER_USER $PGHOME
    su - $PG_SUPER_USER -c "$PGHOME/bin/pg_ctl -D $PGDATA stop -m immediate" > /dev/null 2>&1
    rm -rf $PGDATA
    mkdir $PGDATA
    chown $PG_SUPER_USER:$PG_SUPER_USER $PGDATA

    # -------------------------------------------------------------------

    # 2. initdb
    su - $PG_SUPER_USER -c "$PGHOME/bin/initdb -D $PGDATA $INITDB_OPTION" > /dev/null 2>&1

    if [ $? -ne 0 ]; then
        echo "Failed."
        echo "    Please initdb manually like \"$PGHOME/bin/initdb -D $PGDATA $INITDB_OPTION\"".

    # -------------------------------------------------------------------

    else
        echo "OK."
        INITDB_OK=1

        # 3. put conf files
        echo "- rewrite postgresql.conf and pcp.conf ... OK."
        cp editted/postgresql.conf $PGDATA
        cp editted/pg_hba.conf $PGDATA

        # -------------------------------------------------------------------

        # 4. setup online recovery
        echo "- put scripts for online recovery ... OK."
        cp editted/basebackup*.sh $PGDATA
        cp templates/pgpool_remote_start $PGDATA
        chmod 755 $PGDATA/*.sh $PGDATA/pgpool_remote_start
        chown $PG_SUPER_USER:$PG_SUPER_USER $PGDATA/*

        if [ $MODE = "stream" ]; then
            cp editted/recovery.conf $PGDATA/recovery.done
        else
            cp editted/pgpool_recovery_pitr $PGDATA
            chmod 755 $PGDATA/pgpool_recovery_pitr
        fi
    fi
fi

# -------------------------------------------------------------------
# [4-2] Setup database
# -------------------------------------------------------------------

echo
echo "* Setup database"

# 1. setup failover
echo "- put scripts for failover ... OK."
cp editted/config_for_script $PGPOOL_CONF_DIR
if [ $MODE = "stream" ]; then
    cp templates/failover*.sh $PGPOOL_CONF_DIR
    chmod 755 $PGPOOL_CONF_DIR/failover.sh
fi

# -------------------------------------------------------------------

# 2. set permissions to conf files
chownToApache "$PGPOOL_CONF_DIR/*.conf"
chownToApache $PGPOOL_CONF_DIR
chmod 444 $PGPOOL_CONF_DIR/config_for_script
chmod 600 $PGPOOL_CONF_DIR/*.conf
chmod 755 $PGPOOL_CONF_DIR


# -------------------------------------------------------------------

# 3. setup WAL archiving
echo "- create archive directory ... OK."
mkdir -p $ARCHIVE_DIR
chown $PG_SUPER_USER:$PG_SUPER_USER $ARCHIVE_DIR

if [ $INITDB_OK -eq 1 ]; then
    doQueries
fi

# -------------------------------------------------------------------
# [4-3] Cleaning
# -------------------------------------------------------------------

if [ $NODE_NO -eq 1 ]; then
    echo
    echo "* Cleaning"
    echo "Remove configuration information on node 0."
    su $PG_SUPER_USER -c "ssh $PG_SUPER_USER@$DEST_HOST rm -rf $TEMP_CONF"
fi

# -------------------------------------------------------------------

echo
echo "----------------------------------------------------------------------"
echo
echo "Completed!"
echo
echo "   * See pgpoolAdmin."
echo "         http://$THIS_HOST/pgpoolAdmin/"
echo
if [ $NODE_NO -eq 0 ]; then
echo "   * Start PostgreSQL by the user $PG_SUPER_USER"
echo "         ex.) $PGHOME/bin/pg_ctl start -D $PGDATA"
echo
echo "   * Start pgppool from pgpoolAdmin"
echo
echo "   * Install pgpool-II to $NODE1_HOST using install.sh"
echo
else
echo "   * Do online recovery of node 1 from pgpoolAdmin"
fi

exit 0
