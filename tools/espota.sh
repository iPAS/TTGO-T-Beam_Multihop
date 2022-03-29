#!/bin/bash

whereami() {
    SOURCE=$1
    [[ "$SOURCE" == "" ]] && SOURCE="${BASH_SOURCE[ $((${#BASH_SOURCE[*]}-1)) ]}"  # Last one is the first that was run.

    while [ -h "$SOURCE" ]; do  # resolve $SOURCE until the file is no longer a symlink
        DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
        SOURCE="$(readlink "$SOURCE")"
        [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE"  # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
    done
    DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
    echo $DIR
}

currentpos() {
    whereami "${BASH_SOURCE[ 0 ]}"
}

python $(currentpos)/espota.py -r -i 192.168.4.1 -p 3232 --auth= -f src/tmp/Main.ino.bin
