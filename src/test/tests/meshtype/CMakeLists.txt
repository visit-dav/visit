# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

SET(PY_TESTS degen_mesh)

PY_3RDPARTY_TEST(PY_TESTS SILO
    arbpoly csg globe point emptydomains rect3d)

ADD_VISIT_PY_TESTS(MeshType "${PY_TESTS}")
