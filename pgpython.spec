Name: pg-python
Version: 1.0.1
Release: 1%{?dist}
Group: Applications/System
Summary: The PgPython module
License: GPLv2+
Source0: pg-python-1.0.1.tar.bz2
BuildRoot: %(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

### Patches ###

### Dependencies ###

Requires: python3-libs

### Build Dependencies ###

BuildRequires: python3-devel
BuildRequires: postgresql90-devel

%description
The PgPython Python3 procedural language for PostrgeSQL

%prep
%setup -q

%build
PATH=/usr/pgsql-9.0/bin:$PATH %configure
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
PATH=/usr/pgsql-9.0/bin:$PATH make install DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%attr(755,root,root)/usr/pgsql-9.0/lib/python.so
/usr/pgsql-9.0/share/contrib/python--%{version}.sql


%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%changelog
* Thu Dec 20 2011 Jan Kundrát <kundratj@fzu.cz> - 1.0.1-1
- Initial release
