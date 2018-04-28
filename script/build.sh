#!/bin/bash
this_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source_dir="${this_dir}/.."
binary_dir="${this_dir}/../build"
install_dir="${this_dir}/../stage"
if test ! -e "${binary_dir}"
then
    bash "${this_dir}/configure.sh"
fi
pushd "${binary_dir}" >/dev/null
cmake --build "${binary_dir}" "${@}"
popd >/dev/null
