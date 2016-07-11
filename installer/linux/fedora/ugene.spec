Name:    ugene
Summary: Integrated bioinformatics toolkit
Version: 1.22.0
Release: 5%{?dist}
#The entire source code is GPLv2+ except:
#file src/libs_3rdparty/qtbindings_core/src/qtscriptconcurrent.h which is GPLv2
#files in src/plugins_3rdparty/script_debuger/src/qtscriptdebug/ which are GPLv2
License: GPLv2+ and GPLv2
Group:   Applications/Engineering
URL:     http://ugene.unipro.ru
Source0: http://ugene.unipro.ru/downloads/%{name}-%{version}.tar.gz

BuildRequires: desktop-file-utils
BuildRequires: mesa-libGLU-devel
BuildRequires: procps-devel
BuildRequires: qt5-qtbase-devel
BuildRequires: qt5-qtbase-mysql
BuildRequires: qt5-qtmultimedia-devel
BuildRequires: qt5-qtquick1-devel
BuildRequires: qt5-qtscript-devel
BuildRequires: qt5-qtsensors-devel
BuildRequires: qt5-qtsvg-devel
BuildRequires: qt5-qttools-devel
BuildRequires: qt5-qtwebchannel-devel
BuildRequires: qt5-qtwebkit-devel
BuildRequires: qt5-qtxmlpatterns-devel
BuildRequires: zlib-devel

BuildConflicts: qt-devel
#We need strict versions of qt for correct work of src/libs_3rdparty/qtbindings_*
%{?_qt5_version:Requires: qt5%{?_isa} >= %{_qt5_version}}

Provides: bundled(sqlite)
Provides: bundled(samtools)
ExclusiveArch: %{ix86} x86_64

%description
Unipro UGENE is a cross-platform visual environment for DNA and protein
sequence analysis. UGENE integrates the most important bioinformatics
computational algorithms and provides an easy-to-use GUI for performing
complex analysis of the genomic data. One of the main features of UGENE
is a designer for custom bioinformatics workflows.

%prep
%setup -q

%build
%_bindir/qmake-qt5 -r -spec linux-g++\
        INSTALL_BINDIR=%{_bindir} \
        INSTALL_LIBDIR=%{_libdir} \
        INSTALL_DATADIR=%{_datadir} \
        INSTALL_MANDIR=%{_mandir} \
%if 0%{?_ugene_with_non_free}
        UGENE_WITHOUT_NON_FREE=0 \
%else
        UGENE_WITHOUT_NON_FREE=1 \
%endif
        UGENE_EXCLUDE_LIST_ENABLED=1\
        QMAKE_CFLAGS_RELEASE="${CFLAGS}"\
        QMAKE_CXXFLAGS_RELEASE="${CXXFLAGS}"\
        QMAKE_LFLAGS_RELEASE="${LDFLAGS}"\
        QMAKE_STRIP=\
        QMAKE_CFLAGS_ISYSTEM=\

make %{?_smp_mflags}

%install
make install INSTALL_ROOT=%{buildroot}
desktop-file-validate %{buildroot}/%{_datadir}/applications/%{name}.desktop

%files
%{!?_licensedir:%global license %%doc}
%license COPYRIGHT LICENSE LICENSE.3rd_party
%{_bindir}/*
%{_libdir}/%{name}/
%{_datadir}/applications/*
%{_datadir}/pixmaps/*
%{_datadir}/icons/*
%{_datadir}/mime/*
%{_datadir}/%{name}/
%{_mandir}/man1/*

%changelog
* Mon May 30 2016 Yuliya Algaer <yalgaer@gmail.com> 1.22.0-5
- Push latest version to F-24+ too

* Mon May 30 2016 Yuliya Algaer <yalgaer@gmail.com> 1.22.0-4
- Push latest version to F-24+ too

* Mon May 30 2016 Yuliya Algaer <yalgaer@gmail.com> 1.22.0-3
- Push latest version to F-24+ too

* Sun May  8 2016 Peter Robinson <pbrobinson@fedoraproject.org> 1.22.0-2
- Push latest version to F-24+ too
- Use %%license