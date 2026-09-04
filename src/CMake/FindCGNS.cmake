# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Bonnell, Tue Apr 20 19:11:27 MST 2010
#   Change lib name on windows from cgnslib to cgnsdll.
#
#   Kathleen Biagas, Wed Oct 19 09:58:16 MST 2011
#   Remove ${VISIT_MSVC_VERSION} from lib location.
#
#   Kathleen Biagas, Thu July 15, 2021
#   Add CGNS_WIN32_DEFINES (used by xml2cmake to add preprocessor defines).
#
#   Kathleen Biagas, Mon Mar 31, 2025
#   Utilize visit_import_third_party.
#
#****************************************************************************/

# Uses the CGNS_DIR hint from the config-site .cmake file

visit_import_third_party(CGNS
     LIBNAMES     cgns
                  cgnsdll
     WIN32DEFINES USE_DLL)

