# This is the "site file" for use when building VisIt on Windows
# with the Bilder methodology, which has no use of IDEs.
#
# See https://ice.txcorp.com/trac/bilder/wiki/BilderOnWindows
#
# Contact cary@txcorp.com if any problems
#
# $Id: windows-bilder.cmake 484 2011-08-04 23:12:43Z cary $
#

# Version issues
IF (MSVC_VERSION)
  SET(ERRMSG "You have chosen a compiler version not currently supported.  You will need to build all dependent 3rd party libraries with this compiler, and provide the correct locations for them.")
  IF (NOT MSVC_VERSION LESS 1600)
    MESSAGE(WARNING "${ERRMSG}")
  ELSEIF (NOT MSVC_VERSION LESS 1500)
    SET(VISIT_MSVC_VERSION "MSVC90")
  ELSEIF (NOT MSVC_VERSION LESS 1400)
    SET(VISIT_MSVC_VERSION "MSVC80")
  ELSEIF (NOT MSVC_VERSION LESS 1310)
    SET(VISIT_MSVC_VERSION "MSVC71")
  ELSE ()
    MESSAGE(SEND_ERROR "${ERRMSG}")
  ENDIF ()
ELSE ()
# For some reason, cmake does not set MSVC_VERSION when .Net2003 compiler
# is chosen. Check a different flag.
  IF (MSVC71)
    SET(VISIT_MSVC_VERSION "MSVC71")
  ELSE (MSVC71)
    MESSAGE(SEND_ERROR "${ERRMSG}")
  ENDIF (MSVC71)
ENDIF ()

# Modify for x64
IF (CMAKE_CL_64)
  SET(VISIT_MSVC_VERSION "${VISIT_MSVC_VERSION}-x64")
ENDIF ()

# To find the resources
GET_FILENAME_COMPONENT(VISIT_WINDOWS_DIR ${VISIT_SOURCE_DIR}/../visitwindows REALPATH)

