# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Bonnell, Wed Jan 05 10:25:13 PST 2010
#   Add FOLDER property.
#
#****************************************************************************

set(mpeg2encode_SOURCES
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
    writepic.c)

set(mpeg2encode_HEADERS
   config.h
   global.h
   mpeg2enc.h
   vlc.h)

visit_add_executable(
    NAME        mpeg2encode
    SOURCES     ${mpeg2encode_SOURCES}
    HEADERS     ${mpeg2encode_HEADERS}
    INCLUDES    ${CMAKE_CURRENT_SOURCE_DIR}
    FOLDER      tools/mpeg2encode)

if(NOT WIN32)
    visit_patch_target(
        NAME        mpeg2encode
        DEPENDS_ON  m)
endif()

