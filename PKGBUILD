pkgname=anime-cli
pkgver=1.0.0
pkgrel=1
pkgdesc="A command-line interface for searching and selecting anime using the Consumet API"
arch=("x86_64")
url="https://github.com/yourusername/anime-cli"
license=("MIT")
depends=("gcc" "make" "curl" "json-c" "ncurses" "mpv")
source=("git+https://github.com/yourusername/anime-cli.git")
md5sums=("SKIP")

build() {
    cd "$srcdir/$pkgname"
    make
}

package() {
    cd "$srcdir/$pkgname"
    install -Dm755 anime-cli "$pkgdir/usr/bin/anime-cli"
    install -Dm644 LICENSE "$pkgdir/usr/share/licenses/$pkgname/LICENSE"
    install -Dm644 README.md "$pkgdir/usr/share/doc/$pkgname/README.md"
}