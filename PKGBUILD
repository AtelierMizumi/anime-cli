# Maintainer: Thuan Tran thuan.tran@thuanc177.me
pkgname=anime-cli
pkgver=0.3.3
pkgrel=1
pkgdesc="Command-line interface for searching and streaming anime and manga via Consumet API"
arch=('x86_64')
url="https://github.com/AtelierMizumi/anime-cli"
license=('MIT')
depends=('curl' 'json-c' 'ncurses' 'mpv' 'sxiv')
makedepends=('gcc' 'make')
optdepends=(
    'ffmpeg: for downloading anime episodes'
    'mcomix: recommended manga viewer with comic book navigation'
    'zathura: document viewer with manga mode'
    'sxiv: alternative image viewer'
    'feh: alternative image viewer'
    'imv: alternative image viewer for X11/Wayland'
)
# Use branch instead of tag
source=("git+${url}.git#branch=master")
sha256sums=('SKIP')

build() {
    cd "${srcdir}/${pkgname}"
    make -j$(nproc)
}

check() {
    cd "${srcdir}/${pkgname}"
    # Skip tests if not available yet
    if [ -f Makefile ] && grep -q "test" Makefile; then
        make test
    fi
}

package() {
    cd "${srcdir}/${pkgname}"
    install -Dm755 anime-cli "${pkgdir}/usr/bin/anime-cli"
    install -Dm644 LICENSE "${pkgdir}/usr/share/licenses/${pkgname}/LICENSE"
    install -Dm644 README.md "${pkgdir}/usr/share/doc/${pkgname}/README.md"
    
    # Install config file example if available
    if [ -f anime-cli.conf.example ]; then
        install -Dm644 anime-cli.conf.example "${pkgdir}/etc/${pkgname}/anime-cli.conf.example"
    fi
}