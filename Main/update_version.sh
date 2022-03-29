#!/bin/bash

version=$(git describe --tags --long)
# version=$(git rev-parse --short HEAD)

echo "#ifndef __VERSION_H__
#define __VERSION_H__


#define __GIT_SHA1_ID__ \"${version}\"


#endif  // __GIT_SHA1_ID__"  >  version.h
