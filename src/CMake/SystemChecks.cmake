# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

# *****************************************************************************
#   Performs all checks for system headers/function prototypes, etc
# *****************************************************************************

#******************************************************************************
# Modifications:
#   Eric Brugger, Tue Jul 25 11:23:55 EDT 2023
#   Switched pthreads check to modern usage using find_package.
#
#   Kathleen Biagas, Tue Sep 26, 2023
#   Removed use of CMakeBackwardCompatibility.
#
#******************************************************************************

function(visit_check_isfinite_exists VAR)
  if(${VAR} MATCHES "^${VAR}$")
    message(STATUS "Check for isfinite")
    try_compile(tmpVar
            ${CMAKE_CURRENT_BINARY_DIR}
            ${VISIT_SOURCE_DIR}/CMake/Testisfinite.cpp
            CMAKE_FLAGS -DLINK_LIBRARIES:STRING=m
            OUTPUT_VARIABLE outvar)
    if(tmpVar)
        set(${VAR} 1 CACHE INTERNAL "isfinite found")
        message(STATUS "Check for isfinite - found")
    else(tmpVar)
        set(${VAR} 0 CACHE INTERNAL "isfinite found")
        message(STATUS "Check for isfinite - not found")
        if (NOT WIN32)
            message(STATUS "Check for isfinite - ${outvar}")
        endif (NOT WIN32)
    endif(tmpVar)
  endif()
endfunction()

# maybe can replace this with CMake's 'check_prototye_definition'?
function(visit_check_strtof_prototype VAR)
  if(${VAR} MATCHES "^${VAR}$")
    message(STATUS "Check for strtof prototype")
    try_compile(tmpVar
            ${CMAKE_CURRENT_BINARY_DIR}
            ${VISIT_SOURCE_DIR}/CMake/Teststrtofprototype.cpp
            OUTPUT_VARIABLE outvar)
    if(tmpVar)
        set(${VAR} 1 CACHE INTERNAL "strtof prototype")
        message(STATUS "Check for strtof prototype - found")
    else()
        set(${VAR} 0 CACHE INTERNAL "strtof prototype")
        message(STATUS "Check for strtof prototype - not found")
    endif()
  endif()
endfunction()

# maybe can replace this with CMake's 'check_prototye_definition'?
function(visit_check_ftime_prototype VAR)
  if(${VAR} MATCHES "^${VAR}$")
    message(STATUS "Check for ftime prototype")
    try_compile(tmpVar
            ${CMAKE_CURRENT_BINARY_DIR}
            ${VISIT_SOURCE_DIR}/CMake/Testftimeprototype.cpp
            OUTPUT_VARIABLE outvar)
    if(tmpVar)
        set(${VAR} 1 CACHE INTERNAL "ftime prototype")
        message(STATUS "Check for ftime prototype - found")
    else()
        set(${VAR} 0 CACHE INTERNAL "ftime prototype")
        message(STATUS "Check for ftime prototype - not found")
    endif()
  endif()
endfunction()



