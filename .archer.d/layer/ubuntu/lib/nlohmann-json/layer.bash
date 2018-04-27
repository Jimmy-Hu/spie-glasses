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
    test -f /usr/local/include/nlohmann/json.hpp
}

layer_install() {
    test ! -e /tmp/source/nlohmann-json && git clone https://github.com/nlohmann/json /tmp/source/nlohmann-json
    mkdir -p /tmp/build/nlohmann-json 2>/dev/null
    pushd /tmp/source/nlohmann-json
    git checkout master
    cmake -B/tmp/build/nlohmann-json -H/tmp/source/nlohmann-json
    # cmake --build /tmp/build/nlohmann-json
    sudo cmake --build /tmp/build/nlohmann-json --target install
    popd
}
