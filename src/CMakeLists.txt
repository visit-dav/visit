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
  STRING( REGEX REPLACE "([0-9]+).[0-9]+.[0-9]+[.0-9]*.*" "\\1" ver_major ${ver_ascii} )
  STRING( REGEX REPLACE "[0-9]+.([0-9]+).[0-9]+[.0-9]*.*" "\\1" ver_minor ${ver_ascii} )
  STRING( REGEX REPLACE "[0-9]+.[0-9]+.([0-9]+)[.0-9]*.*" "\\1" ver_patch ${ver_ascii} )
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
include(${VISIT_SOURCE_DIR}/CMake/SystemChecks.cmake)
include(${VISIT_SOURCE_DIR}/CMake/SetUpPlatformDefs.cmake)


#-----------------------------------------------------------------------------
# Setup out of source build indicator.
#-----------------------------------------------------------------------------
IF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    SET(VISIT_OUT_OF_SOURCE_BUILD 0)
ELSE("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    SET(VISIT_OUT_OF_SOURCE_BUILD 1)
    # Have to make this or an executable is made of this name.
    # Suspect makemili_driver
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/exe")
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

OPTION(IGNORE_THIRD_PARTY_LIB_PROBLEMS "Ignore problems finding requested third party libraries")
OPTION(VISIT_FORCE_SSH_TUNNELING "Force ssh tunnelling for sockets" OFF)
OPTION(VISIT_CREATE_APPBUNDLE_PACKAGE "Create DMG file with Mac App bundle with make package" OFF)
IF(WIN32)
  OPTION(VISIT_MESA_REPLACE_OPENGL "Use mesa as a drop-in replacment for opengl when system opengl is insufficient." OFF)
  OPTION(VISIT_MAKE_NSIS_INSTALLER "Create an installer package using NSIS" OFF)
  OPTION(VISIT_WINDOWS_APPLICATION "Create Windows-style applications with no console" ON)
  IF(VISIT_WINDOWS_APPLICATION)
      SET(VISIT_APPLICATION_STYLE "WIN32")
  ENDIF(VISIT_WINDOWS_APPLICATION)
ENDIF(WIN32)

OPTION(VISIT_CREATE_XMLTOOLS_GEN_TARGETS "Create cmake targets to run xmltools code gen targets" ON)
OPTION(VISIT_INSTALL_AS_BETA "Install as beta (symlink beta instead of current to installed version)." OFF)
OPTION(VISIT_MPICH_INSTALL "Install MPICH into VisIt binary distribution" OFF)
IF(APPLE OR WIN32)
    OPTION(VISIT_USE_X "Use X11" OFF)
ELSE(APPLE OR WIN32)
    OPTION(VISIT_USE_X "Use X11" ON)
ENDIF(APPLE OR WIN32)

OPTION(VISIT_THREAD "Build VisIt with thread support." OFF)

# some options that when turned off reduce the build targets
OPTION(VISIT_ENABLE_ANNOTATION_TOOLS "Enable generation of annotation tools (text2polys, time_annotation)" ON)
OPTION(VISIT_ENABLE_DATAGEN "Enable generation of sample data files" ON)
OPTION(VISIT_ENABLE_DATA_MANUAL_EXAMPLES "Build Getting Data Into VisIt examples" OFF)
OPTION(VISIT_ENABLE_DIAGNOSTICS  "Enable building of diagnostices" ON)
OPTION(VISIT_ENABLE_EXAMPLES  "Enable building of examples" ON)
OPTION(VISIT_ENABLE_LIBSIM "Enable building of libsim." ON)
OPTION(VISIT_ENABLE_MANUALS  "Enable building of manuals (requires python3)" ON)
OPTION(VISIT_ENABLE_SILO_TOOLS "Enable building of silo tools (mrgtree2dot, add_visit_searchpath)" ON)
OPTION(VISIT_ENABLE_UNIT_TESTS "Enable building of unit tests (like MRUCache_test, StringHelpers_test)" ON)
OPTION(VISIT_ENABLE_XDB "Enable Intelligent Light XDB database plugin." OFF) # OFF for now

SET(VISIT_INSTALL_PROFILES_TO_HOSTS "" CACHE STRING "Install specified host profiles (no action if empty)." )
SET(VISIT_DEFAULT_SILO_DRIVER "PDB" CACHE STRING "Default driver for generating silo data. Valid values are PDB, HDF5 or empty. An empty value means driver can be specified via make on the command line." )

# flag to turn on preprocessor errors related to obsolete code.
OPTION(VISIT_REMOVE_OBSOLETE_CODE "Enable compiler errors for removal of oboslete code." OFF)

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

# Libraries some third-parties are dependent upon
VISIT_3RDPARTY_VAR(JPEG_DIR   "Path containing the jpeg library's include and lib")
VISIT_3RDPARTY_VAR(SZIP_DIR   "Path containing the szip library's include and lib")

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
VISIT_3RDPARTY_VAR(BLOSC_DIR     "Path containing the Blosc library's include and lib")
VISIT_3RDPARTY_VAR(BOOST_DIR     "Path containing the BOOST library's include and lib")
VISIT_3RDPARTY_VAR(BOXLIB_DIR    "Path containing the Boxlib library's include and lib")
VISIT_3RDPARTY_VAR(CFITSIO_DIR   "Path containing the CFITSIO library's include and lib")
VISIT_3RDPARTY_VAR(CGNS_DIR      "Path containing the CGNS library's include and lib")
VISIT_3RDPARTY_DEP(CGNS_LIBDEP   "CGNS library dependencies")
VISIT_3RDPARTY_VAR(CONDUIT_DIR   "Path containing the Conduit library's include and lib")
VISIT_3RDPARTY_DEP(CONDUIT_LIBDEP "Conduit library dependencies")
VISIT_3RDPARTY_VAR(DAMARIS_DIR   "Path containing the Damaris library's include and lib")
VISIT_3RDPARTY_VAR(FMS_DIR       "Path containing the FMS library's include and lib")
VISIT_3RDPARTY_DEP(FMS_LIBDEP    "FMS library dependencies")
VISIT_3RDPARTY_VAR(GDAL_DIR      "Path containing the GDAL library's include and lib")
VISIT_3RDPARTY_VAR(GFORTRAN_DIR  "Path containing the GFortran library's include and lib")
VISIT_3RDPARTY_VAR(HDF5_DIR      "Path containing the HDF5 library's include and lib")
VISIT_3RDPARTY_DEP(HDF5_LIBDEP   "HDF5 library dependencies")
VISIT_3RDPARTY_VAR(HDF5_MPI_DIR  "Path containing the parallel HDF5 library's include and lib")
VISIT_3RDPARTY_DEP(HDF5_MPI_LIBDEP "HDF5 parallel library dependencies")
VISIT_3RDPARTY_VAR(H5PART_DIR    "Path containing the H5Part library's include and lib")
VISIT_3RDPARTY_DEP(H5PART_LIBDEP "H5Part library dependencies")
VISIT_3RDPARTY_VAR(LLVM_DIR      "Path containing the LLVM library's include and lib")
VISIT_3RDPARTY_VAR(MFEM_DIR      "Path containing the mfem library's include and lib")
VISIT_3RDPARTY_DEP(MFEM_INCDEP   "MFEM include dependencies")
VISIT_3RDPARTY_DEP(MFEM_LIBDEP   "MFEM library dependencies")
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
# Init BLT
#-----------------------------------------------------------------------------
# This also includes
# VisIt's BLT defaults
include(CMake/SetupBLT.cmake)

#-----------------------------------------------------------------------------
# Set up some installation related value and macros (needs version).
#-----------------------------------------------------------------------------

IF(WIN32)
    # override cmake's default of %Program Files% for CMAKE_INSTALL_PREFIX
    # if it wasn't specified via command line,
    IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
      IF( NOT APPENDED_VISIT_VERSION_TO_INSTALL_PATH)
        SET(CMAKE_INSTALL_PREFIX "${VISIT_BINARY_DIR}/VisIt${VISIT_VERSION}"
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


include(${VISIT_SOURCE_DIR}/CMake/SetUpThirdParty.cmake)


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
endif(VISIT_PYTHON_SCRIPTING)

# Disable Python filters if we don't have Python.
if(VISIT_PYTHON_FILTERS)
  set(msg "Python Engine Filters disabled because")
  if ( NOT PYTHONLIBS_FOUND)
    set(VISIT_PYTHON_FILTERS OFF)
    set(msg "${msg} Python was not found.")
  elseif (VISIT_STATIC)
    set(VISIT_PYTHON_FILTERS OFF)
    set(msg "${msg} a static build of VisIt was selected.")
  elseif (NOT VTK_PYTHON_WRAPPERS_FOUND)
    set(VISIT_PYTHON_FILTERS OFF)
    set(msg "${msg} VTK python wrappers are not installed.")
  elseif (VISIT_NOLINK_MPI_WITH_LIBRARIES)
    # Disable python filters if 'VISIT_NOLINK_MPI_WITH_LIBRARIES' is true.
    # The mpicom python module (a shared lib) must be linked to mpi.
    set(VISIT_PYTHON_FILTERS OFF)
    set(msg "${msg} VISIT_NOLINK_MPI_WITH_LIBRARIES was selected.")
  elseif (VISIT_DBIO_ONLY)
    set(VISIT_PYTHON_FILTERS OFF)
    set(msg "${msg} VISIT_DBIO_ONLY was selected.")
  endif()
  if (NOT VISIT_PYTHON_FILTERS)
      message(STATUS ${msg})
  endif()
  unset(msg)
endif()



# dl and large file support.
IF(CMAKE_HOST_UNIX)
  SET(DL_LIB dl)
  ADD_DEFINITIONS(-D_LARGEFILE64_SOURCE)
ENDIF(CMAKE_HOST_UNIX)

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

SET(VISIT_INCLUDE_DIR ${VISIT_SOURCE_DIR}  CACHE INTERNAL "Path to VisIt's includes")

# This include sets up vars that contains lists of include directories
include(${VISIT_SOURCE_DIR}/CMake/VisItIncludeVars.cmake)

# macros/defines needed by CMake/VisItMacros.cmake
if (NOT DEFINED VISIT_REPLACE_DEBUG_FLAGS)
  set(VISIT_REPLACE_DEBUG_FLAGS TRUE)
endif ()


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
else (EXISTS ${VISIT_SOURCE_DIR}/../.git)
    if (EXISTS ${VISIT_SOURCE_DIR}/GIT_VERSION)
        file(STRINGS ${VISIT_SOURCE_DIR}/GIT_VERSION VISIT_GIT_VERSION)
    endif (EXISTS ${VISIT_SOURCE_DIR}/GIT_VERSION)
endif()

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

if(LINUX)
    #-------------------------------------------------------------------------
    # Install the system libraries required by Qt to the lib directory.
    # The versions of these libraries change frequently, so including
    # them means VisIt may continue to run even if the OS is upgraded.
    # "/lib64" is for rhel7 and fedora31, "/usr/lib/x86_64-linux-gnu"
    # is for ubuntu 18, debian 10 and debian 11, "/lib/x86_64-linux-gnu"
    # is for ubuntu 20.
    #-------------------------------------------------------------------------
    if(EXISTS "/lib64/libicui18n.so")
        file(GLOB libicui18n_libs "/lib64/libicui18n.so*")
        file(GLOB libicudata_libs "/lib64/libicudata.so*")
        file(GLOB libicuuc_libs "/lib64/libicuuc.so*")
        set(icu_libs "")
        list(APPEND icu_libs ${libicui18n_libs})
        list(APPEND icu_libs ${libicudata_libs})
        list(APPEND icu_libs ${libicuuc_libs})
        install(FILES ${icu_libs}
                DESTINATION ${VISIT_INSTALLED_VERSION_LIB}
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                            GROUP_READ GROUP_WRITE GROUP_EXECUTE
                            WORLD_READ             WORLD_EXECUTE)
    elseif(EXISTS "/lib/x86_64-linux-gnu/libicui18n.so")
        file(GLOB libicui18n_libs "/lib/x86_64-linux-gnu/libicui18n.so*")
        file(GLOB libicudata_libs "/lib/x86_64-linux-gnu/libicudata.so*")
        file(GLOB libicuuc_libs "/lib/x86_64-linux-gnu/libicuuc.so*")
        set(icu_libs "")
        list(APPEND icu_libs ${libicui18n_libs})
        list(APPEND icu_libs ${libicudata_libs})
        list(APPEND icu_libs ${libicuuc_libs})
        install(FILES ${icu_libs}
                DESTINATION ${VISIT_INSTALLED_VERSION_LIB}
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                            GROUP_READ GROUP_WRITE GROUP_EXECUTE
                            WORLD_READ             WORLD_EXECUTE)
    elseif(EXISTS "/usr/lib/x86_64-linux-gnu/libicui18n.so")
        file(GLOB libicui18n_libs "/usr/lib/x86_64-linux-gnu/libicui18n.so*")
        file(GLOB libicudata_libs "/usr/lib/x86_64-linux-gnu/libicudata.so*")
        file(GLOB libicuuc_libs "/usr/lib/x86_64-linux-gnu/libicuuc.so*")
        set(icu_libs "")
        list(APPEND icu_libs ${libicui18n_libs})
        list(APPEND icu_libs ${libicudata_libs})
        list(APPEND icu_libs ${libicuuc_libs})
        install(FILES ${icu_libs}
                DESTINATION ${VISIT_INSTALLED_VERSION_LIB}
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                            GROUP_READ GROUP_WRITE GROUP_EXECUTE
                            WORLD_READ             WORLD_EXECUTE)
    endif()

    #-------------------------------------------------------------------------
    # If the g++ standard library is specified, then install it the lib
    # directory. This is typically needed if using a non-standard version
    # of the g++ compiler.
    #-------------------------------------------------------------------------
    if(VISIT_CXX_LIBRARY)
        file(GLOB libstdcxx_libs "${VISIT_CXX_LIBRARY}*")
        install(FILES ${libstdcxx_libs}
                DESTINATION ${VISIT_INSTALLED_VERSION_LIB}
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                            GROUP_READ GROUP_WRITE GROUP_EXECUTE
                            WORLD_READ             WORLD_EXECUTE)
    endif()
endif()

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
    if(VISIT_ENABLE_LIBSIM)
      ADD_SUBDIRECTORY(sim)
    endif()
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
    # always add java subdir, so code gen targets can be active even
    # if java is not enabled
    ADD_SUBDIRECTORY(java)
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

add_subdirectory(doc)

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
        SET (CPACK_BUNDLE_STARTUP_COMMAND "${VISIT_BINARY_DIR}/exe/visit_macos_launcher")
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
ELSE () # on windows
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
        if(CODESIGN_HASH)
           set(codesign "/DCODESIGN_HASH=${CODESIGN_HASH}")
        endif()
        ADD_CUSTOM_COMMAND(OUTPUT visit${VISIT_VERSION}.exe
            COMMAND "${VISIT_WINDOWS_DIR}/MSVC2017/p7zip/18.05/7z.exe" a -t7z -m0=lzma2 -mx9  visit${VISIT_VERSION}.7z VisIt${VISIT_VERSION}
            COMMAND ${MAKENSIS}
                    /DVISIT_SOURCE_DIR=${VSD_NATIVE}
                    /DVISIT_WINDOWS_DIR=${VWD_NATIVE}
                    /DINSTALL_PREFIX=${CIP_NATIVE}
                    /DBIN_DIR=${CBD_NATIVE}
                    ${VISIT_WINDOWS_DIR}/distribution/installation/CreateDBSections.nsi
            COMMAND ${MAKENSIS}
                    ${codesign}
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
        if(NOT CODESIGN_HASH)
          # not building a signed release
          # make INSTALL be a prereq for _PACKAGE.
          ADD_CUSTOM_COMMAND(TARGET _PACKAGE PRE_BUILD
              COMMAND ${CMAKE_COMMAND} -DBUILD_TYPE=${CMAKE_CFG_INTDIR} -P cmake_install.cmake)
        endif()
        SET_TARGET_PROPERTIES(_PACKAGE PROPERTIES
                              EXCLUDE_FROM_DEFAULT_BUILD 1
                              EXCLUDE_FROM_ALL 1 )

        if(DEFINED ARCHIVER_EXE)
            get_filename_component(VDEP ${VWD_NATIVE} PATH)
            file(TO_NATIVE_PATH ${VDEP} VDEP_NATIVE)
            set(zipname visit_windowsdev_${VISIT_VERSION}.zip)

            file(TO_NATIVE_PATH ${ARCHIVER_EXE} ARCHIVER_NATIVE)
            add_custom_command(OUTPUT ${CBD_NATIVE}/${zipname}
                COMMAND cd ${VDEP_NATIVE}
                COMMAND "${ARCHIVER_NATIVE}" a -m0=LZMA -mx=9
                    ${CBD_NATIVE}/${zipname}
                    windowsbuild/distribution
                    windowsbuild/MSVC2017
                    windowsbuild/thirdparty-projects
                COMMAND cd ${CBD_NATIVE}
                WORKING_DIRECTORY ${DEP_NATIVE})
            add_custom_target(_WINDEV_PACKAGE ALL DEPENDS ${CBD_NATIVE}/${zipname})
            set_target_properties(_WINDEV_PACKAGE PROPERTIES
                              EXCLUDE_FROM_DEFAULT_BUILD 1
                              EXCLUDE_FROM_ALL 1)
            unset(VDEP)
            unset(VDEP_NATIVE)
            unset(zipname)
        endif()
        UNSET(VSD_NATIVE)
        UNSET(VWD_NATIVE)
        UNSET(CIP_NATIVE)
        UNSET(CBD_NATIVE)
      ELSE(${MAKENSIS_FOUND})
        MESSAGE(FATAL_ERROR "NSIS installer package requested, but 'makensis' could not be found.")
      ENDIF(${MAKENSIS_FOUND})
    ENDIF(VISIT_MAKE_NSIS_INSTALLER)
ENDIF (NOT WIN32)

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

if(WIN32 AND VISIT_CREATE_XMLTOOLS_GEN_TARGETS)
    message("\nCreation of code gen targets has been turned on. If you don't want the xmltools to be run during the buid, build using the ALL_BUILD project.")
endif()

IF (NOT WIN32)
    MESSAGE(STATUS "\n\nUse recmake_visit.sh or search for `CMAKE_INVOKE' in CMakeCache.txt to re-run CMake with the same arguments\n\n")
ELSE (NOT WIN32)
    MESSAGE(STATUS "\n\nSearch for `CMAKE_INVOKE' in CMakeCache.txt to re-run CMake with the same arguments\n\n")
ENDIF (NOT WIN32)

if(VISIT_INSTALL_THIRD_PARTY)
    include(${VISIT_SOURCE_DIR}/CMake/PluginVsInstallHelpers.cmake)
endif()
