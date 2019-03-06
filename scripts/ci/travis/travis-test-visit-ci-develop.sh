#!/bin/bash
set -ev
cd /home/build
mkdir travis-build
cd travis-build
cmake -DVISIT_CONFIG_SITE:PATH="/visit-ci-cfg.cmake" ../src
make -j 2