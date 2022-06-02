#!/bin/bash

here=$(dirname $(readlink -f "$0"))
source "${here}/env.sh"
pushd . >/dev/null
cd "${here}"

python show_cm_ls.py --log_dir "${log_dir}" 2>&1

popd >/dev/null
echo

