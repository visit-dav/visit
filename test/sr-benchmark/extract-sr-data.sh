#!/bin/sh
#####
# Tom Fogal, Mon Jul 14 06:58:20 PDT 2008
# For the VisIt project.
#####
# Searches for `.timing' files from the current directory.  Extracts the
# total time time for scalable renderings from both the IceT and traditional
# rendering paths.  Creates a file named $DATA with this information, in the
# format:
#
#      # renderer type     render time
#      0                   0.074
#      0                   0.128
#      1                   0.064
#      0                   0.042
#      ...
#
# `0' means the traditional rendering path.
# `1' means the IceT rendering path.
#####
DATA="sr.data"
STRING_MATCH="NM::Render"
DIR="${@-.}"

find ${DIR} -iname \*.timings -exec grep ${STRING_MATCH} {} > .pre.data \;
sort -r -o .post.data .pre.data
awk -f timing.awk .post.data > ${DATA}
gnuplot rendering.gnu
