# How to build RPM:
#   rpmbuild -ba pgpool.spec --define="pgpool_version 3.4.0" --define="pg_version 93" --define="pghome /usr/pgsql-9.3" --define="dist .rhel6"
#
# expecting RPM name are:
#   pgpool-II-pg{pg_version}-{pgpool_version}-{rel}pgdg.rhel{v}.{arch}.rpm
#   pgpool-II-pg{pg_version}-devel-{pgpool_version}-{rel}pgdg.rhel{v}.{arch}.rpm
#   pgpool-II-pg{pg_version}-extensions-{pgpool_version}-{rel}pgdg.rhel{v}.{arch}.rpm
#   pgpool-II-pg{pg_version}-{pgpool_version}-{rel}pgdg.rhel{v}.src.rpm

%global short_name  pgpool-II

%if 0%{?rhel} && 0%{?rhel} <= 6
  %global systemd_enabled 0
%else
  %global systemd_enabled 1
%endif

%global _varrundir %{_localstatedir}/run/pgpool

Summary:        Pgpool is a connection pooling/replication server for PostgreSQL
Name:           pgpool-II-pg%{pg_version}
Version:        %{pgpool_version}
Release:        1pgdg%{?dist}
License:        BSD
Group:          Applications/Databases
Vendor:         Pgpool Global Development Group
URL:            http://www.pgppol.net/
Source0:        pgpool-II-%{version}.tar.gz
Source1:        pgpool.init
Source2:        pgpool.sysconfig
Source3:        pgpool.service
#Patch1:         pgpool.conf.sample.patch
Patch2:         pgpool-II-head.patch
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires:  postgresql%{pg_version}-devel pam-devel openssl-devel libmemcached-devel
%if %{systemd_enabled}
BuildRequires:    systemd
Requires:         systemd
Requires(post):   systemd-sysv
Requires(post):   systemd
Requires(preun):  systemd
Requires(postun): systemd
%else
Requires(post):   chkconfig
Requires(preun):  chkconfig
Requires(preun):  initscripts
Requires(postun): initscripts
%endif
Obsoletes:      postgresql-pgpool

# original pgpool archive name
%define archive_name pgpool-II-%{version}

%description
pgpool-II is a inherited project of pgpool (to classify from
pgpool-II, it is sometimes called as pgpool-I). For those of
you not familiar with pgpool-I, it is a multi-functional
middle ware for PostgreSQL that features connection pooling,
replication and load balancing functions. pgpool-I allows a
user to connect at most two PostgreSQL servers for higher
availability or for higher search performance compared to a
single PostgreSQL server.

pgpool-II, on the other hand, allows multiple PostgreSQL
servers (DB nodes) to be connected, which enables queries
to be executed simultaneously on all servers. In other words,
it enables "parallel query" processing. Also, pgpool-II can
be started as pgpool-I by changing configuration parameters.
pgpool-II that is executed in pgpool-I mode enables multiple
DB nodes to be connected, which was not possible in pgpool-I.

%package devel
Summary:     The development files for pgpool-II
Group:       Development/Libraries
Requires:    %{name} = %{version}

%description devel
Development headers and libraries for pgpool-II.

%package extensions
Summary:     Postgersql extensions for pgpool-II
Group:       Applications/Databases
%description extensions
Postgresql extensions libraries and sql files for pgpool-II.

%prep
%setup -q -n %{archive_name}
#%patch1 -p0
%patch2 -p1

%build
%configure --with-pgsql=%{pghome} \
           --disable-static \
           --with-pam \
           --with-openssl \
           --with-memcached=%{_usr} \
           --disable-rpath \
           --sysconfdir=%{_sysconfdir}/%{short_name}/

make %{?_smp_mflags}

%install
rm -rf %{buildroot}

# make pgpool-II
export PATH=%{pghome}/bin:$PATH
make %{?_smp_mflags} DESTDIR=%{buildroot} install

# install to PostgreSQL
make %{?_smp_mflags} DESTDIR=%{buildroot} install -C src/sql/pgpool-recovery
%if %{pg_version} <= 93
# From PostgreSQL 9.4 pgpool-regclass.so is not needed anymore
# because 9.4 or later has to_regclass.
make %{?_smp_mflags} DESTDIR=%{buildroot} install -C src/sql/pgpool-regclass
%endif

