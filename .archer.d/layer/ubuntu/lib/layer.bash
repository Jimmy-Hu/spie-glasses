layer_help() {
    echo "lib"
}

layer_metadata() {
    LAYER_DEPENDENCIES=(
        lib/boost
        lib/opencv
        lib/dlib
        lib/lz4
        lib/zlib
        lib/nlohmann-json
        lib/msgpack
    )
}

layer_is_installed() {
    true
}

layer_install() {
    true
}
