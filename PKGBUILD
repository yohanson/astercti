pkgname=astercti
_pkgname="${pkgname%-git}"
pkgver=0.0.24
pkgrel=2
pkgdesc='Computer-Telephone Integration app for Asterisk'
arch=('x86_64' 'i686')
url="https://github.com/yohanson/astercti"
license=('GPL')
depends=('wxwidgets-gtk3' 'curl' 'jsoncpp')
makedepends=('git')
checkdepends=()
provides=("$_pkgname")
conflicts=("$_pkgname")
source=("git+$url")
b2sums=('SKIP')

pkgver() {
  cd "$_pkgname"

  git tag | sort -Vr | grep -v -- '-' | head -1
}

build() {
  cd "$_pkgname"

  make release
}

package () {
  cd "$_pkgname"

  DESTDIR="$pkgdir" make install
}

# vim: et ts=2 sw=2
