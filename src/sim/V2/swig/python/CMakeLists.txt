# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#  Cyrus Harrison, Thu May 12 15:46:45 PDT 2011
#  Remove COMPONENT RUNTIME statement from INSTALL command to avoid
#  cpack problems.
#
#  Eric BRugger, Wed Dec 26 15:33:28 PST 2012
#  Link against symV2dyn instead of symV2.
#
#  Brad Whitlock, Fri May 17 13:42:43 PDT 2013
#  Build in the libsim sources directly.
#
#  Kathleen Biagas, Fri Jun  6 11:24:37 PDT 2014
#  Copy simV2.py to lib dir when doing out-of-source builds.  Makes running
#  python examples easier.
#
#  Kathleen Biagas, Mon Jun  9 08:28:07 MST 2014
#  Fix copy of simV2.lib.
#
#  Cyrus Harrison, Tue Jun 30 16:23:15 PDT 2020
#  Use updated Find NumPy.
#
#  Kathleen Biagas, Wed Nov 30, 2022
#  Remove test for VISIT_OUT_OF_SOURCE_BUILD as out-of-source is required.
#
#****************************************************************************

INCLUDE_DIRECTORIES(
    ${VISIT_SOURCE_DIR}/sim/V2/lib
    ${VISIT_SOURCE_DIR}/sim/V2/swig/python
    ${VISIT_BINARY_DIR}/sim/V2/swig/python
    ${VISIT_BINARY_DIR}/include
    ${PYTHON_INCLUDE_PATH}
    # for python 2 + 3 helper header
    ${VISIT_INCLUDE_DIR}/visitpy/common
    )

include(${CMAKE_CURRENT_SOURCE_DIR}/CMake/FindNumpy.cmake)
SET(SIMV2_USE_NUMPY FALSE)
IF (NUMPY_FOUND AND (NUMPY_VERSION VERSION_GREATER 1.5.0))
    SET(SIMV2_USE_NUMPY TRUE)
    INCLUDE_DIRECTORIES(${NUMPY_INCLUDE_DIRS})
ENDIF()
CONFIGURE_FILE(simV2_python_config.h.in simV2_python_config.h)

PYTHON_ADD_MODULE(pysimV2
    simV2_wrap.cxx
    simV2_custom.cxx
    simV2_PyObject.cxx
    ${LIBSIM_SOURCES}
    )

# We pick "pysimV2" as the target name because "simV2" is already taken by
# libsimV2. We then reset its output name to _simV2 since that's what SWIG
# wants us to call it.
SET_TARGET_PROPERTIES(pysimV2 PROPERTIES PREFIX "" OUTPUT_NAME _simV2)
TARGET_LINK_LIBRARIES(pysimV2 ${PYTHON_LIBRARY})
IF(NOT APPLE)
    SET_TARGET_PROPERTIES(pysimV2 PROPERTIES INSTALL_RPATH "$ORIGIN")
ENDIF(NOT APPLE)

VISIT_INSTALL_TARGETS(pysimV2)
INSTALL(FILES simV2.py
    DESTINATION ${VISIT_INSTALLED_VERSION_LIB}
    )

#Copy simV2.py for use during dev work
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy 
    ${CMAKE_CURRENT_SOURCE_DIR}/simV2.py
    ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/simV2.py)

