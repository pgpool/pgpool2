#! /bin/bash
PATH=/bin:/sbin:/usr/bin:/usr/sbin

MJ_VERSION=3.3
PKG_NAME_FOR_RPM=pgpool
PKG_NAME_FOR_SHOW=pgpool-II

NOBODY_SBIN=/var/private/nobody/sbin
PID_FILE_DIR=/var/run/pgpool/
PGPOOL_LOG_DIR=/var/log/pgpool
PGPOOL_CONF_DIR=/etc/pgpool-II
ADMIN_DIR=/var/www/html/pgpoolAdmin

rpm -qa | grep -q $PKG_NAME_FOR_RPM
if [ $? -ne 0 ]; then
    echo "$PKG_NAME_FOR_SHOW $MJ_VERSION is not installed."
    exit 1
fi

if [ $(id -un) != root ]; then
    echo "Must be uninstalled as root."
    exit 1
fi

while :; do
    echo -n "Do you uninstall $PKG_NAME_FOR_SHOW $MJ_VERSION (yes/no): "
    read reply
    case $reply in
    [yY] | [yY][eE][sS])
        break
        ;;
    [nN] | [nN][oO])
        echo "Uninstallation is canceled."
        exit 1
        ;;
    esac
done
echo

killall pgpool
echo "Uninstalling packages..."
rpm -qa | grep ${PKG_NAME_FOR_RPM} | xargs rpm -ev
if [ $? -ne 0 ]; then
    echo "Failed to uninstall packages."
    exit 1
fi
echo

rm -rf $NOBODY_SBIN
rm -rf $PID_FILE_DIR
rm -rf $PGPOOL_LOG_DIR
rm -rf $PGPOOL_CONF_DIR
rm -rf $ADMIN_DIR

echo "Uninstallation is completed successfully."

exit 0
