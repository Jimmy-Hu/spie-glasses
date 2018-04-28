layer_help() {
    echo "lib/zlib"
}

layer_metadata() {
    LAYER_DEPENDENCIES=(
        app/git
        app/cmake
        lang/c-c++
    )
}

layer_is_installed() {
    test -f /usr/local/include/zlib.h
}

layer_install() {
    test ! -e /tmp/source/zlib && git clone https://github.com/madler/zlib /tmp/source/zlib
    mkdir -p /tmp/build/zlib 2>/dev/null
    pushd /tmp/source/zlib
    git checkout master
    test -d /tmp/build/zlib || cp -R /tmp/source/zlib /tmp/build
    pushd /tmp/build/zlib
    ./configure
    make
    sudo make install
    popd
    popd

}
