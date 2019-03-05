#!/bin/bash
set -ev
mkdir travis-build
cd travis-build
cmake -DVISIT_CONFIG_SITE:PATH="/visit-ci-cfg.cmake" ../src
make -j 2