install -d %{buildroot}%{_datadir}/%{short_name}
install -d %{buildroot}%{_sysconfdir}/%{short_name}
mv %{buildroot}%{_sysconfdir}/%{short_name}/pcp.conf.sample %{buildroot}%{_sysconfdir}/%{short_name}/pcp.conf
mv %{buildroot}%{_sysconfdir}/%{short_name}/pgpool.conf.sample %{buildroot}%{_sysconfdir}/%{short_name}/pgpool.conf
mv %{buildroot}%{_sysconfdir}/%{short_name}/pool_hba.conf.sample %{buildroot}%{_sysconfdir}/%{short_name}/pool_hba.conf

%if %{systemd_enabled}
install -d %{buildroot}%{_unitdir}
install -m 644 %{SOURCE3} %{buildroot}%{_unitdir}/pgpool.service

mkdir -p %{buildroot}%{_tmpfilesdir}
cat > %{buildroot}%{_tmpfilesdir}/%{name}.conf <<EOF
d %{_varrundir} 0755 root root -
EOF
%else
install -d %{buildroot}%{_initrddir}
install -m 755 %{SOURCE1} %{buildroot}%{_initrddir}/pgpool
%endif

install -d %{buildroot}%{_sysconfdir}/sysconfig
install -m 644 %{SOURCE2} %{buildroot}%{_sysconfdir}/sysconfig/pgpool

# nuke libtool archive and static lib
rm -f %{buildroot}%{_libdir}/libpcp.{a,la}

%clean
rm -rf %{buildroot}

%post
/sbin/ldconfig

%if %{systemd_enabled}
%systemd_post pgpool.service
%else
/sbin/chkconfig --add pgpool
%endif
%tmpfiles_create

%preun
%if %{systemd_enabled}
%systemd_preun pgpool.service
%else
if [ $1 = 0 ] ; then
  /sbin/service pgpool condstop >/dev/null 2>&1
  chkconfig --del pgpool
fi
%endif

%postun
/sbin/ldconfig

%if %{systemd_enabled}
%systemd_postun_with_restart pgpool.service

%triggerun -- pgpool < 3.1-1
# Save the current service runlevel info
# User must manually run systemd-sysv-convert --apply pgpool
# to migrate them to systemd targets
/usr/bin/systemd-sysv-convert --save pgpool >/dev/null 2>&1 ||:

# Run these because the SysV package being removed won't do them
/sbin/chkconfig --del pgpool >/dev/null 2>&1 || :
/bin/systemctl try-restart pgpool.service >/dev/null 2>&1 || :

%else
if [ $1 -ge 1 ] ; then
  /sbin/service pgpool condrestart >/dev/null 2>&1 || :
fi
%endif

