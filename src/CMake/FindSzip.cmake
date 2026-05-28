# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Tues Oct 1 09:33:47 MST 2013
#   Removed VISIT_MSVC_VERSION from windows handling.
#
#   Kathleen Biagas, Mon Mar 31, 2025
#   Utilize visit_import_third_party
#
#****************************************************************************/

# Uses the SZIP_DIR hint from the config-site .cmake file

visit_import_third_party(SZIP LIBNAMES sz szip)

