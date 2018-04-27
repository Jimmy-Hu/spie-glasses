layer_help() {
    echo "."
}

layer_metadata() {
    LAYER_DEPENDENCIES=(
        lang/c-c++
        lib
    )
}

layer_is_installed() {
    true
}

layer_install() {
    true
}
