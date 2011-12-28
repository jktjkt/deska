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

#%global servicename sssd
#%global sssdstatedir %{_localstatedir}/lib/sss
#%global dbpath %{sssdstatedir}/db
#%global pipepath %{sssdstatedir}/pipes
#%global pubconfpath %{sssdstatedir}/pubconf

### Build Dependencies ###

BuildRequires: cmake >= 2.6
BuildRequires: boost-devel >= 1.41.0
BuildRequires: cpp
BuildRequires: readline-devel
BuildRequires: python-devel

%description
FIXME

#%package client
#Summary: The Deska CLI application
#Group: Applications/System
#License: LGPLv3+
#
#%description client
#The command line client application for accessing the Deska database

%package devel
Summary: Development files for the Deska system
Group: Application/System
License: GPLv2+

%description devel
The include files required for compiling against the libDeskaDb library

%prep
%setup -q

%build
mkdir _build && cd _build
%cmake -DPYTHON_SITE_PACKAGES=%{python_sitelib} -DPYTHON_SITE_PACKAGES_ARCH=%{python_sitearch} ..
make -j20
#make %{?_smp_mflags}

%check
cd _build
echo "Testing is disabled"

%install
cd _build
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

# Copy default sssd.conf file
#mkdir -p $RPM_BUILD_ROOT/%{_sysconfdir}/sssd
#install -m600 src/examples/sssd.conf $RPM_BUILD_ROOT%{_sysconfdir}/sssd/sssd.conf
#install -m444 src/config/etc/sssd.api.conf $RPM_BUILD_ROOT%{_sysconfdir}/sssd/sssd.api.conf
#install -m444 src/config/etc/sssd.api.d/* $RPM_BUILD_ROOT%{_sysconfdir}/sssd/sssd.api.d/

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%{_bindir}/deska-cli
%{_bindir}/deska-server
%{_libdir}/libDeskaCli.so.0.10
%{_libdir}/libDeskaDb.so.0.10
%{python_sitearch}/deska/libLowLevelPyDeska.so
%{python_sitelib}/deska/*.py*
%{python_sitelib}/deska_server_utils/*.py*
%{python_sitelib}/deska_server_utils/config_generators/*.py*
%{python_sitelib}/deska_server_utils/config_generators/git-new-workdir

%files devel
%{_includedir}/deska/db/*.h
%{_libdir}/libDeskaDb.so
%{_libdir}/libDeskaCli.so

#%files -f sssd.lang
#%defattr(-,root,root,-)
#%doc COPYING
#%{_initrddir}/%{name}
#%{_sbindir}/sssd
#%{_libexecdir}/%{servicename}/
#%{_libdir}/%{name}/
#%{_libdir}/ldb/memberof.so
#%dir %{sssdstatedir}
#%attr(700,root,root) %dir %{dbpath}
#%attr(755,root,root) %dir %{pipepath}
#%attr(750,root,root) %dir %{_var}/log/%{name}
#%attr(711,root,root) %dir %{_sysconfdir}/sssd
#%config(noreplace) %verify(not md5 size mtime) %{_sysconfdir}/sssd/sssd.conf
#%config %{_sysconfdir}/sssd/sssd.api.conf
#%attr(755,root,root) %dir %{_sysconfdir}/sssd/sssd.api.d
#%config %{_sysconfdir}/sssd/sssd.api.d/
#%{_mandir}/man5/sssd.conf.5*
#%{python_sitearch}/pysss.so
#%{python_sitelib}/*.py*

#%files client
#%defattr(-,root,root,-)
#%doc src/sss_client/COPYING src/sss_client/COPYING.LESSER
#/%{_lib}/libnss_sss.so.2
#/%{_lib}/security/pam_sss.so
#%{_libdir}/krb5/plugins/libkrb5/sssd_krb5_locator_plugin.so
#%{_mandir}/man8/pam_sss.8*
#%{_mandir}/man8/sssd_krb5_locator_plugin.8*

%post
/sbin/ldconfig
#/sbin/chkconfig --add %{servicename}
#
#if [ $1 -ge 1 ] ; then
#    /sbin/service %{servicename} condrestart 2>&1 > /dev/null
#fi

#%preun
#if [ $1 = 0 ]; then
#    /sbin/service %{servicename} stop 2>&1 > /dev/null
#    /sbin/chkconfig --del %{servicename}
#fi

%postun -p /sbin/ldconfig

#%post client -p /sbin/ldconfig
#
#%postun client -p /sbin/ldconfig

%changelog
* Wed Dec 28 2011 Jan Kundrát <kundratj@fzu.cz> - 0.11.742-1
- Initial release
