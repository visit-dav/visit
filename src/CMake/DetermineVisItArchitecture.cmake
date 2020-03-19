# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Eric Brugger, Thu Feb 25 16:06:22 PST 2010
#   I enclosed all variables that were used in tests involving STREQUAL
#   in ${} and enclosed all literal strings that were used in tests
#   involving STREQUAL in double quotes so that it would create the
#   correct architecture string on AIX.
#
#   I modified the architecture string it generates on AIX to be either
#   ibm-aix-pwr or ibm-aix-pwr64.
#
#   Brad Whitlock, Tue Jan 25 12:28:55 PST 2011
#   I made Mac 10.x and later use darwin-x86_64.
#
#****************************************************************************

MACRO(DETERMINE_VISIT_ARCHITECTURE ARCH)
    IF(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        IF(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "ppc")
            SET(${ARCH} linux-ppc)
        ELSEIF(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "ppc64")
            SET(${ARCH} linux-ppc64)
        ELSEIF(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
            SET(${ARCH} linux-x86_64)
        ELSEIF(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "ia64")
            SET(${ARCH} linux-ia64)
        ELSE(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "ppc")
            SET(${ARCH} linux-intel)
        ENDIF(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "ppc")
    ELSEIF(${CMAKE_SYSTEM_NAME} STREQUAL "AIX")
        IF($ENV{OBJECT_MODE} STREQUAL "32")
            SET(${ARCH} "ibm-aix-pwr")
        ELSE($ENV{OBJECT_MODE} STREQUAL "32")
            SET(${ARCH} "ibm-aix-pwr64")
        ENDIF($ENV{OBJECT_MODE} STREQUAL "32")
    ELSEIF(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        IF(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i386")
            EXECUTE_PROCESS(COMMAND uname -r
               OUTPUT_STRIP_TRAILING_WHITESPACE
               OUTPUT_VARIABLE _OSX_VERSION)
            STRING(SUBSTRING ${_OSX_VERSION} 0 1 _OSX_MAJOR_VERSION)
            IF(${_OSX_MAJOR_VERSION} STREQUAL "1")
                # This will match 10, 11, 12, ...
                SET(${ARCH} darwin-x86_64)
            ELSE(${_OSX_MAJOR_VERSION} STREQUAL "1")
                SET(${ARCH} darwin-i386)
            ENDIF(${_OSX_MAJOR_VERSION} STREQUAL "1")
        ELSEIF(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
            SET(${ARCH} darwin-x86_64)
        ELSE(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i386")
            SET(${ARCH} darwin-ppc)
        ENDIF(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i386")
    ELSEIF(${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
        SET(${ARCH} "freebsd-${CMAKE_SYSTEM_VERSION}")
    ELSEIF(${CMAKE_SYSTEM_NAME} STREQUAL "IRIX")
        SET(${ARCH} sgi-irix6-mips2)
    ELSEIF(${CMAKE_SYSTEM_NAME} STREQUAL "SunOS")
        SET(${ARCH} "sun4-${CMAKE_SYSTEM_VERSION}-sparc")
    ELSEIF(${CMAKE_SYSTEM_NAME} STREQUAL "Tru64")
        SET(${ARCH} dec-osf1-alpha)
    ELSE(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        # Unhandled case. Make up a string.
        SET(VISITARCHTMP "${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
        STRING(TOLOWER ${VISITARCHTMP} ${ARCH})
    ENDIF(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
ENDMACRO(DETERMINE_VISIT_ARCHITECTURE ARCH)
