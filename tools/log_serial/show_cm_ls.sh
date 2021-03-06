#!/bin/bash

here=$(dirname $(readlink -f "$0"))
source "${here}/env.sh"
pushd . >/dev/null
cd "${here}"

function get_lines () {
    python show_cm_ls.py --log_dir "${log_dir}" 2>&1
}

data=`get_lines`

echo
echo 'Oldest:'
echo "${data}" | grep '.* .* .* .*' | sort    -t\  -k3,4 -k1,2 | uniq -f2
echo
echo 'Newest:'
echo "${data}" | grep '.* .* .* .*' | sort -r -t\  -k3,4 -k1,2 | uniq -f2

popd >/dev/null
echo
