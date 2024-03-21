# Script to create a VisIt release tarball from a container.
#
# It must be in the same directory as the Docker file and supporting
# scripts for building TP libs, and building, installing and testing VisIt.
#
# For use on LC.
# 
# You must run this script on an allocated node, so first run:
#   salloc -N 1 -t 300 --userns
#
# Note: the five hours requested for the allocation might be overkill,
# but you don't want to run out of allocation before all the steps
# are completed, because the container will be deleted.
#
# The script can be used with other docker files as long as they are known
# to work from end-to-end. Just modify the necessary docker file name
# and image tags.

/collab/usr/gapps/lcweg/containers/scripts/enable-podman.sh
podman build -f Dockerfile-ubuntu22 -t visitdev:3.4.1-ubuntu22 

podman save visitdev:3.4.1-ubuntu22 > visitdev3.4.1-ubuntu22.tar

podman create --name dummy visitdev:3.4.1-ubuntu22

podman cp dummy:/home/visit/visit3_4_1.linux-x86_64.tar.gz ./visit3_4_1.linux-x86_64-ubuntu22.tar.gz
podman cp dummy:/home/visit/visit0000.png .

podman rm -f dummy

