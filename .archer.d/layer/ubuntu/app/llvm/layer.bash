layer_help() {
    echo "app/llvm"
}

layer_metadata() {
    LAYER_DEPENDENCIES=(
    )
}

layer_is_installed() {
    command -v llvm-config 1>/dev/null 2>/dev/null
}

layer_install() {
    sudo apt-get -y install llvm
}
