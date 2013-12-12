#!/bin/sh

username=`whoami`
rpm_dir=$HOME/rpm
work_dir=`pwd`/work

# ---------------------------------------------------------------------
# configuration
# ---------------------------------------------------------------------

git_dir=$HOME/git

## pgpool-II
pgpool_src_dir=${git_dir}/pgpool2
pgpool_tarball_dir=${pgpool_src_dir}
pgpool_version=3.3.2

## pgpoolAdmin
admin_src_dir=${git_dir}/pgpooladmin
admin_tarball_dir=${admin_src_dir}/tools
admin_version=3.3.0

## postgresql92
pg_home=/usr/pgsql-9.2
pg_version=92
export PATH=${pg_home}/bin:$PATH

echo "* Setup starts."
echo

# ---------------------------------------------------------------------
# setup for rpmbuild
# ---------------------------------------------------------------------

echo -n "    Setup for rpmbuild ... "

mkdir -p ${rpm_dir}/{BUILD,RPMS,SOURCES,SPECS,SRPMS}

cat > ~/.rpmmacros <<EOT
%_topdir /home/${username}/rpm
%_mandir %_prefix/man
%debug_package %{nil}
%dist .pgdg
%pgdg 1
EOT

echo "done."

# ---------------------------------------------------------------------
# get sources for pgpool to rpmbuild
# ---------------------------------------------------------------------

rm -rf ${work_dir}
mkdir ${work_dir}
cd ${work_dir}

echo -n "    Get sources to rpmbuild ... "

# each spec files
cp -f ${pgpool_src_dir}/pgpool.spec     ${work_dir}
cp -f ${admin_src_dir}/pgpoolAdmin.spec ${work_dir}

# pgpool-II-*.tar.gz
if [ ! -f ${pgpool_tarball_dir}/pgpool-II-${pgpool_version}.tar.gz ]; then
    cd ${pgpool_src_dir}
    ./configure --with-pgsql=${pg_home}
    make
    make dist > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "Failed."
        exit
    fi
    if [ ! -f ${pgpool_tarball_dir}/pgpool-II-${pgpool_version}.tar.gz ]; then
        echo "${pgpool_tarball_dir}/pgpool-II-${pgpool_version}.tar.gz not found."
        exit
    fi
    mv pgpool-II-${pgpool_version}.tar.gz ${rpm_dir}/SOURCES
fi

# pgpoolAdmin-*.tar.gz
if [ ! -f ${admin_tarball_dir}/pgpoolAdmin-${admin_version}.tar.gz ]; then
    echo "${admin_tarball_dir}/pgpoolAdmin-${admin_version}.tar.gz not found."
    exit
fi
cp ${admin_tarball_dir}/pgpoolAdmin-${admin_version}.tar.gz ${rpm_dir}/SOURCES

# pgpool.conf.sample.patch, pgpool.init, pgpool.sysconfig
cp -f ${pgpool_src_dir}/redhat/pgpool.conf.sample.patch ${rpm_dir}/SOURCES/
cp -f ${pgpool_src_dir}/redhat/pgpool.init              ${rpm_dir}/SOURCES/
cp -f ${pgpool_src_dir}/redhat/pgpool.sysconfig         ${rpm_dir}/SOURCES/

echo "done."

# ---------------------------------------------------------------------
# get sources for pgpool for installer
# ---------------------------------------------------------------------

echo -n "    Get sources for installer ... "

installer_dir=${work_dir}/installer-pg${pg_version}-${pgpool_version}

# scripts
mkdir -p ${installer_dir}
cp -f ${pgpool_src_dir}/redhat/rpm_installer/install.sh   ${installer_dir}/
cp -f ${pgpool_src_dir}/redhat/rpm_installer/uninstall.sh ${installer_dir}/
cp -f ${pgpool_src_dir}/COPYING                           ${installer_dir}/

# each conf files
mkdir -p ${installer_dir}/templates
cp -f ${pgpool_src_dir}/pgpool.conf.sample ${installer_dir}/templates/
cp -f ${pgpool_src_dir}/pcp.conf.sample    ${installer_dir}/templates/
cp -f ${admin_src_dir}/conf/pgmgt.conf.php ${installer_dir}/templates/

initdb -D data_tmp --no-locale -E UTF8 > /dev/null 2>&1
cp -f data_tmp/postgresql.conf  ${installer_dir}/templates/
sed -i "s/\(replication \+\)$USER/\1postgres/g" data_tmp/pg_hba.conf
cp -f data_tmp/pg_hba.conf      ${installer_dir}/templates/
rm -rf data_tmp

cp -f ${pgpool_src_dir}/redhat/rpm_installer/config_for_script      ${installer_dir}/templates/
cp -f ${pgpool_src_dir}/redhat/rpm_installer/basebackup-*.sh        ${installer_dir}/templates/
cp -f ${pgpool_src_dir}/redhat/rpm_installer/failover.sh            ${installer_dir}/templates/
cp -f ${pgpool_src_dir}/redhat/rpm_installer/pgpool_recovery_pitr   ${installer_dir}/templates/
cp -f ${pgpool_src_dir}/redhat/rpm_installer/pgpool_remote_start    ${installer_dir}/templates/
cp -f ${pgpool_src_dir}/redhat/rpm_installer/recovery.conf          ${installer_dir}/templates/

echo "done."

echo
echo "* Setup Finished. See \"work\" directory."

# ---------------------------------------------------------------------

echo
echo "* Next ..."
echo
echo "  - rpmbuild -ba work/pgpool.spec --define\"pgpool_version 3.3.2\" --define=\"pg_version 93\" --define=\"pghome /usr/pgsql-9.3\""
echo "  - rpmbuild -ba work/pgpoolAdmin.spec"
echo "  - move ~/rpm/RPMS/../pgpool*.rpm (except for *.src.rpm) to work/installer-pg${pg_version}/"
echo "  - create tar ball from work/installer-pg${pg_version}/"
echo
