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
#    Kathleen Biagas, Wed Dec 18 08:51:04 PST 2019
#    Remove VISIT_ZLIB as we no longer have an internal (third_party_builtin)
#    zlib.
#
#    Kathleen Biagas, Thu Feb 20 14:45:33 MST 2020
#    Fix sphinx docs build for Windows: sphinx-build executable is in a
#    different directory, don't build them in-source. Add install command.
#
#    Kathleen Biagas, Thu Feb 27 2020
#    Add a few CMake vars to control optional targets and reduce the number of
#    build targets when turned off (all on by default):
#        VISIT_ENABLE_ANNOTATION_TOOLS
#        VISIT_ENABLE_DATAGEN
#        VISIT_ENABLE_LIBSIM
#        VISIT_ENABLE_MANUALS
#        VISIT_ENABLE_SILO_TOOLS
#    Rename a few CMake vars for consistency in naming:
#      VISIT_DATA_MANUAL_EXAMPLES => VISIT_ENABLE_DATA_MANUAL_EXAMPLES
#      VISIT_BUILD_DIAGNOSTICS    => VISIT_ENABLE_DIAGNOSTICS
#      VISIT_BUILD_AVTEXAMPLES    => VISIT_ENABLE_EXAMPLES
#
#    Kathleen Biagas, Thu Feb 20 14:45:33 MST 2020
#    Having trouble with sphinx-build exe (Win10), so use script instead.
#
#    Brad Whitlock, Wed Jul 22 16:15:59 PDT 2020
#    I added support for the FMS library.
#
#    Alister Maguire, Thu Aug 13 13:41:20 PDT 2020
#    Updated the manuals target so that it builds in the binary directory.
#
#    Kathleen Biagas, Thu Oct 1, 2020
#    Add retrieval of git revision hash.
#
#    Eddie Rusu, Wed Oct 14 15:52:13 PDT 2020
#    Removed FastBit and FastQuery
#
#    Kevin Griffin, Fri Oct 16 17:13:32 PDT 2020
#    For Apple, copied manuals directory to the install directory so it can
#    be found by the packaging target.
#
#    Kathleen Biagas, Wed Oct 21, 2020
#    Removed VISIT_SPHINX, moved Python-3 doc generation logic into doc
#    subdir's CMakeLists.txt
#
#    Kathleen Biagas, Mon Oct 26, 2020
#    Add VISIT_ENABLE_UNIT_TESTS option (on by default).
#
#    Kathleen Biagas, Thu Feb 18, 2021
#    Added logic to support codesigning Windows releases.
#
#    Kathleen Biagas, Thu Apr 22, 2021
#    Removed addition of tools/DataManualExamples/Simulations for static
#    builds, the directory structure has changed and the examples are now
#    in 'tools/data'.  'tools' and 'tools/data' are added unconditionally.
#    'DataManualExamples' is dependent on VISIT_ENABLE_DATA_MANUAL_EXAMPLES
#    being ON.
#
#    Eric Brugger, Wed May  5 08:31:16 PDT 2021
#    Set VISIT_GIT_VERSION from the file GIT_VERSION if we are not in a
#    git checkout.
#
#    Kathleen Biagas, Tue Feb 1, 2022
#    Change Windows _SRC_PACKAGE to _WINDEV_PACKAGE, to simply zip up the
#    needed windows TP development files.
#
#    Kathleen Biagas, Wed Jun 8, 2022
#    Moved setting of VISIT_COMMON_INCLUDES to CMake/VisItIncludeVars.cmake
#    so that it could also be used by PluginVsInstall.
#
#    Eric Brugger, Fri Jun 10 09:53:54 PDT 2022
#    Added logic to include the libicui18n, libicudata and libicuuc
#    libraries used by Qt to the lib directory of a distribution. Added
#    logic to include the libstdc++ libraries to the lib directory of a
#    distribution. The libstdc++ library depends on VISIT_CXX_LIBRARY
#    being defined. This would typically be added to the config site file.
#
#    Kathleen Biagas, Thu Jun 16, 2022
#    Changed minimum cmake version to 3.15.
#
#    Kathleen Biagas, Fri July 1, 2022
#    For Windows package creation: added custom command to 7zip the install
#    dir for use by installer creation script. Changed default install
#    prefix to remove the space between 'VisIt' and '<verison>'.
#
#    Eric Brugger, Tue Jul  5 09:10:18 PDT 2022
#    Added support for the Blosc library.
#
#    Eric Brugger, Thu Jul  7 13:07:51 PDT 2022
#    Added other directories to look for libicui18n, libicudata and libicuuc.
#
#    Kathleen Biagas, Sat July 16, 2022
#    Prep for VTK 9: reorder calling of Find module so that libraries that
#    VTK depends on are found first.
#    Consolidated some python scripting logic.
#    Disable pyside temporarily.
#
#    Kathleen Biagas, Mon Sep 26, 2022
#    Remove HDF4 support.
#
#    Cyrus Harrison, Fri Nov  4 13:38:24 PDT 2022
#    Wire up BLT CMake Build System Helpers.
#
#    Kathleen Biagas, Mon Nov 7, 2022
#    Update CMake minimum version to 3.23. Part of the CMake modernization
#    effort.
#
#    Kathleen Biagas, Mon Nov 7, 2022
#    Move include checks, symbol checks, etc to SystemChecks.cmake.
#
#    Kathleen Biagas, Tue Nov 29, 2022
#    Move FindXXX calls to SetUpThirdParty.cmake. Add include for that file.
#
#    Kathleen Biagas, Thu Dec 1, 2022
#    Move CREATE_PLUGIN_DEPENDENCIES and VISIT_SELECTED_PLUGIN_ERROR into
#    PluginMacros.cmake.
#
#    Kathleen Biagas, Thu Dec 1, 2022
#    Remove VISIT_OUT_OF_SOURCE_BUILD var. Out-of-source builds
#    are now required, and checked by BLT.
#    Moved parallel settings/functions into VisItParallel.cmake.
#
#    Kathleen Biagas, Tue Dec 6, 2022
#    Move some visit-defined functions into VisItFunctions.cmake.
#    Consolidated logic setting VISIT_3RDPARTY_VAR.
#
#    Kathleen Biagas, Wed Dec 7, 2022
#    Moved OPTIONS into VisItOptions.cmake.
#
#    Kathleen Biagas, Tue Jan 31, 2023
#    Moved Windows installation, resources code into VisItWindowsFunctions.
#    Moved MPICH install and linux system libs install to VisItFunctions.
#    Remove ADD_TARGET_XXX_FLAGS and COMPILE, LINK, not used.
#    Simplify config-site file inclusion logic and messaging.
#    Rename Windows visit_special_builds to visit_util_builds, add function
#    for easier use of this uber-target.
#
#****************************************************************************

cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0033 OLD)
    if(WIN32)
        # don't automatically link with qtmain
        cmake_policy(SET CMP0020 OLD)
        if(POLICY CMP0091) # introduced in cmake 3.15
            # Tell CMake to always use /MD, even for debug builds.
            # Done because prebuilt TP libraries are built with /MD and
            # if debug build of VisIt uses /MDd there are runtime issues.
            cmake_policy(SET CMP0091 NEW) # MSVC_RUNTIME_LIBRARY
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
        endif()
    endif()
endif()


#
# Try to capture the initial set of cmake command line args passed by
# the user for configuration.
# Recipe taken from http://stackoverflow.com/questions/10205986/how-to-capture-cmake-command-line-arguments
#
# Note: The entries will live in CMakeCache.txt, so re-configuring with
# a command line that doesn't include an option won't remove it. You need
# to remove the CMakeCache.txt file, or override the value via the command line.
#
#
get_cmake_property(CACHE_VARS CACHE_VARIABLES)
foreach(CACHE_VAR ${CACHE_VARS})
    get_property(CACHE_VAR_HELPSTRING CACHE ${CACHE_VAR} PROPERTY HELPSTRING)
    if(CACHE_VAR_HELPSTRING STREQUAL "No help, variable specified on the command line.")
        get_property(CACHE_VAR_TYPE CACHE ${CACHE_VAR} PROPERTY TYPE)
        if(CACHE_VAR_TYPE STREQUAL "UNINITIALIZED")
            set(CACHE_VAR_TYPE)
        else()
            set(CACHE_VAR_TYPE :${CACHE_VAR_TYPE})
        endif()
        set(CMAKE_INVOKE_ARGS "${CMAKE_INVOKE_ARGS} -D${CACHE_VAR}${CACHE_VAR_TYPE}=\"${${CACHE_VAR}}\"")
    endif()
endforeach()
# Record the full command line invocation.
set(CMAKE_INVOKE "${CMAKE_COMMAND} ${CMAKE_INVOKE_ARGS} ${CMAKE_CURRENT_SOURCE_DIR}" CACHE STRING "Command used to invoke cmake" FORCE)
# Create a simple shell script that allows us to reinvoke cmake with the captured command line.
if(NOT WIN32)
    file(WRITE ${CMAKE_BINARY_DIR}/recmake_visit.sh "#!/bin/sh\n"
                                                     "rm -f CMakeCache.txt\n"
                                                     "${CMAKE_INVOKE}\n")
