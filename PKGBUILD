# Maintainer: Fr_nk <freec AT vi-di.fr>
pkgname=freec
pkgver=1.2
pkgrel=1
pkgdesc="Display memory usage using graphs and nice colors."
arch=('x86_64' 'i686')
url="http://git.vi-di.fr/freec/"
license=('BSD')
depends=('glibc')
makedepends=('make')
provides=('freec')
conflicts=()
source=(http://git.vi-di.fr/${pkgname}/src/${pkgname}-${pkgver}.tar.gz)
md5sums=('b47a41f972abbdbe483fe6217af8450e')

build() {
  cd ${pkgname}
  make
}

package() {
  cd ${pkgname}
  make DESTDIR=${pkgdir} install
}
