# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#-----------------------------------------------------------------------------
# Modifications:
#   Kathleen Biagas, Tue June 25, 2024
#   Create exportable library via blt_import_library.
#
#-----------------------------------------------------------------------------

# Use the MFEM_DIR hint from the config-site .cmake file

SET_UP_THIRD_PARTY(MFEM LIBS mfem)

if(MFEM_FOUND)
    # create a list of full-path libraries to be used when linking
    foreach(libdir ${MFEM_LIBRARY_DIR})
        foreach(lib ${MFEM_LIB})
            if(EXISTS ${libdir}/${lib})
                list(APPEND mll $<BUILD_INTERFACE:${libdir}/${lib}>)
            endif()
        endforeach()
    endforeach()

    # create include dirs for build and install interface
    foreach(incdir ${MFEM_INCLUDE_DIR})
        list(APPEND mfem_inc $<BUILD_INTERFACE:${incdir}>)
        if(${incdir} MATCHES "mfem")
            list(APPEND mfem_inc 
                $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/mfem/include>)
        endif()
    endforeach()

    # create import library
    blt_import_library(
        NAME       mfem
        INCLUDES   ${mfem_inc}
        LIBRARIES  ${mll}
        EXPORTABLE ON)

    # Add install interface for link libraries
    foreach(libdir ${MFEM_LIBRARY_DIR})
        foreach(lib ${MFEM_LIB})
            if(EXISTS ${libdir}/${lib})
                target_link_libraries(mfem INTERFACE
                    $<INSTALL_INTERFACE:\${_IMPORT_PREFIX}/${VISIT_INSTALLED_VERSION_LIB}/${lib}>)
            endif()
        endforeach()
    endforeach()

    # install export target
    visit_install_export_targets(mfem)
endif()


