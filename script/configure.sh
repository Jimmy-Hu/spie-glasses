#!/bin/bash
this_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source_dir="${this_dir}/.."
binary_dir="${this_dir}/../build"
install_dir="${this_dir}/../stage"
if test ! -d "${binary_dir}"
then
    test -e "${binary_dir}" && exit 1
    mkdir "binary_dir"
fi
pushd "${binary_dir}" >/dev/null
if test ! -z ${MSYSTEM+defined}
then
    cmake -GUnix\ Makefiles \
          "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON" \
          "-DCMAKE_PREFIX_PATH=${MSYSTEM_PREFIX}/local" \
          "-DCMAKE_INSTALL_PREFIX=${install_dir}" \
          -B"${binary_dir}" -H"${source_dir}" "$@"
else
    cmake \
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON" \
        "-DCMAKE_INSTALL_PREFIX=${install_dir}" \
        -B"${binary_dir}" -H"${source_dir}" "$@"
fi
popd >/dev/null
