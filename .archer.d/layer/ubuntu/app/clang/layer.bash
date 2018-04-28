layer_help() {
    echo "app/clang"
}

layer_metadata() {
    LAYER_DEPENDENCIES=(
    )
}

layer_is_installed() {
    command -v clang 1>/dev/null 2>/dev/null
}

layer_install() {
    sudo apt-get -y install clang
}
