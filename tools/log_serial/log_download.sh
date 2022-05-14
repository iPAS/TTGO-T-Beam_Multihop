#!/bin/bash

here=$(dirname $(readlink -f "$0"))
source "${here}/env.sh"

python dropbox_download.py

