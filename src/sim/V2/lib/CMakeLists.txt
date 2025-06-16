# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Dirk Schubert (Allinea Software), Mon Oct 15, 2012
#   Build symV2dyn only if VISIT_DDT is chosen.
#
#   Eric Brugger, Wed Dec 26 15:31:11 PST 2012
#   Always build symV2dyn, so that it can be used in the swig/python
#   directory.
#
#   Kathleen Biagas, Mon Jan 7 18:04:23 PST 2012
#   Don't build symV2dyn on Windows.
#
#   Kathleen Biagas, Thu Nov  7 07:57:14 PST 2013
#   Add VisItControlInterface_V2_plotting.h to files being installed.
#
#   Brad Whitlock, Fri Aug 14 12:28:12 PDT 2015
#   Add VisIt_OptionList.
#
#****************************************************************************

# See ../CMakeLists.txt for LIBSIM_SOURCES

set(FORTRAN_LIBSIM_SOURCES visitfortransimV2interface.c)
set(FORTRAN_LIBSIM_HEADERS VisItFortran.h)


if(NOT WIN32 AND NOT VISIT_STATIC)
    # Allinea Software:
    # Create a libsimV2.so shared library for the standard VisIt
    # interface (normally provided by statically linking with libsimV2.a).
    # -fvisibility is set to default (it is normally hidden).
    # Preferred solution would be to tag each function to be exposed in
    # the shared library individually instead of exposing everything,
    # but this will do for now. Also provide debug info.
    visit_add_library(
        NAME     simV2dyn 
        SOURCES  ${LIBSIM_SOURCES}
        HEADERS  ${LIBSIM_HEADERS}
        INCLUDES $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                 $<BUILD_INTERFACE:${VISIT_BINARY_DIR}/include>
        SKIP_INSTALL)

    set_target_properties(simV2dyn PROPERTIES
        COMPILE_OPTIONS "-g;-fvisibility=default")

    set(LIBSIM_DYN simV2dyn)
endif()

# Use SKIP_INSTALL so that the libraries can be installed to a different
# location than other visit libraries.

visit_add_library(
    NAME     simV2
    SOURCES  ${LIBSIM_SOURCES}
    HEADERS  ${LIBSIM_HEADERS}
    INCLUDES $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
             $<BUILD_INTERFACE:${VISIT_BINARY_DIR}/include>
    FORCE_STATIC 
    SKIP_INSTALL)

visit_add_library(
    NAME     simV2f
    SOURCES  ${FORTRAN_LIBSIM_SOURCES}
    HEADERS  ${FORTRAN_LIBSIM_HEADERS}
    INCLUDES $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
             $<BUILD_INTERFACE:${VISIT_BINARY_DIR}/include>
    FORCE_STATIC
    SKIP_INSTALL)

install(TARGETS simV2 simV2f ${LIBSYM_DYN}
    ARCHIVE DESTINATION ${VISIT_INSTALLED_VERSION}/libsim/V2/lib
    LIBRARY DESTINATION ${VISIT_INSTALLED_VERSION}/libsim/V2/lib
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_WRITE GROUP_EXECUTE
                 WORLD_READ            WORLD_EXECUTE)

install(FILES  ${LIBSIM_HEADERS} ${FORTRAN_LIBSIM_HEADERS}
               visitfortransimV2interface.inc
    DESTINATION ${VISIT_INSTALLED_VERSION}/libsim/V2/include
    PERMISSIONS OWNER_READ OWNER_WRITE
                GROUP_READ GROUP_WRITE
                WORLD_READ)

