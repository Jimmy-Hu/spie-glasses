layer_help() {
    echo "lib/c-c++"
}

layer_metadata() {
    LAYER_DEPENDENCIES=(
        app/git
        app/cmake
        app/gcc
        app/clang
        app/llvm
    )
}

layer_is_installed() {
    true
}

layer_install() {
    true
}
