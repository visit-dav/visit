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
                    $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/qwt>
        LIBRARIES   $<BUILD_INTERFACE:${_qwt_LIBRARY}>
        EXPORTABLE  ON)

    target_link_libraries(qwt INTERFACE
        $<INSTALL_INTERFACE:\${_IMPORT_PREFIX}/${VISIT_INSTALLED_VERSION_LIB}/${lib_qwt}>)

    target_compile_definitions(qwt INTERFACE HAVE_QWT)
    # install and export
    if(VISIT_INSTALL_THIRD_PARTY)
        visit_install_export_targets(qwt)
        THIRD_PARTY_INSTALL_LIBRARY(${_qwt_LIBRARY})
        THIRD_PARTY_INSTALL_INCLUDE(qwt ${_qwt_INCLUDE_DIR})
    endif()
    if(WIN32)
        # need to copy the dll to the build dir
        visit_get_dll_from_library(${_qwt_LIBRARY} _qwt_DLL)
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy
                        ${_qwt_DLL}
                        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ThirdParty)
    endif()

    set(HAVE_QWT TRUE CACHE BOOL "Have Qwt library")
endif()

unset(_qwt_INCLUDE_DIR CACHE)
unset(_qwt_LIBRARY CACHE)
