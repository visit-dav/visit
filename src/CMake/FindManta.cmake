###############################################################################
# This scripts prompts the user to set the variable MANTA_SOURCE_DIR and 
# MANTA_BUILD_PREFIX. After this is done, the script sets variables:
#
# MANTA_INCLUDE               -- Paths containing Manta header files.
# MANTA_TARGET_LINK_LIBRARIES -- List of Manta shared libraries.  (-l on link line)
# MANTA_LINK_DIRECTORIES      -- Path containing shared libraries (-L on link line)
#
# Additionally several .cmake scripts from the Manta build are executed to 
# insure a similar build environment will be used by the project.
# 
###############################################################################

IF   (MANTA_SOURCE_DIR AND MANTA_BUILD_PREFIX)

  # Set the include and link variables.
  SET(MANTA_INCLUDE
    ${MANTA_SOURCE_DIR}
    ${MANTA_SOURCE_DIR}/SCIRun
    ${MANTA_SOURCE_DIR}/SCIRun/include
    ${MANTA_BUILD_PREFIX}/include
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
#    SCIRun_Core
    About
    )

  SET(MANTA_LINK_DIRECTORIES
    ${MANTA_BUILD_PREFIX}/lib
    )


  # Include Manta header files.
  INCLUDE_DIRECTORIES(
    ${MANTA_INCLUDE}
    )
  
  # Include Manta library directory.
  LINK_DIRECTORIES(
    ${MANTA_LINK_DIRECTORIES}
    )

  # Initialize Python/SWIG.
  SET(CMAKE_SWIG_OUTDIR ${LIBRARY_OUTPUT_PATH})
  FIND_PATH(SWIG_DIR swig)
  FIND_PACKAGE(SWIG)

  # Important: Must use Manta's copy of UseSWIG.cmake
  INCLUDE(${MANTA_SOURCE_DIR}/CMake/MantaUseSWIG.cmake)

  FIND_PACKAGE(PythonLibs)
  INCLUDE_DIRECTORIES(
    ${PYTHON_INCLUDE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

  # Load Manta macros.
  INCLUDE(${MANTA_SOURCE_DIR}/CMake/Macros.cmake)

  # Set flags based on the architecture we are on (no compiler stuff)
  INCLUDE (${MANTA_SOURCE_DIR}/CMake/ConfigArchitecture.cmake)

  # Determine information about the compiler
  INCLUDE (${MANTA_SOURCE_DIR}/CMake/CompilerInfo.cmake)

  # Set various options based on the build type.
  INCLUDE (${MANTA_SOURCE_DIR}/CMake/BuildType.cmake)

  # Check if the build supports SSE
  INCLUDE (${MANTA_SOURCE_DIR}/CMake/CheckSSE.cmake)  

  # Force compilation options for all code that includes Manta headers.
  FORCE_ADD_FLAGS("-DSCI_NOPERSISTENT")


# Otherwise prompt the user to enter the desired Manta build to use.
ELSE (MANTA_SOURCE_DIR AND MANTA_BUILD_PREFIX)

  SET(MANTA_SOURCE_DIR "" CACHE PATH "Directory Manta was checked out into.")
  SET(MANTA_BUILD_PREFIX "" CACHE PATH "Build directory containing lib/ bin/ etc. sub-directories.")

  MESSAGE(FATAL_ERROR "Manually set the paths MANTA_SOURCE_DIR and MANTA_BUILD_PREFIX")

ENDIF(MANTA_SOURCE_DIR AND MANTA_BUILD_PREFIX)
