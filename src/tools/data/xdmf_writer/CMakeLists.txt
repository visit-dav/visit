# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Brad Whitlock, Tue Apr 16 09:31:42 PDT 2013
#   Add libm.
#
#****************************************************************************

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${HDF5_INCLUDE_DIR}
)

# Add link directories
LINK_DIRECTORIES(${VISIT_LIBRARY_DIR}
${HDF5_LIBRARY_DIR}
)


#********************************* SERIAL ************************************
ADD_EXECUTABLE(xdmf_curv3d xdmf_curv3d.c)
TARGET_LINK_LIBRARIES(xdmf_curv3d xdmf_writer_ser m) 

ADD_EXECUTABLE(xdmf_ucd3d xdmf_ucd3d.c)
TARGET_LINK_LIBRARIES(xdmf_ucd3d xdmf_writer_ser m) 

ADD_EXECUTABLE(xdmf_curv3d_wtime xdmf_curv3d_wtime.c)
TARGET_LINK_LIBRARIES(xdmf_curv3d_wtime xdmf_writer_ser m) 

ADD_EXECUTABLE(xdmf_multi_curv3d xdmf_multi_curv3d.c)
TARGET_LINK_LIBRARIES(xdmf_multi_curv3d xdmf_writer_ser m) 

ADD_LIBRARY(xdmf_writer_ser xdmf_writer.c)
TARGET_LINK_LIBRARIES(xdmf_writer_ser ${HDF5_LIB} ${ZLIB_LIBRARY} m)

#********************************* PARALLEL **********************************
IF(VISIT_PARALLEL)
    ADD_PARALLEL_EXECUTABLE(xdmf_mpi_example xdmf_mpi_example.c)
    TARGET_LINK_LIBRARIES(xdmf_mpi_example xdmf_writer_par m)

    ADD_PARALLEL_EXECUTABLE(xdmf_mpi_example2 xdmf_mpi_example2.c)
    TARGET_LINK_LIBRARIES(xdmf_mpi_example2 xdmf_writer_par m)

    ADD_PARALLEL_EXECUTABLE(xdmf_mpi_example3 xdmf_mpi_example3.c)
    TARGET_LINK_LIBRARIES(xdmf_mpi_example3 xdmf_writer_par m)

    ADD_PARALLEL_LIBRARY(xdmf_writer_par xdmf_writer.c)
    TARGET_LINK_LIBRARIES(xdmf_writer_par ${HDF5_LIB} ${ZLIB_LIBRARY} m)
ENDIF(VISIT_PARALLEL)
