# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#****************************************************************************

SET(PY_TESTS)

PY_3RDPARTY_TEST(PY_TESTS SILO
    colortable correlationsession legendproperties rect3d-contour
    restorewithsources selection sessionexpression sessionview
    simplekeyframe textannot view3d wavecontour)

SET(TEST_EXTRA_ARGS)
IF (NOT HAVE_OSMESA)
    SET(TEST_EXTRA_ARGS "--threshold-error=wavecontour00:12")
ENDIF()

ADD_VISIT_PY_TESTS(Session "${PY_TESTS}" ${TEST_EXTRA_ARGS})
