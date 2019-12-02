# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#   Mark C. Miller, Wed Jan 13 18:30:38 PST 2010
#   Added logic to ensure FORCE_ITAPS_REGEN is never cached in an ON state.
#
#   Mark C. Miller, Thu Jan 14 10:36:49 PST 2010
#   Undid above change as it turns out the value gets cached regardless of
#   an SET() command here. This was not observed in earlier tests because
#   I was ctrl-C'ing after the cmake process had past the point I was testing.
#
#   Mark C. Miller, Thu Jan 14 12:01:48 PST 2010
#   Added -D_LARGEFILE64_SOURCE to definitions for UNIX systems. This could
#   be a bit too global of a context in which to set it. We'll see. I also
#   added logic to set CMAKE_REQUIRED_DEFINITIONS prior to checking size
#   of off64_t. In theory, we might need to set CMAKE_REQUIRED_INCLUDES
#   too but that was not required on the systems I tested.
#
#   David M. Camp, Th Jan 14 11:50:00 PST 2010
#   Added new function ADD_TARGET_DEFINITIONS to add defines to targets.
#   This was needed for the plots to define ENGINE only for the engine build.
#
#    Gunther H. Weber, Thu Jan 28 14:33:36 PST 2010
#    Added hack/workaround that ensures that the static MPI libraries get added
#    to the end of the link line. In essence, we add a dummy/empty library
#    that has the MPI libraries as dependencies. This libary is added to the
#    end of the link line of parallel executables. Doing so ensures that cmake
#    will add the depencies of this dummy library, i.e., the MPI libraries, to
#    the end of the link line.
#
#    Gunther H. Weber, Fri Jan 29 10:42:11 PST 2010
#    Added a new option that supports setting the rpath relative to the
#    executable location. The purpose of this is to make VisIt not depend on
#    a properly set LD_LIBRARY_PATH as some MPI variants do not pass
#    environment variables.
#
#    Kathleen Bonnell, Wed Feb  3 11:28:41 PST 2010
#    Encapsulated VISIT_INSTALL_x_PLUGINS into one Macro that accepts a
#    type parameter.  Changed VISIT_THIRDPARTY_OPTION into two macros that
#    set a PATH var (for VISIT_x_DIR) or STRING var (for VISIT_x_LIBDEP).
#
#    Kathleen Bonnell, Tue Feb 16 14:04:16 MST 2010
#    Removed conditional setting of VISIT_RENDERING_SIZE_LIMIT, in case
#    initial configure pass was missing something. Install our zlib on windows.
#
#    Cyrus Harrison, Sun Feb 21 19:12:33 PST 2010
#    Added support for enable/disable of Python Engine Filters.
#
#    Eric Brugger, Mon Mar  8 13:02:08 PST 2010
#    Modified the logic that sets the VISIT_SVN_REVISION to get it from
#    the file SVN_REVISION before trying svn to get it.
#
#    Cyrus Harrison, Tue Mar  9 08:02:50 PST 2010
#    Enabled python filters support by default.
#
#    Kathleen Bonnell, Wed Mar 17 10:06:44 MST 2010
#    Change cmake minimum required to 2.8.0, to support FILE(COPY .
#
#    Kathleen Bonnell, Thu April 1 19:29:24 MST 2010
#    Find MakeNSIS on windows. Add PACKAGE target for windows, that uses
#    our NSIS scripts to create the self-installer.
#
#    Kathleen Bonnell, Wed April 14 16:33:27 MST 2010
#    Renable FindTortoiseSVN on windows to retrieve SVN revision number.
#
#    Jeremy Meredith, Mon Apr 19 16:34:02 EDT 2010
#    Support multiple paths returned for include files from mpi compiler.
#
#    Cyrus Harrison, Tue Apr 27 13:39:38 PDT 2010
#    Added ability to bake rpaths for MPI libs into exes & libs when
#    'make install' & 'make package' are used.
#
#    Cyrus Harrison, Thu Apr 29 09:32:01 PDT 2010
#    Small change to rpath detection to avoid cmake error w/
#    LIST(REMOVE_DUPLCIATES) if no rpaths are found.
#
#    Tom Fogal, Fri Apr 30 09:35:51 MDT 2010
#    Define HAVE_LIBSLIVR as appropriate.
#
#    Eric Brugger, Thu May  6 12:23:34 PDT 2010
#    I corrected the rpath detection logic to handle the case were there
#    were multiple rpaths.  In particular, INSTALL_RPATH was being set as a
#    list of rpaths (the rpaths end up semi-colon separated) and I changed
#    it to a string of space separated rpaths.
#
#    Kathleen Bonnell, Tue May 11 13:01:44 MST 2010
#    Set HAVE_SELECT to TRUE on windows.
#
#    Kathleen Bonnell, Fri Jun 4 7:58:34 PDT 2010
#    Changed logic for finding SVN_REVISION so check is not performed on
#    any platform if not building from a working copy.
#
#    Eric Brugger, Wed Jun  9 13:06:18 PDT 2010
#    I made a correction to a MESSAGE statement in the code that adds the
#    current link to the installation directory and added a comment that
#    explains why the flawed code we have there is actually what we want.
#
#    Eric Brugger, Thu Jun 17 13:38:03 PDT 2010
#    I modified the file to add -DMPICH_IGNORE_CXX_SEEK wherever -DPARALLEL
#    is specified to eliminate a compile failure with some versions of mpi.
#
#    Mark C. Miller, Thu Jul 29 17:36:48 PDT 2010
#    Added logic to include compiler version information as a comment int
#    The PluginVsInstall.cmake file.
#
#    Mark C. Miller, Thu Jul 29 23:43:52 PDT 2010
#    Added option to ignore problems finding third party libs. It is OFF
#    by default.
#
#    Kathleen Bonnell, Thu Aug 5 9:34:52 PDT 2010
#    For winodws, only override cmake's default CMAKE_INSTALL_PREFIX if it
#    wasn't specified on the command line.
#
#    Tom Fogal, Thu Aug  5 18:28:17 MDT 2010
#    Add missing include/visit directory to default include path.
#
#    Kathleen Bonnell, Fri Aug 13 9:59:37 MST 2010
#    Allow host-specific config-site files for Windows build.  If there is no
#    host-specific file, then windows.cmake is still used.
#
#    Kathleen Bonnell, Fri Aug 13 10:50:15 MST 2010
#    Allow config-site file to be specified on command-line.  Overrides
#    host-specific (all platforms) or windows.cmake (on windows).
#    usage:  -D VISIT_CONFIG_SITE="/path/to/config-site/file"
#
#    Cyrus Harrison, Mon Aug 16 13:12:17 PDT 2010
#    1) Disable python filters if VISIT_NOLINK_MPI_WITH_LIBRARIES is true b/c
#       the mpicom module (a shared lib) must link to mpi.
#    2) (Non-windows) add extra info about success of config-site include.
#
#    Kathleen Bonnell, Thu Aug 19 18:05:27 MST 2010
#    Fix broken plugin install target on windows.
#
#    Kathleen Bonnell, Thu Sep 2 15:44:22 PDT 2010
#    Add flag to windows nsis-installer script for 64 bit.
#
#    Mark C. Miller, Tue Sep 21 13:47:27 PDT 2010
#    Fix DBIO_ONLY mode.
#
#    Tom Fogal, Wed Sep 22 13:45:36 MDT 2010
#    Fix HAVE_LIBGLEW definition.
#
#    Mark C. Miller, Fri Oct 22 12:28:25 PDT 2010
#    Added missing LIBDEP support for ExodusII dependence on (its own) netcdf.
#
#    Brad Whitlock, Fri Oct 29 16:56:34 PDT 2010
#    I added some more filtering when generating VisItLibraryDependencies.cmake
#    so it does not include local path names for Qt and Python.
#
#    Kathleen Bonnell, Thu Dec 2 15:49:04 MST 2010
#    Changed how ZLIB is handled on Windows.  Add compiler version messages
#    for MSVC to PluginVsInstall.cmake.  Make windows-specific string
#    replacements in VisItLibraryDependencies.cmake.  Allow '*.hpp' when
#    adding headers.  Add VisItGenerator.cmake file for plugin development
#    against an Installed VisIt on Windows.
#
#    Kathleen Bonnell, Fri Dec 10 14:37:15 PST 2010
#    Add VISIT_3RDPARTY_VAR(VISIT_ZLIB_DIR for windows.
#
#    Cyrus Harrison, Mon Dec 20 10:50:29 PST 2010
#    Use FORCE when setting various install paths.
#
#    Kathleen Bonnell, Tue Dec 28 17:06:41 MST 2010
#    Add VISIT_3RDPARTY_VAR for VISIT_SZIP_DIR and VISIT_JPEG_DIR for Windows,
#    and VISIT_H5PART_LIBDEP for all platforms. Call find for szip, zlib and
#    jpeg before Finds for any libs that may have dependencies on them.
#
#    Kathleen Bonnell, Wed Jan 5 10:36:29 PST 2010
#    Turn on use of FOLDER property.
#    Move Plugin macros to their own CMake file.
#
#    Kathleen Bonnell, Thu Jan 6 15:31:12 PST 2010
#    Allow data dir to be built on windows.
#
#    Eric Brugger, Fri Jan  7 13:24:41 PST 2011
#    I replaced the BOXLIB2D and BOXLIB3D variables with just BOXLIB.
#
#    Cyrus Harrison, Wed Jan 12 11:41:21 PST 2011
#    Add support for optional TYPE [CMAKE_TYPE] arg par to the
#    VISIT_OPTION_DEFAULT function.
#
#    Cyrus Harrison, Wed Jan 12 11:41:21 PST 2011
#    Added init of opt_ARG_PREV in VISIT_OPTION_DEFAULT to avoid cmake
#    warning message.
#
#    Kathleen Bonnell, Mon Feb 14 11:09:17 MST 2011
#    Remove strerror test, HAVE_STRERROR no longer used in VisIt.
#
#    Kathleen Bonnell, Wed Feb 16 08:33:33 PST 2011
#    Set CMAKE_BUILD_TYPE to Release, if not already set.
#
#    Mark C. Miller, Tue Mar 22 17:42:35 PDT 2011
#    I fixed problems overriding behavior of VISIT_3RDPARTY_VAR/DEP macros
#    when cmake is invoked with -DFOO_DIR:PATH=<path> to specify path to a
#    3rd party FOO lib. It was not working previously. I also removed
#    redundancy in args passed to VISIT_3RDPARTY_VAR/DEP macros
#    (VISIT_FOO_DIR and FOO_DIR replaced with just FOO_DIR).
#
#    Mark C. Miller, Wed Mar 30 09:30:49 PDT 2011
#    Fixed VISIT_3RDPARTY_VAR/DEP macros to define empty string ("") when
#    a lib's symbols are not defined instead of -NOTFOUND.
#
#    Eric Brugger, Thu May 12 12:50:58 PDT 2011
#    Add VISIT_FORCE_SSH_TUNNELING that always forces ssh tunneling for all
#    data connections.
#
#    Gunther H. Weber, Wed Jul 20 15:32:26 PDT 2011
#    Add support for creating a Mac App bundle by setting
#    VISIT_CREATE_APPBUNDLE_PACKAGE to ON.
#
#    Kathleen Biagas, Mon Aug 8 08:08:42 MST 2011
#    Use VISIT_3RDPARTY_DEP for all libdep vars.
#
#    Kathleen Biagas, Wed Aug 17 10:59:33 PDT 2011
#    Remove vtkzlib as a substitute for zlib.
#
#    Kathleen Biagas, Wed Aug 17 12:31:50 PDT 2011
#    Make use of NSIS to create an installer an option.
#
#    Gunther H. Weber, Tue Sep 20 17:15:48 PDT 2011
#    Use NERSC_HOST environment variable instead of hostname to determine if
#    we are running on Franklin or Hopper.
#
#    Kathleen Biagas, Mon Sep 26 14:59:32 MST 2011
#    Add mpi search support for Windows.
#
#    Gunther H. Weber, Wed Oct  5 19:14:27 PDT 2011
#    Change NERSC_HOST to VISIT_HOSTNAME to make it more general
#
#    Dave Pugmire, Tue Jan 10 15:30:02 EST 2012
#    Added support for R.
#
#    Kathleen Biagas, Tue Jan 24 12:22:47 MST 2012
#    On windows, use VisIt version of FindMPI.cmake to ensure correct search
#    paths.  Also ensure paths returned from FindMPI are converted to NATIVE
#    style.
#
#    Eric Brugger, Wed Feb 22 10:14:46 PST 2012
#    I added support for building without mesa.
#
#    Kathleen Biagas, Fri Mar  9 15:30:58 PST 2012
#    Prevent retesting of PROTOTYPE functions on re-configure.
#
#    Brad Whitlock, Fri May 18 16:35:34 PST 2012
#    Enhance support for resource files on Windows so we fill in more
#    application information.
#
#    Cyrus Harrison, Tue May 22 10:52:56 PDT 2012
#    Make PySide search conditional on VISIT_PYTHON_SCRIPTING.
#    Make Python search conditional on VISIT_PYTHON_SCRIPTING OR
#    VISIT_PYTHON_FILTERS
#
#    Eric Brugger, Fri May 25 09:50:53 PDT 2012
#    I set the VISIT_RENDERING_SIZE_LIMIT to 16384 so that we are not
#    unnecessarily constraining the user. There is no way to set this
#    properly since this is used in the viewer and the limit really comes
#    from the engine, which may have a different size if running
#    client/server. This setting should be removed and a runtime check
#    should be added to the engine.
#
#    Kathleen Biagas, Thu Jun 21 11:12:57 MST 2012
#    Include paths for vtk, exodusii, python and lib names for ptyhon are
#    different on windows than unix, so use special vars for use with
#    PluginVsInstall.
#    Added function 'ADD_TARGET_INCLUDE'.  (requires cmake 2.8.8 to work)
#    Modify how parallel flags are handled on windows.
#    Fix glitch with ADD_TARGET_DEFINITIONS when target
#    had more than one definition set already.
#
#    Kathleen Biagas, Tue Jun 26 13:38:27 MST 2012
#    Fixed parallel definitions for windows (removed -D).
#
#    Kathleen Biagas, Wed Sep 12 16:02:02 PDT 2012
#    Added VISIT_INSTALL_PROFILES_TO_HOST as a first-class VisIt option.
#
#    Kathleen Biagas, Mon Dec 17 16:56:38 MST 2012
#    Use CMake's FindMPI module for windows.
#
#    Cyrus Harrison, Thu Jan 10 10:03:50 PST 2013
#    Try to capture the full cmake invocation command line.
#    On non windows systems, generate a script that allows re-config
#    from scratch using the captured invocation command line.
#
#    Mark C. Miller, Tue Jan 15 17:15:43 PST 2013
#    Modified cmake invokation logic to ensure the CMAKE_INVOKE variable
#    is indeed (forced) cached so it winds up in CMakeCache.txt and
#    adjusted name of the shell script to recmake_visit.sh.
#
#    Mark C. Miller, Wed Feb  6 16:23:09 PST 2013
#    Fix missing '#' in first line of recmake_visit.sh file.
#
#    Kathleen Biagas, Tue Feb 26 13:58:12 MST 2013
#    Added VISIT_XXX_DEFAULT internal cache vars that store values set in
#    config-site cmake files via VISIT_OPTION_DEFAULT.  This allows for
#    VISIT_XXX vars to be overridden by changes to CMakeCache.txt.  If values
#    in the config-site cmake files change, they will still take precedence
#    over changes made to CMakeCache.txt (ie preserves old behavior).
#
#    Eric Brugger, Tue Mar 19 10:40:49 PDT 2013
#    Added code to only build the paraDIS reader if boost 1.36.0 or later
#    was installed on the system.
#
#    Eric Brugger, Wed Mar 20 17:23:28 PDT 2013
#    I modified the script to have VisIt use the system boost everywhere
#    instead of the internal boost if the system boost is usable by
#    paraDIS. This is to avoid using the system boost in the paraDIS
#    reader and the internal boost in the rest of visit. This fix was
#    recommended by Gunther. The other option was to add more of boost
#    to the internal boost so that it could be used by paraDIS, but that
#    seemed like more work.
#
#    Kathleen Biagas, Thur May 2 14:08:27 MST 2013
#    Don't bother finding bulk of thirdparty libs if building minimal plugins.
#
#    Kathleen Biagas, Wed May  8 16:24:01 PDT 2013
#    Remove EXODUS, no longer needed.
#
#    Eric Brugger, Thu May  9 17:09:27 PDT 2013
#    Removed support for mangled mesa.
#
#    Kathleen Biagas, Tue Sep  3 11:06:51 PDT 2013
#    Turn off SLIVR if we aren't using GLEW.
#
#   Kathleen Biagas, Tues Oct 1 09:33:47 MST 2013
#   Handle reorganization of windowsbuild directory.
#
#    Cyrus Harrison, on Oct 28 14:39:05 PDT 2013
#    Fix exported VTK targets for VTK-6. Fix VTK-6 include dirs
#    for make install / package.
#
#    Kathleen Biagas, Wed Nov  6 17:42:32 PST 2013
#    Remove setting of CMAKE_Java_xxx, rely instead on the Java_xxx vars
#    set by FindJava.cmake
#
#    Kathleen Biagas, Tue Nov 26 10:14:29 PST 2013
#    Only add osxfixup dir if on APPLE.
#
#    Eric Brugger, Fri Feb 28 15:18:49 PST 2014
#    I added -DMPI_NO_CPPBIND to the VISIT_PARALLEL_CFLAGS and
#    VISIT_PARALLEL_CXXFLAGS.
#
#    Kathleen Biagas, Mon May 12 12:58:02 PDT 2014
#    Add VISIT_SELECTED_PLUGIN_ERROR macro.
#
#    Kathleen Biagas, Wed Oct 22 16:31:49 MST 2014
#    Always use -DNOMINMAX on Windows.
#
#    Cyrus Harrison, Tue Feb 10 20:06:07 PST 2015
#    Change boost support.
#
#    Kathleen Biagas, Tue Apr 14 14:56:12 MST 2015
#    Added find_file for MS HPC Scheduler.
#
#    Matthieu Dorier  via Cyrus Harrison, Thu Aug 20 09:07:08 PDT 2015
#    Added Damaris support.
#
#    Burlen Loring, Thu Oct  8 10:45:36 PDT 2015
#    Added define for aligned_alloc in visit-config.h
#
#    Eric Brugger, Wed Dec  9 12:05:18 PST 2015
#    Added support for VTKm.
#
#    Eric Brugger, Thu Jan 14 08:43:03 PST 2016
#    Added vtkm directory that builds vtkm library.
#
#    Cyrus Harrison, Thu Jun 16 08:44:43 PDT 2016
#    Added support for conduit (software.llnl.gov/conduit).
#
#    Iulian R. Grindeanu & Vijay S. Mahadevan via Mark C Miller
#    Wed Aug 10 14:54:05 PDT 2016
#    Added support for ANL's Mesh Object datABase (MOAB)
#
#    Mark C. Miller, Mon Aug 22 15:23:12 PDT 2016
#    Add tools/writer to installed headers
#
#    Kathleen Biagas, Mon Oct 31 15:23:31 PDT 2016
#    Add ZLIB_INCLUDE_DIR to VISIT_COMMON_INCLUDES when HAVE_ZLIB_H is true.
#
#    Kathleen Biagas, Fri Mar 17 12:12:34 PDT 2017
#    Move setting of HAVE_LIBxxx to SetUpThirdParty or FindXXX in the case
#    where a special HAVE_XXX is neeeded.
#
#    Eric Brugger, Wed May 17 16:53:48 PDT 2017
#    I added support for the LLVM and OpenSWR packages.
#
#    Kathleen Biagas, Fri Mar  2 14:52:00 MST 2018
#    Removed TUVOK.
#
#    Eric Brugger, Tue Mar 13 13:24:56 PDT 2018
#    I modified the header install command to exclude any directories named
#    icons. Previously, this resulted in a couple of invalid symbolic links
#    ending up in the distribution file.
#
#    Qi WU, Sat Jun 10 22:21:27 MST 2018
#    I added support to find OSPRay
#
#    Eric Brugger, Tue Aug 28 14:35:30 PDT 2018
#    I replaced support for vtkm with vtkh.
#
#    Kathleen Biagas, Fri Sep 28 16:34:09 PDT 2018
#    Moved code for filtering lib dependencies (needed for building a plugin
#    vs installed version) into separate CMake files, all handled by
#    PluginVsInstallHelpers.cmake.
#
#    Kathleen Biagas, Mon Dec 10 14:41:27 MST 2018
#    Have CMake find the MSVC runtime libraries needed for Windows distro.
#
#    Kevin Griffin, Wed Dec 12 18:00:27 PST 2018
#    Added back osxfixup logic.
#
#    Eric Brugger, Mon Jan 28 15:14:54 PST 2019
#    Changed SVN_REVISION to GIT_VERSION.
#
#    Kathleen Biagas, Tue May 28 09:44:02 PDT 2019
#    ZLIB is no longer optional.
#
#    Eric Brugger, Wed Jun 26 10:06:57 PDT 2019
#    Removed the CMM reader.
#
#    Alister Maguire, Thu Nov  7 16:22:47 PST 2019
#    Build the sphinx documentation.
#
#****************************************************************************

