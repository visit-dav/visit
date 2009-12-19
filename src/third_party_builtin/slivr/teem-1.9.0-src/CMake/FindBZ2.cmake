#
# Find the native BZ2 includes and library
#
# BZ2_INCLUDE_DIR - where to find bzlib.h, etc.
# BZ2_LIBRARIES   - List of fully qualified libraries to link against when using bzlib.
# BZ2_FOUND       - Do not attempt to use zlib if "no" or undefined.

FIND_PATH(BZ2_INCLUDE_DIR bzlib.h
  /usr/local/include
  /usr/include
)

FIND_LIBRARY(BZ2_LIBRARY bz2
  /usr/lib
  /usr/local/lib
)

IF(BZ2_INCLUDE_DIR)
  IF(BZ2_LIBRARY)
    SET( BZ2_LIBRARIES ${BZ2_LIBRARY} )
    SET( BZ2_FOUND "YES" )
  ENDIF(BZ2_LIBRARY)
ENDIF(BZ2_INCLUDE_DIR)

MARK_AS_ADVANCED(
  BZ2_LIBRARY
  BZ2_INCLUDE_DIR
  )
