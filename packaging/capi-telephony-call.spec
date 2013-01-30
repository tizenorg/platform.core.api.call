Name:       capi-telephony-call
Summary:    Telephony Call Framework
Version: 0.1.0
Release:    1
Group:      API/C API
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
BuildRequires:  cmake
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(capi-base-common)
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
%build
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
%cmake .  -DFULLVER=%{version} -DMAJORVER=${MAJORVER}

make %{?jobs:-j%jobs}

%install
%make_install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%{_libdir}/libcapi-telephony-call.so.*

%files devel
%{_includedir}/telephony/call.h
%{_libdir}/pkgconfig/*.pc
%{_libdir}/libcapi-telephony-call.so