CMAKE_MINIMUM_REQUIRED(VERSION 3.8 FATAL_ERROR)

IF (COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
    cmake_policy(SET CMP0011 NEW)
    cmake_policy(SET CMP0033 OLD)
    cmake_policy(SET CMP0057 NEW)
    cmake_policy(SET CMP0062 NEW)
    IF(WIN32)
        # don't automatically link with qtmain
        cmake_policy(SET CMP0020 OLD)
    ENDIF()
ENDIF (COMMAND cmake_policy)

#
# Try to capture the initial set of cmake command line args passed by
# the user for configuration.
# Recipe taken from http://stackoverflow.com/questions/10205986/how-to-capture-cmake-command-line-arguments
#
# Note: The entrires will live on CMakeCache.txt, so re-configuring with
# a command line that doesn't include an option won't remove it. You need
# to remove the CMakeCache.txt file, or override the value via the command line.
#
#
GET_CMAKE_PROPERTY(CACHE_VARS CACHE_VARIABLES)
FOREACH(CACHE_VAR ${CACHE_VARS})
  GET_PROPERTY(CACHE_VAR_HELPSTRING CACHE ${CACHE_VAR} PROPERTY HELPSTRING)
  IF(CACHE_VAR_HELPSTRING STREQUAL "No help, variable specified on the command line.")
    GET_PROPERTY(CACHE_VAR_TYPE CACHE ${CACHE_VAR} PROPERTY TYPE)
    IF(CACHE_VAR_TYPE STREQUAL "UNINITIALIZED")
      SET(CACHE_VAR_TYPE)
    ELSE(CACHE_VAR_TYPE STREQUAL "UNINITIALIZED")
      SET(CACHE_VAR_TYPE :${CACHE_VAR_TYPE})
    ENDIF()
    SET(CMAKE_INVOKE_ARGS "${CMAKE_INVOKE_ARGS} -D${CACHE_VAR}${CACHE_VAR_TYPE}=\"${${CACHE_VAR}}\"")
  ENDIF()
ENDFOREACH(CACHE_VAR ${CACHE_VARS})
# Record the full command line invocation.
SET(CMAKE_INVOKE "${CMAKE_COMMAND} ${CMAKE_INVOKE_ARGS} ${CMAKE_CURRENT_SOURCE_DIR}" CACHE STRING "Command used to invoke cmake" FORCE)
# Create a simple shell script that allows us to reinvoke cmake with the captured command line.
IF (NOT WIN32)
    FILE(WRITE ${CMAKE_BINARY_DIR}/recmake_visit.sh "#!/bin/sh\n"
                                                     "rm -f CMakeCache.txt\n"
                                                     "${CMAKE_INVOKE}\n")
ENDIF (NOT WIN32)


# Tell the CMake makefile generator to not have rules depend on
# themselves.  This causes extra rebuilds when the include path
# changes from turning a kit on or off.
SET(CMAKE_SKIP_RULE_DEPENDENCY 1)

# for now...
SET(BUILD_SHARED_LIBS 1)

# this turns on the FOLDER property for generators that may make use of it, like Visual Studio
# should have no effect for generators that do not use.
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

#-----------------------------------------------------------------------------
# Get this computer's hostname using cmake's cross platform helper function.
#----------------------------------------------------------------------------

SITE_NAME(HOSTNAME)

# Use NERSC_HOST environment variable to determine if we are running on Franklin
# or Hopper. If so, use that hostname instead of the result of SITE_NAME which is
# only the name of the login node we are running on.
IF(NOT "$ENV{VISIT_HOSTNAME}" STREQUAL "")
    SET(HOSTNAME "$ENV{VISIT_HOSTNAME}")
ENDIF(NOT "$ENV{VISIT_HOSTNAME}" STREQUAL "")


#-----------------------------------------------------------------------------
# Helper function for setting build options via config-site file.
# Supports optional argument pair: TYPE [CMAKE_TYPE], default type = PATH
#-----------------------------------------------------------------------------
FUNCTION(VISIT_OPTION_DEFAULT var)
  SET(opt_ARGS "")
  SET(opt_TYPE "PATH")
  SET(opt_ARG_PREV "[unset]")
  FOREACH(opt_ARG ${ARGN})
    ## if arg is TYPE:
    ##   we want to use the next value as the cmake var type
    ## otherwise:
    ##   we want to append the argument to the list of args used for "SET"
    IF( ${opt_ARG_PREV} STREQUAL "TYPE")
        SET(opt_TYPE ${opt_ARG})
    ELSEIF(NOT ${opt_ARG} STREQUAL "TYPE")
        LIST(APPEND opt_ARGS ${opt_ARG})
    ENDIF(${opt_ARG_PREV} STREQUAL "TYPE")
    SET(opt_ARG_PREV ${opt_ARG})
  ENDFOREACH(opt_ARG in ${ARGN})
  ##
  SET(default_base "${var}_DEFAULT")
  SET(default_base_val  "${${default_base}}")
  IF(NOT "${default_base_val}" STREQUAL "${opt_ARGS}")
      SET("${default_base}" "${opt_ARGS}" CACHE INTERNAL "${var} default value" FORCE)
      SET(${var} "${opt_ARGS}" CACHE ${opt_TYPE} "${var} value" FORCE)
  ENDIF(NOT "${default_base_val}" STREQUAL "${opt_ARGS}")
ENDFUNCTION(VISIT_OPTION_DEFAULT var)

SET(VTK_MINIMUM_VERSION "8.1.0" CACHE INTERNAL "minimum version of vtk that VisIt can use")

SET(NEKTAR++_MINIMUM_VERSION "4.0.0" CACHE INTERNAL "minimum version of Nektar++ that VisIt can use")

FUNCTION(SETUP_APP_VERSION name ver_ascii )
  IF(${ver_ascii} VERSION_LESS ${${name}_MINIMUM_VERSION}})
      MESSAGE(FATAL_ERROR "${name} version must be at least ${${name}_MINIMUM_VERSION}}")
  ENDIF()
  STRING( REGEX REPLACE "([0-9]+).[0-9]+.[0-9]+[.0-9]*" "\\1" ver_major ${ver_ascii} )
  STRING( REGEX REPLACE "[0-9]+.([0-9]+).[0-9]+[.0-9]*" "\\1" ver_minor ${ver_ascii} )
  STRING( REGEX REPLACE "[0-9]+.[0-9]+.([0-9]+)[.0-9]*" "\\1" ver_patch ${ver_ascii} )
  SET(${name}_VERSION "${ver_ascii}"  CACHE INTERNAL "")
  SET(${name}_MAJOR_VERSION "${ver_major}" CACHE INTERNAL "")
  SET(${name}_MINOR_VERSION "${ver_minor}" CACHE INTERNAL "")
  SET(${name}_PATCH_VERSION "${ver_patch}" CACHE INTERNAL "")
  math( EXPR ver_hex "(${ver_major}<<16)|(${ver_minor}<<8)|(${ver_patch})")
  SET(${name}_VERSION_HEX "${ver_hex}" CACHE INTERNAL "")
ENDFUNCTION()

#-----------------------------------------------------------------------------
# Include this computer's config-site if one exists
#-----------------------------------------------------------------------------
IF (NOT WIN32)
  IF(VISIT_CONFIG_SITE)
      SET(VISIT_CONFIG_SITE_FILE ${VISIT_CONFIG_SITE})
  ELSE(VISIT_CONFIG_SITE)
      SET(VISIT_CONFIG_SITE_FILE "config-site/${HOSTNAME}.cmake")
  ENDIF(VISIT_CONFIG_SITE)

  MESSAGE(STATUS "Trying to include config-site file '${VISIT_CONFIG_SITE_FILE}'")
  INCLUDE(${VISIT_CONFIG_SITE_FILE} OPTIONAL RESULT_VARIABLE VISIT_CONFIG_SITE_FILE_FOUND)
  IF("${VISIT_CONFIG_SITE_FILE_FOUND}" STREQUAL  "NOTFOUND")
        MESSAGE(STATUS "NOT FOUND: '${VISIT_CONFIG_SITE_FILE}'")
  ELSE("${VISIT_CONFIG_SITE_FILE_FOUND}" STREQUAL  "NOTFOUND")
        MESSAGE(STATUS "Included: '${VISIT_CONFIG_SITE_FILE}'")
  ENDIF("${VISIT_CONFIG_SITE_FILE_FOUND}" STREQUAL  "NOTFOUND")

ENDIF (NOT WIN32)

#-----------------------------------------------------------------------------
# If the config-site file above has not set a VTK version set a
# default VTK version of 8.1.0.
#-----------------------------------------------------------------------------
IF(NOT VTK_VERSION)
  IF(VISIT_VTK_VERSION)
      IF(${VISIT_VTK_VERSION} VERSION_LESS ${VTK_MINIMUM_VERSION})
          MESSAGE(FATAL_ERROR "VTK version must be at least ${VTK_MINIMUM_VERSION}")
      ENDIF()
      SETUP_APP_VERSION(VTK ${VISIT_VTK_VERSION})
  ELSE()
      MESSAGE(STATUS "VISIT_VTK_VERSION NOT FOUND assuming 8.1.0")
      SETUP_APP_VERSION(VTK 8.1.0)
  ENDIF()
ENDIF()


#-----------------------------------------------------------------------------
# If the config-site file above has not set a NEKTAR++ version set a
# default NEKTAR++ version of 4.0.0.
#-----------------------------------------------------------------------------
IF(EXISTS ${VISIT_NEKTAR++_DIR})
  IF(NOT NEKTAR++_VERSION)
    IF(VISIT_NEKTAR++_VERSION)
      IF(${VISIT_NEKTAR++_VERSION} VERSION_LESS ${NEKTAR++_MINIMUM_VERSION})
          MESSAGE(FATAL_ERROR "NEKTAR++ version must be at least ${NEKTAR++_MINIMUM_VERSION}")
      ENDIF()
      SETUP_APP_VERSION(NEKTAR++ ${VISIT_NEKTAR++_VERSION})
    ELSE()
      MESSAGE(STATUS "VISIT_NEKTAR++_VERSION NOT FOUND assuming 4.0.0")
      SETUP_APP_VERSION(NEKTAR++ 4.0.0)
    ENDIF()
  ENDIF()
ENDIF()


#-----------------------------------------------------------------------------
# If not already set, we use a default build type of Release
#-----------------------------------------------------------------------------

