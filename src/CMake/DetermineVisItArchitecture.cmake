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
#   Eric Brugger, Tue Oct  8 10:56:56 PDT 2024
#   I added support for gpu architectures on x86_64 systems. I removed
#   support for obsolete architectures. I changed the default linux
#   architecture to be x86_64 and added support for ppc64le.
#
#****************************************************************************

function(GET_GPU_ARCHITECTURE arch)
    list(APPEND dirs
	 "/opt/rocm-6.2.2/lib/llvm/bin/offload-arch"
	 "/opt/rocm-6.2.1/lib/llvm/bin/offload-arch"
	 "/opt/rocm-6.2.0/lib/llvm/bin/offload-arch"
	 "/opt/rocm-6.1.3/lib/llvm/bin/offload-arch"
	 "/opt/rocm-6.1.2/lib/llvm/bin/offload-arch"
	 "/opt/rocm-6.1.1/lib/llvm/bin/offload-arch"
         "/opt/rocm-6.1.0/lib/llvm/bin/offload-arch")

    set(gpu_arch "None")
    foreach(dir ${dirs})
        if(EXISTS ${dir})
            execute_process(COMMAND ${dir} OUTPUT_VARIABLE gpu_arch_tmp OUTPUT_STRIP_TRAILING_WHITESPACE)
	    if (NOT "${gpu_arch_tmp}" STREQUAL "")
                set(gpu_arch ${gpu_arch_tmp})
            endif()
	    break()
        endif()
    endforeach()

    set(${arch} ${gpu_arch} PARENT_SCOPE)
endfunction()

MACRO(DETERMINE_VISIT_ARCHITECTURE ARCH)
    IF(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        IF(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "ppc64le")
            SET(${ARCH} linux-ppc64le)
        ELSEIF(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
	    GET_GPU_ARCHITECTURE(gpu_arch)
	    if(${gpu_arch} STREQUAL "None")
                set(${ARCH} linux-x86_64)
            else()
                set(${ARCH} "linux-x86_64-${gpu_arch}")
            endif()
	    message(STATUS "ARCH=${${ARCH}}")
        ELSE(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "ppc64le")
            SET(${ARCH} linux-x86_64)
        ENDIF(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "ppc64le")
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
    ELSE(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        # Unhandled case. Make up a string.
        SET(VISITARCHTMP "${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
        STRING(TOLOWER ${VISITARCHTMP} ${ARCH})
    ENDIF(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
ENDMACRO(DETERMINE_VISIT_ARCHITECTURE ARCH)
