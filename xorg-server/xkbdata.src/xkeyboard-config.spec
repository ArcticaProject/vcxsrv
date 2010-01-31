Name:		xkeyboard-config
Summary:	XML-based XKB configuration registry
Version:	1.8
Release:	1
License:	X11/MIT
Group:		User Interface/X

Url: http://gswitchit.sourceforge.net/

Source: http://gswitchit.sourceforge.net/%{name}-%{version}.tar.gz
Buildroot: %{_tmppath}/%{name}-%{version}-%{release}-root

BuildArch:      noarch

%description
Just XML stuff. Later hopefully will be part of XFree86

%prep
%setup -q

%build

if [ ! -f configure ]; then
    CFLAGS="$RPM_OPT_FLAGS" ./autogen.sh
fi

CFLAGS="$RPM_OPT_FLAGS" ./configure 

make 

%install
rm -rf $RPM_BUILD_ROOT

make prefix=$RPM_BUILD_ROOT%{_prefix} install 
rm -rf $RPM_BUILD_ROOT%{_prefix}/share/locale

%clean
rm -rf %{buildroot}
rm -rf $RPM_BUILD_DIR/%{name}-%{version}

%files
%defattr(-, root, root)

%doc AUTHORS CREDITS ChangeLog NEWS README COPYING docs/README.config docs/README.enhancing docs/README.symbols docs/HOWTO.transition docs/HOWTO.testing
%{_prefix}/lib/X11/xkb/*
