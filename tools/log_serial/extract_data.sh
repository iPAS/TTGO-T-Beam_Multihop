#!/bin/bash

source ./env.sh

python extract_data.py --log_dir "${log_dir}" --response_db "${response_db}"
