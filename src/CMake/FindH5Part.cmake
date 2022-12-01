# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Wed Oct 19 09:58:16 MST 2011
#   Remove ${VISIT_MSVC_VERSION} from lib location.
#
#****************************************************************************/

# Use the H5PART_DIR hint from the config-site .cmake file

IF (WIN32)
  SET_UP_THIRD_PARTY(H5PART LIBS h5partdll)
ELSE (WIN32)
  SET_UP_THIRD_PARTY(H5PART LIBS H5Part)
ENDIF (WIN32)

