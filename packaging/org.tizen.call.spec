Name:       org.tizen.call
Summary:    call application
Version:    0.2.382
Release:    1
Group:      comm
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
BuildRequires:  pkgconfig(ui-gadget-1)
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
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(capi-base-common)
BuildRequires:  pkgconfig(notification)
BuildRequires:  pkgconfig(minicontrol-provider)
BuildRequires:  pkgconfig(capi-media-sound-manager)
BuildRequires:  pkgconfig(dbus-1)
BuildRequires:  pkgconfig(dbus-glib-1)
BuildRequires:  pkgconfig(capi-system-info)
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
# 5000 is inhouse user id
# do not use relative path
chown -R 5000:5000 /opt/apps/org.tizen.call/data
/usr/bin/vconftool set -t int memory/call/state "0" -g 6521 -i
/usr/bin/vconftool set -t int "db/call/vol_level" "4" -g 6521

%files
%defattr(-,root,root,-)
/opt/apps/org.tizen.call/bin/voice-call-ui
/opt/apps/org.tizen.call/res/edje/*.edj
/opt/apps/org.tizen.call/res/images/*.png
/opt/apps/org.tizen.call/res/locale/*/LC_MESSAGES/voice-call-ui.mo
/opt/apps/org.tizen.call/res/media/*.wav
/opt/share/packages/org.tizen.call.xml
/opt/share/icons/default/small/org.tizen.call.png