IF (NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build,
options are: Debug Release RelWithDebInfo MinSizeRel.")
ENDIF()

#-----------------------------------------------------------------------------
# Set compiler from config-site
#-----------------------------------------------------------------------------
SET(CMAKE_C_COMPILER ${VISIT_C_COMPILER})
SET(CMAKE_CXX_COMPILER ${VISIT_CXX_COMPILER})
SET(CMAKE_Fortran_COMPILER ${VISIT_FORTRAN_COMPILER})

#-----------------------------------------------------------------------------
# VisIt project. Declare the project after including the input because
#                this lets us set up defaults in the config-site.
#-----------------------------------------------------------------------------

PROJECT(VISIT)

include(${VISIT_SOURCE_DIR}/CMake/CheckMinimumCompilerVersion.cmake)


#-----------------------------------------------------------------------------
# Set extended platlform defs.
#-----------------------------------------------------------------------------
INCLUDE(${VISIT_SOURCE_DIR}/CMake/SetUpPlatformDefs.cmake)


#-----------------------------------------------------------------------------
# Setup out of source build indicator.
#-----------------------------------------------------------------------------
IF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    SET(VISIT_OUT_OF_SOURCE_BUILD 0)
ELSE("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    SET(VISIT_OUT_OF_SOURCE_BUILD 1)
ENDIF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")

#-----------------------------------------------------------------------------
# Windows needs values that are only set at this point, so now include
# the windows cmake file.
#-----------------------------------------------------------------------------
IF (WIN32)
  IF (VISIT_CONFIG_SITE)
      MESSAGE(STATUS "Trying to include ${VISIT_CONFIG_SITE}")
      INCLUDE(${VISIT_CONFIG_SITE})
  ELSEIF (EXISTS ${VISIT_SOURCE_DIR}/config-site/${HOSTNAME}.cmake)
      MESSAGE(STATUS "Trying to include config-site/${HOSTNAME}.cmake")
      INCLUDE(config-site/${HOSTNAME}.cmake)
  ELSE (VISIT_CONFIG_SITE)
      MESSAGE(STATUS "Trying to include config-site/windows.cmake")
      INCLUDE(config-site/windows.cmake)
  ENDIF (VISIT_CONFIG_SITE)
  SET(CMAKE_SKIP_RPATH ON CACHE INTERNAL "VisIt builds with rpath set." FORCE)
  IF(NOT VISIT_WINDOWS_DIR)
    MESSAGE(FATAL_ERROR "VISIT_WINDOWS_DIR is not defined.")
  ENDIF()
  IF(NOT EXISTS ${VISIT_WINDOWS_DIR})
    MESSAGE(FATAL_ERROR "${VISIT_WINDOWS_DIR} does not exist.")
  ENDIF()
ENDIF (WIN32)

#-----------------------------------------------------------------------------
# Output directories.
#-----------------------------------------------------------------------------

IF(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
  SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${VISIT_BINARY_DIR}/lib CACHE INTERNAL "Single output directory for building all libraries.")
ENDIF(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)

IF(NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
  SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${VISIT_BINARY_DIR}/lib CACHE INTERNAL "Single output directory for building all libraries.")
ENDIF(NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)

IF (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
      SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${VISIT_BINARY_DIR}/exe CACHE INTERNAL "Single output directory for building all executables.")
ENDIF(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)


SET(VISIT_LIBRARY_DIR ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR})
SET(VISIT_EXECUTABLE_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR})
SET(CXX_TEST_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

#-----------------------------------------------------------------------------
# configure options
#-----------------------------------------------------------------------------
OPTION(VISIT_PARALLEL "Build VisIt's parallel compute engine." OFF)
OPTION(VISIT_SLIVR    "Build VisIt with support for the SLIVR volume rendering library." ON)
OPTION(VISIT_OSPRAY   "Build VisIt with support for the OSPRay volume rendering library." OFF)
OPTION(VISIT_STATIC   "Build VisIt statically." OFF)
IF(VISIT_STATIC)
    ADD_DEFINITIONS(-DVISIT_STATIC)
ENDIF(VISIT_STATIC)
OPTION(VISIT_OSX_USE_RPATH      "Use rpath instead of executable_path while installing" OFF)
OPTION(VISIT_PYTHON_SCRIPTING      "Build VisIt with Python scripting support." ON)
OPTION(VISIT_PYTHON_FILTERS        "Build VisIt with Python Engine Filter support." ON)
OPTION(VISIT_DDT      "Build VisIt with support for the DDT debugger." OFF)
IF(VISIT_DDT)
  ADD_DEFINITIONS(-DHAVE_DDT)
ENDIF(VISIT_DDT)

OPTION(VISIT_BUILD_ALL_PLUGINS     "Build all of VisIt's plugins." OFF)
OPTION(VISIT_BUILD_MINIMAL_PLUGINS "Build a minimal set of VisIt's plugins." OFF)
OPTION(VISIT_ZLIB "Use VisIt's internal libz" OFF)
OPTION(VISIT_JAVA "Build the VisIt Java client interface" OFF)
OPTION(VISIT_PARADIS "Build the VisIt paraDIS client interface" ON)
OPTION(VISIT_SERVER_COMPONENTS_ONLY "Build only vcl, mdserver, engine and their plugins" OFF)
OPTION(VISIT_ENGINE_ONLY "Build only the compute engine and its plugins" OFF)
OPTION(VISIT_DBIO_ONLY "Build only visitconvert and engine plugins" OFF)
# If we're doing DBIO_ONLY then compile all sources with this flag so we don't
# ever have to do things like include visit-config.h to get it.
IF(VISIT_DBIO_ONLY)
    ADD_DEFINITIONS(-DDBIO_ONLY)
ENDIF(VISIT_DBIO_ONLY)
OPTION(VISIT_DISABLE_SELECT "Disable use of the select() function" OFF)
OPTION(VISIT_USE_NOSPIN_BCAST "Use VisIt's no-spin Bcast in parallel" ON)
OPTION(VISIT_INSTALL_THIRD_PARTY "Install VisIt's 3rd party I/O libs and includes to permit plugin development" OFF)
OPTION(VISIT_NOLINK_MPI_WITH_LIBRARIES "Do not link MPI with VisIt's parallel shared libraries; just with executables" OFF)
OPTION(VISIT_CREATE_SOCKET_RELAY_EXECUTABLE "Create a separate executable that forwards VisIt's socket connection between engine and component launcher" ON)
OPTION(VISIT_RPATH_RELATIVE_TO_EXECUTABLE_PATH "Install rpath relative to executable location using \$ORIGIN tag" OFF)
OPTION(VISIT_FORTRAN "Enable compilation of Fortran example progams" OFF)
OPTION(VISIT_DATA_MANUAL_EXAMPLES "Build Getting Data Into VisIt examples" OFF)
OPTION(VISIT_SPHINX "Enable ability to build sphinx documentation" OFF)

OPTION(IGNORE_THIRD_PARTY_LIB_PROBLEMS "Ignore problems finding requested third party libraries")
OPTION(VISIT_FORCE_SSH_TUNNELING "Force ssh tunnelling for sockets" OFF)
OPTION(VISIT_CREATE_APPBUNDLE_PACKAGE "Create DMG file with Mac App bundle with make package" OFF)
IF(WIN32)
  OPTION(VISIT_MAKE_NSIS_INSTALLER "Create an installer package using NSIS" OFF)
  OPTION(VISIT_WINDOWS_APPLICATION "Create Windows-style applications with no console" ON)
  IF(VISIT_WINDOWS_APPLICATION)
      SET(VISIT_APPLICATION_STYLE "WIN32")
  ENDIF(VISIT_WINDOWS_APPLICATION)
ENDIF(WIN32)

OPTION(VISIT_INSTALL_AS_BETA "Install as beta (symlink beta instead of current to installed version)." OFF)
OPTION(VISIT_MPICH_INSTALL "Install MPICH into VisIt binary distribution" OFF)
IF(APPLE OR WIN32)
    OPTION(VISIT_USE_X "Use X11" OFF)
ELSE(APPLE OR WIN32)
    OPTION(VISIT_USE_X "Use X11" ON)
ENDIF(APPLE OR WIN32)

OPTION(VISIT_THREAD "Build VisIt with thread support." OFF)
OPTION(VISIT_BUILD_AVTEXAMPLES  "Enable building of avtexamples" ON)
OPTION(VISIT_BUILD_DIAGNOSTICS  "Enable building of diagnostices" ON)

OPTION(VISIT_ENABLE_XDB "Enable Intelligent Light XDB database plugin." OFF) # OFF for now

SET(VISIT_INSTALL_PROFILES_TO_HOSTS "" CACHE STRING "Install specified host profiles (no action if empty)." )
SET(VISIT_DEFAULT_SILO_DRIVER "PDB" CACHE STRING "Default driver for generating silo data. Valid values are PDB, HDF5 or empty. An empty value means driver can be specified via make on the command line." )

MACRO(VISIT_3RDPARTY_VAR libvar comment)
    set(cache_type ${ARGN})
    if(NOT cache_type)
        set(cache_type "PATH")
    endif()
    IF(NOT DEFINED VISIT_${libvar})
        # Create an empty entry for this path variable
        SET(VISIT_${libvar} "" CACHE ${cache_type} "${comment}")
    ELSE(NOT DEFINED VISIT_${libvar})
        IF(DEFINED ${libvar})
            # Ensure VISIT_FOO_DIR=FOO_DIR (override from CL)
            SET(VISIT_${libvar} ${${libvar}} CACHE ${cache_type} "${comment}" FORCE)
        ELSE(DEFINED ${libvar})
            # Set FOO_DIR=VISIT_FOO_DIR (default from config-site file)
            SET(${libvar} ${VISIT_${libvar}})
        ENDIF(DEFINED ${libvar})
    ENDIF(NOT DEFINED VISIT_${libvar})
ENDMACRO(VISIT_3RDPARTY_VAR libvar comment)

MACRO(VISIT_3RDPARTY_DEP libvar comment)
    VISIT_3RDPARTY_VAR(${libvar} ${comment} "STRING")
ENDMACRO(VISIT_3RDPARTY_DEP libvar comment)

# Define options that let us pick our standard libraries: VTK,Mesa,Qt,Python
# and so on.
VISIT_3RDPARTY_VAR(VTK_DIR    "Path containing the VTK library's bin and lib")
VISIT_3RDPARTY_VAR(QT_DIR     "Path containing the Qt library's bin and lib")
VISIT_3RDPARTY_VAR(QWT_DIR    "Path containing the Qwt library's include and lib")
VISIT_3RDPARTY_VAR(PYTHON_DIR "Path containing the Python library's include and lib")
IF (NOT WIN32)
  VISIT_3RDPARTY_VAR(MESAGL_DIR   "Path containing the Mesa GL library's include and lib")
  VISIT_3RDPARTY_VAR(OSMESA_DIR   "Path containing the OSMesa library's include and lib")
ENDIF (NOT WIN32)
VISIT_3RDPARTY_VAR(ICET_DIR   "Path containing the Ice-T library's include and lib")
VISIT_3RDPARTY_VAR(TCMALLOC_DIR   "Path containing the tcmalloc library's include and lib")

# Libraries some third-parties are dependent upon
IF(WIN32)
  VISIT_3RDPARTY_VAR(JPEG_DIR   "Path containing the jpeg library's include and lib")
  VISIT_3RDPARTY_VAR(SZIP_DIR   "Path containing the szip library's include and lib")
ENDIF(WIN32)

# Define the options that let us pick I/O library installation locations. When
# the library option is defined then we also define the variable name passed as
# the 3rd argument. That variable is what is used in our various Find routines.
#
# Note: Define LIBDEP ONLY for libs a plugin is INdirectly dependent on (e.g.
#       needs to link but not to compile). Use the plugin's .xml file for
#       libs a plugin is DIRECTLY dependent on (e.g. needs to both compile
#       and link).
#
# _MPI_ variables are needed *only* for TPLs that support *both* serial and mpi
# parallel variants. Otherwise, the TPL is assumed to be either serial only or parallel
# only and not require separate varibles for both serial and MPI parallel variants.
#
VISIT_3RDPARTY_VAR(ADIOS_DIR     "Path containing the ADIOS library's include and lib")
VISIT_3RDPARTY_VAR(ADIOS2_DIR     "Path containing the ADIOS library's include and lib")
VISIT_3RDPARTY_VAR(ADIOS2_PAR_DIR "Path containing the ADIOS MPI library's include and lib")
VISIT_3RDPARTY_VAR(ADVIO_DIR     "Path containing the AdvIO library's include and lib")
VISIT_3RDPARTY_VAR(BOOST_DIR     "Path containing the BOOST library's include and lib")
VISIT_3RDPARTY_VAR(BOXLIB_DIR    "Path containing the Boxlib library's include and lib")
VISIT_3RDPARTY_VAR(CFITSIO_DIR   "Path containing the CFITSIO library's include and lib")
VISIT_3RDPARTY_VAR(CGNS_DIR      "Path containing the CGNS library's include and lib")
VISIT_3RDPARTY_DEP(CGNS_LIBDEP   "CGNS library dependencies")
VISIT_3RDPARTY_VAR(CONDUIT_DIR   "Path containing the Conduit library's include and lib")
VISIT_3RDPARTY_DEP(CONDUIT_LIBDEP "Conduit library dependencies")
VISIT_3RDPARTY_VAR(DAMARIS_DIR   "Path containing the Damaris library's include and lib")
VISIT_3RDPARTY_VAR(FASTBIT_DIR   "Path containing the FastBit library's include and lib")
VISIT_3RDPARTY_VAR(FASTQUERY_DIR   "Path containing the FastQuery library's include and lib")
VISIT_3RDPARTY_VAR(GDAL_DIR      "Path containing the GDAL library's include and lib")
VISIT_3RDPARTY_VAR(GFORTRAN_DIR  "Path containing the GFortran library's include and lib")
VISIT_3RDPARTY_VAR(HDF4_DIR      "Path containing the HDF4 library's include and lib")
VISIT_3RDPARTY_DEP(HDF4_LIBDEP   "HDF4 library dependencies")
VISIT_3RDPARTY_VAR(HDF5_DIR      "Path containing the HDF5 library's include and lib")
VISIT_3RDPARTY_DEP(HDF5_LIBDEP   "HDF5 library dependencies")
VISIT_3RDPARTY_VAR(HDF5_MPI_DIR  "Path containing the parallel HDF5 library's include and lib")
VISIT_3RDPARTY_DEP(HDF5_MPI_LIBDEP "HDF5 parallel library dependencies")
VISIT_3RDPARTY_VAR(H5PART_DIR    "Path containing the H5Part library's include and lib")
VISIT_3RDPARTY_DEP(H5PART_LIBDEP "H5Part library dependencies")
VISIT_3RDPARTY_VAR(LLVM_DIR      "Path containing the LLVM library's include and lib")
VISIT_3RDPARTY_VAR(MFEM_DIR      "Path containing the mfem library's include and lib")
VISIT_3RDPARTY_DEP(MFEM_INCDEP   "MFEM include dependencies")
VISIT_3RDPARTY_VAR(MILI_DIR      "Path containing the Mili library's include and lib")
VISIT_3RDPARTY_VAR(NEKTAR++_DIR  "Path containing the Nektar++ library's include and lib")
VISIT_3RDPARTY_DEP(NEKTAR++_LIBDEP "Nektar++ library dependencies")
VISIT_3RDPARTY_VAR(OPENGL_DIR    "Path containing the OpenGL library's include and lib")
VISIT_3RDPARTY_VAR(OPENGL_LIBRARY "Path to OpenGL lib")
VISIT_3RDPARTY_VAR(GLU_LIBRARY "Path to GLU lib")
VISIT_3RDPARTY_VAR(OPENEXR_DIR   "Path containing the OpenEXR library's include and lib")
VISIT_3RDPARTY_VAR(NETCDF_DIR    "Path containing the NETCDF library's include and lib")
VISIT_3RDPARTY_DEP(NETCDF_LIBDEP "NETCDF library dependencies")
VISIT_3RDPARTY_VAR(PYSIDE_DIR    "Path containing the PySide installation")
VISIT_3RDPARTY_VAR(SILO_DIR      "Path containing the Silo library's include and lib")
VISIT_3RDPARTY_DEP(SILO_LIBDEP   "Silo library dependencies")
VISIT_3RDPARTY_VAR(UINTAH_DIR    "Path containing the Uintah library's include and lib")
VISIT_3RDPARTY_VAR(PIDX_DIR      "Path containing the PIDX library's include and lib")
VISIT_3RDPARTY_VAR(VTKM_DIR      "Path containing the VTKm library's include and lib")
VISIT_3RDPARTY_VAR(XDMF_DIR      "Path containing the Xdmf library's include and lib")
VISIT_3RDPARTY_DEP(XDMF_LIBDEP   "Xdmf library dependencies")
VISIT_3RDPARTY_VAR(XERCESC_DIR   "Path containing the Xerces-C library's include and lib")
VISIT_3RDPARTY_VAR(XSD_DIR       "Path containing the XSD library's include and lib")
VISIT_3RDPARTY_VAR(MDSPLUS_DIR   "Path containing the MDSplus library's include and lib")
VISIT_3RDPARTY_VAR(ZLIB_DIR      "Path containing the zlib library's include and lib")
VISIT_3RDPARTY_VAR(MOAB_DIR      "Path containing the MOAB library's include and lib")
VISIT_3RDPARTY_VAR(MOAB_MPI_DIR  "Path containing the parallel MOAB library's include and lib")

VISIT_3RDPARTY_VAR(OSPRAY_DIR    "Path containing the OSPRay library's include and lib")
VISIT_3RDPARTY_VAR(EMBREE_DIR    "Path containing the embree library's include and lib")
VISIT_3RDPARTY_VAR(ISPC_DIR      "Path containing the ISPC library's include and lib")
# Note by Qi
# should we change TBB_ROOT to TBB_DIR ?
VISIT_3RDPARTY_VAR(TBB_ROOT      "Path containing the TBB library's include and lib")

#-----------------------------------------------------------------------------
# Read the version
#-----------------------------------------------------------------------------
FILE(STRINGS VERSION VERSION)
IF(NOT VERSION)
    MESSAGE(FATAL_ERROR
        "File \"VERSION\" with ascii string of VisIt version does not exist")
ENDIF(NOT VERSION)
SET(VISIT_VERSION ${VERSION})

# Want to split VISIT_VERSION into major,minor and patch vars.
# Could use SETUP_APP_VERSION, but it sets the vars as CACHE (needed for TP).
# This conflicts with the non-cache var VISIT_VERSION automagically set by
# the call to PROJECT(VISIT).  (Non-cache vars overshadow cache vars).
# There is a 'VERSION' argument to PROJECT, but it only accepts integers, thus
# it bails on things like '3.0.0b'.  So we will parse the visit version here
# instead.
string(REPLACE "." ";" v_list ${VERSION})
list(GET v_list 0 VISIT_VERSION_MAJOR)
list(GET v_list 1 VISIT_VERSION_MINOR)
list(GET v_list 2 VISIT_VERSION_PATCH)
unset(v_list)

#-----------------------------------------------------------------------------
# Set up some installation related value and macros (needs version).
#-----------------------------------------------------------------------------

IF(WIN32)
    # override cmake's default of %Program Files% for CMAKE_INSTALL_PREFIX
    # if it wasn't specified via command line,
    IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
      IF( NOT APPENDED_VISIT_VERSION_TO_INSTALL_PATH)
        SET(CMAKE_INSTALL_PREFIX "${VISIT_BINARY_DIR}/VisIt ${VISIT_VERSION}"
            CACHE PATH "Install path prefix, prepended onto install directories" FORCE)
        SET(APPENDED_VISIT_VERSION_TO_INSTALL_PATH TRUE CACHE INTERNAL "")
      ENDIF( NOT APPENDED_VISIT_VERSION_TO_INSTALL_PATH)
    ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

    # set up directories
    SET(VISIT_INSTALLED_VERSION ${CMAKE_INSTALL_PREFIX}
        CACHE PATH "VisIt install directory" FORCE)
    SET(VISIT_INSTALLED_VERSION_BIN ${CMAKE_INSTALL_PREFIX}
        CACHE PATH "VisIt bin install directory" FORCE)
    SET(VISIT_INSTALLED_VERSION_LIB ${CMAKE_INSTALL_PREFIX}/lib
        CACHE PATH "VisIt lib install directory" FORCE)
    SET(VISIT_INSTALLED_VERSION_INCLUDE ${CMAKE_INSTALL_PREFIX}/include
        CACHE PATH "VisIt include install directory" FORCE)
    SET(VISIT_INSTALLED_VERSION_PLUGINS ${CMAKE_INSTALL_PREFIX}
        CACHE PATH "VisIt plugins install directory" FORCE)
    SET(VISIT_INSTALLED_VERSION_ARCHIVES ${CMAKE_INSTALL_PREFIX}/lib
        CACHE PATH "3rd party I/O archives install directory" FORCE)
    SET(VISIT_INSTALLED_VERSION_RESOURCES ${CMAKE_INSTALL_PREFIX}/resources
        CACHE PATH "VisIt resources install directory" FORCE)
ELSE(WIN32)
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/DetermineVisItArchitecture.cmake)
    DETERMINE_VISIT_ARCHITECTURE(VISIT_INSTALL_PLATFORM)
    SET(VISIT_INSTALLED_VERSION ${VISIT_VERSION}/${VISIT_INSTALL_PLATFORM}
        CACHE PATH "VisIt install directory" FORCE)
    SET(VISIT_INSTALLED_VERSION_BIN ${VISIT_INSTALLED_VERSION}/bin
        CACHE PATH "VisIt bin install directory" FORCE)
    SET(VISIT_INSTALLED_VERSION_LIB ${VISIT_INSTALLED_VERSION}/lib
        CACHE PATH "VisIt lib install directory" FORCE)
    SET(VISIT_INSTALLED_VERSION_INCLUDE ${VISIT_INSTALLED_VERSION}/include
        CACHE PATH "VisIt include install directory" FORCE)
    SET(VISIT_INSTALLED_VERSION_PLUGINS ${VISIT_INSTALLED_VERSION}/plugins
        CACHE PATH "VisIt plugins install directory" FORCE)
    SET(VISIT_INSTALLED_VERSION_ARCHIVES ${VISIT_INSTALLED_VERSION}/archives
        CACHE PATH "3rd party I/O archives install directory" FORCE)
    SET(VISIT_INSTALLED_VERSION_RESOURCES ${VISIT_INSTALLED_VERSION}/resources
        CACHE PATH "VisIt resources install directory" FORCE)

    IF(VISIT_RPATH_RELATIVE_TO_EXECUTABLE_PATH)
        SET(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
        SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    ENDIF(VISIT_RPATH_RELATIVE_TO_EXECUTABLE_PATH)
    #
    # Note that the last line in the EXECUTE_PROCESS command below should
    # have a "\" before the ${CMAKE_INSTALL_PREFIX} to be truely correct.
    # The code creates a symbolic link from "current" to the current visit
    # version number.  Because of the coding error it succeeds when doing
    # a "make install" and fails when doing a "make package", which is
    # exactly what we want.  We do this because CPACK may create an
    # invalid tar file if the distribution contains a link to a directory.
    # This happens because CPACK removes the symbolic link and replaces all
    # the files in the directory pointed to by the symbolic link with hard
    # links.  It only stores 100 characters of path information in the hard
    # link so long paths may get truncated resulting in invalid files.  We
    # have a bunch of files with paths greater than 100 character so this
    # fails in our case.
    #
    IF(VISIT_INSTALL_AS_BETA)
        INSTALL(CODE "
          MESSAGE(\"Symlinking beta to ${VISIT_VERSION}\")
          EXECUTE_PROCESS(WORKING_DIRECTORY \${CMAKE_INSTALL_PREFIX}
            COMMAND \${CMAKE_COMMAND} -E create_symlink
            \"${VISIT_VERSION}\"
            \"${CMAKE_INSTALL_PREFIX}/beta\")
        ")
    ELSE(VISIT_INSTALL_AS_BETA)
        INSTALL(CODE "
          MESSAGE(\"Symlinking current to ${VISIT_VERSION}\")
          EXECUTE_PROCESS(WORKING_DIRECTORY \${CMAKE_INSTALL_PREFIX}
            COMMAND \${CMAKE_COMMAND} -E create_symlink
            \"${VISIT_VERSION}\"
            \"${CMAKE_INSTALL_PREFIX}/current\")
        ")
    ENDIF(VISIT_INSTALL_AS_BETA)
ENDIF(WIN32)
MARK_AS_ADVANCED(
VISIT_INSTALLED_VERSION_BIN
VISIT_INSTALLED_VERSION_LIB
VISIT_INSTALLED_VERSION_PLUGINS
VISIT_INSTALLED_VERSION_ARCHIVES
VISIT_INSTALLED_VERSION_INCLUDE
)

# experimental!!
# ctest/cdash testing support
SET(BUILD_CTEST_TESTING OFF CACHE BOOL "Enable ctest hooks into VisIt's regression suite")
IF(BUILD_CTEST_TESTING)
    # enable native cmake testing
    INCLUDE(CTest)
    CONFIGURE_FILE(
        "${CMAKE_CURRENT_SOURCE_DIR}/CTestCustom.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/CTestCustom.cmake"
        @ONLY)
    # notes:
    # * visit test sources aren't in the source tree
    # * and baseline images are currently with test sources
    # * test data is generated from a set of programs
    #   that need to be built, also not in the source tree,
    #   but configured by this cmake lists file
    SET(VISIT_TEST_DIR "${VISIT_SOURCE_DIR}/test"
        CACHE PATH "Path to VisIt's regression suite")
    SET(VISIT_BASELINE_DIR "${VISIT_SOURCE_DIR}/../test/baseline"
        CACHE PATH "Path to baseline images used by VisIt's regression suite")
    SET(VISIT_DATA_DIR "${CMAKE_BINARY_DIR}/data"
        CACHE PATH "Path to data used by VisIt's regression suite")
    # make it obvous if we do not have any of these, as
    # this will likely be a source of confusion for the
    # uninitiated
    IF ((NOT EXISTS ${VISIT_TEST_DIR}) OR (NOT EXISTS ${VISIT_BASELINE_DIR})
       OR ((NOT EXISTS ${VISIT_DATA_DIR}) AND (NOT EXISTS "${VISIT_SOURCE_DIR}/data")))
        MESSAGE(FATAL_ERROR
           "CTest based testing requires a checkout of trunk/test and trunk/data")
    ENDIF()
    # add a phony target to be sure that data for tests is
    # generated/updated during the build
    ADD_CUSTOM_TARGET(testing ALL)
    ADD_DEPENDENCIES(testing testdata)
    # configure tests
    SUBDIRS(${VISIT_TEST_DIR})
ENDIF()

MACRO(VISIT_INSTALL_TARGETS_RELATIVE dest_dir)
    IF(VISIT_STATIC)
        # Skip installation of static libraries when we build statically
        FOREACH(T ${ARGN})
            GET_TARGET_PROPERTY(pType ${T} TYPE)
            IF(NOT ${pType} STREQUAL "STATIC_LIBRARY")
                INSTALL(TARGETS ${T}
                    RUNTIME DESTINATION ${VISIT_INSTALLED_VERSION_BIN}/${dest_dir}
                    BUNDLE  DESTINATION ${VISIT_INSTALLED_VERSION_BIN}/${dest_dir}
                    LIBRARY DESTINATION ${VISIT_INSTALLED_VERSION_LIB}/${dest_dir}
                    ARCHIVE DESTINATION ${VISIT_INSTALLED_VERSION_ARCHIVES}/${dest_dir}
                    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                                GROUP_READ GROUP_WRITE GROUP_EXECUTE
                                WORLD_READ             WORLD_EXECUTE
                    CONFIGURATIONS "" None Debug Release RelWithDebInfo MinSizeRel
                )
            ENDIF(NOT ${pType} STREQUAL "STATIC_LIBRARY")
        ENDFOREACH(T)
    ELSE(VISIT_STATIC)
        INSTALL(TARGETS ${ARGN}
            RUNTIME DESTINATION ${VISIT_INSTALLED_VERSION_BIN}/${dest_dir}
            BUNDLE  DESTINATION ${VISIT_INSTALLED_VERSION_BIN}/${dest_dir}
            LIBRARY DESTINATION ${VISIT_INSTALLED_VERSION_LIB}/${dest_dir}
            ARCHIVE DESTINATION ${VISIT_INSTALLED_VERSION_ARCHIVES}/${dest_dir}
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                        GROUP_READ GROUP_WRITE GROUP_EXECUTE
                        WORLD_READ             WORLD_EXECUTE
            CONFIGURATIONS "" None Debug Release RelWithDebInfo MinSizeRel
        )
    ENDIF(VISIT_STATIC)
ENDMACRO(VISIT_INSTALL_TARGETS_RELATIVE)

MACRO(VISIT_INSTALL_TARGETS)
    VISIT_INSTALL_TARGETS_RELATIVE("" ${ARGN})
ENDMACRO(VISIT_INSTALL_TARGETS)

# Configure OpenGL support.
if(NOT VISIT_DBIO_ONLY)
  include(${VISIT_SOURCE_DIR}/CMake/VisItOpenGL.cmake)
endif()

# Install the headers for directories that we need in order to build plugins
IF(VISIT_HEADERS_SKIP_INSTALL)
    MESSAGE(STATUS "Skipping VisIt headers installation")
ELSE(VISIT_HEADERS_SKIP_INSTALL)
    INSTALL(DIRECTORY avt common engine gui launcher mdserver third_party_builtin tools/writer viewer visit_vtk visitpy vtkqt winutil
        DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}/visit
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE
                         GROUP_READ GROUP_WRITE
                         WORLD_READ
        DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                              GROUP_READ GROUP_WRITE GROUP_EXECUTE
                              WORLD_READ             WORLD_EXECUTE
        CONFIGURATIONS "" None Debug Release RelWithDebInfo MinSizeRel
        FILES_MATCHING PATTERN "*.h"
        PATTERN "*.hpp"
        PATTERN ".svn" EXCLUDE
        PATTERN "CMakeFiles" EXCLUDE
        PATTERN "icons" EXCLUDE
    )
ENDIF(VISIT_HEADERS_SKIP_INSTALL)

#-----------------------------------------------------------------------------
# Detect packages here. We could probably write macros that we can include from
# elsewhere for this.
#-----------------------------------------------------------------------------
INCLUDE(${CMAKE_ROOT}/Modules/CheckIncludeFiles.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/CMakeBackwardCompatibilityC.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/CMakeBackwardCompatibilityCXX.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/CheckTypeSize.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/CheckSymbolExists.cmake)

IF(VISIT_USE_X)
    INCLUDE(${CMAKE_ROOT}/Modules/FindX11.cmake)
    IF(NOT X11_X11_LIB)
        SET(VISIT_USE_X OFF)
        SET(HAVE_LIBX11 0)
    ELSE(NOT X11_X11_LIB)
        SET(HAVE_LIBX11 1)
    ENDIF(NOT X11_X11_LIB)
ELSE(VISIT_USE_X)
    SET(HAVE_LIBX11 0)
    SET(CMAKE_X_LIBS "" CACHE INTERNAL "X11 libs.")
    SET(CMAKE_X_CFLAGS "" CACHE INTERNAL "X11 extra flags.")
    SET(CMAKE_HAS_X 0 CACHE INTERNAL "Is X11 around.")
ENDIF(VISIT_USE_X)
INCLUDE(${CMAKE_ROOT}/Modules/TestBigEndian.cmake)

# Configure Nektar++ support.
# Do it before the other libraries like boost because many can depend on it.
INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindNektar++.cmake)

INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindVisItDamaris.cmake)

INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindVisItBoost.cmake)

IF( (VISIT_PYTHON_SCRIPTING OR VISIT_PYTHON_FILTERS) AND NOT VISIT_DBIO_ONLY)
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindVisItPython.cmake)
ENDIF((VISIT_PYTHON_SCRIPTING OR VISIT_PYTHON_FILTERS) AND NOT VISIT_DBIO_ONLY)

IF(VISIT_JAVA)
    INCLUDE(${CMAKE_ROOT}/Modules/FindJava.cmake)
    ENABLE_LANGUAGE(Java)
ENDIF(VISIT_JAVA)

CHECK_INCLUDE_FILES (fcntl.h     HAVE_FCNTL_H)
CHECK_INCLUDE_FILES (inttypes.h  HAVE_INTTYPES_H)
IF(NOT APPLE)
    CHECK_INCLUDE_FILES (malloc.h    HAVE_MALLOC_H)
ELSE(NOT APPLE)
    CHECK_INCLUDE_FILES (malloc/malloc.h    HAVE_MALLOC_H)
ENDIF(NOT APPLE)
CHECK_INCLUDE_FILES (limits.h    HAVE_LIMITS_H)
CHECK_INCLUDE_FILES (memory.h    HAVE_MEMORY_H)
CHECK_INCLUDE_FILES (stdint.h    HAVE_STDINT_H)
CHECK_INCLUDE_FILES (stdlib.h    HAVE_STDLIB_H)
CHECK_INCLUDE_FILES (strings.h   HAVE_STRINGS_H)
CHECK_INCLUDE_FILES (string.h    HAVE_STRING_H)
CHECK_INCLUDE_FILES (sys/time.h  HAVE_SYS_TIME_H)
CHECK_INCLUDE_FILES (sys/types.h HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILES (sys/stat.h  HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILES (unistd.h    HAVE_UNISTD_H)
CHECK_INCLUDE_FILES (stdbool.h   HAVE_STDBOOL_H)

# Configure VTK support.
INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindVisItVTK.cmake)

# Enable thread build of VisIt
IF(VISIT_THREAD)
    ADD_DEFINITIONS(-DVISIT_ATOMIC_USE_GCC_ATOMIC)
    ADD_DEFINITIONS(-DVISIT_THREADS)

    IF(APPLE)
        ADD_DEFINITIONS(-DTURN_OFF_THREAD_SET_AFFINITY)
    ENDIF(APPLE)

    MESSAGE(STATUS "Threading for VisIt: ON")
ELSE(VISIT_THREAD)
    MESSAGE(STATUS "Threading for VisIt: OFF")
ENDIF(VISIT_THREAD)


include(${VISIT_SOURCE_DIR}/CMake/FindOSPRay.cmake)


#-----------------------------------------------------------------------------
# If we are doing a static build or the VTK Python wrappers do not exist:
#   Disable python filters.
#-----------------------------------------------------------------------------

# If we did not find Python then turn off VISIT_PYTHON_SCRIPTING
IF(VISIT_PYTHON_SCRIPTING AND NOT PYTHONLIBS_FOUND)
    SET(VISIT_PYTHON_SCRIPTING OFF)
    MESSAGE(STATUS "Python scripting disabled because Python was not found.")
ENDIF(VISIT_PYTHON_SCRIPTING AND NOT PYTHONLIBS_FOUND)

# Disable Python scripting if we're doing a static build for engine-only or server-components-only.
IF(VISIT_PYTHON_SCRIPTING AND VISIT_STATIC)
    IF(VISIT_ENGINE_ONLY OR VISIT_SERVER_COMPONENTS_ONLY)
        SET(VISIT_PYTHON_SCRIPTING OFF)
        MESSAGE(STATUS "Python scripting disabled because an engine-only or server-components-only build was selected.")
    ENDIF(VISIT_ENGINE_ONLY OR VISIT_SERVER_COMPONENTS_ONLY)
ENDIF(VISIT_PYTHON_SCRIPTING AND VISIT_STATIC)

# Disable Python filters if we don't have Python.
IF(VISIT_PYTHON_FILTERS AND NOT PYTHONLIBS_FOUND)
    SET(VISIT_PYTHON_FILTERS OFF)
    MESSAGE(STATUS "Python Engine Filters disabled because Python was not found.")
ENDIF(VISIT_PYTHON_FILTERS AND NOT PYTHONLIBS_FOUND)

# VTK Python's wrappers are not available with a static build of VTK.
IF(VISIT_PYTHON_FILTERS AND VISIT_STATIC)
    SET(VISIT_PYTHON_FILTERS OFF)
    MESSAGE(STATUS "Python Engine Filters disabled because a static build of VisIt was selected.")
ENDIF(VISIT_PYTHON_FILTERS AND VISIT_STATIC)

# Disable python filters if the VTK Python's wrappers are not found.
IF(VISIT_PYTHON_FILTERS AND NOT VTK_PYTHON_WRAPPERS_FOUND)
    SET(VISIT_PYTHON_FILTERS OFF)
    MESSAGE(STATUS "Python Engine Filters disabled because VTK python wrappers are not installed.")
ENDIF(VISIT_PYTHON_FILTERS AND NOT VTK_PYTHON_WRAPPERS_FOUND)

# Disable python filters if 'VISIT_NOLINK_MPI_WITH_LIBRARIES' is true.
# The mpicom python module (a shared lib) must be linked to mpi.
IF(VISIT_PYTHON_FILTERS AND VISIT_NOLINK_MPI_WITH_LIBRARIES)
    SET(VISIT_PYTHON_FILTERS OFF)
    MESSAGE(STATUS "Python Engine Filters disabled because VISIT_NOLINK_MPI_WITH_LIBRARIES was selected.")
ENDIF(VISIT_PYTHON_FILTERS AND VISIT_NOLINK_MPI_WITH_LIBRARIES)

IF(VISIT_PYTHON_FILTERS AND VISIT_DBIO_ONLY)
    SET(VISIT_PYTHON_FILTERS OFF)
    MESSAGE(STATUS "Python Engine Filters disabled because VISIT_DBIO_ONLY was selected.")
ENDIF(VISIT_PYTHON_FILTERS AND VISIT_DBIO_ONLY)

# Configure Qt and Qwt support.
IF(NOT VISIT_DBIO_ONLY AND NOT VISIT_ENGINE_ONLY AND NOT VISIT_SERVER_COMPONENTS_ONLY)
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindVisItQt.cmake)
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindQwt.cmake)
ENDIF(NOT VISIT_DBIO_ONLY AND NOT VISIT_ENGINE_ONLY AND NOT VISIT_SERVER_COMPONENTS_ONLY)

# Configure PySide Support
if(VISIT_PYTHON_SCRIPTING AND PYTHONLIBS_FOUND
                          AND NOT VISIT_DBIO_ONLY
                          AND NOT VISIT_ENGINE_ONLY
                          AND NOT VISIT_SERVER_COMPONENTS_ONLY)
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindPySide.cmake)
endif()

