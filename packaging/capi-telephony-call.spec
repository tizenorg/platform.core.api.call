%define major 3
%define minor 0
%define patchlevel 1

Name:       capi-telephony-call
Summary:    Telephony Call Framework
Version:    %{major}.%{minor}.%{patchlevel}
Release:    1
Group:      API/C API
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1001: 	capi-telephony-call.manifest
BuildRequires:  cmake
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(capi-base-common)
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
%description
Telephony Call Framework.


%package devel
Summary:  Telephony Call Framework (Development)
Group:    API/C API
Requires: %{name} = %{version}-%{release}

%description devel
Telephony Call Framework.



%prep
%setup -q
cp %{SOURCE1001} .
%build
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
%cmake .  -DFULLVER=%{version} -DMAJORVER=${MAJORVER}

make %{?jobs:-j%jobs}

%install
%make_install
mkdir -p %{buildroot}/usr/share/license
cp LICENSE %{buildroot}/usr/share/license/%{name}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%manifest %{name}.manifest
%{_libdir}/libcapi-telephony-call.so.*
/usr/share/license/%{name}

%files devel
%manifest %{name}.manifest
%{_includedir}/telephony/call.h
%{_libdir}/pkgconfig/*.pc
%{_libdir}/libcapi-telephony-call.so


