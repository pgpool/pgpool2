%define short_name	pgpool-II

Summary:	Pgpool is a connection pooling/replication server for PostgreSQL
Name:		postgresql-%{short_name}
Version:	2.2
Release:	1%{?dist}
License:	BSD
Group:		Applications/Databases
URL:		http://pgpool.projects.PostgreSQL.org/pgpool-II/en
Source0:	http://pgfoundry.org/frs/download.php/1726/%{short_name}-%{version}.tar.gz
Source1:        pgpool.init
Source2:        pgpool.sysconfig
Patch1:		pgpool.conf.sample.patch
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires:	postgresql-devel pam-devel

Obsoletes:	postgresql-pgpool

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
Summary:	The  development files for pgpool-II
Group:		Development/Libraries
Requires:	%{name} = %{version}-%{release}

%description devel
Development headers and libraries for pgpool-II.

%prep
%setup -q -n %{short_name}-%{version}
%patch1 -p0

%build
%configure --with-pgsql-includedir=%{_includedir}/pgsql --with-pgsql-lib=%{_libdir}/pgsql --disable-static --with-pam --disable-rpath

make %{?_smp_flags}

%install
rm -rf %{buildroot}
make %{?_smp_flags} DESTDIR=%{buildroot} install
install -d %{buildroot}%{_datadir}/%{short_name}
mv %{buildroot}/%{_sysconfdir}/*.conf.sample %{buildroot}%{_datadir}/%{short_name}
install -d %{buildroot}%{_initrddir}
install -m 755 %{SOURCE1} %{buildroot}%{_initrddir}/pgpool
install -d %{buildroot}%{_sysconfdir}/sysconfig
install -m 644 %{SOURCE2} %{buildroot}%{_sysconfdir}/sysconfig/pgpool

# nuke libtool archive and static lib
rm -f %{buildroot}%{_libdir}/libpcp.{a,la}

%clean
rm -rf %{buildroot}

%post 
/sbin/ldconfig
chkconfig --add pgpool

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%doc README README.euc_jp TODO COPYING INSTALL AUTHORS ChangeLog NEWS doc/pgpool-en.html doc/pgpool-ja.html doc/pgpool.css doc/tutorial-en.html doc/tutorial-ja.html
%{_bindir}/pgpool
%{_bindir}/pcp_attach_node
%{_bindir}/pcp_detach_node
%{_bindir}/pcp_node_count
%{_bindir}/pcp_node_info
%{_bindir}/pcp_proc_count
%{_bindir}/pcp_proc_info
%{_bindir}/pcp_recovery_node
%{_bindir}/pcp_stop_pgpool
%{_bindir}/pcp_systemdb_info
%{_bindir}/pg_md5
%{_mandir}/man8/pgpool*
%{_datadir}/%{short_name}/system_db.sql
%{_libdir}/libpcp.so.*
%attr(764,root,apache) %{_datadir}/%{short_name}/*.conf.sample
%{_datadir}/%{short_name}/pgpool.pam
%{_initrddir}/pgpool
%config(noreplace) %{_sysconfdir}/sysconfig/pgpool

%files devel
%defattr(-,root,root,-)
%{_includedir}/pcp.h
%{_includedir}/pool_type.h
%{_libdir}/libpcp.so

%changelog
* Tue Feb 17 2009 Devrim Gunduz <devrim@CommandPrompt.com> 2.2-1
- Update to 2.2 (Download URL is broken)

* Fri Apr 11 2008 Devrim Gunduz <devrim@CommandPrompt.com> 2.1-0.2.beta2
- Fix Requires: issue, per #442021 (Alex Lancaster)

* Sun Apr 6 2008 Devrim Gunduz <devrim@CommandPrompt.com> 2.1-beta2
- Update to 2.1 beta2

* Mon Feb 18 2008 Fedora Release Engineering <rel-eng@fedoraproject.org> - 2.0.1-3.1
- Autorebuild for GCC 4.3

* Mon Jan 21 2008 Devrim GUNDUZ <devrim@commandprompt.com> 2.0.1-2.1
- Rebuilt against PostgreSQL 8.3

* Sat Jan 19 2008 Devrim Gunduz <devrim@CommandPrompt.com> 2.0.1-2
- Fix Requires of -devel package, per bz#429436

* Sun Jan 13 2008 Devrim Gunduz <devrim@CommandPrompt.com> 2.0.1-1
- Update to 2.0.1
- Add a temp patch that will disappear in 2.0.2

* Tue Oct 23 2007 Devrim Gunduz <devrim@CommandPrompt.com> 1.3-1
- Update to 1.3

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

* Thu Apr 22 2007 Devrim Gunduz <devrim@CommandPrompt.com> 1.0.2-4
- Added postgresql-devel as BR, per bugzilla review.
- Added --disable-static flan, per bugzilla review.
- Removed superfluous manual file installs, per bugzilla review.

* Thu Apr 22 2007 Devrim Gunduz <devrim@CommandPrompt.com> 1.0.2-3
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

* Thu Sep 27 2006 - Devrim GUNDUZ <devrim@commandprompt.com> 1.0.1-3
- Fix spec, per Yoshiyuki Asaba

* Thu Sep 26 2006 - Devrim GUNDUZ <devrim@commandprompt.com> 1.0.1-2
- Fixed rpmlint errors
- Fixed download url
- Added ldconfig for .so files

* Thu Sep 21 2006 - David Fetter <david@fetter.org> 1.0.1-1
- Initial build pgpool-II 1.0.1 for PgPool Global Development Group

