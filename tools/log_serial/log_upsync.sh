#!/bin/bash

# dropbox_token.conf contains the token without newline
TOKEN=$(cat dropbox_token.conf)

python dropbox_upsync.py  \
    --token ${TOKEN}  \
    "log"  \
    log --yes
