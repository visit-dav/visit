# - Extract information from a subversion working copy using TortoiseSVN
# The module defines the following variables:
#  TortoiseSVN_EXECUTABLE - path to svn command line client
#  TortoiseSVN_FOUND - true if the command line client was found
# If the client executable is found the macro
#  TortoiseSVN_SubWCREV(<dir> <var-prefix>)
# is defined to extract information of a subversion working copy at
# a given location. The macro defines the following variables:
#  <var-prefix>_WC_REVISION - current revision
#  <var-prefix>_WC_INFO - output of command `SubWCRev <dir>'
# Example usage:
#  FIND_PACKAGE(TortoiseSVN)
#  IF(TortoiseSVN_FOUND)
#    TortoiseSVN_WC_INFO(${PROJECT_SOURCE_DIR} Project)
#    MESSAGE("Current revision is ${Project_WC_REVISION}")
#  ENDIF(TortoiseSVN_FOUND)
#
#------------------------------------------------------------------------------
#  Modifications:
#    Kathleen Bonnell, Wed Apr 14 16:15:23 MST 2010
#    Use "SubWCRev WorkingCopyPath SrcVersionFile DstVersionFile" version of
#    command to extract revision number.  More consistent results this way.
#------------------------------------------------------------------------------

SET(TortoiseSVN_FOUND FALSE)

FIND_PROGRAM(TortoiseSVN_EXECUTABLE SubWCRev
  DOC "Tortoise SVN client")
MARK_AS_ADVANCED(TortoiseSVN_EXECUTABLE)

IF(TortoiseSVN_EXECUTABLE)
  SET(TortoiseSVN_FOUND TRUE)

  MACRO(TortoiseSVN_WC_INFO dir prefix)
    FILE(WRITE "${VISIT_SOURCE_DIR}/svnrev.in" "$WCREV$")
       
    EXECUTE_PROCESS(COMMAND ${TortoiseSVN_EXECUTABLE} "." "${VISIT_SOURCE_DIR}/svnrev.in" "${VISIT_SOURCE_DIR}/svnrev"
      WORKING_DIRECTORY ${VISIT_SOURCE_DIR}
      OUTPUT_VARIABLE TortoiseSVN_info_output
      ERROR_VARIABLE TortoiseSVN_info_error
      RESULT_VARIABLE TortoiseSVN_info_result
      OUTPUT_STRIP_TRAILING_WHITESPACE)

    IF(NOT ${TortoiseSVN_info_result} EQUAL 0)
        MESSAGE(SEND_ERROR "Command \"${TortoiseSVN_EXECUTABLE} ${dir}\" failed with output:\n${TortoiseSVN_info_error}")
    ELSE(NOT ${TortoiseSVN_info_result} EQUAL 0)
        FILE(STRINGS ${VISIT_SOURCE_DIR}/svnrev ${prefix}_WC_REVISION)
    ENDIF(NOT ${TortoiseSVN_info_result} EQUAL 0)
    FILE(REMOVE ${VISIT_SOURCE_DIR}/svnrev.in ${VISIT_SOURCE_DIR}/svnrev)
  ENDMACRO(TortoiseSVN_WC_INFO)

ENDIF(TortoiseSVN_EXECUTABLE)

IF(NOT TortoiseSVN_FOUND)
  IF(NOT TortoiseSVN_FIND_QUIETLY)
    MESSAGE(STATUS "TortoiseSVN was not found.")
  ELSE(NOT TortoiseSVN_FIND_QUIETLY)
    IF(TortoiseSVN_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "TortoiseSVN was not found.")
    ENDIF(TortoiseSVN_FIND_REQUIRED)
  ENDIF(NOT TortoiseSVN_FIND_QUIETLY)
ENDIF(NOT TortoiseSVN_FOUND)

