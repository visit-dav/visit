# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kevin Griffin, Thu Jan  4 12:45:28 PST 2018
#   Changed the linked directory lib/qwt.framework/Headers to the actual path
#   it was linked to. This fixes the make install symlink error.
#
#   Kathleen Biagas, Thu Feb  8 08:30:19 PST 2018
#   Set QWT_LIBRARY to full path, for use in target_link_libraries.
#
#   Kathleen Biagas, Wed July 17, 2024
#   Allow QWT to be optional, and allow use of system version via
#   new VISIT_USE_SYTEM_QWT option.
#
#*****************************************************************************

if(VISIT_USE_SYSTEM_QWT)
   find_path(_qwt_INCLUDE_DIR qwt.h
             PATH_SUFFIXES include
                           include/qt6
                           include/qt6/qwt)

    find_library(_qwt_LIBRARY
             NAMES qwt qwt-qt6
             PATH_SUFFIXES lib lib64)

elseif(VISIT_QWT_DIR)

    find_path(_qwt_INCLUDE_DIR qwt.h
        PATHS ${QWT_DIR}
        PATH_SUFFIXES include
                      lib/qwt.framework/Versions/Current/Headers
        NO_DEFAULT_PATH)

    find_library(_qwt_LIBRARY
        NAMES qwt 
        PATHS ${QWT_DIR}
        PATH_SUFFIXES lib lib64
        NO_DEFAULT_PATH)

else()
    message(STATUS "Qwt not requested.")
    return()
endif()

find_package_handle_standard_args(QWT DEFAULT_MSG
    _qwt_INCLUDE_DIR
    _qwt_LIBRARY)


if(QWT_FOUND)
    get_filename_component(lib_qwt ${_qwt_LIBRARY} NAME)

    blt_import_library(
        NAME        qwt
        INCLUDES    $<BUILD_INTERFACE:${_qwt_INCLUDE_DIR}>
                    $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/qwt/include>
        LIBRARIES   $<BUILD_INTERFACE:${_qwt_LIBRARY}>
        EXPORTABLE  ON)


    # CMake doesn't prepend ${_IMPORT_PREFIX} in the generated export
    # set for INTERFACE libs, so "${IMPORT_PREFIX}" needs to be
    # explicitly added to the INSTALL_INTERFACE, and escaped so it
    # doesn't get evaluated.
    #
    # Also, it seems if the INSTALL_INTERFACE is used in the
    # blt_import_library,  the ${_IMPORT_PREFIX} is stripped, even
    # if it is escaped, so it appears to be getting evaluated.
    # That's why it is added separately here instead of being added
    # to the LIBRARIES above.
    target_link_libraries(qwt INTERFACE
        $<INSTALL_INTERFACE:\${_IMPORT_PREFIX}/${VISIT_INSTALLED_VERSION_LIB}/${lib_qwt}>)

    target_compile_definitions(qwt INTERFACE HAVE_QWT)

    # install and export
    if(VISIT_INSTALL_THIRD_PARTY)
        visit_install_export_targets(qwt)

        # headers aren't being installed. Perhaps because it is INTERFACE lib?
        # There is a PUBLIC_HEADER property for INTERFACE libraries
        # that will allow the installation of the headers in a normal
        # install(TARGETS) command (like the one being used by
        # visit_install_export_targets).
        # However, you cannot specifiy a directory as PUBLIC_HEADER
        # property, but must instead list them all, so using this
        # property would require a file(glob). Do we want to do that?
        # I think it is easier to install the directory the way we do
        # for TP libs in the following function:

        THIRD_PARTY_INSTALL_INCLUDE(qwt ${_qwt_INCLUDE_DIR})
    endif()
    if(WIN32)
        # need to copy the dll to the build dir
        cmake_path(REPLACE_EXTENSION lib_qwt dll OUTPUT_VARIABLE _qwt_DLL)
        get_filename_component(_qwt_LIBRRARY_DIR ${_qwt_LIBRARY} PATH)
        if(EXISTS ${_qwt_DLL})
            cmake_path(SET _qwt_DLL ${_qwt_LIBRARY_DIR}/${_qwt_DLL})
        else()    
            cmake_path(SET _qwt_DLL NORMALIZE ${_qwt_LIBRARY_DIR}/../bin/${_qwt_DLL})
        endif()
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy
                        ${_qwt_DLL}
                        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ThirdParty)
    endif()

    set(HAVE_QWT TRUE CACHE BOOL "Have Qwt library")
endif()

