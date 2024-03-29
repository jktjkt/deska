%if ! (0%{?fedora} > 12 || 0%{?rhel} > 5)
%{!?python_sitelib: %global python_sitelib %(%{__python} -c "from distutils.sysconfig import get_python_lib; print(get_python_lib())")}
%{!?python_sitearch: %global python_sitearch %(%{__python} -c "from distutils.sysconfig import get_python_lib; print(get_python_lib(1))")}
%endif

%define with_doc %{?_with_doc: 1} %{?!_with_doc: 0}

# Yeah, a braindead file URL
%global redminefile FIXME

Name: deska
Version: 1.0
Release: 1%{?dist}
Group: Applications/System
Summary: Tool for Central Administration of a Grid Site
License: GPLv2+
URL: http://deska.flaska.net/
Source0: https://projects.flaska.net/attachments/download/%{redminefile}/%{name}-%{version}.tar.bz2
BuildRoot: %(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

### Patches ###

### Dependencies ###

### Build Dependencies ###

BuildRequires: cmake >= 2.6
BuildRequires: boost-devel >= 1.41.0
BuildRequires: cpp
BuildRequires: readline-devel
BuildRequires: python-devel
%if %{with_doc}
BuildRequires: texlive-a4wide
BuildRequires: texlive-etoolbox
BuildRequires: texlive-minted
BuildRequires: texlive-todonotes
BuildRequires: texlive-iopart-num
%endif

%description
Empty, as this package shall not be generated at all -- we use subpackages.

%package libs
Summary: The Deska shared libraries
Group: Applications/System
License: GPLv2+
Requires: boost-system >= 1.41.0
Requires: boost-filesystem >= 1.41.0
Requires: boost-date-time >= 1.41.0
Requires: boost-python >= 1.41.0

%description libs
Shared libraries and scripts for the Deska system

%package python-libs
Summary: Library for accessing the Deska database from Python
Group: Applications/System
License: GPLv2+
Requires: deska-libs

%description python-libs
This package contains the deska Python library which provides native object
hierarchy for a high-level access to the Deska database.

%package client
Summary: The Deska CLI application
Group: Applications/System
License: GPLv2+
Requires: deska-libs

%description client
The command line client application for accessing the Deska database

%package devel
Summary: Development files for the Deska system
Group: Application/System
License: GPLv2+
Requires: deska-libs
Requires: deska-python-libs

%description devel
The include files required for compiling against the libDeskaDb library

%package server
Summary: The Deska server daemon
Group: Application/System
License: GPLv2+
Requires: deska-python-libs
Requires: deska-client
Requires: python3
Requires: postgresql90-server
Requires: python-psycopg2

%description server
The server daemon responsible for talking to the PostgreSQL database and the supporting utilities

%if %{with_doc}
%package doc
Summary: Documentation for the Deska system
Group: Application/System
License: GPLv2+

%description doc
User's guide and complete developer documentation for the Deska system
%endif

%prep
%setup -q

%if %{with_doc}
%global doc_opts -DBUILD_DOCS=1 -DSKIP_INSTALL_DOCS=1
%else
%global doc_opts -DBUILD_DOCS=0
%endif

%build
mkdir _build && cd _build
%cmake \
	-DPYTHON_SITE_PACKAGES=%{python_sitelib} \
	-DPYTHON_SITE_PACKAGES_ARCH=%{python_sitearch} \
	-DRUN_SQL_TESTS=1 \
	%{doc_opts} \
	..
make -j20
%py_byte_compile %{__python} %{buildroot}/src/deska/python/deska
%py_byte_compile %{__python} %{buildroot}/src/deska/server/app/deska_server_utils
%py_byte_compile %{__python} %{buildroot}/src/deska/server/app/deska_server_utils/config_generators
%py_byte_compile %{__python} %{buildroot}/src/deska/server/db/gen_sql
%if %{with_doc}
mv doc/technical/deska.pdf ../deska.pdf
%endif
#make %{?_smp_mflags}

%check
cd _build
PATH=/usr/pgsql-9.0/bin:$PATH ../run-standalone-tests.sh

%install
cd _build
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%files libs
%defattr(-,root,root,-)
%{_libdir}/libDeskaDb.so.1.0
%{_libdir}/libDeskaCli.so.1.0

%files python-libs
%defattr(-,root,root,-)
%{python_sitearch}/deska/libLowLevelPyDeska.so
%{python_sitelib}/deska/*.py*

%files client
%defattr(-,root,root,-)
%{_bindir}/deska-cli

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
%attr(755,root,root)%{python_sitelib}/deska_server_utils/config_generators/git-new-workdir
%attr(755,root,root)%{_datadir}/deska/install-scripts/install/*.sh
%attr(755,root,root)%{_datadir}/deska/install-scripts/tests/*.sh
%attr(755,root,root)%{_datadir}/deska/install-scripts/tests/sql/*.sh
%{_datadir}/deska/install-scripts/install/*.sql
%{_datadir}/deska/install-scripts/install/modules/demo/*.sql
%{_datadir}/deska/install-scripts/install/modules/fzu/*.sql
%attr(755,root,root)%{_datadir}/deska/install-scripts/src/deska/server/db/gen_sql/*.py*
%{_datadir}/deska/install-scripts/src/deska/server/db/*.py*
%{_datadir}/deska/install-scripts/src/deska/server/db/*.sql

%if %{with_doc}
%files doc
%defattr(-,root,root,-)
%doc deska.pdf
%endif

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%post client -p /sbin/ldconfig

%postun client -p /sbin/ldconfig

%changelog
* Thu Dec 29 2011 Jan Kundrát <kundratj@fzu.cz> - 1.0-1
- Initial release
