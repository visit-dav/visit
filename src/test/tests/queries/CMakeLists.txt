# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#****************************************************************************

SET(PY_TESTS kurtosis)

PY_3RDPARTIES_TEST(TESTLIST PY_TESTS
    LIBNAMES SILO NETCDF
    TESTNAMES conncomp)

PY_3RDPARTIES_TEST(TESTLIST PY_TESTS
    LIBNAMES SILO BOXLIB MILI
    TESTNAMES pick)

PY_3RDPARTIES_TEST(TESTLIST PY_TESTS
    LIBNAMES SILO MILI
    TESTNAMES queriesOverTime)

PY_3RDPARTY_TEST(PY_TESTS SILO
    avg_value bestfitline casetest centroid consistencyChecks curvature
    database hohlraum IntegralCurveInfo l2norm length pickarray
    pickcleanzonesonly pickNamedArgs py_queries queryMultiWindow
    revolved_surface_area scf watertight xrayimage)

IF (HAVE_OSMESA)
    # osmesa rasterizes lines differently and
    # thus this test fails with other GL
    PY_3RDPARTY_TEST(PY_TESTS SILO line_scan)
ENDIF()

ADD_VISIT_PY_TESTS(Queries "${PY_TESTS}")
