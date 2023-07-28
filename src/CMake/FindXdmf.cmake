# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Bonnell, Thu May 27 17:01:22 MST 2010
#   Windows builds can be handled the same as non-windows for XDMF.
#
#   Mark C. Miller, Fri Jul 30 22:06:06 PDT 2010
#   Removed logic setting XDMF_DIR (that is done to SET_UP_THIRD_PARTY)
#   as well as FIND_PACKAGE (also done by SET_UP_THIRD_PARTY).
#
#   Brad Whitlock, Fri Apr  6 11:00:10 PDT 2012
#   Also look for vtklibxml2 if we're building statically.
#
#   Cyrus Harrison, Fri Apr  6 11:00:10 PDT 2012
#   Static build: Only look for vtklibxml2 on OSX.
#
#   Cyrus Harrison, Tue Apr 10 13:07:08 PDT 2012
#   Revert to standard setup. Build_visit now handles vtk deps correctly
#   in the generated config-site.
#
#   Kathleen Biagas, Fri May 3 16:55:12 MST 2013
#   If our xdmf depends on vtlibxml2, ensure it exists.
#
#   Kathleen Biagas, Fri May 3 16:55:12 MST 2013
#   Don't test for vtklibxml2 here, SET_UP_THIRD_PARTY checks already,
#   and other libs with LIBDEP don't perform an extra check.
#
#****************************************************************************/

# Use the XDMF_DIR hint from the config-site .cmake file
#

SET_UP_THIRD_PARTY(XDMF LIBS Xdmf)

