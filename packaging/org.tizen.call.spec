Name:       org.tizen.call
Summary:    call application
Version:    0.2.333
Release:    1
Group:      Applications
License:    Flora Software License 
Source0:    %{name}-%{version}.tar.gz
BuildRequires:  pkgconfig(appcore-efl)
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(utilX)
BuildRequires:  pkgconfig(alsa)
BuildRequires:  pkgconfig(pmapi)
BuildRequires:  pkgconfig(sensor)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(aul)
BuildRequires:  pkgconfig(contacts-service)
BuildRequires:  pkgconfig(ui-gadget)
BuildRequires:  pkgconfig(tapi)
BuildRequires:  pkgconfig(mm-sound)
BuildRequires:  pkgconfig(mm-camcorder)
BuildRequires:  pkgconfig(mm-session)
BuildRequires:  pkgconfig(mm-player)
BuildRequires:  pkgconfig(devman_haptic)
BuildRequires:  pkgconfig(msg-service)
BuildRequires:  pkgconfig(ecore-x)
BuildRequires:  pkgconfig(ecore-input)
BuildRequires:  pkgconfig(icu-i18n)
BuildRequires:  pkgconfig(appsvc)
BuildRequires:  pkgconfig(notification)
BuildRequires:  libug-contacts-devel
BuildRequires:  cmake
BuildRequires:  gettext-tools
BuildRequires:  edje-tools
Requires(post): /usr/bin/vconftool


%description
call application.

%prep
%setup -q

%build

%define PREFIX    "/opt/apps/org.tizen.call"
cmake . -DCMAKE_INSTALL_PREFIX=%{PREFIX}

make %{?jobs:-j%jobs}


%install
rm -rf %{buildroot}
%make_install

%post
/usr/bin/vconftool set -t int "memory/call/state" "0" -g 6521 -i
/usr/bin/vconftool set -t int "db/call/vol_level" "4" -g 6521
chown -R 5000:5000 /opt/apps/org.tizen.call/data

%files
/opt/apps/org.tizen.call/bin/*
/opt/apps/org.tizen.call/res/*
/opt/share/applications/org.tizen.call.desktop
%dir /opt/apps/org.tizen.call/data

