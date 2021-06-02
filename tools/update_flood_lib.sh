#!/bin/bash

sources='flood.c flood.h commqueue.c commqueue.h queue.c queue.h delivery_hist.c delivery_hist.h'
srcdir=motelib_flooding

for f in $sources; do
    if [ "${f: -2}" == ".c" ]; then
        cp  -v "${srcdir}/$f" src/${f/.c/.cpp}
    else
        cp  -v "${srcdir}/$f" src/
    fi
done
