REM Script to create a VisIt release tarball from a container end-to-end.
REM This example is for debian12 version of VisIt 3.4.1
REM Place it in the same directory as the Docker file being used along with
REM the other scripts needed.

REM Can be modified for other containers, as long as the Dockerfile is known
REM to work without intervention, building TP libs, and building, installing,
REM and testing VisIt.
REM The extracted visit0000.png should be inspected for a valid image.

docker build -f Dockerfile-debian12 -t visitdev:3.4.1-debian12 .

docker create --name dummy visitdev:3.4.1-debian12
docker cp dummy:/home/visit/visit3_4_1.linux-x86_64.tar.gz ./visit3_4_1.linux-x86_64-debian12.tar.gz
docker cp dummy:/home/visit/visit0000.png .
docker rm -f dummy

