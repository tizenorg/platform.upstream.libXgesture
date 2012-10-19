#sbs-git:slp/pkgs/xorg/lib/libXgesture libXgesture 0.1.0 589298a12fdb2ac318a89eb12e9afdab9aac402d
Name:	libXgesture
Summary:    X11 Gesture Extension library
Version: 0.1.0
Release:    2
Group:      System/Libraries
License:    MIT
URL:        http://www.x.org
Source0:    %{name}-%{version}.tar.gz
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires:  pkgconfig(x11)
BuildRequires:  pkgconfig(xext)
BuildRequires:  pkgconfig(xproto)
BuildRequires:  pkgconfig(xextproto)
BuildRequires:  xorg-x11-proto-gesture
BuildRequires:  pkgconfig(xorg-macros)


%description
X.Org X11 libXt library


%package devel
Summary:    X11 Gesture Extension library (development headers)
Group:      Development/Libraries
Provides: libxgesture-devel 
Requires:   %{name} = %{version}-%{release}
Requires:   pkgconfig(x11), pkgconfig(xext), pkgconfig(gestureproto)

%description devel
libXgesture provides an X Window System client interface to the X Gesture
Extension to the X protocol.


%prep
%setup -q

%build

%reconfigure --disable-static

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/share/license
cp -af COPYING %{buildroot}/usr/share/license/%{name}

%make_install




%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig



%files
/usr/share/license/%{name}
%{_libdir}/libXgesture.so.*

%files devel
%{_includedir}/X11/extensions/*
%{_libdir}/libXgesture.so
%{_libdir}/pkgconfig/xgesture.pc
