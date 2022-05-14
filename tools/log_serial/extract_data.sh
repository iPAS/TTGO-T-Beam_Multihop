#!/bin/bash

here=$(dirname $(readlink -f "$0"))
source "${here}/env.sh"

python extract_data.py --log_dir "${log_dir}" --db "${response_db}"
