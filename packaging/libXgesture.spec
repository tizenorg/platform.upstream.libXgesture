%bcond_with x

Name:	libXgesture
Summary:    X11 Gesture Extension library
Version: 0.1.0
Release:    2
Group:      System/Libraries
License:    MIT
URL:        http://www.x.org
Source0:    %{name}-%{version}.tar.gz
Source1001: 	libXgesture.manifest
BuildRequires:  pkgconfig(x11)
BuildRequires:  pkgconfig(xext)
BuildRequires:  pkgconfig(xproto)
BuildRequires:  pkgconfig(xextproto)
BuildRequires:  pkgconfig(gestureproto)
BuildRequires:  pkgconfig(xorg-macros)

%if !%{with x}
ExclusiveArch:
%endif

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
cp %{SOURCE1001} .

%build
%reconfigure --disable-static
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%manifest %{name}.manifest
%license COPYING
%{_libdir}/libXgesture.so.*

%files devel
%manifest %{name}.manifest
%{_includedir}/X11/extensions/*
%{_libdir}/libXgesture.so
%{_libdir}/pkgconfig/xgesture.pc
