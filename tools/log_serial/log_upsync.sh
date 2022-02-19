#!/bin/bash

source ./stations.sh

# dropbox_token.conf contains the token without newline
TOKEN=$(cat dropbox_token.conf)

python dropbox_upsync.py  \
    --token ${TOKEN}  \
    "${dropbox_dir}"  \
    "${log_dir}" --yes
