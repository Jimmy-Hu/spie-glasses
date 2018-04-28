layer_help() {
    echo "lib/fmt"
}

layer_metadata() {
    LAYER_DEPENDENCIES=(
        app/git
        app/cmake
        lang/c-c++
    )
}

layer_is_installed() {
    test -f /usr/local/include/fmt/format.h
}

layer_install() {
    test ! -e /tmp/source/fmt && git clone https://github.com/fmtlib/fmt /tmp/source/fmt
    mkdir -p /tmp/build/fmt 2>/dev/null
    cmake -B/tmp/build/fmt -H/tmp/source/fmt
    cmake --build /tmp/build/fmt
    sudo cmake --build /tmp/build/fmt --target install
}
