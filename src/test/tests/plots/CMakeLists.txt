# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

SET(PY_TESTS
    truecolor)

PY_3RDPARTY_TEST(PY_TESTS SILO
    contour curve mesh multicolor parcoords pseudocolor ray_trace scatter
    singlecolor surface tensor vector volumePlot)

PY_3RDPARTIES_TEST(TESTLIST PY_TESTS
    LIBNAMES SILO BOXLIB
    TESTNAMES label subset)

SET(TEST_EXTRA_ARGS)
IF (NOT HAVE_OSMESA)
    SET(TEST_EXTRA_ARGS "--threshold-error=ray_trace_06:12,ray_trace_07:12,hf_0_1:21")
ENDIF()

ADD_VISIT_PY_TESTS(Plots "${PY_TESTS}" ${TEST_EXTRA_ARGS})
