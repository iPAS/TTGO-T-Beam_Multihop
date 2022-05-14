#!/bin/bash

here=$(dirname $(readlink -f "$0"))
source "${here}/env.sh"

python upload_data.py --db "${response_db}"
