#!/bin/bash

function whereami() {
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

function currentpos() {
    whereami "${BASH_SOURCE[ 0 ]}"
}

function make_version () {
    echo "$(git describe --tags), $(git describe --all --long)"
}

version_file="$(currentpos)/../Main/version.h"
echo "#ifndef __VERSION_H__
#define __VERSION_H__


#define __GIT_SHA1_ID__ \"$(make_version)\"


#endif  // __GIT_SHA1_ID__
" > ${version_file}



git add ${version_file}

# read -p '... for debugging, press to continue ...'

exit 0