# Include Ice-T support if we can.
IF(VISIT_PARALLEL)
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindIceT.cmake)
ENDIF(VISIT_PARALLEL)

# Include TCMALLOC support if we can.
INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindTCMALLOC.cmake)

# zlib
INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindZlib.cmake)

# szip and jpeg
IF(WIN32)
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindJPEG.cmake)
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindSzip.cmake)
ENDIF(WIN32)

# dl and large file support.
IF(CMAKE_HOST_UNIX)
  SET(DL_LIB dl)
  ADD_DEFINITIONS(-D_LARGEFILE64_SOURCE)
ENDIF(CMAKE_HOST_UNIX)

IF(NOT VISIT_BUILD_MINIMAL_PLUGINS OR VISIT_SELECTED_DATABASE_PLUGINS)
    # Configure HDF5 support.
    # Do it before the other libraries since many can depend on it.
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindHDF5.cmake)

    # Configure adios support.
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindADIOS.cmake)

    # Configure adios2 support.
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindADIOS2.cmake)

    # Configure advio support.
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindADVIO.cmake)

    # Configure Boxlib support.
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindBoxlib.cmake)

    # Configure cfitsio support.
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindCFITSIO.cmake)

    # Configure CGNS support.
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindCGNS.cmake)

    # Configure Conduit support.
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindConduit.cmake)

    # Configure FastBit support.
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindFastBit.cmake)

    # Configure FastQuery support.
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindFastQuery.cmake)

    # Configure gdal support.
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindGDAL.cmake)

    # Configure HDF4 support.
    IF(VISIT_STATIC)
      # There's a problem including HDF4 and Netcdf in the same executable due
      # to some netcdf-like symbols in libmfhdf
      MESSAGE(STATUS "HDF4 disabled for static builds due to collision with NetCDF.")
    ELSE(VISIT_STATIC)
      INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindHDF4.cmake)
    ENDIF(VISIT_STATIC)

    # Configure h5part support.
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindH5Part.cmake)

    # Configure MOAB support.
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindMOAB.cmake)

    # Configure mfem support.
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindMFEM.cmake)

    # Configure Mili support.
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindMili.cmake)

    # Configure netcdf support.
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindNetcdf.cmake)

    # Configure OpenEXR support.
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindOpenEXR.cmake)

    # Configure Silo support.
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindSilo.cmake)

    # Configure Xdmf support.
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindXdmf.cmake)

    # Configure MDSplus support.
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindMDSplus.cmake)

    # Configure Uintah support.
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindUintah.cmake)

    # Configure PIDX support.
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindPIDX.cmake)

    # Configure VTKh support.
    if(VISIT_VTKH_DIR)
        INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindVTKh.cmake)
    endif()

    # Configure GFortran support.
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindGFortran.cmake)
ENDIF()

