%if ! (0%{?fedora} > 12 || 0%{?rhel} > 5)
%{!?python_sitelib: %global python_sitelib %(%{__python} -c "from distutils.sysconfig import get_python_lib; print(get_python_lib())")}
%{!?python_sitearch: %global python_sitearch %(%{__python} -c "from distutils.sysconfig import get_python_lib; print(get_python_lib(1))")}
%endif

# Yeah, a braindead file URL
%global redminefile FIXME

Name: deska
Version: 0.11.742
Release: 1%{?dist}
Group: Applications/System
Summary: Tool for Central Administration of a Grid Site
License: GPLv2+
URL: http://deska.flaska.net/
Source0: https://projects.flaska.net/attachments/download/%{redminefile}/%{name}-%{version}.tar.bz2
BuildRoot: %(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

### Patches ###

### Dependencies ###

Requires: boost-system >= 1.41.0
Requires: boost-date-time >= 1.41.0
Requires: boost-python >= 1.41.0

### Build Dependencies ###

BuildRequires: cmake >= 2.6
BuildRequires: boost-devel >= 1.41.0
BuildRequires: cpp
BuildRequires: readline-devel
BuildRequires: python-devel

%description
Shared libraries and scripts for the Deska system

%package client
Summary: The Deska CLI application
Group: Applications/System
License: GPLv2+

%description client
The command line client application for accessing the Deska database

%package devel
Summary: Development files for the Deska system
Group: Application/System
License: GPLv2+

%description devel
The include files required for compiling against the libDeskaDb library

%package server
Summary: The Deska server daemon
Group: Application/System
License: GPLv2+

%description server
The server daemon responsible for talking to the PostgreSQL database and the supporting utilities


%prep
%setup -q

%build
mkdir _build && cd _build
%cmake -DPYTHON_SITE_PACKAGES=%{python_sitelib} -DPYTHON_SITE_PACKAGES_ARCH=%{python_sitearch} ..
make -j20
#make %{?_smp_mflags}
%py_byte_compile %{__python} %{buildroot}/_build/src/deska/python/deska
%py_byte_compile %{__python} %{buildroot}/_build/src/deska/server/app/deska_server_utils
%py_byte_compile %{__python} %{buildroot}/_build/src/deska/server/app/deska_server_utils/config_generators

%check
cd _build
echo "Testing is disabled"

%install
cd _build
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%{_libdir}/libDeskaDb.so.0.10
%{python_sitearch}/deska/libLowLevelPyDeska.so
%{python_sitelib}/deska/*.py*

%files client
%defattr(-,root,root,-)
%{_bindir}/deska-cli
%{_libdir}/libDeskaCli.so.0.10

%files devel
%defattr(-,root,root,-)
%{_includedir}/deska/db/*.h
%{_libdir}/libDeskaDb.so
%{_libdir}/libDeskaCli.so

%files server
%defattr(-,root,root,-)
%{_bindir}/deska-server
%{python_sitelib}/deska_server_utils/*.py*
%{python_sitelib}/deska_server_utils/config_generators/*.py*
%{python_sitelib}/deska_server_utils/config_generators/git-new-workdir

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%post client -p /sbin/ldconfig

%postun client -p /sbin/ldconfig

%changelog
* Wed Dec 28 2011 Jan Kundrát <kundratj@fzu.cz> - 0.11.742-1
- Initial release
