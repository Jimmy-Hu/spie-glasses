layer_help() {
    echo "lib/msgpack"
}

layer_metadata() {
    LAYER_DEPENDENCIES=(
        app/git
        app/cmake
        lang/c-c++
    )
}

layer_is_installed() {
    test -f /usr/local/include/msgpack.h
}

layer_install() {
    test ! -e /tmp/source/msgpack && git clone https://github.com/msgpack/msgpack-c /tmp/source/msgpack
    mkdir -p /tmp/build/msgpack 2>/dev/null
    pushd /tmp/source/msgpack
    git checkout master
    cmake -B/tmp/build/msgpack -H/tmp/source/msgpack
    # cmake --build /tmp/build/msgpack
    sudo cmake --build /tmp/build/msgpack --target install
    popd
}
