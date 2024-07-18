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
   find_path(QWT_INCLUDE_DIR qwt.h
             PATH_SUFFIXES include
                           include/qt6
                           include/qt6/qwt)

    find_library(QWT_LIBRARY
             NAMES qwt qwt-qt6
             PATH_SUFFIXES lib lib64)

    find_package_handle_standard_args(QWT DEFAULT_MSG
        QWT_INCLUDE_DIR
        QWT_LIBRARY)

    if(QWT_FOUND)
        message(STATUS "Qwt include: ${QWT_INCLUDE_DIR}")
        message(STATUS "Qwt library: ${QWT_LIBRARY}")
    else()
        message(STATUS "Qwt could not be found in the system.")
    endif()

elseif(VISIT_QWT_DIR)
    if(APPLE)
        if(VISIT_STATIC)
            SET_UP_THIRD_PARTY(QWT LIBS qwt)
        else()
            SET_UP_THIRD_PARTY(QWT
                INCDIR lib/qwt.framework/Versions/Current/Headers
                LIBS qwt)
        endif()
    else()
        SET_UP_THIRD_PARTY(QWT LIBS qwt)
    endif()

    if(QWT_FOUND)
        set(QWT_LIBRARY ${QWT_LIBRARY_DIR}/${QWT_LIB} CACHE FILEPATH "full path to qwt library" FORCE)
    else()
        message(STATUS "Qwt installation (${VISIT_QWT_DIR}) could not be used.")
    endif()
else()
    message(STATUS "Qwt not requested.")
endif()

if(QWT_FOUND)
    set(HAVE_QWT TRUE CACHE BOOL "Have Qwt library")
endif()

