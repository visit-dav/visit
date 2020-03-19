# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Bonnell, Wed Jan 05 10:25:13 PST 2010
#   Add FOLDER property.
#
#****************************************************************************

SET(MPEG2ENCODE_SOURCES
conform.c
fdctref.c
idct.c
motion.c
mpeg2enc.c
predict.c
putbits.c
puthdr.c
putmpg.c
putpic.c
putseq.c
putvlc.c
quantize.c
ratectl.c
readpic.c
stats.c
transfrm.c
writepic.c
)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
)

ADD_EXECUTABLE(mpeg2encode ${MPEG2ENCODE_SOURCES})
IF(NOT WIN32)
    TARGET_LINK_LIBRARIES(mpeg2encode m)
ENDIF(NOT WIN32)

VISIT_INSTALL_TARGETS(mpeg2encode)
VISIT_TOOLS_ADD_FOLDER(mpeg2encode)
