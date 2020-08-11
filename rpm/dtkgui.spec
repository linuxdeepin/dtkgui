Name:           dtkgui
Version:        5.2.0
Release:        1%{?dist}
Summary:        Deepin dtkgui
License:        GPLv3
URL:            https://shuttle.deepin.com/cache/repos/apricot/release-candidate/RERFLWR0a2NvcmXmm7TmlrA1Njg/pool/main/d/dtkgui/
Source0:        %{name}_%{version}.orig.tar.xz
BuildRequires:  qt5-qtx11extras-devel
BuildRequires:  dtkcore-devel
BuildRequires:  librsvg2-devel
BuildRequires:  gcc-c++
BuildRequires:  annobin
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(gsettings-qt)


%description
Deepin tool kit core modules.libdtkcore5/experimentallibdtkcore5/experimentallibdtkcore5/experimentallibdtkcore5/experimental

%package devel
Summary:        Development package for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       qt5-qtbase-devel

%description devel
Header files and libraries for %{name}.

%prep
%setup -q

#sed -i 's|/lib|/libexec|' tools/settings/settings.pro
## consider relying on %%_qt5_bindir (see %%build below) instead of patching -- rex
#sed -i 's|qmake|qmake-qt5|' src/dtk_module.prf
#sed -i 's|lrelease|lrelease-qt5|' tools/script/dtk-translate.py src/dtk_translation.prf

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

%ldconfig_scriptlets


%files
%doc README.md
%license LICENSE
%{_libdir}/libdtkgui.so*
%{_libexecdir}/dtk5/deepin-gui-settings
%{_libexecdir}/dtk5/taskbar
/etc/dbus-1/system.d/com.deepin.dtk.FileDrag.conf

%files devel
%{_includedir}/libdtk-*/
%{_libdir}/pkgconfig/dtkgui.pc
%{_libdir}/qt5/mkspecs/modules/qt_lib_dtkgui.pri
%{_libdir}/cmake/DtkGui/

%changelog
* Thu Jun 09 2020 uoser <uoser@uniontech.com> - 5.2.2.1-1
- Update to 5.2.2.1