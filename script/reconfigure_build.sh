#!/bin/sh
this_dir="$(dirname $(readlink -f $0))"
source_dir="${this_dir}/.."
binary_dir="${this_dir}/../build"
install_dir="${this_dir}/../stage"

test -d "${binary_dir}" && rm -rf -- "${binary_dir}" 
mkdir "${binary_dir}"
pushd "${binary_dir}" >/dev/null
if test ! -z ${MSYSTEM+defined}
then
    cmake -GUnix\ Makefiles \
          "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON" \
          "-DCMAKE_PREFIX_PATH=${MSYSTEM_PREFIX}/local" \
          "-DCMAKE_INSTALL_PREFIX=${install_dir}" \
          "$@" "${source_dir}"
else
    cmake \
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON" \
        "-DCMAKE_INSTALL_PREFIX=${install_dir}" \
        "$@" "${source_dir}"
fi
cmake --build .
popd >/dev/null
