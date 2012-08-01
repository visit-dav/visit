###############################################################################
# This script sets these variables:
#
# MANTA_INCLUDE               -- Paths containing Manta header files.
# MANTA_TARGET_LINK_LIBRARIES -- List of Manta shared libraries.  (-l on link line)
# MANTA_LINK_DIRECTORIES      -- Path containing shared libraries (-L on link line)
#
# Additionally several .cmake scripts from the Manta build are executed to 
# insure a similar build environment will be used by the project.
# 
# Modifications:
#   
#   Hank Childs, Fri Jul 27 14:13:40 PDT 2012
#   Remove SWIG/Python support, since it isn't needed and screws up Python paths
#   for the rest of the VisIt build.
#
#   Eric Brugger, Mon Jul 30 15:36:54 PDT 2012
#   Completely removed Python support. Made the code more like how other third
#   party libraries are handled in visit.
#
###############################################################################

INCLUDE(${VISIT_SOURCE_DIR}/CMake/SetUpThirdParty.cmake)

SET_UP_THIRD_PARTY(MANTA lib include
  Manta_Factory
  Manta_UserInterface
  Manta_Engine
  Manta_Model
  Manta_Image
  Manta_Interface
  Manta_Core_XUtils
  Manta_Core About)

IF (VISIT_MANTA_DIR)
  SET(VISIT_MANTA ON TYPE BOOL)
  SET(MANTA_SOURCE_DIR ${VISIT_MANTA_DIR}/include)
  SET(MANTA_BUILD_PREFIX ${VISIT_MANTA_DIR})

  SET(MANTA_INCLUDE
    ${VISIT_MANTA_DIR}/include
    )
  
  SET(MANTA_TARGET_LINK_LIBRARIES
    Manta_Factory
    Manta_UserInterface
    Manta_Engine
    Manta_Model
    Manta_Image
    Manta_Interface
    Manta_Core_XUtils
    Manta_Core
    About
    )

  SET(MANTA_LINK_DIRECTORIES
    ${VISIT_MANTA_DIR}/lib
    )

  # Include Manta header files.
  INCLUDE_DIRECTORIES(
    ${MANTA_INCLUDE}
    )
  
  # Include Manta library directory.
  LINK_DIRECTORIES(
    ${MANTA_LINK_DIRECTORIES}
    )

  # Load Manta macros.
  INCLUDE(${VISIT_MANTA_DIR}/include/CMake/Macros.cmake)

  # Set flags based on the architecture we are on (no compiler stuff)
  INCLUDE(${VISIT_MANTA_DIR}/include/CMake/ConfigArchitecture.cmake)

  # Determine information about the compiler
  INCLUDE(${VISIT_MANTA_DIR}/include/CMake/CompilerInfo.cmake)

  # Set various options based on the build type.
  INCLUDE(${VISIT_MANTA_DIR}/include/CMake/BuildType.cmake)

  # Check if the build supports SSE
  INCLUDE(${VISIT_MANTA_DIR}/include/CMake/CheckSSE.cmake)  

  # Force compilation options for all code that includes Manta headers.
  FORCE_ADD_FLAGS("-DSCI_NOPERSISTENT")

ENDIF(VISIT_MANTA_DIR)
