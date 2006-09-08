Summary: Pgpool is a connection pooling/replication server for PostgreSQL.
Name: pgpool
Version: 3.0.0
Release: 1
License: Berkeley/BSD
Vendor:	 PgPool Global Development Group
Group: Applications/Databases
URL: http://pgfoundry.org/projects/pgpool/
Source0: pgpool-%{version}.tar.gz
Buildroot: %{_tmppath}/%{name}-%{version}-root

%description
Pgpool is a connection pooling/replication server for PostgreSQL.

%prep
%setup -q -n pgpool-%{version}
%build
CFLAGS="${CFLAGS:-%optflags}" ; export CFLAGS
CXXFLAGS="${CXXFLAGS:-%optflags}" ; export CXXFLAGS

./configure --bindir /usr/bin --sysconfdir=/etc --mandir=%{_mandir}

make 

%install
rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT install
install -m 755 pgpool $RPM_BUILD_ROOT/usr/bin/
install -m 755 pgpool.conf.sample $RPM_BUILD_ROOT/etc
install -m 744 pgpool.8 %{_mandir}/man8/

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%doc README README.euc_jp TODO COPYING INSTALL AUTHORS  ChangeLog NEWS 
%{_bindir}/pgpool
%{_sysconfdir}/pgpool.conf.sample
%{_mandir}/man8/*

%changelog
* Thu Feb 05 2006 - Devrim GUNDUZ <devrim@commandprompt.com> 3.0.0
- Update to 3.0.0 for PgPool Global Development Group

* Thu Feb 02 2006 - Devrim GUNDUZ <devrim@commandprompt.com> 2.7.2-1
- Update to 2.7.2

* Thu Jan 26 2006 - Devrim GUNDUZ <devrim@commandprompt.com> 2.7.1-1
- Update to 2.7.1

* Sun Jan 15 2006 - Devrim GUNDUZ <devrim@commandprompt.com> 2.7-1
- Update to 2.7

* Wed Dec 28 2005 Devrim Gunduz <devrim@commandprompt.com> pgpool-2.6.5
- Update to 2.6.5
- Removed %post
- Updated %doc files

* Sat Oct 22 2005 Devrim Gunduz <devrim@PostgreSQL.org> pgpool-2.6.4
- Update to 2.6.4
