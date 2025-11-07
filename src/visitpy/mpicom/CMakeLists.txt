# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#  Cyrus Harrison, Wed Mar 10 11:06:40 PST 2010 
#  Use CMAKE_LIBRARY_OUTPUT_DIRECTORY instead of LIBRARY_OUTPUT_DIRECTORY.
#
#  Cyrus Harrison,Thu Apr  5 09:14:27 PDT 2012
#  Use new python module creation setup.
#
#  Kathleen Biagas, Thu Jun 21 11:32:12 PST 2012 
#  Change how parallel flags are handled on windows. 
#
#  Kathleen Biagas, Thu Sep 27 11:35:34 PDT 2018
#  Change PYTHON_LIBRARIES to PYTHON_LIBRARY for consistency throughout visit.
#
#  Kathleen Biagas, Tue Mar 5, 2024
#  For PYTHON_ADD_PIP_SETUP change DEST_DIR from lib/site-packages to
#  simply site-packages. The lib part handled in PYTHON_ADD_PIP_SETUP.
#
#  Kathleen Biagas, Thu Nov 6, 2025
#  Use MPI::MPI_C target for linking with mpi.
#
#****************************************************************************

set(MPICOM_PY_SOURCES py_src/__init__.py
                      py_src/mpistub.py)

# if we have a parallel build - create the mpicom python C module.
if(VISIT_PARALLEL)
    set(MPICOM_SOURCES
        src/mpicom.C
        src/Buffer.C)

    # Create the mpicom
    PYTHON_ADD_HYBRID_MODULE(NAME mpicom
                             DEST_DIR site-packages
                             PY_MODULE_DIR mpicom
                             PY_SETUP_FILE setup.py
                             PY_SOURCES ${MPICOM_PY_SOURCES}
                             SOURCES ${MPICOM_SOURCES})

    target_include_directories(mpicom PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/>
        $<BUILD_INTERFACE:${VISIT_SOURCE_DIR}/visitpy/common>)

    if(UNIX)
        set_target_properties(mpicom PROPERTIES COMPILE_FLAGS "-fno-strict-aliasing")
    endif()

    target_compile_definitions(mpicom PUBLIC ${VISIT_PARALLEL_DEFINES})

    target_link_libraries(mpicom PRIVATE
                          ${PYTHON_LIBRARY}
                          MPI::MPI_C)

else()
    # we still need to build the module w/ the mpi stub
    PYTHON_ADD_PIP_SETUP(NAME mpicom_py_setup
                         DEST_DIR site-packages
                         PY_MODULE_DIR mpicom
                         PY_SETUP_FILE setup.py
                         PY_SOURCES ${MPICOM_PY_SOURCES})

# add to main target?
endif()

