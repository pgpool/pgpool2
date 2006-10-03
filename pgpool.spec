Summary:	Pgpool is a connection pooling/replication server for PostgreSQL
Name:		pgpool-II
Version:	1.0.1
Release:	5%{?dist}
License:	BSD
Vendor:		PgPool Global Development Group
Group:		Applications/Databases
URL:		http://pgpool.projects.PostgreSQL.org
Source0:	http://pgfoundry.org/frs/download.php/1083/%{name}-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
Patch1:		%{name}-configure.patch

%description
pgpool-II is a connection pooling/replication server for PostgreSQL.
pgpool-II runs between PostgreSQL's clients(front ends) and servers
(backends). A PostgreSQL client can connect to pgpool-II as if it were
a standard PostgreSQL server.

%package devel
Summary:	The devel headers of  pgpool-II
Group:		Development/Libraries
Requires:	%{name}-%{version}

%description devel
The pgpool-II-devel package provides the essential shared libraries for pgpool 
program or its interface. You will need to install this package
to use pgpool-II .

%prep
%setup -q -n %{name}-%{version}
%patch1 -p1

%build
CFLAGS="${CFLAGS:-%optflags}" ; export CFLAGS
CXXFLAGS="${CXXFLAGS:-%optflags}" ; export CXXFLAGS

%configure --bindir=%{_bindir} --sysconfdir=%{_sysconfdir} --mandir=%{_mandir} --libdir=%{_libdir} --with-pgsql-includedir=%{_includedir}/pgsql --with-pgsql-lib=%{_libdir}/pgsql

make %{?smp_flags}

%install
rm -rf %{buildroot}
make DESTDIR=%{buildroot} install
install -m 644 pgpool.conf.sample %{buildroot}%{_sysconfdir}/
install -m 644 pgpool.8 %{buildroot}%{_mandir}/man8/
install -m 644 pcp/pcp.h  %{buildroot}%{_includedir}/
#install -m 755 pcp/.libs/libpcp.so* %{buildroot}%{_libdir}/
install -m 644 pool_type.h  %{buildroot}%{_includedir}/
install -m 644 pcp/.libs/libpcp.a  %{buildroot}%{_libdir}/
install -m 644 pcp/.libs/libpcp.la  %{buildroot}%{_libdir}/

%clean
rm -rf %{buildroot}

%post -p /sbin/ldconfig
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
%{_bindir}/pcp_stop_pgpool
%{_bindir}/pcp_systemdb_info
%{_bindir}/pg_md5
%{_mandir}/man8/*
%{_datadir}/system_db.sql
%{_libdir}/libpcp.so.0*
%config(noreplace) %{_sysconfdir}/pgpool.conf.sample
%config(noreplace) %{_sysconfdir}/pcp.conf.sample

%files devel
%defattr(-,root,root)
%{_libdir}/libpcp.la
%{_libdir}/libpcp.a
%{_includedir}/pcp.h
%{_includedir}/pool_type.h
%{_libdir}/libpcp.so

%changelog
* Mon Oct 02 2006 Devrim Gunduz <devrim@commandprompt.com> 1.0.1-5
- Rebuilt

* Mon Oct 02 2006 Devrim Gunduz <devrim@commandprompt.com> 1.0.1-4
- Added -libs and -devel RPM
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
