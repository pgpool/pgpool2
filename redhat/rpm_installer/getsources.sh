#!/bin/sh

username=`whoami`
rpm_dir=$HOME/rpm
work_dir=`pwd`/work

# ---------------------------------------------------------------------
# configuration
# ---------------------------------------------------------------------

git_dir=$HOME/git

## pgpool-II
pgpool_src_dir=$git_dir/pgpool2
pgpool_tarball_dir=$pgpool_src_dir
pgpool_version=3.3.2

## pgpoolAdmin
admin_src_dir=$git_dir/pgpooladmin
admin_tarball_dir=$admin_src_dir/tools
admin_version=3.3.0

## postgresql92
bin_path=/usr/pgsql-9.2/bin
export PATH=$bin_path:$PATH

echo "* Setup starts."
echo

# ---------------------------------------------------------------------
# setup for rpmbuild
# ---------------------------------------------------------------------

echo -n "    Setup for rpmbuild ... "

mkdir -p $rpm_dir/{BUILD,RPMS,SOURCES,SPECS,SRPMS}

cat > ~/.rpmmacros <<EOT
%_topdir /home/$username/rpm
%_mandir %_prefix/man
%debug_package %{nil}
%dist .pgdg
%pgdg 1
EOT

echo "done."

# ---------------------------------------------------------------------
# get sources for pgpool to rpmbuild
# ---------------------------------------------------------------------

rm -rf $work_dir
mkdir $work_dir
cd $work_dir

echo -n "    Get sources to rpmbuild ... "

# each spec files
cp -f $pgpool_src_dir/pgpool.spec     $work_dir
cp -f $admin_src_dir/pgpoolAdmin.spec $work_dir

# pgpool-II-*.tar.gz
cd $pgpool_src_dir
make dist > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "Failed."
    exit
fi
if [ ! -f $pgpool_tarball_dir/pgpool-II-$pgpool_version.tar.gz ]; then
    echo "$pgpool_tarball_dir/pgpool-II-$pgpool_version.tar.gz not found."
	exit
fi
mv pgpool-II-$pgpool_version.tar.gz $rpm_dir/SOURCES

# pgpoolAdmin-*.tar.gz
if [ ! -f $admin_tarball_dir/pgpoolAdmin-$admin_version.tar.gz ]; then
    echo "$admin_tarball_dir/pgpoolAdmin-$admin_version.tar.gz not found."
	exit
fi
cp $admin_tarball_dir/pgpoolAdmin-$admin_version.tar.gz $rpm_dir/SOURCES

# pgpool.conf.sample.patch, pgpool.init, pgpool.sysconfig
cp -f $pgpool_src_dir/redhat/pgpool.conf.sample.patch $rpm_dir/SOURCES/
cp -f $pgpool_src_dir/redhat/pgpool.init              $rpm_dir/SOURCES/
cp -f $pgpool_src_dir/redhat/pgpool.sysconfig         $rpm_dir/SOURCES/

echo "done."

# ---------------------------------------------------------------------
# get sources for pgpool for installer
# ---------------------------------------------------------------------

echo -n "    Get sources for installer ... "

# scripts
mkdir -p $work_dir/installer
cp -f $pgpool_src_dir/redhat/rpm_installer/install.sh   $work_dir/installer/
cp -f $pgpool_src_dir/redhat/rpm_installer/uninstall.sh $work_dir/installer/
cp -f $pgpool_src_dir/COPYING                           $work_dir/installer/

# each conf files
mkdir -p $work_dir/installer/templates
cp -f $pgpool_src_dir/pgpool.conf.sample $work_dir/installer/templates/
cp -f $pgpool_src_dir/pcp.conf.sample    $work_dir/installer/templates/
cp -f $admin_src_dir/conf/pgmgt.conf.php $work_dir/installer/templates/

initdb -D data_tmp --no-locale -E UTF8 > /dev/null 2>&1
cp -f data_tmp/postgresql.conf  $work_dir/installer/templates/
sed -i "s/\(replication \+\)$USER/\1postgres/g" data_tmp/pg_hba.conf
cp -f data_tmp/pg_hba.conf      $work_dir/installer/templates/
rm -rf data_tmp

cp -f $pgpool_src_dir/redhat/rpm_installer/config_for_script      $work_dir/installer/templates/
cp -f $pgpool_src_dir/redhat/rpm_installer/basebackup-*.sh        $work_dir/installer/templates/
cp -f $pgpool_src_dir/redhat/rpm_installer/failover.sh            $work_dir/installer/templates/
cp -f $pgpool_src_dir/redhat/rpm_installer/pgpool_recovery_pitr   $work_dir/installer/templates/
cp -f $pgpool_src_dir/redhat/rpm_installer/pgpool_remote_start    $work_dir/installer/templates/
cp -f $pgpool_src_dir/redhat/rpm_installer/recovery.conf          $work_dir/installer/templates/

echo "done."

echo
echo "* Setup Finished. See \"work\" directory."

# ---------------------------------------------------------------------

echo
echo "* Next ..."
echo
echo "  - rpmbuild -ba work/pgpool.spec"
echo "  - rpmbuild -ba work/pgpoolAdmin.spec"
echo "  - move ~/rpm/RPMS/../pgpool*.rpm (except for *.src.rpm) to work/installer/"
echo "  - create tar ball from work/installer/"
echo