endif()


# Tell the CMake makefile generator to not have rules depend on
# themselves.  This causes extra rebuilds when the include path
# changes from turning a kit on or off.
set(CMAKE_SKIP_RULE_DEPENDENCY 1)

# for now...
set(BUILD_SHARED_LIBS 1)

# this turns on the FOLDER property for generators that may make use of it, like Visual Studio
# should have no effect for generators that do not use.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#-----------------------------------------------------------------------------
# Get this computer's hostname using cmake's cross platform helper function.
#----------------------------------------------------------------------------

site_name(HOSTNAME)

# Use NERSC_HOST environment variable to determine if we are running on Franklin
# or Hopper. If so, use that hostname instead of the result of SITE_NAME which is
# only the name of the login node we are running on.
if(NOT "$ENV{VISIT_HOSTNAME}" STREQUAL "")
    set(HOSTNAME "$ENV{VISIT_HOSTNAME}")
endif()


#-----------------------------------------------------------------------------

set(VTK_MINIMUM_VERSION "8.1.0" CACHE INTERNAL "minimum version of vtk that VisIt can use")

set(NEKTAR++_MINIMUM_VERSION "4.0.0" CACHE INTERNAL "minimum version of Nektar++ that VisIt can use")

# this needs to be included beofre the config-site file.
include(${CMAKE_CURRENT_SOURCE_DIR}/CMake/VisItFunctions.cmake)