IF (WIN32 AND VISIT_MAKE_NSIS_INSTALLER)
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindNSIS.cmake)
ENDIF (WIN32 AND VISIT_MAKE_NSIS_INSTALLER)

IF(VISIT_STATIC)
    SET(BUILD_SHARED_LIBS 0)
ENDIF(VISIT_STATIC)

IF(VISIT_FORTRAN)
    ENABLE_LANGUAGE(Fortran)
ENDIF(VISIT_FORTRAN)

FUNCTION(DETECT_MPI_SETTINGS COMP mlibs mflags mlflags mrpath)
    # Unset any variables that may have been set before by FindMPI
    UNSET(MPI_FOUND CACHE)
    UNSET(MPI_INCLUDE_PATH CACHE)
    UNSET(MPI_LIB CACHE)
    UNSET(MPI_COMPILE_FLAGS CACHE)
    UNSET(MPI_LIBRARIES CACHE)
    UNSET(MPI_LIBRARY CACHE)
    UNSET(MPI_EXTRA_LIBRARY CACHE)
    UNSET(MPI_LINK_FLAGS CACHE)
    UNSET(MPI_COMPILER CACHE)
    UNSET(MPI_RPATH CACHE)

    # Set the compiler and call FindMPI
    SET(MPI_COMPILER ${COMP})
    INCLUDE(${CMAKE_ROOT}/Modules/FindMPI.cmake)

    # Return the values
    IF(NOT MPI_FOUND)
        MESSAGE(FATAL_ERROR  "Failed to setup MPI using compiler wrapper: ${COMP}")
    ELSE(NOT MPI_FOUND)
        # Take the MPI include path and split it into -I's
        SET(MPI_INCLUDE_PATH_CONV "")
        FOREACH(I ${MPI_INCLUDE_PATH})
            SET(MPI_INCLUDE_PATH_CONV "${MPI_INCLUDE_PATH_CONV} -I${I}")
        ENDFOREACH(I)

        SET(${mlibs}   ${MPI_LIBRARIES} CACHE STRING "MPI libraries")
        SET(${mflags}  "-DPARALLEL -DMPICH_IGNORE_CXX_SEEK ${MPI_INCLUDE_PATH_CONV} ${MPI_COMPILE_FLAGS}" CACHE STRING "Parallel compiler flags")
        SET(${mlflags} "${MPI_LINK_FLAGS}" CACHE STRING "Parallel linker flags")

        #
        # Detect all mpi library paths, we need these to keep the RPATH intact
        # for installs that link to MPI.
        #
        SET(MPI_RPATH "")
        FOREACH(MLIB ${MPI_LIBRARIES})
            GET_FILENAME_COMPONENT(MLIB_PATH ${MLIB} PATH)
            # make sure it is not an implicit path - we need to skip these
            LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${MLIB_PATH}" pidx)
            IF("${pidx}" EQUAL -1)
                LIST(APPEND MPI_RPATH ${MLIB_PATH})
            ENDIF()
        ENDFOREACH(MLIB ${MPI_LIBRARIES})
        LIST(REMOVE_DUPLICATES MPI_RPATH)
        SET(${mrpath} "${MPI_RPATH}" CACHE STRING "Parallel rpath(s)")

    ENDIF(NOT MPI_FOUND)
ENDFUNCTION(DETECT_MPI_SETTINGS)

