# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Wed Oct 19 09:58:16 MST 2011
#   Remove ${VISIT_MSVC_VERSION} from lib location.
#
#   Kathleen Biagas, Mon Mar 31 2025
#   Utilize visit_import_third_party.
#
#   Kathleen Biagas, Wed Mar 4, 2026
#   Update h5part library name on windows.
#
#****************************************************************************/

# Uses the H5PART_DIR hint from the config-site .cmake file

visit_import_third_party(H5PART LIBNAMES H5Part h5part)