#-----------------------------------------------------------------------------
# Set VISIT_CONFIG_SITE_FILE and include it if it exists
#-----------------------------------------------------------------------------
macro(include_visit_config_site)
    if(VISIT_CONFIG_SITE)
        set(VISIT_CONFIG_SITE_FILE ${VISIT_CONFIG_SITE})
    else()
        set(VISIT_CONFIG_SITE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/config-site/${HOSTNAME}.cmake")
        if(WIN32)
            if(NOT EXISTS ${VISIT_CONFIG_SITE_FILE})
                set(VISIT_CONFIG_SITE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/config-site/windows.cmake")
            endif()
        endif()
    endif()
    if(EXISTS ${VISIT_CONFIG_SITE_FILE})
        message(STATUS "Including config-site file '${VISIT_CONFIG_SITE_FILE}'")
        include(${VISIT_CONFIG_SITE_FILE})
    else()
        message(STATUS "The config-site file (${VISIT_CONFIG_SITE_FILE}) does not exist")
    endif()

    unset(VISIT_CONFIG_SITE_FILE)
endmacro()

if(NOT WIN32)
    include_visit_config_site()
endif()


#-----------------------------------------------------------------------------
# If not already set, we use a default build type of Release
#-----------------------------------------------------------------------------

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build,
        options are: Debug Release RelWithDebInfo MinSizeRel.")
endif()

#-----------------------------------------------------------------------------
# Set compiler from config-site
#-----------------------------------------------------------------------------
set(CMAKE_C_COMPILER ${VISIT_C_COMPILER})
set(CMAKE_CXX_COMPILER ${VISIT_CXX_COMPILER})
set(CMAKE_Fortran_COMPILER ${VISIT_FORTRAN_COMPILER})


#-----------------------------------------------------------------------------
# VisIt project. Declare the project after including the input because
#                this lets us set up defaults in the config-site.
#-----------------------------------------------------------------------------

project(VISIT)

if(WIN32)
    include_visit_config_site()
endif()

include(${VISIT_SOURCE_DIR}/CMake/CheckMinimumCompilerVersion.cmake)
include(${VISIT_SOURCE_DIR}/CMake/SystemChecks.cmake)
include(${VISIT_SOURCE_DIR}/CMake/SetUpPlatformDefs.cmake)


# Have to make this or an executable is made of this name.
# Suspect makemili_driver
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/exe")

#-----------------------------------------------------------------------------
# Output directories.
#-----------------------------------------------------------------------------

if(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${VISIT_BINARY_DIR}/lib CACHE INTERNAL "Single output directory for building all libraries.")
endif()

if(NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${VISIT_BINARY_DIR}/lib CACHE INTERNAL "Single output directory for building all libraries.")
endif()

if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${VISIT_BINARY_DIR}/exe CACHE INTERNAL "Single output directory for building all executables.")
endif()


set(VISIT_LIBRARY_DIR ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR})
set(VISIT_EXECUTABLE_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR})
set(CXX_TEST_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

#-----------------------------------------------------------------------------
# configure options
#-----------------------------------------------------------------------------
include("${VISIT_SOURCE_DIR}/CMake/VisItOptions.cmake")

# Set up placeholders for package vars: XXX_DIR
foreach(pkg ADIOS ADIOS2 ADIOS2_PAR ADVIO BLOSC BOOST BOXLIB CFITSIO CGNS
            CONDUIT DAMARIS EMBREE FMS GDAL GFORTRAN HDF5 HDF5_MPI H5PART ICET
            ISPC JPEG LLVM MDSPLUS MFEM MILI MOAB MOAB_MPI NEKTAR++ NETCDF
            OPENGL OPENEXR OSPRAY ANARI PIDX PYSIDE PYTHON QT QWT SILO SZIP TBB
            TIFF UINTAH VTK VTKM XDMF XSD ZLIB)
    VISIT_3RDPARTY_DIR(${pkg})
endforeach()

if(NOT WIN32)
    VISIT_3RDPARTY_DIR(MESAGL)
    VISIT_3RDPARTY_DIR(OSMESA)
endif()

# Set up placeholders for package vars: XXX_LIBDEP
foreach(pkg CGNS CONDUIT FMS HDF5 HDF5_MPI H5PART MFEM NEKTAR++ NETCDF
            SILO XDMF)
    VISIT_3RDPARTY_LIBDEP(${pkg})
endforeach()


# Set up placeholders for package vars: XXX_INCDEP
VISIT_3RDPARTY_INCDEP(MFEM)

# Set up placeholders misc pkg vars
VISIT_3RDPARTY_VAR(OPENGL_LIBRARY "Path to OpenGL lib")
VISIT_3RDPARTY_VAR(GLU_LIBRARY "Path to GLU lib")


#-----------------------------------------------------------------------------
# Read the version
#-----------------------------------------------------------------------------
file(STRINGS VERSION VERSION)
if(NOT VERSION)
    message(FATAL_ERROR
        "File \"VERSION\" with ascii string of VisIt version does not exist")
endif()
set(VISIT_VERSION ${VERSION})

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
# Init BLT
#-----------------------------------------------------------------------------
# This also includes
# VisIt's BLT defaults
include(CMake/SetupBLT.cmake)

#-----------------------------------------------------------------------------
# Set up some installation related value and macros (needs version).
#-----------------------------------------------------------------------------

if(WIN32)
    # override cmake's default of %Program Files% for CMAKE_INSTALL_PREFIX
    # if it wasn't specified via command line,
    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set(CMAKE_INSTALL_PREFIX "${VISIT_BINARY_DIR}"
            CACHE PATH "Install path prefix, prepended onto install directories" FORCE)
    endif()
    set(VISIT_INSTALLED_VERSION VisIt${VISIT_VERSION}
        CACHE PATH "VisIt install directory" FORCE)
    set(VISIT_INSTALLED_VERSION_BIN ${VISIT_INSTALLED_VERSION}
        CACHE PATH "VisIt bin install directory" FORCE)
    set(VISIT_INSTALLED_VERSION_PLUGINS ${VISIT_INSTALLED_VERSION}
        CACHE PATH "VisIt plugins install directory" FORCE)
    set(VISIT_INSTALLED_VERSION_ARCHIVES ${VISIT_INSTALLED_VERSION}/lib
        CACHE PATH "3rd party I/O archives install directory" FORCE)
else()
    include(${VISIT_SOURCE_DIR}/CMake/DetermineVisItArchitecture.cmake)
    DETERMINE_VISIT_ARCHITECTURE(VISIT_INSTALL_PLATFORM)
    set(VISIT_INSTALLED_VERSION ${VISIT_VERSION}/${VISIT_INSTALL_PLATFORM}
        CACHE PATH "VisIt install directory" FORCE)
    set(VISIT_INSTALLED_VERSION_BIN ${VISIT_INSTALLED_VERSION}/bin
        CACHE PATH "VisIt bin install directory" FORCE)
    set(VISIT_INSTALLED_VERSION_PLUGINS ${VISIT_INSTALLED_VERSION}/plugins
        CACHE PATH "VisIt plugins install directory" FORCE)
    set(VISIT_INSTALLED_VERSION_ARCHIVES ${VISIT_INSTALLED_VERSION}/archives
        CACHE PATH "3rd party I/O archives install directory" FORCE)
endif()

# locations in common on all platforms
set(VISIT_INSTALLED_VERSION_LIB ${VISIT_INSTALLED_VERSION}/lib
    CACHE PATH "VisIt lib install directory" FORCE)
set(VISIT_INSTALLED_VERSION_INCLUDE ${VISIT_INSTALLED_VERSION}/include
    CACHE PATH "VisIt include install directory" FORCE)
set(VISIT_INSTALLED_VERSION_RESOURCES ${VISIT_INSTALLED_VERSION}/resources
    CACHE PATH "VisIt resources install directory" FORCE)

mark_as_advanced(VISIT_INSTALLED_VERSION_BIN
                 VISIT_INSTALLED_VERSION_LIB
                 VISIT_INSTALLED_VERSION_PLUGINS
                 VISIT_INSTALLED_VERSION_ARCHIVES
                 VISIT_INSTALLED_VERSION_INCLUDE)

if(NOT WIN32)
    if(VISIT_RPATH_RELATIVE_TO_EXECUTABLE_PATH)
        set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
        set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    endif()
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
    if(VISIT_INSTALL_AS_BETA)
        install(CODE "
          message(\"Symlinking beta to ${VISIT_VERSION}\")
          EXECUTE_PROCESS(WORKING_DIRECTORY \${CMAKE_INSTALL_PREFIX}
            COMMAND \${CMAKE_COMMAND} -E create_symlink
            \"${VISIT_VERSION}\"
            \"${CMAKE_INSTALL_PREFIX}/beta\")
        ")
    else()
        install(CODE "
          message(\"Symlinking current to ${VISIT_VERSION}\")
          EXECUTE_PROCESS(WORKING_DIRECTORY \${CMAKE_INSTALL_PREFIX}
            COMMAND \${CMAKE_COMMAND} -E create_symlink
            \"${VISIT_VERSION}\"
            \"${CMAKE_INSTALL_PREFIX}/current\")
        ")
    endif()
endif()


# Install the headers for directories that we need in order to build plugins
if(VISIT_HEADERS_SKIP_INSTALL)
    message(STATUS "Skipping VisIt headers installation")
else()
    install(DIRECTORY avt common engine gui launcher mdserver third_party_builtin tools/writer viewer visit_vtk visitpy vtkqt winutil
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
endif()

#-----------------------------------------------------------------------------
# Misc other things needed by windows
#-----------------------------------------------------------------------------
if(WIN32)
    include(CMake/VisItWindowsFunctions.cmake)

    visit_setup_windows()
endif()


include(${VISIT_SOURCE_DIR}/CMake/SetUpThirdParty.cmake)


# Enable thread build of VisIt
if(VISIT_THREAD)
    add_definitions(-DVISIT_ATOMIC_USE_GCC_ATOMIC)
    add_definitions(-DVISIT_THREADS)

    if(APPLE)
        add_definitions(-DTURN_OFF_THREAD_SET_AFFINITY)
    endif()

    message(STATUS "Threading for VisIt: ON")
else()
    message(STATUS "Threading for VisIt: OFF")
endif()


#-----------------------------------------------------------------------------
# If we are doing a static build or the VTK Python wrappers do not exist:
#   Disable python filters.
#-----------------------------------------------------------------------------

if(VISIT_PYTHON_SCRIPTING)
    set(msg "Python scripting disabled because ")
    if (NOT PYTHONLIBS_FOUND)
        # If we did not find Python then turn off VISIT_PYTHON_SCRIPTING
        set(VISIT_PYTHON_SCRIPTING OFF)
        string(APPEND msg "Python was not found.")
    elseif(VISIT_STATIC AND (VISIT_ENGINE_ONLY OR VISIT_SERVER_COMPONENTS_ONLY))
        # Disable Python scripting if we're doing a static build for engine-only
        # or server-components-only.
        set(VISIT_PYTHON_SCRIPTING OFF)
        string(APPEND msg "an engine-only or server-components-only build was selected.")
    endif()
    if(NOT VISIT_PYTHON_SCRIPTING)
        message(STATUS ${msg})
    endif()
    unset(msg)
endif()

# Disable Python filters if we don't have Python.
if(VISIT_PYTHON_FILTERS)
    set(msg "Python Engine Filters disabled because")
    if( NOT PYTHONLIBS_FOUND)
        set(VISIT_PYTHON_FILTERS OFF)
        set(msg "${msg} Python was not found.")
    elseif(VISIT_STATIC)
        set(VISIT_PYTHON_FILTERS OFF)
        set(msg "${msg} a static build of VisIt was selected.")
    elseif(NOT VTK_PYTHON_WRAPPERS_FOUND)
        set(VISIT_PYTHON_FILTERS OFF)
        set(msg "${msg} VTK python wrappers are not installed.")
    elseif(VISIT_NOLINK_MPI_WITH_LIBRARIES)
        # Disable python filters if 'VISIT_NOLINK_MPI_WITH_LIBRARIES' is true.
        # The mpicom python module (a shared lib) must be linked to mpi.
        set(VISIT_PYTHON_FILTERS OFF)
        set(msg "${msg} VISIT_NOLINK_MPI_WITH_LIBRARIES was selected.")
    elseif(VISIT_DBIO_ONLY)
        set(VISIT_PYTHON_FILTERS OFF)
        set(msg "${msg} VISIT_DBIO_ONLY was selected.")
    endif()
    if(NOT VISIT_PYTHON_FILTERS)
        message(STATUS ${msg})
    endif()
    unset(msg)
endif()

# dl and large file support.
if(CMAKE_HOST_UNIX)
    set(DL_LIB dl)
    add_definitions(-D_LARGEFILE64_SOURCE)
endif()

if(VISIT_STATIC)
    set(BUILD_SHARED_LIBS 0)
endif()

if(VISIT_FORTRAN)
    enable_language(Fortran)
endif()

if(VISIT_PARALLEL)
    include(${VISIT_SOURCE_DIR}/CMake/VisItParallel.cmake)
    message(STATUS "PARALLEL version of VisIt")
else()
    message(STATUS "Serial version of VisIt")
endif()

set(VISIT_INCLUDE_DIR ${VISIT_SOURCE_DIR}  CACHE INTERNAL "Path to VisIt's includes")

# This include sets up vars that contains lists of include directories
include(${VISIT_SOURCE_DIR}/CMake/VisItIncludeVars.cmake)

# macros/defines needed by CMake/VisItMacros.cmake
if(NOT DEFINED VISIT_REPLACE_DEBUG_FLAGS)
    set(VISIT_REPLACE_DEBUG_FLAGS TRUE)
endif()

# macros/defines also needed by PluginVsInstall.cmake.in
include(${VISIT_SOURCE_DIR}/CMake/VisItMacros.cmake)

# Mac-specific function
function(MAC_NIB_INSTALL exename)
    if(VISIT_STATIC AND APPLE)
        foreach(x classes.nib info.nib keyedobjects.nib)
            file(COPY ${VISIT_SOURCE_DIR}/include/qt_menu.nib/${x}
                 DESTINATION ${VISIT_EXECUTABLE_DIR}/${exename}.app/Contents/Resources/qt_menu.nib/)
        endforeach()
    endif()
endfunction()

#-----------------------------------------------------------------------------
# Read the GIT version
# If we are in a git repo, get the version using git, otherwise try the
# file GIT_VERSION, which will be present in a source code tar file
# created by visit-dist.
#-----------------------------------------------------------------------------
#dummy revision, in case commands fail
set(VISIT_GIT_VERSION "Unknown")
if (EXISTS ${VISIT_SOURCE_DIR}/../.git)
    # Only try and determine the version number if it looks like
    # we're in a working copy (a .git directory should be present)
    find_package(Git QUIET)
    if(Git_FOUND)
        # if we would prefer the full hash, remove --short in the command
        execute_process(
            COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD
            WORKING_DIRECTORY "${VISIT_SOURCE_DIR}"
            RESULT_VARIABLE found_it
            OUTPUT_VARIABLE git_rev
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE)
        if(${found_it} EQUAL 0)
            set(VISIT_GIT_VERSION ${git_rev})
        endif()
        unset(found_it)
        unset(git_rev)
    endif()
elseif(EXISTS ${VISIT_SOURCE_DIR}/GIT_VERSION)
    file(STRINGS ${VISIT_SOURCE_DIR}/GIT_VERSION VISIT_GIT_VERSION)
endif()


#-----------------------------------------------------------------------------
# Create visit-config.h
#-----------------------------------------------------------------------------

# Set the slash characters based on the platform
if(WIN32)
    set(VISIT_SLASH_CHAR   "'\\\\'")
    set(VISIT_SLASH_STRING "\"\\\\\"")
else()
    set(VISIT_SLASH_CHAR   "'/'")
    set(VISIT_SLASH_STRING "\"/\"")
endif()

# Check for plugin extension
# Used by PluginManager.C
if(VISIT_STATIC)
    if(WIN32)
        SET(VISIT_PLUGIN_EXTENSION   ".a")
    else()
        SET(VISIT_PLUGIN_EXTENSION   ".lib")
    endif()
else()
    if(WIN32)
        SET(VISIT_PLUGIN_EXTENSION   ".dll")
    elseif(APPLE)
        SET(VISIT_PLUGIN_EXTENSION   ".dylib")
    else()
        SET(VISIT_PLUGIN_EXTENSION   ".so")
    endif()
endif()

# Set the rendering size limit to 16384 so that we are not unnecessarily
# constraining the user. There is no way to set this properly since this
# is used in the viewer and the limit really comes from the engine, which
# may have a different size if running client/server. This setting should
# be removed and a runtime check should be added to the engine.
message(STATUS "Setting VISIT_RENDERING_SIZE_LIMIT to 16384")
set(VISIT_RENDERING_SIZE_LIMIT 16384 CACHE INTERNAL "rendering size limit")

configure_file(${VISIT_SOURCE_DIR}/include/visit-cmake.h.in
               ${VISIT_BINARY_DIR}/include/visit-config.h @ONLY IMMEDIATE)

configure_file(${VISIT_SOURCE_DIR}/include/visit-python-cmake.h.in
               ${VISIT_BINARY_DIR}/include/visit-python-config.h @ONLY IMMEDIATE)


#-----------------------------------------------------------------------------
# Create common/misc/ConfigureInfo.C
#-----------------------------------------------------------------------------

# Store some configure information into a C file (like version, SVN version
# and platform things that change).
configure_file(${VISIT_SOURCE_DIR}/common/misc/ConfigureInfo.C.in
               ${VISIT_BINARY_DIR}/common/misc/ConfigureInfo.C @ONLY IMMEDIATE)


# The normal plugin case. We redefine these when we build against an installed
# version of VisIt
if(NOT WIN32)
    set(VISIT_PLUGIN_DIR  ${VISIT_BINARY_DIR}/plugins CACHE INTERNAL "Path to VisIt's plugins")
else()
    set(VISIT_PLUGIN_DIR  ${VISIT_EXECUTABLE_DIR} CACHE INTERNAL "Path to VisIt's plugins")
endif()

#-----------------------------------------------------------------------------
# If we're installing MPICH into our VisIt distribution then let's do that by
# adding some install commands.
#-----------------------------------------------------------------------------
if(VISIT_MPICH_INSTALL)
    visit_install_mpich()
endif()

if(LINUX)
    visit_install_system_libs()
endif()

#-----------------------------------------------------------------------------
# Set flags & options from config-site files after everything is set up since
# setting them beforehand can upset some of CMake's own find routines. Use
# quotes to unite the strings or CMake will create a list.
#-----------------------------------------------------------------------------
set(CMAKE_VERBOSE_MAKEFILE ${VISIT_VERBOSE_MAKEFILE})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${VISIT_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${VISIT_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${VISIT_EXE_LINKER_FLAGS}")

#-----------------------------------------------------------------------------
# Top-level subdirectories
#-----------------------------------------------------------------------------


#
# We pull in a few libs using VisIt's setup third party logic
# that we want to use in the engine beyond the just DB plugins.
#
# Conduit and MFEM are examples of this.
#
# The the libsim runtime and engine example programs all need to follow
# the same linking rules as the engine. Specifically, they need
# the linking dirs for any dependencies introduced.
# (For example, for conduit this includes conduit, hdf5, zlib, etc dirs)
#
# To simplify the logic needed downstream, we provide AVT_REQUIRED_LIB_DIR

# Ideally in the future with proper imported cmake targets,
# we would simply rely on cmake to propgate transitive dependency details.
#

set(__AVT_REQUIRED_LIB_DIR_LIST "")
if(CONDUIT_FOUND)
    list(APPEND __AVT_REQUIRED_LIB_DIR_LIST ${CONDUIT_LIBRARY_DIR})
endif()

if(MFEM_FOUND)
    list(APPEND __AVT_REQUIRED_LIB_DIR_LIST ${MFEM_LIBRARY_DIR})
endif()


set(AVT_REQUIRED_LIB_DIR "${__AVT_REQUIRED_LIB_DIR_LIST}" CACHE INTERNAL "Engine AVT Required I/O lib dirs")

add_subdirectory(third_party_builtin)
add_subdirectory(common)
add_subdirectory(visit_vtk)
add_subdirectory(avt)

if(NOT VISIT_DBIO_ONLY)
    if(VISIT_ENABLE_LIBSIM)
      add_subdirectory(sim)
    endif()
    add_subdirectory(viewer)   # sim depends on viewercore
    add_subdirectory(engine)   # sim depends on engine

    if(NOT VISIT_ENGINE_ONLY)
        add_subdirectory(launcher)
        add_subdirectory(mdserver)
    endif()
endif()

if(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)
    add_subdirectory(vtkqt)
    add_subdirectory(winutil)
    add_subdirectory(gui)
    if(VISIT_PYTHON_SCRIPTING AND PYTHONLIBS_FOUND)
        if(NOT VISIT_STATIC)
            add_subdirectory(visitpy)
        endif()
    endif()
    # always add java subdir, so code gen targets can be active even
    # if java is not enabled
    add_subdirectory(java)
endif()

if(NOT VISIT_DBIO_ONLY)
   add_subdirectory(plots)
   add_subdirectory(operators)
endif()

add_subdirectory(databases)

# Must come after plugins in case we're doing a static build
add_subdirectory(tools)

# Now that plugins have been added, do the mains last if we're creating static
# binaries. This is needed to link the plugins into the exe.
if(VISIT_STATIC)
    add_subdirectory(engine/main)
    add_subdirectory(sim/V2/static)

    if(NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)
        add_subdirectory(mdserver/main)
    endif()

    if(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)
        add_subdirectory(gui/main)
        add_subdirectory(viewer/main)
        if(VISIT_PYTHON_SCRIPTING AND PYTHONLIBS_FOUND)
            add_subdirectory(visitpy)
        endif()
    endif()
endif()

add_subdirectory(bin)
add_subdirectory(lib)
add_subdirectory(include)
add_subdirectory(resources)

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

add_subdirectory(doc)

#-----------------------------------------------------------------------------
# CPack -- This leverages our install targets to provide a "make package" that
#          packages up a VisIt distribution.
#-----------------------------------------------------------------------------

configure_file(${VISIT_SOURCE_DIR}/tools/dev/scripts/Info.plist.in
               ${VISIT_BINARY_DIR}/tools/dev/scripts/Info.plist @ONLY IMMEDIATE)

if(NOT WIN32)
    if (APPLE AND VISIT_CREATE_APPBUNDLE_PACKAGE)
        set(CPACK_GENERATOR "Bundle")

        set(CPACK_PACKAGE_FILE_NAME "VisIt-${VISIT_VERSION}")
        set(CPACK_BUNDLE_NAME "VisIt")
        set(CPACK_BUNDLE_ICON "${VISIT_SOURCE_DIR}/common/icons/VisItIcon.icns")
        set(CPACK_BUNDLE_PLIST "${VISIT_BINARY_DIR}/tools/dev/scripts/Info.plist")
        set(CPACK_BUNDLE_STARTUP_COMMAND "${VISIT_BINARY_DIR}/exe/visit_macos_launcher")
    else()
        set(CPACK_GENERATOR "TGZ")

        set(CPACK_PACKAGE_NAME "visit")
        set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "VisIt ${VISIT_VERSION} is a parallel visualization and data analysis tool")
        set(CPACK_PACKAGE_VENDOR "LLNS")
        set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/resources/help/en_US/copyright.html")

        # We should probably parse ${VISIT_VERSION} into its component parts instead.
        set(CPACK_PACKAGE_VERSION_MAJOR ${VISIT_VERSION_MAJOR})
        set(CPACK_PACKAGE_VERSION_MINOR ${VISIT_VERSION_MINOR})
        set(CPACK_PACKAGE_VERSION_PATCH ${VISIT_VERSION_PATCH})

        set(UL_VERSION "${VISIT_VERSION_MAJOR}_${VISIT_VERSION_MINOR}_${VISIT_VERSION_PATCH}")
        set(CPACK_PACKAGE_FILE_NAME "visit${UL_VERSION}.${VISIT_INSTALL_PLATFORM}")

        if(NOT APPLE)
            set(CPACK_STRIP_FILES TRUE)
        endif()
    endif()
    include(CPack)

else() # on windows
    visit_create_windows_installer()
endif()

#CMake add subdirectory forces this action to go last
#todo: find way for this function to execute as last
#step of make install process
#add_custom_target(osxfixup ALL
#     COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/osxfixup/osxfixup.py \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${VISIT_INSTALLED_VERSION} @executable_path/.. 3)
#add_dependencies(osxfixup install)

#todo: replace add_subdirectory with the lines above
set(FIXUP_OSX TRUE CACHE BOOL "Whether to fix up osx.")
if(APPLE AND FIXUP_OSX)
    add_subdirectory(osxfixup)
endif()

if(WIN32 AND VISIT_CREATE_XMLTOOLS_GEN_TARGETS)
    message("\nCreation of code gen targets has been turned on. If you don't want the xmltools to be run during the buid, build using the ALL_BUILD project.")
endif()

if(WIN32)
    message(STATUS "\n\nSearch for `CMAKE_INVOKE' in CMakeCache.txt to re-run CMake with the same arguments\n\n")
else()
    message(STATUS "\n\nUse recmake_visit.sh or search for `CMAKE_INVOKE' in CMakeCache.txt to re-run CMake with the same arguments\n\n")
endif()

if(VISIT_INSTALL_THIRD_PARTY)
    include(${VISIT_SOURCE_DIR}/CMake/PluginVsInstallHelpers.cmake)
endif()
