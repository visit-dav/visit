# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#****************************************************************************


option(VISIT_PARALLEL "Build VisIt's parallel compute engine." OFF)
option(VISIT_SLIVR    "Build VisIt with support for the SLIVR volume rendering library." ON)
option(VISIT_STATIC   "Build VisIt statically." OFF)
if(VISIT_STATIC)
    add_definitions(-DVISIT_STATIC)
endif()
option(VISIT_OSX_USE_RPATH      "Use rpath instead of executable_path while installing" OFF)
option(VISIT_PYTHON_SCRIPTING      "Build VisIt with Python scripting support." ON)
option(VISIT_PYTHON_FILTERS        "Build VisIt with Python Engine Filter support." ON)
option(VISIT_DDT      "Build VisIt with support for the DDT debugger." OFF)
if(VISIT_DDT)
  add_definitions(-DHAVE_DDT)
endif()

option(VISIT_BUILD_ALL_PLUGINS     "Build all of VisIt's plugins." OFF)
option(VISIT_BUILD_MINIMAL_PLUGINS "Build a minimal set of VisIt's plugins." OFF)
option(VISIT_JAVA "Build the VisIt Java client interface" OFF)
option(VISIT_PARADIS "Build the VisIt paraDIS client interface" ON)

#----------------
# components
#----------------
option(VISIT_SERVER_COMPONENTS_ONLY "Build only vcl, mdserver, engine and their plugins" OFF)
option(VISIT_ENGINE_ONLY "Build only the compute engine and its plugins" OFF)
option(VISIT_DBIO_ONLY "Build only visitconvert and engine plugins" OFF)

# If we're doing DBIO_ONLY then compile all sources with this flag so we don't
# ever have to do things like include visit-config.h to get it.
if(VISIT_DBIO_ONLY)
    add_definitions(-DDBIO_ONLY)
endif()


#----------------
option(VISIT_DISABLE_SELECT "Disable use of the select() function" OFF)
option(VISIT_USE_NOSPIN_BCAST "Use VisIt's no-spin Bcast in parallel" ON)
option(VISIT_NOLINK_MPI_WITH_LIBRARIES "Do not link MPI with VisIt's parallel shared libraries; just with executables" OFF)
option(VISIT_CREATE_SOCKET_RELAY_EXECUTABLE "Create a separate executable that forwards VisIt's socket connection between engine and component launcher" ON)
option(VISIT_FORTRAN "Enable compilation of Fortran example progams" OFF)

option(IGNORE_THIRD_PARTY_LIB_PROBLEMS "Ignore problems finding requested third party libraries")
option(VISIT_FORCE_SSH_TUNNELING "Force ssh tunnelling for sockets" OFF)
option(VISIT_CREATE_APPBUNDLE_PACKAGE "Create DMG file with Mac App bundle with make package" OFF)

if(WIN32)
  option(VISIT_MESA_REPLACE_OPENGL "Use mesa as a drop-in replacment for opengl when system opengl is insufficient." OFF)
  option(VISIT_MAKE_NSIS_INSTALLER "Create an installer package using NSIS" OFF)
  option(VISIT_WINDOWS_APPLICATION "Create Windows-style applications with no console" ON)
  if(VISIT_WINDOWS_APPLICATION)
      set(VISIT_APPLICATION_STYLE "WIN32")
  endif()
endif(WIN32)

option(VISIT_CREATE_XMLTOOLS_GEN_TARGETS "Create cmake targets to run xmltools code gen targets" ON)

# install related
option(VISIT_INSTALL_AS_BETA "Install as beta (symlink beta instead of current to installed version)." OFF)
option(VISIT_MPICH_INSTALL "Install MPICH into VisIt binary distribution" OFF)
option(VISIT_INSTALL_THIRD_PARTY "Install VisIt's 3rd party I/O libs and includes to permit plugin development" OFF)
option(VISIT_RPATH_RELATIVE_TO_EXECUTABLE_PATH "Install rpath relative to executable location using \$ORIGIN tag" OFF)

if(APPLE OR WIN32)
    option(VISIT_USE_X "Use X11" OFF)
else()
    option(VISIT_USE_X "Use X11" ON)
endif()

option(VISIT_THREAD "Build VisIt with thread support." OFF)

# some options that when turned off reduce the build targets
option(VISIT_ENABLE_ANNOTATION_TOOLS "Enable generation of annotation tools (text2polys, time_annotation)" ON)
option(VISIT_ENABLE_DATAGEN "Enable generation of sample data files" ON)
option(VISIT_ENABLE_DATA_MANUAL_EXAMPLES "Build Getting Data Into VisIt examples" OFF)
option(VISIT_ENABLE_DIAGNOSTICS  "Enable building of diagnostices" ON)
option(VISIT_ENABLE_EXAMPLES  "Enable building of examples" ON)
option(VISIT_ENABLE_LIBSIM "Enable building of libsim." ON)
option(VISIT_ENABLE_MANUALS  "Enable building of manuals (requires python3)" ON)
option(VISIT_ENABLE_SILO_TOOLS "Enable building of silo tools (mrgtree2dot, add_visit_searchpath)" ON)
option(VISIT_ENABLE_UNIT_TESTS "Enable building of unit tests (like MRUCache_test, StringHelpers_test)" ON)
option(VISIT_ENABLE_XDB "Enable Intelligent Light XDB database plugin." OFF) # OFF for now

set(VISIT_INSTALL_PROFILES_TO_HOSTS "" CACHE STRING "Install specified host profiles (no action if empty)." )
set(VISIT_DEFAULT_SILO_DRIVER "PDB" CACHE STRING "Default driver for generating silo data. Valid values are PDB, HDF5 or empty. An empty value means driver can be specified via make on the command line." )

# flag to turn on preprocessor errors related to obsolete code.
option(VISIT_REMOVE_OBSOLETE_CODE "Enable compiler errors for removal of oboslete code." OFF)