IF(VISIT_PARALLEL)
    IF(WIN32)
        SET(VPFLAGS "PARALLEL MPICH_IGNORE_CXX_SEEK MPICH_SKIP_MPICXX OMPI_SKIP_MPICXX MPI_NO_CPPBIND")
    ELSE(WIN32)
        SET(VPFLAGS "-DPARALLEL -DMPICH_IGNORE_CXX_SEEK -DMPICH_SKIP_MPICXX -DOMPI_SKIP_MPICXX -DMPI_NO_CPPBIND")
    ENDIF(WIN32)
    IF(VISIT_MPI_COMPILER)
        MESSAGE(STATUS "Setting up MPI using compiler wrapper")

        # Detect the MPI settings that C++ wants
        DETECT_MPI_SETTINGS(${VISIT_MPI_COMPILER}
            VISIT_PARALLEL_LIBS
            VISIT_PARALLEL_CFLAGS
            VISIT_PARALLEL_LINKER_FLAGS
            VISIT_PARALLEL_RPATH
        )
        SET(VISIT_PARALLEL_CXXFLAGS ${VISIT_PARALLEL_CFLAGS} CACHE STRING "Parallel CXXFLAGS")
        SET(VISIT_PARALLEL_CFLAGS "${VPFLAGS} ${VISIT_PARALLEL_CFLAGS}")
        SET(VISIT_PARALLEL_CXXFLAGS "${VPFLAGS} ${VISIT_PARALLEL_CXXFLAGS}")

        # Detect the MPI settings that Fortran wants
        IF(VISIT_FORTRAN AND VISIT_MPI_FORTRAN_COMPILER)
            DETECT_MPI_SETTINGS(${VISIT_MPI_FORTRAN_COMPILER}
                VISIT_PARALLEL_FORTRAN_LIBS
                VISIT_PARALLEL_FORTRAN_FLAGS
                VISIT_PARALLEL_FORTRAN_LINKER_FLAGS
                VISIT_PARALLEL_RPATH
            )
            SET(VISIT_PARALLEL_FORTRAN_FLAGS "${VPFLAGS} ${VISIT_PARALLEL_FORTRAN_FLAGS}")
        ENDIF(VISIT_FORTRAN AND VISIT_MPI_FORTRAN_COMPILER)

    ELSE(VISIT_MPI_COMPILER)
        IF(VISIT_MPI_LIBS OR VISIT_MPI_FORTRAN_LIBS)
            MESSAGE(STATUS "Setting up MPI using user defined flags")
            SET(VISIT_PARALLEL_LIBS          ${VISIT_MPI_LIBS}
                CACHE STRING "MPI libraries")
            SET(VISIT_PARALLEL_FORTRAN_LIBS  ${VISIT_MPI_FORTRAN_LIBS}
                CACHE STRING "MPI libraries for Fortran")

            IF(VISIT_MPI_C_FLAGS)
                SET(VISIT_PARALLEL_CFLAGS  " ${VPFLAGS} ${VISIT_MPI_C_FLAGS}"
                    CACHE STRING "Parallel CFLAGS")
            ELSE(VISIT_MPI_C_FLAGS)
                SET(VISIT_PARALLEL_CFLAGS  "${VPFLAGS}"
                    CACHE STRING "Parallel CFLAGS")
            ENDIF(VISIT_MPI_C_FLAGS)

            IF(VISIT_MPI_CXX_FLAGS)
                SET(VISIT_PARALLEL_CXXFLAGS "${VPFLAGS} ${VISIT_MPI_CXX_FLAGS}"
                    CACHE STRING "Parallel CXXFLAGS")
            ELSE(VISIT_MPI_CXX_FLAGS)
                SET(VISIT_PARALLEL_CXXFLAGS  "${VPFLAGS}"
                    CACHE STRING "Parallel CXXFLAGS")
            ENDIF(VISIT_MPI_CXX_FLAGS)

            IF(VISIT_MPI_FORTRAN_FLAGS)
                SET(VISIT_PARALLEL_FORTRAN_FLAGS "${VPFLAGS} ${VISIT_MPI_FORTRAN_FLAGS}"
                    CACHE STRING "Parallel Fortran flags")
            ELSE(VISIT_MPI_FORTRAN_FLAGS)
                SET(VISIT_PARALLEL_FORTRAN_FLAGS  "${VPFLAGS}"
                    CACHE STRING "Parallel flags for Fortran")
            ENDIF(VISIT_MPI_FORTRAN_FLAGS)

            IF(VISIT_MPI_LD_FLAGS)
                SET(VISIT_PARALLEL_LINKER_FLAGS "${VISIT_MPI_LD_FLAGS}"
                    CACHE STRING "Parallel LDFLAGS")
                SET(VISIT_PARALLEL_FORTRAN_LINKER_FLAGS "${VISIT_MPI_LD_FLAGS}"
                    CACHE STRING "Parallel LDFLAGS")
            ELSE(VISIT_MPI_LD_FLAGS)
                SET(VISIT_PARALLEL_LINKER_FLAGS ""
                    CACHE STRING "Parallel LDFLAGS")
                SET(VISIT_PARALLEL_FORTRAN_LINKER_FLAGS ""
                    CACHE STRING "Parallel LDFLAGS")
            ENDIF(VISIT_MPI_LD_FLAGS)
        ELSE(VISIT_MPI_LIBS OR VISIT_MPI_FORTRAN_LIBS)
            IF(WIN32)
              IF(NOT MPI_FOUND)
                INCLUDE(${CMAKE_ROOT}/Modules/FindMPI.cmake)
                IF(MPI_FOUND)
                  STRING(REPLACE " " ";" VPFLAGS "${VPFLAGS}")
                  SET(VISIT_PARALLEL_DEFS "${VPFLAGS}"
                      CACHE STRING "parallel defines")
                  SET(VISIT_PARALLEL_LIBS "${MPI_LIBRARY}"
                      CACHE STRING "MPI libraries")
                  SET(VISIT_PARALLEL_INCLUDE "${MPI_INCLUDE_PATH}"
                      CACHE STRING "MPI include dir")
                  IF(MPI_EXTRA_LIBRARY)
                       LIST(APPEND VISIT_PARALLEL_LIBS "${MPI_EXTRA_LIBRARY}")
                  ENDIF(MPI_EXTRA_LIBRARY)

                  IF(MPI_COMPILE_FLAGS)
                      SET(VISIT_PARALLEL_CFLAGS "${MPI_COMPILE_FLAGS}"
                          CACHE STRING "Parallel CFLAGS")
                      SET(VISIT_PARALLEL_CXXFLAGS "${MPI_COMPILE_FLAGS}"
                          CACHE STRING "Parallel CXXFLAGS")
                  ENDIF(MPI_COMPILE_FLAGS)
                  IF(MPI_LINK_FLAGS)
                      SET(VISIT_PARALLEL_LINKER_FLAGS "${MPI_LINK_FLAGS}"
                          CACHE STRING "Parallel LDFLAGS")
                  ENDIF(MPI_LINK_FLAGS)
                  find_file(HAVE_HPC_SCHEDULER "Microsoft.Hpc.Scheduler.tlb")
                ELSE(MPI_FOUND)
                  MESSAGE(FATAL_ERROR "To build parallel VisIt, you must at a "
                          "minimum define VISIT_MPI_COMPILER or VISIT_MPI_LIBS. "
                          "You may also define VISIT_MPI_LD_FLAGS, "
                          "VISIT_MPI_CXX_FLAGS." )
                ENDIF(MPI_FOUND)
              ENDIF(NOT MPI_FOUND)
            ELSE(WIN32)
                MESSAGE(FATAL_ERROR "To build parallel VisIt, you must at a "
                        "minimum define VISIT_MPI_COMPILER or VISIT_MPI_LIBS. "
                        "You may also define VISIT_MPI_LD_FLAGS, "
                        "VISIT_MPI_CXX_FLAGS." )
            ENDIF(WIN32)

        ENDIF(VISIT_MPI_LIBS OR VISIT_MPI_FORTRAN_LIBS)
        UNSET(VPFLAGS)
    ENDIF(VISIT_MPI_COMPILER)

    MESSAGE(STATUS "Parallel version of VisIt")
    MESSAGE(STATUS "    VISIT_PARALLEL_CFLAGS = ${VISIT_PARALLEL_CFLAGS}")
    MESSAGE(STATUS "    VISIT_PARALLEL_CXXFLAGS = ${VISIT_PARALLEL_CXXFLAGS}")
    MESSAGE(STATUS "    VISIT_PARALLEL_LINKER_FLAGS = ${VISIT_PARALLEL_LINKER_FLAGS}")
    MESSAGE(STATUS "    VISIT_PARALLEL_LIBS = ${VISIT_PARALLEL_LIBS}")
    MESSAGE(STATUS "    VISIT_PARALLEL_RPATH = ${VISIT_PARALLEL_RPATH}")
    MESSAGE(STATUS "    VISIT_PARALLEL_DEFS = ${VISIT_PARALLEL_DEFS}")
    MESSAGE(STATUS "    VISIT_PARALLEL_INCLUDE = ${VISIT_PARALLEL_INCLUDE}")

    IF(VISIT_FORTRAN AND VISIT_PARALLEL_FORTRAN_LIBS)
        MESSAGE(STATUS "")
        MESSAGE(STATUS "    VISIT_PARALLEL_FORTRAN_FLAGS = ${VISIT_PARALLEL_FORTRAN_FLAGS}")
        MESSAGE(STATUS "    VISIT_PARALLEL_FORTRAN_LINKER_FLAGS = ${VISIT_PARALLEL_FORTRAN_LINKER_FLAGS}")
        MESSAGE(STATUS "    VISIT_PARALLEL_FORTRAN_LIBS = ${VISIT_PARALLEL_FORTRAN_LIBS}")
    ENDIF(VISIT_FORTRAN AND VISIT_PARALLEL_FORTRAN_LIBS)
ELSE(VISIT_PARALLEL)
    MESSAGE(STATUS "Serial version of VisIt")
ENDIF(VISIT_PARALLEL)

FUNCTION(ADD_PARALLEL_EXECUTABLE target)
    ADD_EXECUTABLE(${target} ${ARGN})

    IF(UNIX)
      IF(VISIT_PARALLEL_CXXFLAGS)
        SET(PAR_COMPILE_FLAGS "")
        FOREACH(X ${VISIT_PARALLEL_CXXFLAGS})
            SET(PAR_COMPILE_FLAGS "${PAR_COMPILE_FLAGS} ${X}")
        ENDFOREACH(X)
        SET_TARGET_PROPERTIES(${target} PROPERTIES
            COMPILE_FLAGS ${PAR_COMPILE_FLAGS}
        )

        IF(VISIT_PARALLEL_LINKER_FLAGS)
            SET(PAR_LINK_FLAGS "")
            FOREACH(X ${VISIT_PARALLEL_LINKER_FLAGS})
                SET(PAR_LINK_FLAGS "${PAR_LINK_FLAGS} ${X}")
            ENDFOREACH(X)
            SET_TARGET_PROPERTIES(${target} PROPERTIES
                LINK_FLAGS ${PAR_LINK_FLAGS}
            )
        ENDIF(VISIT_PARALLEL_LINKER_FLAGS)

        IF(VISIT_PARALLEL_RPATH)
            SET(PAR_RPATHS "")
            FOREACH(X ${CMAKE_INSTALL_RPATH})
                SET(PAR_RPATHS "${PAR_RPATHS} ${X}")
            ENDFOREACH(X)
            FOREACH(X ${VISIT_PARALLEL_RPATH})
                SET(PAR_RPATHS "${PAR_RPATHS} ${X}")
            ENDFOREACH(X)
            SET_TARGET_PROPERTIES(${target} PROPERTIES
                INSTALL_RPATH ${PAR_RPATHS}
            )
        ENDIF(VISIT_PARALLEL_RPATH)
      ENDIF(VISIT_PARALLEL_CXXFLAGS)
    ELSE(UNIX)
          ADD_TARGET_INCLUDE(${target} ${VISIT_PARALLEL_INCLUDE})
          ADD_TARGET_DEFINITIONS(${target} ${VISIT_PARALLEL_DEFS})
          TARGET_LINK_LIBRARIES(${target} ${VISIT_PARALLEL_LIBS})
    ENDIF(UNIX)

    # If we're on doing this "nolink mpi" option, we rely on the
    # PARALLEL_TARGET_LINK_LIBRARIES function to actually link the
    # target with MPI.
    IF(NOT VISIT_NOLINK_MPI_WITH_LIBRARIES) # This is a new if test
        TARGET_LINK_LIBRARIES(${target} ${VISIT_EXE_LINKER_FLAGS} ${VISIT_PARALLEL_LIBS})
    ENDIF(NOT VISIT_NOLINK_MPI_WITH_LIBRARIES)
ENDFUNCTION(ADD_PARALLEL_EXECUTABLE)

FUNCTION(ADD_PARALLEL_FORTRAN_EXECUTABLE target)
    ADD_EXECUTABLE(${target} ${ARGN})
    IF(VISIT_PARALLEL_FORTRAN_FLAGS)
        SET(PAR_COMPILE_FLAGS "")
        FOREACH(X ${VISIT_PARALLEL_FORTRAN_FLAGS})
            SET(PAR_COMPILE_FLAGS "${PAR_COMPILE_FLAGS} ${X}")
        ENDFOREACH(X)
        SET_TARGET_PROPERTIES(${target} PROPERTIES
            COMPILE_FLAGS ${PAR_COMPILE_FLAGS}
        )

        IF(VISIT_PARALLEL_FORTRAN_LINKER_FLAGS)
            SET(PAR_LINK_FLAGS "")
            FOREACH (X ${VISIT_PARALLEL_FORTRAN_LINKER_FLAGS})
                SET(PAR_LINK_FLAGS "${PAR_LINK_FLAGS} ${X}")
            ENDFOREACH (X)
            SET_TARGET_PROPERTIES(${target} PROPERTIES
                LINK_FLAGS ${PAR_LINK_FLAGS}
            )
        IF(VISIT_PARALLEL_RPATH)
            SET(PAR_RPATHS "")
            FOREACH(X ${CMAKE_INSTALL_RPATH})
                LIST(APPEND PAR_RPATHS ${X})
            ENDFOREACH(X)
            FOREACH(X ${VISIT_PARALLEL_RPATH})
                LIST(APPEND PAR_RPATHS ${X})
            ENDFOREACH(X)
            SET_TARGET_PROPERTIES(${target} PROPERTIES
                INSTALL_RPATH ${PAR_RPATHS}
            )
        ENDIF(VISIT_PARALLEL_RPATH)
        ENDIF(VISIT_PARALLEL_FORTRAN_LINKER_FLAGS)
        TARGET_LINK_LIBRARIES(${target} ${VISIT_EXE_LINKER_FLAGS} ${VISIT_PARALLEL_FORTRAN_LIBS})
    ENDIF(VISIT_PARALLEL_FORTRAN_FLAGS)
ENDFUNCTION(ADD_PARALLEL_FORTRAN_EXECUTABLE)

FUNCTION(PARALLEL_EXECUTABLE_LINK_LIBRARIES target)
    IF(VISIT_NOLINK_MPI_WITH_LIBRARIES)
        TARGET_LINK_LIBRARIES(${target} ${ARGN} link_mpi_libs)
    ELSE(VISIT_NOLINK_MPI_WITH_LIBRARIES)
        IF(VISIT_STATIC)
            TARGET_LINK_LIBRARIES(${target} ${ARGN} ${VISIT_PARALLEL_LIBS})
        ELSE(VISIT_STATIC)
            TARGET_LINK_LIBRARIES(${target} ${ARGN})
        ENDIF(VISIT_STATIC)
    ENDIF(VISIT_NOLINK_MPI_WITH_LIBRARIES)
ENDFUNCTION(PARALLEL_EXECUTABLE_LINK_LIBRARIES)


# Set up easy to use includes for the common directory
SET(VISIT_COMMON_INCLUDES
    ${VISIT_BINARY_DIR}/include
    ${VISIT_SOURCE_DIR}/include
    ${VISIT_SOURCE_DIR}/common/Exceptions/Database
    ${VISIT_SOURCE_DIR}/common/Exceptions/Pipeline
    ${VISIT_SOURCE_DIR}/common/Exceptions/Plotter
    ${VISIT_SOURCE_DIR}/common/comm
    ${VISIT_SOURCE_DIR}/common/expr
    ${VISIT_SOURCE_DIR}/common/icons
    ${VISIT_SOURCE_DIR}/common/misc
    ${VISIT_SOURCE_DIR}/common/parser
    ${VISIT_SOURCE_DIR}/common/plugin
    ${VISIT_SOURCE_DIR}/common/proxybase
    ${VISIT_SOURCE_DIR}/common/state
    ${VISIT_SOURCE_DIR}/common/utility
    ${ZLIB_INCLUDE_DIR}
    )

# macros/defines needed by CMake/VisItMacros.cmake
if (NOT DEFINED VISIT_REPLACE_DEBUG_FLAGS)
  set(VISIT_REPLACE_DEBUG_FLAGS TRUE)
endif ()

# macros/defines also needed by PluginVsInstall.cmake.in
INCLUDE(${VISIT_SOURCE_DIR}/CMake/VisItMacros.cmake)

# Mac-specific function
FUNCTION(MAC_NIB_INSTALL exename)
    IF(VISIT_STATIC AND APPLE)
        FOREACH(x classes.nib info.nib keyedobjects.nib)
            FILE(COPY ${VISIT_SOURCE_DIR}/include/qt_menu.nib/${x}
                 DESTINATION ${VISIT_EXECUTABLE_DIR}/${exename}.app/Contents/Resources/qt_menu.nib/)
        ENDFOREACH(x)
    ENDIF(VISIT_STATIC AND APPLE)
ENDFUNCTION(MAC_NIB_INSTALL)

#-----------------------------------------------------------------------------
# Read the GIT version
#-----------------------------------------------------------------------------
#dummy revision, in case commands fail
SET(VISIT_GIT_VERSION "Unknown")
# Get the subversion revision number
IF(EXISTS ${VISIT_SOURCE_DIR}/GIT_VERSION)
    FILE(STRINGS ${VISIT_SOURCE_DIR}/GIT_VERSION VISIT_GIT_VERSION)
ELSE(EXISTS ${VISIT_SOURCE_DIR}/GIT_VERSION)
    IF (EXISTS ${VISIT_SOURCE_DIR}/../.git)
        # Only try and determine the version number if it looks like
        # we're in a working copy (a .git directory should be present)
#        FIND_PACKAGE(Subversion)
#        IF(Subversion_FOUND)
#            Subversion_WC_INFO(${PROJECT_SOURCE_DIR} VISIT)
#            SET(VISIT_SVN_REVISION "${VISIT_WC_REVISION}")
#        ELSE(Subversion_FOUND)
#            IF(WIN32)
#                INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindTortoiseSVN.cmake)
#                IF(TortoiseSVN_FOUND)
#                    TortoiseSVN_WC_INFO(${PROJECT_SOURCE_DIR} VISIT)
#                    SET(VISIT_SVN_REVISION "${VISIT_WC_REVISION}")
#                ENDIF(TortoiseSVN_FOUND)
#            ENDIF(WIN32)
#        ENDIF(Subversion_FOUND)
    ENDIF (EXISTS ${VISIT_SOURCE_DIR}/../.git)
