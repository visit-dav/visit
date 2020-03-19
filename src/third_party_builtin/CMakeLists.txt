# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#  Cyrus Harrison, Tue Oct  9 15:44:53 PDT 2012
#  Added tess2
#
#  Cyrus Harrison, Mon Jan 26 21:26:34 PST 2015
#  Removed bilib
#
#  Kathleen Biagas, Thu May  2 13:05:57 PDT 2019
#  Removed zlib.
#
#****************************************************************************

PROJECT(thirdparty_builtins)

ADD_SUBDIRECTORY(verdict)
ADD_SUBDIRECTORY(tess2)
IF(VISIT_PARALLEL)
    ADD_SUBDIRECTORY(cognomen)
ENDIF(VISIT_PARALLEL)
ADD_SUBDIRECTORY(bow)
ADD_SUBDIRECTORY(nek5000/jl)
