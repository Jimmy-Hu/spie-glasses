layer_help() {
    echo "lib/dlib"
}

layer_metadata() {
    LAYER_DEPENDENCIES=(
        app/git
        app/cmake
        lang/c-c++
        lib/opencv
    )
}

layer_is_installed() {
    test -d /usr/local/include/dlib
}

layer_install() {
    test ! -e /tmp/source/dlib && git clone https://github.com/davisking/dlib /tmp/source/dlib
    mkdir -p /tmp/build/dlib 2>/dev/null
    pushd /tmp/source/dlib
    git checkout master
    cmake -B/tmp/build/dlib -H/tmp/source/dlib -DCMAKE_PREFIX_PATH=/usr/local
    # cmake --build /tmp/build/dlib
    sudo cmake --build /tmp/build/dlib --target install
    popd
}
