#!/bin/bash

echo "#ifndef __VERSION_H__
#define __VERSION_H__


#define __GIT_SHA1_ID__ \"$(git rev-parse --short HEAD)\"


#endif  // __GIT_SHA1_ID__"  >  version.h