ENDIF(EXISTS ${VISIT_SOURCE_DIR}/GIT_VERSION)

#-----------------------------------------------------------------------------
# Set up resource files for Windows applications.
#-----------------------------------------------------------------------------
IF (WIN32)
    IF(EXISTS ${VISIT_WINDOWS_DIR}/distribution/resources/visit.rc)
        SET(RESOURCEDIR distribution/resources)
    ELSEIF(EXISTS ${VISIT_WINDOWS_DIR}/visitresources/visit.rc)
        SET(RESOURCEDIR visitresources)
    ELSE()
        MESSAGE(FATAL_ERROR "Cannot find visit.rc in either ${VISIT_WINDOWS_DIR}/distribution/resources or ${VISIT_WINDOWS_DIR}/visitresources")
    ENDIF()

    SET(VISIT_VISIT_RESOURCE_FILE      ${VISIT_WINDOWS_DIR}/${RESOURCEDIR}/visit.rc)
    SET(VISIT_GUI_RESOURCE_FILE        ${VISIT_WINDOWS_DIR}/${RESOURCEDIR}/gui.rc)
    SET(VISIT_VIEWER_RESOURCE_FILE     ${VISIT_WINDOWS_DIR}/${RESOURCEDIR}/viewer.rc)
    SET(VISIT_MDSERVER_RESOURCE_FILE   ${VISIT_WINDOWS_DIR}/${RESOURCEDIR}/mdserver.rc)
    SET(VISIT_ENGINE_SER_RESOURCE_FILE ${VISIT_WINDOWS_DIR}/${RESOURCEDIR}/engine_ser.rc)
    SET(VISIT_ENGINE_PAR_RESOURCE_FILE ${VISIT_WINDOWS_DIR}/${RESOURCEDIR}/engine_par.rc)
    SET(VISIT_VCL_RESOURCE_FILE        ${VISIT_WINDOWS_DIR}/${RESOURCEDIR}/vcl.rc)
    SET(VISIT_CLI_RESOURCE_FILE        ${VISIT_WINDOWS_DIR}/${RESOURCEDIR}/cli.rc)
    MESSAGE(STATUS "VISIT_VISIT_RESOURCE_FILE = ${VISIT_VISIT_RESOURCE_FILE}")

    #-------------------------------------------------------------------
    # Create versionvars.rc to include version info into resource files.
    #-------------------------------------------------------------------
    STRING(REPLACE "b" "" filteredver1 "${VISIT_VERSION}")
    STRING(REPLACE "." "," filteredver2 "${filteredver1}")
    SET(VER_FILEVERSION "${filteredver2},0")
    SET(VER_PRODUCTVERSION "${filteredver2},0")
    CONFIGURE_FILE(${VISIT_WINDOWS_DIR}/distribution/resources/versionvars.rc.in
                   ${VISIT_BINARY_DIR}/include/versionvars.rc @ONLY IMMEDIATE)
ENDIF (WIN32)

#-----------------------------------------------------------------------------
# Create visit-config.h
#-----------------------------------------------------------------------------

# Set the slash characters based on the platform
IF(WIN32)
    SET(VISIT_SLASH_CHAR   "'\\\\'")
    SET(VISIT_SLASH_STRING "\"\\\\\"")
ELSE(WIN32)
    SET(VISIT_SLASH_CHAR   "'/'")
    SET(VISIT_SLASH_STRING "\"/\"")
ENDIF(WIN32)

# Check for plugin extension
IF(VISIT_STATIC)
    SET(VISIT_PLUGIN_EXTENSION   ".a")
ELSE(VISIT_STATIC)
    IF(WIN32)
        SET(VISIT_PLUGIN_EXTENSION   ".dll")
    ELSE(WIN32)
        IF(APPLE)
            SET(VISIT_PLUGIN_EXTENSION   ".dylib")
        ELSE(APPLE)
            SET(VISIT_PLUGIN_EXTENSION   ".so")
        ENDIF(APPLE)
    ENDIF(WIN32)
ENDIF(VISIT_STATIC)

# Check for type sizes, endian
CHECK_TYPE_SIZE(bool                 SIZEOF_BOOLEAN)            # ${CMAKE_SIZEOF_BOOLEAN})
CHECK_TYPE_SIZE(char                 SIZEOF_CHAR)               # ${CMAKE_SIZEOF_CHAR})
CHECK_TYPE_SIZE(double               SIZEOF_DOUBLE)             # ${CMAKE_SIZEOF_DOUBLE})
CHECK_TYPE_SIZE(float                SIZEOF_FLOAT)              # ${CMAKE_SIZEOF_FLOAT})
CHECK_TYPE_SIZE(int                  SIZEOF_INT)                # ${CMAKE_SIZEOF_INT})
CHECK_TYPE_SIZE(long                 SIZEOF_LONG)               # ${CMAKE_SIZEOF_LONG})
CHECK_TYPE_SIZE("long double"        SIZEOF_LONG_DOUBLE)        # ${CMAKE_SIZEOF_LONG_DOUBLE})
CHECK_TYPE_SIZE("long float"         SIZEOF_LONG_FLOAT)         # ${CMAKE_SIZEOF_LONG_FLOAT})
CHECK_TYPE_SIZE("long long"          SIZEOF_LONG_LONG)          # ${CMAKE_SIZEOF_LONG_LONG})
CHECK_TYPE_SIZE(short                SIZEOF_SHORT)              # ${CMAKE_SIZEOF_SHORT})
CHECK_TYPE_SIZE("unsigned char"      SIZEOF_UNSIGNED_CHAR)      # ${CMAKE_SIZEOF_UNSIGNED_CHAR})
CHECK_TYPE_SIZE("unsigned int"       SIZEOF_UNSIGNED_INT)       # ${CMAKE_SIZEOF_UNSIGNED_INT})
CHECK_TYPE_SIZE("size_t"             SIZEOF_SIZE_T)             # ${CMAKE_SIZEOF_SIZE_T})
CHECK_TYPE_SIZE("unsigned long"      SIZEOF_UNSIGNED_LONG)      # ${CMAKE_SIZEOF_UNSIGNED_LONG})
CHECK_TYPE_SIZE("unsigned long long" SIZEOF_UNSIGNED_LONG_LONG) # ${CMAKE_SIZEOF_UNSIGNED_LONG_LONG})
CHECK_TYPE_SIZE("unsigned short"     SIZEOF_UNSIGNED_SHORT)     # ${CMAKE_SIZEOF_UNSIGNED_SHORT})
CHECK_TYPE_SIZE(void*                SIZEOF_VOID_P)             # ${CMAKE_SIZEOF_VOID_P})
SET(CMAKE_REQUIRED_DEFINITIONS -D_LARGEFILE64_SOURCE)
CHECK_TYPE_SIZE("off64_t" SIZEOF_OFF64_T)
TEST_BIG_ENDIAN(WORDS_BIGENDIAN)

# Check for threads
IF(WIN32)
    SET(HAVE_THREADS  1)
ELSE(WIN32)
    SET(HAVE_THREADS  ${CMAKE_USE_PTHREADS})
ENDIF(WIN32)

# manually check for socklen_t as CHECK_SYMBOL_EXISTS
# doesn't appear to work on linux (at least)
IF (HAVE_SOCKLEN_T MATCHES "^HAVE_SOCKLEN_T$")
    MESSAGE(STATUS "Check for socklen_t")
    TRY_COMPILE(HAVE_SOCKLEN_T
        ${CMAKE_CURRENT_BINARY_DIR}
        ${VISIT_SOURCE_DIR}/CMake/TestSocklenT.c
        OUTPUT_VARIABLE SLT
    )
    IF (HAVE_SOCKLEN_T)
        SET(HAVE_SOCKLEN_T 1 CACHE INTERNAL "support for socklen_t")
        MESSAGE(STATUS "Check for socklen_t - found")
    ELSE(HAVE_SOCKLEN_T)
        SET(HAVE_SOCKLEN_T 0 CACHE INTERNAL "support for socklen_t")
        MESSAGE(STATUS "Check for socklen_t - not found")
    ENDIF (HAVE_SOCKLEN_T)
ENDIF ()

# Check for setbuf being a public member in fstream. We could test via
# TRY_COMPILE in CMake 2.6.2. I think this is only false on the Suns, which
# we kind of stopped supporting anyway.
SET(NO_SETBUF 1)

# Check for PTY support
IF(NOT WIN32)
    SET(VISIT_USE_PTY 1)
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/TestPTY.cmake)
ENDIF(NOT WIN32)

FUNCTION(CHECK_ISFINITE_EXISTS VAR)
  IF(${VAR} MATCHES "^${VAR}$")
    MESSAGE(STATUS "Check for isfinite")
    TRY_COMPILE(tmpVar
            ${CMAKE_CURRENT_BINARY_DIR}
            ${VISIT_SOURCE_DIR}/CMake/Testisfinite.cpp
            CMAKE_FLAGS -DLINK_LIBRARIES:STRING=m
            OUTPUT_VARIABLE outvar)
    IF(tmpVar)
        SET(${VAR} 1 CACHE INTERNAL "isfinite found")
        MESSAGE(STATUS "Check for isfinite - found")
    ELSE(tmpVar)
        SET(${VAR} 0 CACHE INTERNAL "isfinite found")
        MESSAGE(STATUS "Check for isfinite - not found")
        IF (NOT WIN32)
            MESSAGE(STATUS "Check for isfinite - ${outvar}")
        ENDIF (NOT WIN32)
    ENDIF(tmpVar)
  ENDIF()
ENDFUNCTION(CHECK_ISFINITE_EXISTS)

FUNCTION(CHECK_STRTOF_PROTOTYPE VAR)
  IF(${VAR} MATCHES "^${VAR}$")
    MESSAGE(STATUS "Check for strtof prototype")
    TRY_COMPILE(tmpVar
            ${CMAKE_CURRENT_BINARY_DIR}
            ${VISIT_SOURCE_DIR}/CMake/Teststrtofprototype.cpp
            OUTPUT_VARIABLE outvar)
    IF(tmpVar)
        SET(${VAR} 1 CACHE INTERNAL "strtof prototype")
        MESSAGE(STATUS "Check for strtof prototype - found")
    ELSE(tmpVar)
        SET(${VAR} 0 CACHE INTERNAL "strtof prototype")
        MESSAGE(STATUS "Check for strtof prototype - not found")
    ENDIF(tmpVar)
  ENDIF()
ENDFUNCTION(CHECK_STRTOF_PROTOTYPE)

FUNCTION(CHECK_FTIME_PROTOTYPE VAR)
  IF(${VAR} MATCHES "^${VAR}$")
    MESSAGE(STATUS "Check for ftime prototype")
    TRY_COMPILE(tmpVar
            ${CMAKE_CURRENT_BINARY_DIR}
            ${VISIT_SOURCE_DIR}/CMake/Testftimeprototype.cpp
            OUTPUT_VARIABLE outvar)
    IF(tmpVar)
        SET(${VAR} 1 CACHE INTERNAL "ftime prototype")
        MESSAGE(STATUS "Check for ftime prototype - found")
    ELSE(tmpVar)
        SET(${VAR} 0 CACHE INTERNAL "ftime prototype")
        MESSAGE(STATUS "Check for ftime prototype - not found")
    ENDIF(tmpVar)
  ENDIF()
ENDFUNCTION(CHECK_FTIME_PROTOTYPE)

# Check for some functions
CHECK_FUNCTION_EXISTS(memmove HAVE_MEMMOVE)
CHECK_FUNCTION_EXISTS(div HAVE_DIV)
CHECK_FUNCTION_EXISTS(setenv HAVE_SETENV)
CHECK_FUNCTION_EXISTS(strtof HAVE_STRTOF)
CHECK_FUNCTION_EXISTS(aligned_alloc HAVE_ALIGNED_ALLOC)
CHECK_STRTOF_PROTOTYPE(HAVE_STRTOF_PROTOTYPE)
CHECK_FTIME_PROTOTYPE(HAVE_FTIME_PROTOTYPE)
CHECK_ISFINITE_EXISTS(HAVE_ISFINITE)

# If we're not disabling the select() call then check for it.
IF(NOT VISIT_DISABLE_SELECT)
    IF(WIN32)
        SET(HAVE_SELECT TRUE)
    ELSE(WIN32)
        CHECK_FUNCTION_EXISTS(select HAVE_SELECT)
    ENDIF(WIN32)
ENDIF(NOT VISIT_DISABLE_SELECT)



# Set the rendering size limit to 16384 so that we are not unnecessarily
# constraining the user. There is no way to set this properly since this
# is used in the viewer and the limit really comes from the engine, which
# may have a different size if running client/server. This setting should
# be removed and a runtime check should be added to the engine.
MESSAGE(STATUS "Setting VISIT_RENDERING_SIZE_LIMIT to 16384")
SET(VISIT_RENDERING_SIZE_LIMIT 16384 CACHE INTERNAL "rendering size limit")

CONFIGURE_FILE(${VISIT_SOURCE_DIR}/include/visit-cmake.h.in
               ${VISIT_BINARY_DIR}/include/visit-config.h @ONLY IMMEDIATE)

CONFIGURE_FILE(${VISIT_SOURCE_DIR}/include/visit-python-cmake.h.in
               ${VISIT_BINARY_DIR}/include/visit-python-config.h @ONLY IMMEDIATE)


#-----------------------------------------------------------------------------
# Create common/misc/ConfigureInfo.C
#-----------------------------------------------------------------------------

# Store some configure information into a C file (like version, SVN version
# and platform things that change).
CONFIGURE_FILE(${VISIT_SOURCE_DIR}/common/misc/ConfigureInfo.C.in
               ${VISIT_BINARY_DIR}/common/misc/ConfigureInfo.C @ONLY IMMEDIATE)

SET(VISIT_INCLUDE_DIR ${VISIT_SOURCE_DIR}  CACHE INTERNAL "Path to VisIt's includes")

# The normal plugin case. We redefine these when we build against an installed
# version of VisIt
IF(NOT WIN32)
    SET(VISIT_PLUGIN_DIR  ${VISIT_BINARY_DIR}/plugins CACHE INTERNAL "Path to VisIt's plugins")
ELSE(NOT WIN32)
    SET(VISIT_PLUGIN_DIR  ${VISIT_EXECUTABLE_DIR} CACHE INTERNAL "Path to VisIt's plugins")
    if(VISIT_NO_ALLBUILD)
        add_custom_target(visit_special_builds ALL)
    endif()
ENDIF(NOT WIN32)

#-----------------------------------------------------------------------------
# If we're installing MPICH into our VisIt distribution then let's do that by
# adding some install commands.
#-----------------------------------------------------------------------------
IF(VISIT_MPICH_INSTALL)
    INSTALL(DIRECTORY ${VISIT_MPICH_DIR}/include
        DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}/mpich
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE
                         GROUP_READ GROUP_WRITE
                         WORLD_READ
        DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                              GROUP_READ GROUP_WRITE GROUP_EXECUTE
                              WORLD_READ             WORLD_EXECUTE
        CONFIGURATIONS "" None Debug Release RelWithDebInfo MinSizeRel
    )
    INSTALL(DIRECTORY ${VISIT_MPICH_DIR}/bin
        DESTINATION ${VISIT_INSTALLED_VERSION}
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                         GROUP_READ GROUP_WRITE GROUP_EXECUTE
                         WORLD_READ
        DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                              GROUP_READ GROUP_WRITE GROUP_EXECUTE
                              WORLD_READ             WORLD_EXECUTE
        CONFIGURATIONS "" None Debug Release RelWithDebInfo MinSizeRel
    )
    INSTALL(DIRECTORY ${VISIT_MPICH_DIR}/lib
        DESTINATION ${VISIT_INSTALLED_VERSION}
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                         GROUP_READ GROUP_WRITE GROUP_EXECUTE
                         WORLD_READ
        DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                              GROUP_READ GROUP_WRITE GROUP_EXECUTE
                              WORLD_READ             WORLD_EXECUTE
        CONFIGURATIONS "" None Debug Release RelWithDebInfo MinSizeRel
    )
