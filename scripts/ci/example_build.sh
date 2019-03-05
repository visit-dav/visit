#!/bin/bash

# remove old source tarball if it exists
echo "rm -f visit.masonry.docker.src.tar"
rm -f visit.masonry.docker.src.tar

# get current copy of masonry
echo "cd ../../src/tools/dev/"
cd ../../src/tools/dev/
echo 'tar -czvf ../../../scripts/ci/visit.masonry.docker.src.tar --exclude "build-*" --exclude "*.pyc" masonry'
tar -czvf ../../../scripts/ci/visit.masonry.docker.src.tar --exclude "build-*" --exclude "*.pyc" masonry

# change back to the dir with our Dockerfile
echo "cd ../../../scripts/ci/"
cd ../../../scripts/ci/

# exec docker build to create image
echo "docker build -t visit-ci-ubuntu:current ."
docker build -t visit-ci-ubuntu:current .
