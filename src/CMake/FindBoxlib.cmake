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
#****************************************************************************/

# Use the BOXLIB_DIR hint from the config-site .cmake file 

IF (WIN32)
  SET_UP_THIRD_PARTY(BOXLIB LIBS BoxLib2D BoxLib3D)
ELSE (WIN32)
  SET_UP_THIRD_PARTY(BOXLIB LIBS box2D box3D)
ENDIF (WIN32)

IF(BOXLIB_FOUND)
  # place the 2D and 3D libraries into separate vars for plugin use.
  LIST(GET BOXLIB_LIB 0 tmp)
  SET(BOXLIB_2D_LIB ${tmp} CACHE STRING "2D boxlib" FORCE)

  LIST(GET BOXLIB_LIB 1 tmp)
  SET(BOXLIB_3D_LIB ${tmp} CACHE STRING "3D boxlib" FORCE)

  if(WIN32)
     set(BOXLIB_WIN32_DEFINES "BL_FORT_USE_UPPERCASE")
  endif()

  # unset unneeded vars.
  UNSET(tmp)
  UNSET(BOXLIB_LIB CACHE)
ENDIF(BOXLIB_FOUND)

