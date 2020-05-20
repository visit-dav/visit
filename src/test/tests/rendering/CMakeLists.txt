# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#****************************************************************************

SET(PY_TESTS)

PY_3RDPARTY_TEST(PY_TESTS SILO
    annotation axistitles bigdata colortexture compositing
    evalcubic image legends lighting line2d line3d offscreensave
    renderpoints saveformats scalable2 scalable shadows specular text2d
    text3d timeslider transparency tuvok viewChange view volume ospray)

SET(TEST_EXTRA_ARGS)
IF (NOT HAVE_OSMESA)
    SET(TEST_EXTRA_ARGS "--threshold-error=legends_11:12,legends_12:12,renderpoint_2_05:13")
    SET(TEST_EXTRA_ARGS "${TEST_EXTRA_ARGS},renderpoint_2_06:18,renderpoint_2_07:12,transparency_06:30")
ENDIF()

ADD_VISIT_PY_TESTS(Rendering "${PY_TESTS}" ${TEST_EXTRA_ARGS})
