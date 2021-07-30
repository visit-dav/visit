# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#****************************************************************************

SET(PY_TESTS
    #embedded
    launcher
    mrucache
    namescheme
    protocol
    stringhelpers
    annotation_objects
    )

ADD_VISIT_PY_TESTS(Unit "${PY_TESTS}")
