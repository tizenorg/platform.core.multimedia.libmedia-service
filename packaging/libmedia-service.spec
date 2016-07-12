Name:       libmedia-service
Summary:    Media information service library for multimedia applications
Version: 0.2.76
Release:    0
Group:      Multimedia/Libraries
License:    Apache-2.0 and public domain
Source0:    %{name}-%{version}.tar.gz
Source1001: 	libmedia-service.manifest

Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

BuildRequires:  cmake
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(aul)
BuildRequires:  pkgconfig(libexif)
BuildRequires:  pkgconfig(sqlite3)
BuildRequires:  pkgconfig(db-util)
BuildRequires:  pkgconfig(mm-fileinfo)
BuildRequires:  pkgconfig(media-thumbnail)
BuildRequires:  pkgconfig(libmedia-utils)
BuildRequires:  pkgconfig(uuid)
BuildRequires:  pkgconfig(libtzplatform-config)
BuildRequires:  pkgconfig(iniparser)
BuildRequires:  pkgconfig(capi-media-image-util)

%description
This package is a library that provides the media information service for the multimedia applications.

%package devel
Summary:    Media information service library for multimedia applications. (development)
Group:      Multimedia/Development
Requires:   %{name} = %{version}-%{release}

%description devel
This package is a library that provides the media information service for the multimedia applications. (Development files included)

%prep
%setup -q
cp %{SOURCE1001} .

%build
export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE -DSYSCONFDIR=\\\"%{_sysconfdir}\\\""
%cmake .
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

#License
mkdir -p %{buildroot}/%{_datadir}/license
cp -rf %{_builddir}/%{name}-%{version}/LICENSE.APLv2.0 %{buildroot}/%{_datadir}/license/%{name}

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%files
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_libdir}/libmedia-service.so
%{_libdir}/libmedia-service.so.1
%{_libdir}/libmedia-service.so.1.0.0
%{_libdir}/libmedia-svc-hash.so
%{_libdir}/libmedia-svc-hash.so.1
%{_libdir}/libmedia-svc-hash.so.1.0.0
%{_libdir}/libmedia-content-plugin.so
%{_libdir}/libmedia-content-plugin.so.1
%{_libdir}/libmedia-content-plugin.so.1.0.0

#License
%{_datadir}/license/%{name}

%files devel
%manifest %{name}.manifest
%{_libdir}/pkgconfig/libmedia-service.pc
%{_includedir}/media-service/*.h