include(${CMAKE_ROOT}/Modules/CheckIncludeFiles.cmake)
include(${CMAKE_ROOT}/Modules/CheckTypeSize.cmake)
include(${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)
include(${CMAKE_ROOT}/Modules/CheckSymbolExists.cmake)
include(${CMAKE_ROOT}/Modules/TestBigEndian.cmake)

if(VISIT_USE_X)
    include(${CMAKE_ROOT}/Modules/FindX11.cmake)
    if(NOT X11_X11_LIB)
        SET(VISIT_USE_X OFF)
        SET(HAVE_LIBX11 0)
    else()
        SET(HAVE_LIBX11 1)
    endif()
else()
    set(HAVE_LIBX11 0)
    set(CMAKE_X_LIBS "" CACHE INTERNAL "X11 libs.")
    set(CMAKE_X_CFLAGS "" CACHE INTERNAL "X11 extra flags.")
    set(CMAKE_HAS_X 0 CACHE INTERNAL "Is X11 around.")
endif()


# Check for type sizes, endian
check_type_size(bool                 SIZEOF_BOOLEAN)
check_type_size(char                 SIZEOF_CHAR)
check_type_size(double               SIZEOF_DOUBLE)
check_type_size(float                SIZEOF_FLOAT)
check_type_size(int                  SIZEOF_INT)
check_type_size(long                 SIZEOF_LONG)
check_type_size("long double"        SIZEOF_LONG_DOUBLE)
check_type_size("long float"         SIZEOF_LONG_FLOAT)
check_type_size("long long"          SIZEOF_LONG_LONG)
check_type_size(short                SIZEOF_SHORT)
check_type_size("unsigned char"      SIZEOF_UNSIGNED_CHAR)
check_type_size("unsigned int"       SIZEOF_UNSIGNED_INT)
check_type_size("size_t"             SIZEOF_SIZE_T)
check_type_size("unsigned long"      SIZEOF_UNSIGNED_LONG)
check_type_size("unsigned long long" SIZEOF_UNSIGNED_LONG_LONG)
check_type_size("unsigned short"     SIZEOF_UNSIGNED_SHORT)
check_type_size(void*                SIZEOF_VOID_P)
set(CMAKE_REQUIRED_DEFINITIONS -D_LARGEFILE64_SOURCE)
check_type_size("off64_t" SIZEOF_OFF64_T)

test_big_endian(WORDS_BIGENDIAN)

# Check for threads
find_package(Threads)
message(STATUS "Threads_FOUND=${Threads_FOUND}")
set(HAVE_THREADS  ${Threads_FOUND})

# manually check for socklen_t as CHECK_SYMBOL_EXISTS
# doesn't appear to work on linux (at least)
if(HAVE_SOCKLEN_T MATCHES "^HAVE_SOCKLEN_T$")
    message(STATUS "Check for socklen_t")
    try_compile(HAVE_SOCKLEN_T
        ${CMAKE_CURRENT_BINARY_DIR}
        ${VISIT_SOURCE_DIR}/CMake/TestSocklenT.c
        OUTPUT_VARIABLE SLT
    )
    if (HAVE_SOCKLEN_T)
        set(HAVE_SOCKLEN_T 1 CACHE INTERNAL "support for socklen_t")
        message(STATUS "Check for socklen_t - found")
    else()
        set(HAVE_SOCKLEN_T 0 CACHE INTERNAL "support for socklen_t")
        message(STATUS "Check for socklen_t - not found")
    endif()
endif()

# Check for setbuf being a public member in fstream. We could test via
# TRY_COMPILE in CMake 2.6.2. I think this is only false on the Suns, which
# we kind of stopped supporting anyway.
set(NO_SETBUF 1)

# Check for PTY support
if(NOT WIN32)
    set(VISIT_USE_PTY 1)
    include(${VISIT_SOURCE_DIR}/CMake/TestPTY.cmake)
endif()

# Check for some functions
check_function_exists(memmove HAVE_MEMMOVE)
check_function_exists(div HAVE_DIV)
check_function_exists(setenv HAVE_SETENV)
check_function_exists(strtof HAVE_STRTOF)
check_function_exists(aligned_alloc HAVE_ALIGNED_ALLOC)
visit_check_strtof_prototype(HAVE_STRTOF_PROTOTYPE)
visit_check_ftime_prototype(HAVE_FTIME_PROTOTYPE)
visit_check_isfinite_exists(HAVE_ISFINITE)

# If we're not disabling the select() call then check for it.
if(NOT VISIT_DISABLE_SELECT)
    if(WIN32)
        set(HAVE_SELECT TRUE)
    else()
        check_function_exists(select HAVE_SELECT)
    endif()
endif()

check_include_files(fcntl.h     HAVE_FCNTL_H)
check_include_files(inttypes.h  HAVE_INTTYPES_H)
if(NOT APPLE)
    check_include_files (malloc.h    HAVE_MALLOC_H)
else()
    check_include_files (malloc/malloc.h    HAVE_MALLOC_H)
endif()

check_include_files(limits.h    HAVE_LIMITS_H)
check_include_files(memory.h    HAVE_MEMORY_H)
check_include_files(stdint.h    HAVE_STDINT_H)
check_include_files(stdlib.h    HAVE_STDLIB_H)
check_include_files(strings.h   HAVE_STRINGS_H)
check_include_files(string.h    HAVE_STRING_H)
check_include_files(sys/time.h  HAVE_SYS_TIME_H)
check_include_files(sys/types.h HAVE_SYS_TYPES_H)
check_include_files(sys/stat.h  HAVE_SYS_STAT_H)
check_include_files(unistd.h    HAVE_UNISTD_H)
check_include_files(stdbool.h   HAVE_STDBOOL_H)

