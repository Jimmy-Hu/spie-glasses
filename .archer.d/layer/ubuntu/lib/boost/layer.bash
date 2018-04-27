layer_help() {
    echo "lib/boost"
}

layer_metadata() {
    LAYER_DEPENDENCIES=(
        app/git
        app/gcc
        app/cmake
        lang/c-c++
    )
}

layer_is_installed() {
    test -d /usr/local/include/boost
}

layer_install() {
    test -f /tmp/archive/boost_1_67_0.tar.gz &&
    wget https://dl.bintray.com/boostorg/release/1.67.0/source/boost_1_67_0.tar.gz -P /tmp/archive/boost
    pushd /tmp/source/boost
    mkdir -p /tmp/source/boost 2>/dev/null
    mkdir -p /tmp/build/boost 2>/dev/null
    test ! -d /tmp/source/boost/boost_1_67_0 && tar -zxvf /tmp/archive/boost/boost_1_67_0.tar.gz -C /tmp/source/boost
    test -z "$(ls -A /tmp/build/boost)" && cp -a /tmp/source/boost/boost_1_67_0/. /tmp/build/boost
    pushd /tmp/build/boost
    ./bootstrap.sh --prefix=/usr/local
    sudo ./b2 --prefix=/usr/local \
         --layout=system \
         -a \
         -j8 \
         install
        # --show-libraries \
        # -n \
        # toolset=gcc \
        # variant=release \
        # link=shared \
        # threading=multi \
    popd
    popd

}
