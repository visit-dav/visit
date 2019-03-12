#!/bin/bash
set -ev
export PATH=$PATH:/masonry/build-mb-develop-ci-smoke/thirdparty_shared/third_party/cmake/3.9.3/ci/bin/
mkdir ci-build
cd ci-build
cmake -DVISIT_CONFIG_SITE:PATH="/visit-ci-cfg.cmake" ../src
make -j 4
