# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#****************************************************************************

SET(PY_TESTS
    amr
    aresamr
    csg
    curve
    domainbounds
    domainlist
    ghostcells
    globalids
    interleave
    life
    mandelbrot
    material
    materialvar
    mesh
    multiblock
    point
    polyhedral
    scalar
    species
    ucdcurve
    unstructured
    updateplots
    var
    zerocopy)

SET(TEST_EXTRA_ARGS)
IF (NOT HAVE_OSMESA)
    SET(TEST_EXTRA_ARGS "--threshold-error=mandelbrot02:12")
ENDIF()

ADD_VISIT_PY_TESTS(Simulation "${PY_TESTS}" ${TEST_EXTRA_ARGS})
