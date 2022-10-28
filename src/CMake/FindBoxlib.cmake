# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Bonnell, Thu Dec  3 10:30:15 PST 2009
#   Use only 1 if-def block, fix libarary names for windows.
#
#   Eric Brugger, Fri Jan  7 13:50:15 PST 2011
#   I replaced the BOXLIB2D and BOXLIB3D variables with just BOXLIB.
#
#   Kathleen Bonnell, Thu Jan 13 15:21:47 MST 2011
#   Restore separate vars for libraries (to handle different names on
#   different platforms).
#
#   Kathleen Bonnell, Mon Jan 17 17:24:44 MST 2011
#   Don't set BOXLIB_2D/3D_LIB unless BOXLIB_FOUND.
#
#   Kathleen Biagas, Tues Oct 1 09:33:47 MST 2013
#   Removed VISIT_MSVC_VER from Windows handling.
#
#   Kathleen Biagas, Thu July 15, 2021
#   Add BOXLIB_WIN32_DEFINES (used by xml2cmake to add preprocessor defines).
#
#   Kathleen Biagas, Tue Oct 25, 2022
#   Create import targets for Boxlib::Box2d and Boxlib::Box3d.
#
#****************************************************************************/

message("VISIT_BOXLIB_DIR: ${VISIT_BOXLIB_DIR}")

if(DEFINED VISIT_BOXLIB_DIR AND EXISTS ${VISIT_BOXLIB_DIR})

    set(Boxlib2D_NAMES BoxLib2D box2D)
    set(Boxlib3D_NAMES BoxLib3D box3D)

    find_path(Boxlib_INCLUDE_DIR BoxLib.H PATHS ${VISIT_BOXLIB_DIR} PATH_SUFFIXES include)

    find_library(Boxlib2D_LIBRARY NAMES ${Boxlib2D_NAMES} PATHS ${VISIT_BOXLIB_DIR} PATH_SUFFIXES lib)
    find_library(Boxlib3D_LIBRARY NAMES ${Boxlib3D_NAMES} PATHS ${VISIT_BOXLIB_DIR} PATH_SUFFIXES lib)

    if(Boxlib_INCLUDE_DIR AND Boxlib2D_LIBRARY AND Boxlib3D_LIBRARY)
        set(BOXLIB_FOUND true)
    endif()

    include(${VISIT_SOURCE_DIR}/CMake/ThirdPartyInstallLibrary.cmake)

    if(Boxlib_INCLUDE_DIR)
        THIRD_PARTY_INSTALL_INCLUDE(boxlib ${Boxlib_INCLUDE_DIR})
    endif()

    if(Boxlib2D_LIBRARY)
        if(NOT TARGET Boxlib::Box2D)
            add_library(Boxlib::Box2D UNKNOWN IMPORTED)
            set_target_properties(Boxlib::Box2D PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${Boxlib_INCLUDE_DIR}"
                IMPORTED_LOCATION "${Boxlib2D_LIBRARY}"
                INTERFACE_COMPILE_DEFINITIONS BL_SPACEDIM=2)
            if(WIN32)
                set_target_properties(Boxlib::Box2D PROPERTIES
                    INTERFACE_COMPILE_DEFINITIONS BL_FORT_USE_UPPERCASE)
            endif()
        endif()
        visit_install_tp_import_lib(Boxlib::Box2D)

        # for current expectation of BOXLIB_2D_LIB
        set(BOXLIB_2D_LIB Boxlib::Box2D)
    endif()

    if(Boxlib3D_LIBRARY)
        if(NOT TARGET Boxlib::Box3D)
            add_library(Boxlib::Box3D UNKNOWN IMPORTED)
            set_target_properties(Boxlib::Box3D PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${Boxlib_INCLUDE_DIR}"
                IMPORTED_LOCATION "${Boxlib3D_LIBRARY}"
                INTERFACE_COMPILE_DEFINITIONS BL_SPACEDIM=3)
            if(WIN32)
                set_target_properties(Boxlib::Box3D PROPERTIES
                    INTERFACE_COMPILE_DEFINITIONS BL_FORT_USE_UPPERCASE)
            endif()
        endif()
        visit_install_tp_import_lib(Boxlib::Box3D)

        # for current expectation of BOXLIB_3D_LIB
        set(BOXLIB_3D_LIB Boxlib::Box3D)
    endif()
endif()
