# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Brad Whitlock, Tue Apr 16 09:31:42 PDT 2013
#   Add libm.
#
#****************************************************************************


# Add link directories
LINK_DIRECTORIES(${VISIT_LIBRARY_DIR}
${HDF5_LIBRARY_DIR}
)


#********************************* SERIAL ************************************

visit_add_executable(
    NAME        xdmf_writer_ser
    SOURCES     xdmf_writer.c
    HEADERS     xdmf_writer.h
    INCLUDES    ${CMAKE_CURRENT_SOURCE_DIR}
                ${HDF5_INCLUDE_DIR}
    DEPENDS_ON  ${HDF5_LIB} 
                ${ZLIB_LIBRARY}
                m
    SKIP_INSTALL)  

function add_serial_example(tname)

    visit_add_executable(
        NAME        ${tname}
        SOURCES     ${tname}.c
        HEADERS     xdmf_writer.h
        INCLUDES    ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS_ON  xdmf_writer_ser
                    m
        SKIP_INSTALL)  
endfunction()

add_serial_example(xdmf_curv3d)
add_serial_example(xdmf_ucd3d)
add_serial_example(xdmf_curv3d_wtime)
add_serial_example(xdmf_multi_curv3d)


#********************************* PARALLEL **********************************
IF(VISIT_PARALLEL)
    visit_add_executable(
        NAME        xdmf_writer_par
        SOURCES     xdmf_writer.c
        HEADERS     xdmf_writer.h
        INCLUDES    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${HDF5_INCLUDE_DIR}
        DEPENDS_ON  ${HDF5_LIB} 
                    ${ZLIB_LIBRARY}
                    m
        SKIP_INSTALL)  

    function add_parallel_example(tname)

        visit_add_executable(
            NAME        ${tname}
            SOURCES     ${tname}.c
            HEADERS     xdmf_writer.h
            INCLUDES    ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS_ON  xdmf_writer_par
                        m
            SKIP_INSTALL)  
    endfunction()

    add_parallel_example(xdmf_mpi_example)
    add_parallel_example(xdmf_mpi_example2)
    add_parallel_example(xdmf_mpi_example3)
endif()

