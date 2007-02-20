%define short_name	pgpool-II

Summary:	Pgpool is a connection pooling/replication server for PostgreSQL
Name:		postgresql-%{short_name}
Version:	1.0.2
Release:	1%{?dist}
License:	BSD
Group:		Applications/Databases
URL:		http://pgpool.projects.PostgreSQL.org/pgpool-II/en
Source0:	http://pgfoundry.org/frs/download.php/1258/%{short_name}-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
Requires:	%{name}-libs = %{version}

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

%package libs
Summary:	The libraries for pgpool-II
Group:		Development/Libraries

%description libs
The pgpool-II-libs package provides the essential shared libraries for pgpool
program or its interface. You will need to install this package
to use pgpool-II .

%package devel
Summary:	The development headers for pgpool-II
Group:		Development/Libraries
Requires:	%{name} = %{version}

%description devel
The pgpool-II-devel package provides the essential development headers for 
pgpool program or its interface. 

%prep
%setup -q -n %{short_name}-%{version}

%build
CFLAGS="${CFLAGS:-%optflags}" ; export CFLAGS
CXXFLAGS="${CXXFLAGS:-%optflags}" ; export CXXFLAGS

%configure 

make %{?smp_flags}

%install
rm -rf %{buildroot}
make DESTDIR=%{buildroot} install
install -d %{buildroot}%{_includedir}/
install -d %{buildroot}/%{_datadir}/%{name}
install -m 644 pgpool.conf.sample %{buildroot}%{_sysconfdir}/
install -m 644 pgpool.8 %{buildroot}%{_mandir}/man8/
install -m 644 pcp/pcp.h  %{buildroot}%{_includedir}/
install -m 644 pool_type.h  %{buildroot}%{_includedir}/
install -m 644 pcp/.libs/libpcp.a  %{buildroot}%{_libdir}/
install -m 644 pcp/.libs/libpcp.la  %{buildroot}%{_libdir}/
install -m 644 sql/system_db.sql %{buildroot}/%{_datadir}/%{name}/system_db.sql
rm -f  %{buildroot}/%{_datadir}/system_db.sql

%clean
rm -rf %{buildroot}

%post libs -p /sbin/ldconfig
%postun libs -p /sbin/ldconfig

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
%{_datadir}/%{name}/system_db.sql
%config(noreplace) %{_sysconfdir}/pgpool.conf.sample
%config(noreplace) %{_sysconfdir}/pcp.conf.sample

%files libs
%defattr(-,root,root)
%{_libdir}/libpcp.la
%{_libdir}/libpcp.so.*

%files devel
%defattr(-,root,root)
%{_includedir}/pcp.h
%{_libdir}/libpcp.so
%{_libdir}/libpcp.a
%{_includedir}/pool_type.h

%changelog
* Tue Feb 20 2007 Devrim Gunduz <devrim@commandprompt.com> 1.0.2-1
- Update to 1.0.2-1
- Fix various build problems and prepared for Fedora Submission

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

