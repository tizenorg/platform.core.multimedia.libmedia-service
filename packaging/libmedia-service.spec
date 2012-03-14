Name:	    libmedia-service
Summary:    Media Service
Version:	0.2.0
Release:    0
Group:      System/Libraries
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

BuildRequires: cmake, expat-devel
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(sqlite3)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(db-util)
BuildRequires: pkgconfig(mm-common)
BuildRequires: pkgconfig(libpng)
BuildRequires: pkgconfig(libpng12)
BuildRequires: pkgconfig(mm-fileinfo)
BuildRequires: pkgconfig(drm-service)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(dbus-1)
BuildRequires: pkgconfig(libexif)
BuildRequires: pkgconfig(xdmcp)
BuildRequires: pkgconfig(x11)
BuildRequires: pkgconfig(xrender)
BuildRequires: pkgconfig(xcb)
BuildRequires: pkgconfig(xau)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(ecore-evas)
BuildRequires: pkgconfig(evas)
BuildRequires: pkgconfig(mmutil-imgp)


%description
Media information service library for multimedia applications.


%package devel
Summary:    Media Service
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description devel
Media information service library for multimedia applications. (developement files)

%prep
%setup -q


%build
cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix}


make %{?jobs:-j%jobs}

%install
%make_install


%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
/usr/lib/libmedia-service.so.1
/usr/lib/libmedia-service.so.1.0.0
/usr/lib/libmedia-svc-hash.so.1
/usr/lib/libmedia-svc-hash.so.1.0.0

%files devel
/usr/lib/libmedia-service.so
/usr/lib/libmedia-svc-hash.so
/usr/lib/pkgconfig/libmedia-service.pc
/usr/include/media-service/*.h
