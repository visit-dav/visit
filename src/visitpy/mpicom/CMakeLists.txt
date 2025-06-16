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
        $<BUILD_INTERFACE:${VISIT_SOURCE_DIR}/visitpy/common>
        $<BUILD_INTERFACE:${PYTHON_INCLUDE_PATH}>)

    if(UNIX)
        set_target_properties(mpicom PROPERTIES COMPILE_FLAGS "-fno-strict-aliasing")
        if(VISIT_PARALLEL_CXXFLAGS)
            set(PAR_COMPILE_FLAGS "-fno-strict-aliasing")
            foreach(X ${VISIT_PARALLEL_CXXFLAGS})
                set(PAR_COMPILE_FLAGS "${PAR_COMPILE_FLAGS} ${X}")
            endforeach()
            set_target_properties(mpicom PROPERTIES
                                  COMPILE_FLAGS ${PAR_COMPILE_FLAGS})
        endif()
        if(VISIT_PARALLEL_LINKER_FLAGS)
            set(PAR_LINK_FLAGS "")
            foreach(X ${VISIT_PARALLEL_LINKER_FLAGS})
                set(PAR_LINK_FLAGS "${PAR_LINK_FLAGS} ${X}")
            endforeach()
            set_target_properties(mpicom PROPERTIES
                              LINK_FLAGS ${PAR_LINK_FLAGS})
        endif()
    else()
        target_include_directories(mpicom PRIVATE ${VISIT_PARALLEL_INCLUDE})
        target_compile_definitions(mpicom PRIVATE ${VISIT_PARALLEL_DEFINES})
    endif()

    target_link_libraries(mpicom PRIVATE
                          ${PYTHON_LIBRARY}
                          ${VISIT_PARALLEL_LIBS})

else()
    # we still need to build the module w/ the mpi stub
    PYTHON_ADD_PIP_SETUP(NAME mpicom_py_setup
                         DEST_DIR site-packages
                         PY_MODULE_DIR mpicom
                         PY_SETUP_FILE setup.py
                         PY_SOURCES ${MPICOM_PY_SOURCES})

# add to main target?
endif()

