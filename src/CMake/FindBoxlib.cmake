# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Bonnell, Thu Dec  3 10:30:15 PST 2009
#   Use only 1 if-def block, fix libarary names for windows.
#
#   Eric Brugger, Fri Jan  7 13:50:15 PST 2011
#   I replaced the BOXLIB2D and BOXLIB3D variables with just BOXLIB.
#
#   Kathleen Bonnell, Thu Jan 13 15:21:47 MST 2011
#   Restore separate vars for libraries (to handle different names on
#   different platforms).
#
#   Kathleen Bonnell, Mon Jan 17 17:24:44 MST 2011
#   Don't set BOXLIB_2D/3D_LIB unless BOXLIB_FOUND.
#
#   Kathleen Biagas, Tues Oct 1 09:33:47 MST 2013
#   Removed VISIT_MSVC_VER from Windows handling.
#
#   Kathleen Biagas, Thu July 15, 2021
#   Add BOXLIB_WIN32_DEFINES (used by xml2cmake to add preprocessor defines).
#
#   Kathleen Biagas, Fir Mar 21, 2025
#   Utilize visit_import_third_party.
#
#****************************************************************************/

# Uses the BOXLIB_DIR hint from the config-site .cmake file

set(BOXLIB2D_DIR ${BOXLIB_DIR})

visit_import_third_party(BOXLIB2D
     LIBNAMES     BoxLib2D box2D
     DEFINES      BL_SPACEDIM=2
     WIN32DEFINES BL_FORT_USE_UPPERCASE)

set(BOXLIB3D_DIR ${BOXLIB_DIR})

visit_import_third_party(BOXLIB3D
     LIBNAMES     BoxLib3D box3D
     DEFINES      BL_SPACEDIM=3
     WIN32DEFINES BL_FORT_USE_UPPERCASE)

