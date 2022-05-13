#!/bin/bash

source ./env.sh

python extract_data.py --log_dir "${log_dir}" --db "${response_db}"
