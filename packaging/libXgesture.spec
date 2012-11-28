Name:	libXgesture
Summary:    X11 Gesture Extension library
Version: 0.1.0
Release:    2
Group:      System/Libraries
License:    MIT
URL:        http://www.x.org
Source0:    %{name}-%{version}.tar.gz
BuildRequires:  pkgconfig(x11)
BuildRequires:  pkgconfig(xext)
BuildRequires:  pkgconfig(xproto)
BuildRequires:  pkgconfig(xextproto)
BuildRequires:  pkgconfig(gestureproto)
BuildRequires:  pkgconfig(xorg-macros)


%description
X.Org X11 libXt library


%package devel
Summary:    X11 Gesture Extension library (development headers)
Group:      Development/Libraries
Provides: libxgesture-devel 
Requires:   %{name} = %{version}-%{release}
Requires:   pkgconfig(x11)
Requires:   pkgconfig(xext)
Requires:   pkgconfig(gestureproto)

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
%make_install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%license COPYING
%{_libdir}/libXgesture.so.*

%files devel
%{_includedir}/X11/extensions/*
%{_libdir}/libXgesture.so
%{_libdir}/pkgconfig/xgesture.pc
