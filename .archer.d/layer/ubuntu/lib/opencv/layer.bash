layer_help() {
    echo "lib/opencv"
}

layer_metadata() {
    LAYER_DEPENDENCIES=(
        app/git
        app/cmake
        lang/c-c++
    )
}

layer_is_installed() {
    test -d /usr/local/include/opencv2
}

layer_install() {
    test ! -e /tmp/source/opencv && git clone https://github.com/opencv/opencv /tmp/source/opencv
    mkdir -p /tmp/build/opencv 2>/dev/null
    pushd /tmp/source/opencv
    git checkout 3.4
    cmake -B/tmp/build/opencv -H/tmp/source/opencv
    # cmake --build /tmp/build/opencv
    sudo cmake --build /tmp/build/opencv --target install
    popd
}
