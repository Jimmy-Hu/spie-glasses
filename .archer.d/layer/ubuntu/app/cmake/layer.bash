layer_help() {
    echo "app/cmake"
}

layer_metadata() {
    LAYER_DEPENDENCIES=(
    )
}

layer_is_installed() {
    command -v cmake 1>/dev/null 2>/dev/null
}

layer_install() {
    sudo apt-get -y install cmake
}
