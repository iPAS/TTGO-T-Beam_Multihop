#!/bin/bash

here=$(dirname $(readlink -f "$0"))
source "${here}/env.sh"
pushd . >/dev/null
cd "${here}"

python upload_data.py --db "${response_db}" 2>&1

popd >/dev/null
echo