%files
%defattr(-,root,root,-)
%dir %{_datadir}/%{short_name}
%doc README TODO COPYING INSTALL AUTHORS ChangeLog NEWS doc/pgpool-en.html doc/pgpool-ja.html doc/pgpool.css doc/tutorial-en.html doc/tutorial-ja.html
%{_bindir}/pgpool
%{_bindir}/pcp_attach_node
%{_bindir}/pcp_detach_node
%{_bindir}/pcp_node_count
%{_bindir}/pcp_node_info
%{_bindir}/pcp_pool_status
%{_bindir}/pcp_proc_count
%{_bindir}/pcp_proc_info
%{_bindir}/pcp_promote_node
%{_bindir}/pcp_stop_pgpool
%{_bindir}/pcp_recovery_node
%{_bindir}/pcp_systemdb_info
%{_bindir}/pcp_watchdog_info
%{_bindir}/pg_md5
%{_mandir}/man8/pgpool*
%{_datadir}/%{short_name}/insert_lock.sql
%{_datadir}/%{short_name}/system_db.sql
%{_datadir}/%{short_name}/pgpool.pam
%{_sysconfdir}/%{short_name}/pgpool.conf.sample-master-slave
%{_sysconfdir}/%{short_name}/pgpool.conf.sample-replication
%{_sysconfdir}/%{short_name}/pgpool.conf.sample-stream
%{_libdir}/libpcp.so.*
%if %{systemd_enabled}
%ghost %{_varrundir}
%{_tmpfilesdir}/%{name}.conf
%{_unitdir}/pgpool.service
%else
%{_initrddir}/pgpool
%endif
%attr(764,root,root) %config(noreplace) %{_sysconfdir}/%{short_name}/*.conf
%config(noreplace) %{_sysconfdir}/sysconfig/pgpool

%files devel
%defattr(-,root,root,-)
%{_includedir}/libpcp_ext.h
%{_includedir}/pcp.h
%{_includedir}/pool_process_reporting.h
%{_includedir}/pool_type.h
%{_libdir}/libpcp.so

%files extensions
%defattr(-,root,root,-)
%{pghome}/share/extension/pgpool-recovery.sql
%{pghome}/share/extension/pgpool_recovery--1.1.sql
%{pghome}/share/extension/pgpool_recovery.control
%{pghome}/lib/pgpool-recovery.so
# From PostgreSQL 9.4 pgpool-regclass.so is not needed anymore
# because 9.4 or later has to_regclass.
%if %{pg_version} <= 93
  %{pghome}/share/extension/pgpool_regclass--1.0.sql
  %{pghome}/share/extension/pgpool_regclass.control
  %{pghome}/share/extension/pgpool-regclass.sql
  %{pghome}/lib/pgpool-regclass.so
%endif

%changelog
* Wed Jan 28 2015 Nozomi Anzai <anzai@sraoss.co.jp> 3.4.1
- Fix typo of %{_smp_mflags}
- Change to use systemd if it is available

* Sat Dec 20 2014 Tatsuo Ishii <ishii@sraoss.co.jp> 3.4.0-3
- Fix "error: Installed (but unpackaged) file(s) found"

* Fri Nov 21 2014 Tatsuo Ishii <ishii@sraoss.co.jp> 3.4.0-2
- Re-enable to apply difference from HEAD patch.

* Tue Nov 18 2014 Yugo Nagata <nagata@sraoss.co.jp> 3.4.0-2
- Rename RPM filename to include RHEL version no.

* Tue Nov 11 2014 Tatsuo Ishii <ishii@sraoss.co.jp> 3.4.0-2
- Add memcached support to configure.

* Tue Oct 21 2014 Tatsuo Ishii <ishii@sraoss.co.jp> 3.4beta2
- Adopt to PostgreSQL 9.4

* Thu Sep 25 2014 Tatsuo Ishii <ishii@sraoss.co.jp> 3.3.4-2
- Split pgpool_regclass and pgpool_recovery as a separate extention package.
- Fix wrong OpenSSL build option.

* Fri Sep 5 2014 Yugo Nagata <nagata@sraoss.co.jp> 3.3.4-1
- Update to 3.3.4

* Wed Jul 30 2014 Tatsuo Ishii <ishii@sraoss.co.jp> 3.3.3-4
- Add PATCH2 which is diff between 3.3.3 and 3.3-stable tree head.
- RPM expert said this is the better way.

* Sat May 10 2014 Tatsuo Ishii <ishii@sraoss.co.jp> 3.3.3-3
- Use 3.3-stable tree head

* Sun May 4 2014 Tatsuo Ishii <ishii@sraoss.co.jp> 3.3.3-2
- Fix configure option
- Add openssl support

* Tue Nov 26 2013 Nozomi Anzai <anzai@sraoss.co.jp> 3.3.1-1
- Improved to specify the versions of pgool-II and PostgreSQL

* Mon May 13 2013 Nozomi Anzai <anzai@sraoss.co.jp> 3.3.0-1
- Update to 3.3.0
- Change to install pgpool-recovery, pgpool-regclass to PostgreSQL

* Tue Nov 3 2009 Devrim Gunduz <devrim@CommandPrompt.com> 2.2.5-3
- Remove init script from all runlevels before uninstall. Per #RH Bugzilla
  532177

* Mon Oct 5 2009 Devrim Gunduz <devrim@CommandPrompt.com> 2.2.5-2
- Add 2 new docs, per Tatsuo.

* Sun Oct 4 2009 Devrim Gunduz <devrim@CommandPrompt.com> 2.2.5-1
- Update to 2.2.5, for various fixes described at
  http://lists.pgfoundry.org/pipermail/pgpool-general/2009-October/002188.html
- Re-apply a fix for Red Hat Bugzilla #442372

* Wed Sep 9 2009 Devrim Gunduz <devrim@CommandPrompt.com> 2.2.4-1
- Update to 2.2.4

* Wed May 6 2009 Devrim Gunduz <devrim@CommandPrompt.com> 2.2.2-1
- Update to 2.2.2

* Sun Mar 1 2009 Devrim Gunduz <devrim@CommandPrompt.com> 2.2-1
- Update to 2.2
- Fix URL
- Own /usr/share/pgpool-II directory.
- Fix pid file path in init script, per    pgcore #81.
- Fix spec file -- we don't use short_name macro in pgcore spec file.
- Create pgpool pid file directory, per pgcore #81.
- Fix stop/start routines, also improve init script a bit.
- Install conf files to a new directory (/etc/pgpool-II), and get rid
  of sample conf files.

* Fri Aug 8 2008 Devrim Gunduz <devrim@CommandPrompt.com> 2.1-1
- Update to 2.1
- Removed temp patch #4.

* Sun Jan 13 2008 Devrim Gunduz <devrim@CommandPrompt.com> 2.0.1-1
- Update to 2.0.1
- Add a temp patch that will disappear in 2.0.2

* Fri Oct 5 2007 Devrim Gunduz <devrim@CommandPrompt.com> 1.2.1-1
- Update to 1.2.1

* Wed Aug 29 2007 Devrim Gunduz <devrim@CommandPrompt.com> 1.2-5
- Chmod sysconfig/pgpool to 644, not 755. Per BZ review.
- Run chkconfig --add pgpool during %%post.

* Thu Aug 16 2007 Devrim Gunduz <devrim@CommandPrompt.com> 1.2-4
- Fixed the directory name where sample conf files and sql files
  are installed.

* Sun Aug 5 2007 Devrim Gunduz <devrim@CommandPrompt.com> 1.2-3
- Added a patch for sample conf file to use Fedora defaults

* Sun Aug 5 2007 Devrim Gunduz <devrim@CommandPrompt.com> 1.2-2
- Added an init script for pgpool
- Added /etc/sysconfig/pgpool

* Wed Aug 1 2007 Devrim Gunduz <devrim@CommandPrompt.com> 1.2-1
- Update to 1.2

* Fri Jun 15 2007 Devrim Gunduz <devrim@CommandPrompt.com> 1.1.1-1
- Update to 1.1.1

* Sat Jun 2 2007 Devrim Gunduz <devrim@CommandPrompt.com> 1.1-1
- Update to 1.1
- added --disable-rpath configure parameter.
- Chowned sample conf files, so that they can work with pgpoolAdmin.

* Sun Apr 22 2007 Devrim Gunduz <devrim@CommandPrompt.com> 1.0.2-4
- Added postgresql-devel as BR, per bugzilla review.
- Added --disable-static flan, per bugzilla review.
- Removed superfluous manual file installs, per bugzilla review.

* Sun Apr 22 2007 Devrim Gunduz <devrim@CommandPrompt.com> 1.0.2-3
- Rebuilt for the correct tarball
- Fixed man8 file ownership, per bugzilla review #229321

* Tue Feb 20 2007 Jarod Wilson <jwilson@redhat.com> 1.0.2-2
- Create proper devel package, drop -libs package
- Nuke rpath
- Don't install libtool archive and static lib
- Clean up %%configure line
- Use proper %%_smp_mflags
- Install config files properly, without .sample on the end
- Preserve timestamps on header files

* Tue Feb 20 2007 Devrim Gunduz <devrim@commandprompt.com> 1.0.2-1
- Update to 1.0.2-1

* Mon Oct 02 2006 Devrim Gunduz <devrim@commandprompt.com> 1.0.1-5
- Rebuilt

* Mon Oct 02 2006 Devrim Gunduz <devrim@commandprompt.com> 1.0.1-4
- Added -libs and RPM
- Fix .so link problem
- Cosmetic changes to spec file

* Wed Sep 27 2006 - Devrim GUNDUZ <devrim@commandprompt.com> 1.0.1-3
- Fix spec, per Yoshiyuki Asaba

* Tue Sep 26 2006 - Devrim GUNDUZ <devrim@commandprompt.com> 1.0.1-2
- Fixed rpmlint errors
- Fixed download url
- Added ldconfig for .so files

* Thu Sep 21 2006 - David Fetter <david@fetter.org> 1.0.1-1
- Initial build pgpool-II 1.0.1 for PgPool Global Development Group
