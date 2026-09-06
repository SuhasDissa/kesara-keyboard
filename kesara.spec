Name:           kesara
Version:        1.0.0
Release:        1%{?dist}
Summary:        Kesara Sinhala phonetic keyboard
License:        GPL-3.0-or-later
URL:            https://github.com/SuhasDissa/kesara-keyboard
Source0:        %{url}/archive/refs/tags/v%{version}.tar.gz#/kesara-keyboard-%{version}.tar.gz

%bcond_with fcitx4

BuildRequires:  cmake
BuildRequires:  gcc-c++
BuildRequires:  extra-cmake-modules
BuildRequires:  pkgconfig
BuildRequires:  fcitx5-devel
%if %{with fcitx4}
BuildRequires:  fcitx-devel
%endif

%description
Kesara converts Singlish-style Latin typing into Sinhala Unicode
for IBus and Fcitx.

%package -n ibus-kesara
Summary:        Kesara Sinhala phonetic input method for IBus (m17n)
BuildArch:      noarch
Requires:       ibus-m17n
Requires:       m17n-lib
Requires:       m17n-db

%description -n ibus-kesara
m17n table for IBus (m17n:si:kesara).

%package -n fcitx5-kesara
Summary:        Kesara Sinhala phonetic input method for Fcitx 5
Requires:       fcitx5

%description -n fcitx5-kesara
Native Fcitx 5 engine for the Kesara Sinhala phonetic keyboard.

%if %{with fcitx4}
%package -n fcitx-kesara
Summary:        Kesara Sinhala phonetic input method for Fcitx 4
Requires:       fcitx

%description -n fcitx-kesara
Native Fcitx 4 engine for the Kesara Sinhala phonetic keyboard.
%endif

%prep
%autosetup -n kesara-keyboard-%{version}

%build
%cmake \
  -DBUILD_IBUS=ON \
  -DBUILD_FCITX5=ON \
%if %{with fcitx4}
  -DBUILD_FCITX4=ON \
%else
  -DBUILD_FCITX4=OFF \
%endif
  -DBUILD_TESTS=ON
%cmake_build

%install
%cmake_install

%check
ctest --test-dir %{_vpath_builddir} --output-on-failure

%files -n ibus-kesara
%license LICENSE
%doc README.md
%{_datadir}/m17n/si-kesara.mim

%files -n fcitx5-kesara
%license LICENSE
%{_libdir}/fcitx5/kesara.so
%{_datadir}/fcitx5/addon/kesara.conf
%{_datadir}/fcitx5/inputmethod/kesara.conf

%if %{with fcitx4}
%files -n fcitx-kesara
%license LICENSE
%{_libdir}/fcitx/fcitx-kesara.so
%{_datadir}/fcitx/addon/fcitx-kesara.conf
%endif

%changelog
* Sun Sep 06 2026 Suhas Dissanayake <suhasdissa@gmail.com> - 1.0.0-1
- Initial package
