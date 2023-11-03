# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

SET(PY_TESTS cracksclipper multires tube)

PY_3RDPARTY_TEST(PY_TESTS SILO
    box clip defer_expr dual_mesh elevate explode extrudeStacked ex_surf
    ic_geometry ic_integration ic_streamlines ic_termination indexselect
    isovolume moveoperators onionpeel operators radial_resample reflect
    resample revolve slice sph_resample threeslice threshold transform)

PY_3RDPARTY_TEST(PY_TESTS NETCDF cart_proj)
#PY_3RDPARTY_TEST(PY_TESTS PICS ic_pathlines ic_pics lcs lcs_ic lcs_lc)

PY_3RDPARTIES_TEST(TESTLIST PY_TESTS
     LIBNAMES SILO BOXLIB
     TESTNAMES displace inverse_ghost_zone)

PY_3RDPARTY_TEST(PY_TESTS HDF5 persistent_particles)

SET(TEST_EXTRA_ARGS)
IF (APPLE)
    SET(TEST_EXTRA_ARGS "--threshold-error=ops_resampleop_curv2d:16")
ENDIF()

ADD_VISIT_PY_TESTS(Operators "${PY_TESTS}" ${TEST_EXTRA_ARGS})
