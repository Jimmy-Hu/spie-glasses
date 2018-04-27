layer_help() {
    echo "lib/lz4"
}

layer_metadata() {
    LAYER_DEPENDENCIES=(
        app/git
        app/cmake
        lang/c-c++
    )
}

layer_is_installed() {
    test -f /usr/local/include/lz4.h
}

layer_install() {
    test ! -e /tmp/source/lz4 && git clone https://github.com/lz4/lz4 /tmp/source/lz4
    mkdir -p /tmp/build/lz4 2>/dev/null
    pushd /tmp/source/lz4
    git checkout master
    cp -R '/tmp/source/lz4' '/tmp/build'
    pushd /tmp/build/lz4
    make
    sudo make install
    popd
    popd
}
