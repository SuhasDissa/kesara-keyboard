# Maintainer: Suhas Dissanayake <suhasdissa@gmail.com>
pkgbase=kesara
pkgname=(ibus-kesara fcitx5-kesara)
pkgver=1.0.0
pkgrel=1
pkgdesc='Kesara Sinhala phonetic keyboard'
url='https://github.com/SuhasDissa/kesara-keyboard'
arch=('x86_64')
license=('GPL-3.0-or-later')
makedepends=('cmake' 'extra-cmake-modules' 'fcitx5' 'gcc')
source=("$pkgbase-$pkgver.tar.gz::https://github.com/SuhasDissa/kesara-keyboard/archive/refs/tags/v$pkgver.tar.gz")
sha256sums=('SKIP')

build() {
  cmake -B build -S "$srcdir/kesara-keyboard-$pkgver" \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_BUILD_TYPE=None \
    -DBUILD_IBUS=ON \
    -DBUILD_FCITX5=ON \
    -DBUILD_FCITX4=OFF \
    -DBUILD_TESTS=ON
  cmake --build build
}

check() {
  ctest --test-dir build --output-on-failure
}

package_ibus-kesara() {
  pkgdesc='Kesara Sinhala phonetic input method for IBus (m17n)'
  arch=('any')
  depends=('ibus-m17n' 'm17n-lib')
  DESTDIR="$pkgdir" cmake --install build --component ibus
}

package_fcitx5-kesara() {
  pkgdesc='Kesara Sinhala phonetic input method for Fcitx 5'
  depends=('fcitx5')
  DESTDIR="$pkgdir" cmake --install build --component fcitx5
}
