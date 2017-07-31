#!/bin/sh
this_dir="$(dirname $(readlink -f $0))"
binary_dir="${this_dir}/../build"
install_dir="${this_dir}/../stage"
pushd "${binary_dir}" >/dev/null
cmake --build . "$@"
popd >/dev/null
