#!/bin/bash

sources='flood.c flood.h commqueue.c commqueue.h queue.c queue.h delivery_hist.c delivery_hist.h'
srcdir=motelib_flooding

for f in $sources; do
    cp -f -v "${srcdir}/$f" src/
done
