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


SET(TortoiseSVN_FOUND FALSE)

FIND_PROGRAM(TortoiseSVN_EXECUTABLE SubWCRev
  DOC "Tortoise SVN client")
MARK_AS_ADVANCED(TortoiseSVN_EXECUTABLE)

IF(TortoiseSVN_EXECUTABLE)
  SET(TortoiseSVN_FOUND TRUE)

  MACRO(TortoiseSVN_WC_INFO dir prefix)

    EXECUTE_PROCESS(COMMAND ${TortoiseSVN_EXECUTABLE} ${dir}
      OUTPUT_VARIABLE ${prefix}_WC_INFO
      ERROR_VARIABLE TortoiseSVN_info_error
      RESULT_VARIABLE TortoiseSVN_info_result
      OUTPUT_STRIP_TRAILING_WHITESPACE)

    IF(NOT ${TortoiseSVN_info_result} EQUAL 0)
      MESSAGE(SEND_ERROR "Command \"${TortoiseSVN_EXECUTABLE} info ${dir}\" failed with output:\n${TortoiseSVN_info_error}")
    ELSE(NOT ${TortoiseSVN_info_result} EQUAL 0)

      STRING(REGEX REPLACE "^(.*\n)?Updated to revision ([^\n]+).*"
        "\\2" ${prefix}_WC_REVISION "${${prefix}_WC_INFO}")

    ENDIF(NOT ${TortoiseSVN_info_result} EQUAL 0)

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

