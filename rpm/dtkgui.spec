Name:           dtkgui
Version:        5.6.8.4
Release:        3%{?dist}
Summary:        Deepin dtkgui
License:        LGPLv3+
URL:            https://github.com/linuxdeepin/dtkgui

%if 0%{?fedora}
Source0:        %{url}/archive/%{version}/%{name}-%{version}.tar.gz
%else
Source0:        %{name}-%{version}.orig.tar.xz
%endif
BuildRequires:  qt5-qtx11extras-devel
BuildRequires:  dtkcommon-devel
BuildRequires:  dtkcore-devel
BuildRequires:  librsvg2-devel
BuildRequires:  gtest-devel
BuildRequires:  gcc-c++
BuildRequires:  annobin
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(gsettings-qt)
BuildRequires:  pkgconfig(gmock)
%if 0%{?fedora}
BuildRequires:  qt5-qtbase-private-devel
%{?_qt5:Requires: %{_qt5}%{?_isa} = %{_qt5_version}}
%endif
BuildRequires:  make
BuildRequires:  cmake
BuildRequires:  doxygen
BuildRequires:  LibRaw-devel
BuildRequires:  qt5-qtimageformats
BuildRequires:  libqtxdg-devel
BuildRequires:  freeimage-devel
Requires:       qt5-qtimageformats
%description
Dtkgui is the GUI module for DDE look and feel.

%package devel
Summary:        Development package for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       dtkcore-devel%{?_isa}

%description devel
Header files and libraries for %{name}.

%prep
%autosetup

%build
# help find (and prefer) qt5 utilities, e.g. qmake, lrelease
export PATH=%{_qt5_bindir}:$PATH
%qmake_qt5 PREFIX=%{_prefix} \
           DTK_VERSION=%{version} \
           LIB_INSTALL_DIR=%{_libdir} \
           BIN_INSTALL_DIR=%{_libexecdir}/dtk5 \
           TOOL_INSTALL_DIR=%{_libexecdir}/dtk5
%make_build

%install
%make_install INSTALL_ROOT=%{buildroot}

%files
%doc README.md
%license LICENSE
%{_libdir}/lib%{name}.so.5*
%{_libexecdir}/dtk5/deepin-gui-settings
%{_libexecdir}/dtk5/taskbar
%{_libexecdir}/dtk5/dde-kwin-debug

%files devel
%{_includedir}/libdtk-*/
%{_libdir}/pkgconfig/dtkgui.pc
%{_qt5_archdatadir}/mkspecs/modules/qt_lib_dtkgui.pri
%{_libdir}/cmake/DtkGui/
%{_libdir}/lib%{name}.so

%changelog
* Thu Dec 21 2023 mahailiang <mahailiang@uniontech.com> - 5.6.8.4-3
- fix: add requires qt5-qtimageformats(bugid:226895)

* Wed Oct 11 2023 liweigang <liweiganga@uniontech.com> - 5.6.8.4-2
- spec: change DGui path

* Mon Oct 09 2023 liweigang <liweiganga@uniontech.com> - 5.6.8.4-1
- update: update to version 5.6.8.4

* Wed Aug 16 2023 liuzhilin <liuzhilin@uniontech.com> - 5.6.8.3-1
- update: update to 5.6.8.3

* Fri Dec 23 2022 liweiganga <liweiganga@uniontech.com> - 5.5.27-1
- update: update to 5.5.27

* Mon Jun 13 2022 uoser <uoser@uniontech.com> - 5.5.18-1.01
- update: update to 5.5.18-1.01
