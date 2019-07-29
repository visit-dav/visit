# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

SET(PY_TESTS clonecopy curve_anim curve_expressions defvar_anim
    expr_cmfe image_proc lambda2 qcrit time_cmfe time_deriv_cmfe)

PY_3RDPARTY_TEST(PY_TESTS NETCDF missingdata)

PY_3RDPARTY_TEST(PY_TESTS SILO
    cleanzonesonly clonefirstref conditional conn_cmfe coord_expr
    cylindrical_radius ddf expressions exprList expr2engine
    field_operators keyframe lineout locktime locus matvf math_expr
    matsel mesh_quality mir mir_cache multivar pos_cmfe py_exprs
    replace selections sil specmf symm timelock time_iteration
    val4mat)

PY_3RDPARTIES_TEST(TESTLIST PY_TESTS
     LIBNAMES SILO BOXLIB
     TESTNAMES simplify_mixed)

SET(TEST_EXTRA_ARGS)
IF (NOT HAVE_OSMESA)
    SET(TEST_EXTRA_ARGS "--threshold-error=image_proc_01:15,LineoutSAMRAI:41")
    SET(TEST_EXTRA_ARGS "${TEST_EXTRA_ARGS},CurvesFromSAMRAI_noSampling:15")
    SET(TEST_EXTRA_ARGS "${TEST_EXTRA_ARGS},CurvesFromSAMRAI_withSampling:15")
    SET(TEST_EXTRA_ARGS "${TEST_EXTRA_ARGS},ddf_02:15,ddf_03:15")
ENDIF()

ADD_VISIT_PY_TESTS(Hybrid "${PY_TESTS}" ${TEST_EXTRA_ARGS})