ENDIF(VISIT_MPICH_INSTALL)

#-----------------------------------------------------------------------------
# Set flags & options from config-site files after everything is set up since
# setting them beforehand can upset some of CMake's own find routines. Use
# quotes to unite the strings or CMake will create a list.
#-----------------------------------------------------------------------------
SET(CMAKE_VERBOSE_MAKEFILE ${VISIT_VERBOSE_MAKEFILE})
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${VISIT_C_FLAGS}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${VISIT_CXX_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${VISIT_EXE_LINKER_FLAGS}")

#-----------------------------------------------------------------------------
# Top-level subdirectories
#-----------------------------------------------------------------------------

FUNCTION(CREATE_PLUGIN_DEPENDENCIES target comp type)
    FOREACH(P ${ARGN})
        # Like E + Pseudocolor + Plot_ser
        SET(deptarget "${comp}${P}${type}")
        SET(dependencies ${dependencies} ${deptarget})
    ENDFOREACH(P)
    # Construct a variable that contains the names of the dependencies so we
    # can use that later when we link the target's main exe.
    SET(cachevar "${target}_${comp}${type}")
    SET(${cachevar} ${dependencies} CACHE INTERNAL "dependencies for ${target}")
    #MESSAGE("${cachevar} = ${${cachevar}}")
ENDFUNCTION(CREATE_PLUGIN_DEPENDENCIES)

ADD_SUBDIRECTORY(third_party_builtin)
ADD_SUBDIRECTORY(common)
ADD_SUBDIRECTORY(visit_vtk)
ADD_SUBDIRECTORY(avt)

IF(NOT VISIT_DBIO_ONLY)
    ADD_SUBDIRECTORY(sim)
    ADD_SUBDIRECTORY(viewer)   # sim depends on viewercore
    ADD_SUBDIRECTORY(engine)   # sim depends on engine

    IF(NOT VISIT_ENGINE_ONLY)
        ADD_SUBDIRECTORY(launcher)
        ADD_SUBDIRECTORY(mdserver)
    ENDIF(NOT VISIT_ENGINE_ONLY)
ENDIF(NOT VISIT_DBIO_ONLY)

IF(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)
    ADD_SUBDIRECTORY(vtkqt)
    ADD_SUBDIRECTORY(winutil)
    ADD_SUBDIRECTORY(gui)
    IF(VISIT_PYTHON_SCRIPTING AND PYTHONLIBS_FOUND)
        IF(NOT VISIT_STATIC)
            ADD_SUBDIRECTORY(visitpy)
        ENDIF(NOT VISIT_STATIC)
    ENDIF(VISIT_PYTHON_SCRIPTING AND PYTHONLIBS_FOUND)
    IF(VISIT_JAVA)
        ADD_SUBDIRECTORY(java)
    ENDIF(VISIT_JAVA)
ENDIF(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)


MACRO(ADD_TARGET_XXX_FLAGS target XXX)
    GET_TARGET_PROPERTY(currentFlags ${target} ${XXX})
    IF(NOT ${currentFlags} MATCHES NOTFOUND)
        FOREACH(flag ${currentFlags})
            SET(allFlags "${allFlags} ${flag}")
        ENDFOREACH(flag)
    ENDIF(NOT ${currentFlags} MATCHES NOTFOUND)
    FOREACH(flag ${ARGN})
        SET(allFlags "${allFlags} ${flag}")
    ENDFOREACH(flag)
    SET_TARGET_PROPERTIES(${target} PROPERTIES
        ${XXX} "${allFlags}"
    )
ENDMACRO(ADD_TARGET_XXX_FLAGS)

FUNCTION(ADD_TARGET_COMPILE_FLAGS target)
    ADD_TARGET_XXX_FLAGS(${target} COMPILE_FLAGS ${ARGN})
ENDFUNCTION(ADD_TARGET_COMPILE_FLAGS target)

FUNCTION(ADD_TARGET_LINK_FLAGS target)
    ADD_TARGET_XXX_FLAGS(${target} LINK_FLAGS ${ARGN})
ENDFUNCTION(ADD_TARGET_LINK_FLAGS target)

macro(VISIT_SELECTED_PLUGIN_ERROR type plist msg)
  if(DEFINED VISIT_SELECTED_${type}_PLUGINS)
    foreach(plug ${plist})
      list(FIND VISIT_SELECTED_${type}_PLUGINS ${plug} foundPlug)
      if(NOT foundPlug EQUAL -1)
        message(FATAL_ERROR "Cannot build selected plugin (${plug}): ${msg}")
      endif()
    endforeach()
  endif()
endmacro()

IF(NOT VISIT_DBIO_ONLY)
   ADD_SUBDIRECTORY(plots)
   ADD_SUBDIRECTORY(operators)
ENDIF(NOT VISIT_DBIO_ONLY)
ADD_SUBDIRECTORY(databases)

# Must come after plugins in case we're doing a static build
ADD_SUBDIRECTORY(tools)

# Now that plugins have been added, do the mains last if we're creating static
# binaries. This is needed to link the plugins into the exe.
IF(VISIT_STATIC)
    ADD_SUBDIRECTORY(engine/main)
    ADD_SUBDIRECTORY(sim/V2/static)

    # Make it possible to build some sim examples since we've created the
    # static sim library.
    IF(NOT WIN32)
        IF(VISIT_ENGINE_ONLY OR VISIT_SERVER_COMPONENTS_ONLY)
            ADD_SUBDIRECTORY(tools/DataManualExamples/Simulations)
        ENDIF(VISIT_ENGINE_ONLY OR VISIT_SERVER_COMPONENTS_ONLY)
    ENDIF(NOT WIN32)

    IF(NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)
        ADD_SUBDIRECTORY(mdserver/main)
    ENDIF(NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)

    IF(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)
        ADD_SUBDIRECTORY(gui/main)
        ADD_SUBDIRECTORY(viewer/main)
        IF(VISIT_PYTHON_SCRIPTING AND PYTHONLIBS_FOUND)
            ADD_SUBDIRECTORY(visitpy)
        ENDIF(VISIT_PYTHON_SCRIPTING AND PYTHONLIBS_FOUND)
    ENDIF(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)
ENDIF(VISIT_STATIC)

ADD_SUBDIRECTORY(bin)
ADD_SUBDIRECTORY(lib)
ADD_SUBDIRECTORY(include)
ADD_SUBDIRECTORY(resources)

#-----------------------------------------------------------------------------
# Add the "data" directory on WINDOWS if it exists
#-----------------------------------------------------------------------------
if(IS_DIRECTORY ${VISIT_SOURCE_DIR}/../data)
    # Don't want the data dir files to be added to the main VISIT
    # solution file, so 'EXCLUDE_FROM_ALL' here, and create a new
    # PROJECT in data/CMakeLists.txt
    add_subdirectory(${VISIT_SOURCE_DIR}/../data
                     ${CMAKE_CURRENT_BINARY_DIR}/testdata
                     EXCLUDE_FROM_ALL)
endif()


#-----------------------------------------------------------------------------
# Add the "test" directory
#-----------------------------------------------------------------------------
# I'm trying to make the new tetsing work hramoniously with the old testing
# while it's evaluated. if the new testing were ever made permanent then
# this code would be removed.
if(NOT VISIT_ENGINE_ONLY AND NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT BUILD_CTEST_TESTING)
      add_subdirectory(test)
endif()

IF(VISIT_SPHINX)
    ADD_SUBDIRECTORY(doc)
ENDIF()

#-----------------------------------------------------------------------------
# CPack -- This leverages our install targets to provide a "make package" that
#          packages up a VisIt distribution.
#-----------------------------------------------------------------------------

CONFIGURE_FILE(${VISIT_SOURCE_DIR}/tools/dev/scripts/Info.plist.in
               ${VISIT_BINARY_DIR}/tools/dev/scripts/Info.plist @ONLY IMMEDIATE)

# temporarily turn this off on windows
IF (NOT WIN32)
    IF (APPLE AND VISIT_CREATE_APPBUNDLE_PACKAGE)
        SET (CPACK_GENERATOR "Bundle")

        SET (CPACK_PACKAGE_FILE_NAME "VisIt-${VISIT_VERSION}")
        SET (CPACK_BUNDLE_NAME "VisIt")
        SET (CPACK_BUNDLE_ICON "${VISIT_SOURCE_DIR}/common/icons/VisItIcon.icns")
        SET (CPACK_BUNDLE_PLIST "${VISIT_BINARY_DIR}/tools/dev/scripts/Info.plist")
        SET (CPACK_BUNDLE_STARTUP_COMMAND "${VISIT_SOURCE_DIR}/tools/dev/scripts/LaunchVisItFromAppBundle.sh")
    ELSE(APPLE AND VISIT_CREATE_APPBUNDLE_PACKAGE)
        SET(CPACK_GENERATOR "TGZ")

        SET(CPACK_PACKAGE_NAME "visit")
        SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "VisIt ${VISIT_VERSION} is a parallel visualization and data analysis tool")
        SET(CPACK_PACKAGE_VENDOR "LLNS")
        SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/resources/help/en_US/copyright.html")

        # We should probably parse ${VISIT_VERSION} into its component parts instead.
        SET(CPACK_PACKAGE_VERSION_MAJOR ${VISIT_VERSION_MAJOR})
        SET(CPACK_PACKAGE_VERSION_MINOR ${VISIT_VERSION_MINOR})
        SET(CPACK_PACKAGE_VERSION_PATCH ${VISIT_VERSION_PATCH})

        SET(UL_VERSION "${VISIT_VERSION_MAJOR}_${VISIT_VERSION_MINOR}_${VISIT_VERSION_PATCH}")
        SET(CPACK_PACKAGE_FILE_NAME "visit${UL_VERSION}.${VISIT_INSTALL_PLATFORM}")

        IF(NOT APPLE)
            SET(CPACK_STRIP_FILES TRUE)
        ENDIF(NOT APPLE)
    ENDIF(APPLE AND VISIT_CREATE_APPBUNDLE_PACKAGE)
    INCLUDE(CPack)
ELSE (NOT WIN32)
    IF(VISIT_MAKE_NSIS_INSTALLER)
      IF(${MAKENSIS_FOUND})
        set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ".")
        include("InstallRequiredSystemLibraries")
        # create a custom target that will run our nsis scripts.
        # Make sure it won't run unless specifically called.
        #
        FILE(TO_NATIVE_PATH ${VISIT_SOURCE_DIR} VSD_NATIVE)
        FILE(TO_NATIVE_PATH ${VISIT_WINDOWS_DIR} VWD_NATIVE)
        FILE(TO_NATIVE_PATH ${CMAKE_INSTALL_PREFIX} CIP_NATIVE)
        FILE(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR} CBD_NATIVE)
        ADD_CUSTOM_COMMAND(OUTPUT visit${VISIT_VERSION}.exe
            COMMAND ${MAKENSIS}
                    /DVISIT_SOURCE_DIR=${VSD_NATIVE}
                    /DVISIT_WINDOWS_DIR=${VWD_NATIVE}
                    /DINSTALL_PREFIX=${CIP_NATIVE}
                    /DBIN_DIR=${CBD_NATIVE}
                    ${VISIT_WINDOWS_DIR}/distribution/installation/CreateDBSections.nsi
            COMMAND ${MAKENSIS}
                    /DVisItVersion=${VISIT_VERSION}
                    /DCOMPILER=${VISIT_MSVC_VERSION}
                    /DVISIT_SOURCE_DIR=${VSD_NATIVE}
                    /DVISIT_WINDOWS_DIR=${VWD_NATIVE}
                    /DINSTALL_PREFIX=${CIP_NATIVE}
                    /DBIN_DIR=${CBD_NATIVE}
                    /DMPI_VERSION=${VISIT_MSMPI_VERSION}
                    ${VISIT_WINDOWS_DIR}/distribution/installation/binaryinstallation.nsi
            DEPENDS ${VISIT_WINDOWS_DIR}/distribution/installation/CreateDBSections.nsi
                    ${VISIT_WINDOWS_DIR}/distribution/installation/binaryinstallation.nsi
        )
        ADD_CUSTOM_TARGET(_PACKAGE ALL DEPENDS visit${VISIT_VERSION}.exe)
        ADD_CUSTOM_COMMAND(TARGET _PACKAGE PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -DBUILD_TYPE=${CMAKE_CFG_INTDIR} -P cmake_install.cmake)
        SET_TARGET_PROPERTIES(_PACKAGE PROPERTIES
                              EXCLUDE_FROM_DEFAULT_BUILD 1
                              EXCLUDE_FROM_ALL 1 )

        # source installer
        ADD_CUSTOM_COMMAND(OUTPUT visitdev${VISIT_VERSION}.exe
            COMMAND ${MAKENSIS}
                    /DVisItVersion=${VISIT_VERSION}
                    /DVISIT_SOURCE_DIR=${VSD_NATIVE}
                    /DVISIT_WINDOWS_DIR=${VWD_NATIVE}
                    /DINSTALLER_LOCATION=${VISIT_BINARY_DIR}
                     ${VISIT_WINDOWS_DIR}/distribution/installation/sourceinstallation.nsi
            DEPENDS ${VISIT_WINDOWS_DIR}/distribution/installation/sourceinstallation.nsi
        )
        ADD_CUSTOM_TARGET(_SRC_PACKAGE ALL DEPENDS visitdev${VISIT_VERSION}.exe)
        SET_TARGET_PROPERTIES(_SRC_PACKAGE PROPERTIES
                              EXCLUDE_FROM_DEFAULT_BUILD 1
                              EXCLUDE_FROM_ALL 1)
        UNSET(VSD_NATIVE)
        UNSET(CIP_NATIVE)
        UNSET(CBD_NATIVE)
      ELSE(${MAKENSIS_FOUND})
        MESSAGE(FATAL_ERROR "NSIS installer package requested, but 'makensis' could not be found.")
      ENDIF(${MAKENSIS_FOUND})
    ENDIF(VISIT_MAKE_NSIS_INSTALLER)
ENDIF (NOT WIN32)

add_custom_target(manuals
                  COMMAND ${VISIT_PYTHON3_DIR}/bin/sphinx-build -b html ${VISIT_SOURCE_DIR}/doc ${VISIT_SOURCE_DIR}/resources/help/en_US/manual -a)

#CMake add subdirectory forces this action to go last
#todo: find way for this function to execute as last
#step of make install process
#ADD_CUSTOM_TARGET(osxfixup ALL
#     COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/osxfixup/osxfixup.py \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${VISIT_INSTALLED_VERSION} @executable_path/.. 3)
#ADD_DEPENDENCIES(osxfixup install)

#todo: replace add_subdirectory with the lines above
SET(FIXUP_OSX TRUE CACHE BOOL "Whether to fix up osx.")
IF (APPLE AND FIXUP_OSX)
    ADD_SUBDIRECTORY(osxfixup)
ENDIF ()

IF (NOT WIN32)
    MESSAGE(STATUS "\n\nUse recmake_visit.sh or search for `CMAKE_INVOKE' in CMakeCache.txt to re-run CMake with the same arguments\n\n")
ELSE (NOT WIN32)
    MESSAGE(STATUS "\n\nSearch for `CMAKE_INVOKE' in CMakeCache.txt to re-run CMake with the same arguments\n\n")
ENDIF (NOT WIN32)

if(VISIT_INSTALL_THIRD_PARTY)
    include(${VISIT_SOURCE_DIR}/CMake/PluginVsInstallHelpers.cmake)
endif()
