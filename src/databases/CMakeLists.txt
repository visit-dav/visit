# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#    Jeremy Meredith, Thu Jan 14 12:32:46 EST 2010
#    Added Rect and Hex back to the optional plugins.
#
#    Brad Whitlock, Wed Feb 10 10:02:53 PST 2010
#    I added the contributed Velodyne plugin to the build.
#
#    Hank Childs, Sat Feb 20 09:13:11 PST 2010
#    Added Vs plugin.  Plugin has been available for a while, but we weren't
#    generating a Makefile for it.
#
#    Eric Brugger, Wed Feb 24 09:27:11 PST 2010
#    I added the paraDIS and paraDIS_tecplot plugins to the build.
#
#    Eric Brugger, Wed Feb 24 17:46:10 PST 2010
#    I temporalily removed the paraDIS and paraDIS_tecplot plugins from
#    the build since they don't build.
#
#    Eric Brugger, Fri Feb 26 10:20:38 PST 2010
#    I added back the paraDIS and paraDIS_tecplot plugins to the build since
#    they now build.
#
#    Eric Brugger, Thu Mar  4 14:31:45 PST 2010
#    I deleted the PlasmaState plugin from the build since it no longer
#    exists.
#
#    Cyrus Harrison, Thu Mar 18 17:07:55 PDT 2010
#    Temporarily removed the paraDIS due to a build issue.
#
#    Cyrus Harrison, Mon Mar 22 09:01:32 PDT 2010
#    Added back paraDIS after build issue was resolved.
#
#    Brad Whitlock, Mon Apr 26 13:35:37 PST 2010
#    I made Xdmf use the Xdmf library. I renamed our old XDMF reader to
#    VisItXdmf and I made it get used when the Xdmf reader that uses the
#    Xdmf library is not available.
#
#    Kathleen Bonnell, Wed May 5 12:04:22 PST 2010
#    Enable VASP reader on windows.
#
#    Tom Fogal, Wed Aug  4 15:14:56 MDT 2010
#    Build STAR database.
#
#    Tom Fogal, Tue Aug 10 17:35:23 MDT 2010
#    Don't build STAR on Windows.
#
#    Jeremy Meredith, Thu Aug 12 13:54:57 EDT 2010
#    Added PuReMD.
#
#    Mark C. Miller, Sun Aug 29 23:35:37 PDT 2010
#    Fixed support for DBIO-ONLY build.
#
#    Brad Whitlock, Fri Oct 22 15:06:17 PDT 2010
#    I added a GMV reader.
#
#    Eric Brugger, Fri Jan  7 13:35:31 PST 2011
#    I replaced the BOXLIB2D and BOXLIB3D variables with just BOXLIB.
#
#    Brad Whitlock, Wed Feb 23 11:52:22 PST 2011
#    Add VelodyneCLE "AMR" plugin from Corvid.
#
#    Eric Brugger, Wed Mar 30 11:58:31 PDT 2011
#    I added the lata database reader contributed by CEA.
#
#    Eric Brugger, Mon Apr  4 10:05:55 PDT 2011
#    I temporarily disabled the ITAPS readers since they aren't compiling.
#
#    Mark C. Miller, Mon Apr  4 23:56:43 PDT 2011
#    Re-enabled ITAPS after fixing config-site file
#
#    Brad Whitlock, Thu May  5 10:12:18 PDT 2011
#    Add CaleHDF5 by Rob Managan.
#
#    Oliver Ruebel, Thu May 12 11:30 PDT 2011
#    I added a CUBE reader
#
#    Hank Childs, Thu Aug  4 13:22:36 PDT 2011
#    Add VCell reader on behalf of Ed Boyce and UCHC team.
#
#    Brad Whitlock, Wed Aug 10 16:19:42 PDT 2011
#    Add volimage reader for WPP code team.
#
#    Brad Whitlock, Thu Oct 20 14:01:42 PDT 2011
#    Record that the H5Part plugin can also be dependent on FASTBIT being set
#    so we can statically link.
#
#    Hank Childs, Sun Feb  5 18:52:04 PST 2012
#    Add CarpetHDF5 reader from Christian Reisswig and Erik Schnetter.
#
#    Brad Whitlock, Fri Mar  2 12:04:25 PST 2012
#    Remove BOW reader.
#
#    Brad Whitlock, Fri May 11 15:02:31 PDT 2012
#    Add BATL reader.
#
#    Eric Brugger, Tue Aug 28 16:33:00 PDT 2012
#    Add WPPImage reader.
#
#    Mark C. Miller, Mon Oct  8 18:07:48 PDT 2012
#    Added unv file reader from Olivier Cessenat
#
#    Eric Brugger, Tue Mar 19 10:45:32 PDT 2013
#    Added code to only build the paraDIS reader if boost 1.36.0 or later
#    was installed on the system.
#
#    Brad Whitlock, Fri Apr 19 11:31:21 PDT 2013
#    Reorder HDF5-related libraries so static linking will work. For example,
#    since Silo depends on HDF5, Silo comes before HDF5 since the
#    THIRDPARTY_DEPENDENT_PLUGINS macro appends libraries to long list of
#    3rd party libraries needed for static linking.
#
#    Kathleen Biagas, Fri May 3 16:57:29 MST 2013
#    Only build the EnSight reader if vtkIOEnSight, upon which it depends,
#    is available.
#
#    Kathleen Biagas, Tue Jun 18 16:58:43 PDT 2013
#    Skip paraDIS reader on Windows.
#
#    Eric Brugger, Wed Nov 20 11:16:52 PST 2013
#    I added the AMRTest reader.
#
#    Eric Brugger, Fri Dec 20 12:18:17 PST 2013
#    I added the MRTest reader.
#
#    Brad Whitlock, Tue Jan 14 17:20:42 PST 2014
#    Add XDB plugin.
#
#    Eric Brugger, Mon Jan 13 17:24:13 PST 2014
#    I moved the AMRTest reader from the optional list of databases to
#    build to the required list of databases to build.
#
#    Kathleen Biagas, Mon May 12 12:59:25 PDT 2014
#    Prefer VISIT_SELECTED_DATABASE_PLUGINS if defined.  Test for existence of
#    subdir before adding it.  Call VISIT_SELECTED_PLUGIN_ERROR macro if
#    necessary.
#
#    Kathleen Biagas, Tue Aug 19 16:36:56 PDT 2014
#    Remove SimV1 and SimV2Writer.
#
#    Kathleen Biagas, Tue Aug 19 23:02:45 PDT 2014
#    Removed PLOT2D, as the PLOT3D reader can read 2D data.
#
#    Dave Pugmire, Thu Nov  6 11:42:22 EST 2014
#    Removed TCGA from Windows builds.
#
#    Cyrus Harrison, Tue Feb 10 19:54:31 PST 2015
#    Changed BOOST logic and added CHECK_THIRDPARTY_SATISFIED,
#    which is used for explicitly check 3rd party deps for each db plugin.
#
#    Kathleen Biagas, Wed Sep 23 09:38:22 MST 2015
#    Don't attempt to build HDFS on Windows.
#
#    Kathleen Biagas, Fri Jan 6 12:43:32 MST 2017
#    Enable Claw on Windows.
#
#    Kathleen Biagas, Fri Mar 31 08:57:41 PDT 2017
#    Remove VCellMTMD plugin, no longer needed/used. (Per Jim Schaff at UCHC).
#
#    Mark C. Miller, Tue May  8 21:18:35 PDT 2018
#    Add ffp plugin (from Olivier Cessenat)
#
#    Mark C. Miller, Fri Apr 12 10:03:55 PDT 2019
#    Remove Vista plugin
#
#    Eric Brugger, Wed Jun 26 08:57:57 PDT 2019
#    Remove CCM plugin.
#
#    James Kress, Tue Aug 27; 15:21:30 EDT 2019
#    Add Xolotl plugin
#
#****************************************************************************

IF(NOT WIN32)
    SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${VISIT_PLUGIN_DIR}/databases)
ELSE(NOT WIN32)
    SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${VISIT_PLUGIN_DIR}/databases)
ENDIF(NOT WIN32)

SET(THIRDPARTY_DEPENDENT_PLUGINS)


#####
# Macro that helps us check db plugin dependancies.
#
# Using an explicit list of deps per db plugin gives us all the info we need to
# check each db plugin with one call to this marco.
#####
MACRO(CHECK_THIRDPARTY_SATISFIED dbplugin dep_libs)
    # setup vars used in this macro
    # (due to the nature of cmake macros, these do leak outside of this scope)
    SET(all_satisfied 1)
    SET(deps_found)
    SET(deps_not_found)
    SET(all_deps ${dep_libs})
    # build deps list from arguments passed to the macro
    FOREACH (X ${ARGN})
       LIST(APPEND all_deps ${X})
    ENDFOREACH()
    # make sure we have found ALL of the deps
    MESSAGE(STATUS "Checking ${dbplugin} plugin dependencies: ${all_deps}")
    FOREACH (lib_check ${all_deps})
    SET(dep_lib_check "${lib_check}_FOUND")
        IF(${dep_lib_check})
            LIST(APPEND deps_found ${lib_check})
        ELSE()
            SET(all_satisfied 0)
            LIST(APPEND deps_not_found ${lib_check})
        ENDIF()
    ENDFOREACH()
    # print out results of deps check so we have the full details
    LIST(LENGTH deps_found num_deps_found)
    LIST(LENGTH deps_not_found num_deps_not_found)
    IF(num_deps_found)
        MESSAGE(STATUS " FOUND: ${deps_found}")
    ENDIF()
    IF(num_deps_not_found)
        MESSAGE(STATUS " NOT FOUND: ${deps_not_found}")
    ENDIF()
    IF(all_satisfied)
        MESSAGE(STATUS "Enabling ${dbplugin}")
        LIST(FIND THIRDPARTY_DEPENDENT_PLUGINS ${dbplugin} findVar)
        IF(findVar EQUAL -1)
            LIST(APPEND THIRDPARTY_DEPENDENT_PLUGINS ${dbplugin})
        ELSE()
            # sanity check when we enable a plugin, we could keep a list
            # of all of the plugins and make sure we don't try to add them
            # twice, but checking at enable seems like a good enough check
            MESSAGE(FATAL_ERROR "CHECK_THIRDPARTY_SATISFIED should be called once per database plugin with all dependencies")
        ENDIF()
        #
        # tracking for static linking house of cards logic
        #
        IF(VISIT_STATIC)
            FOREACH(lib_check ${all_deps})
                SET(dep_lib_dir "${lib_check}_LIBRARY_DIR")
                SET(dep_libs "${lib_check}_LIB")
                IF(${dep_lib_dir})
                    LIST(FIND TMP_THIRDPARTY_IO_LIB "${${dep_lib_dir}}" findVar)
                    IF(findVar EQUAL -1)
                        LIST(APPEND TMP_THIRDPARTY_IO_LIBRARY_DIR
                                    ${${dep_lib_dir}})
                    ENDIF()
                ENDIF()
                IF(${dep_libs})
                    FOREACH(dep_lib ${${dep_libs}})
                        LIST(FIND TMP_THIRDPARTY_IO_LIB ${dep_lib} findVar)
                        IF(findVar EQUAL -1)
                            LIST(APPEND TMP_THIRDPARTY_IO_LIB ${dep_lib})
                        ENDIF()
                    ENDFOREACH(lib)
                ENDIF()
            ENDFOREACH()
        ENDIF(VISIT_STATIC)
    ELSE()
        MESSAGE(STATUS "Disabling ${dbplugin}")
        VISIT_SELECTED_PLUGIN_ERROR(DATABASE ${dbplugin}
            " third party dependencies not found: ${deps_not_found}")
    ENDIF()
ENDMACRO()

# These are the various targets that are optional since they depend on 3rd-
# party I/O libraries to be present. We use CHECK_THIRDPARTY_SATISFIED to
# check the dependencies for each db plugin. If all dependencies are
# satisfied, db plugin is added to the mix.

UNSET(TMP_THIRDPARTY_IO_LIBRARY_DIR)
UNSET(TMP_THIRDPARTY_IO_LIB)

MESSAGE(STATUS "Checking database plugin dependencies.")

CHECK_THIRDPARTY_SATISFIED(CGNS CGNS)

CHECK_THIRDPARTY_SATISFIED(Cale SILO)
CHECK_THIRDPARTY_SATISFIED(KullLite SILO)
CHECK_THIRDPARTY_SATISFIED(PDB SILO)
CHECK_THIRDPARTY_SATISFIED(Silo SILO)
CHECK_THIRDPARTY_SATISFIED(SAMI SILO)

CHECK_THIRDPARTY_SATISFIED(Nektar++ NEKTAR++ BOOST)
CHECK_THIRDPARTY_SATISFIED(NETCDF NETCDF)
CHECK_THIRDPARTY_SATISFIED(S3D NETCDF_CXX)
CHECK_THIRDPARTY_SATISFIED(Exodus NETCDF)

CHECK_THIRDPARTY_SATISFIED(MFIXCDF NETCDF_CXX)
CHECK_THIRDPARTY_SATISFIED(Xdmf XDMF)

CHECK_THIRDPARTY_SATISFIED(Chombo HDF5)
CHECK_THIRDPARTY_SATISFIED(Blueprint CONDUIT MFEM)

#CHECK_THIRDPARTY_SATISFIED(ALS HDF5)
CHECK_THIRDPARTY_SATISFIED(AMR HDF5)
CHECK_THIRDPARTY_SATISFIED(BATL HDF5)
CHECK_THIRDPARTY_SATISFIED(CaleHDF5 HDF5)
CHECK_THIRDPARTY_SATISFIED(CarpetHDF5 HDF5)
CHECK_THIRDPARTY_SATISFIED(CosmosPP HDF5)
CHECK_THIRDPARTY_SATISFIED(Denovo HDF5)
CHECK_THIRDPARTY_SATISFIED(Enzo HDF5)
CHECK_THIRDPARTY_SATISFIED(FLASH HDF5)
CHECK_THIRDPARTY_SATISFIED(Geqdsk HDF5)
CHECK_THIRDPARTY_SATISFIED(GTC HDF5)
CHECK_THIRDPARTY_SATISFIED(H5Nimrod HDF5)
CHECK_THIRDPARTY_SATISFIED(M3D HDF5)
CHECK_THIRDPARTY_SATISFIED(M3DC1 HDF5)
CHECK_THIRDPARTY_SATISFIED(OpenPMD HDF5)
CHECK_THIRDPARTY_SATISFIED(PFLOTRAN HDF5)
CHECK_THIRDPARTY_SATISFIED(Pixie HDF5)
CHECK_THIRDPARTY_SATISFIED(PVLD HDF5)
CHECK_THIRDPARTY_SATISFIED(SAMRAI HDF5)
CHECK_THIRDPARTY_SATISFIED(SXRIS HDF5)
CHECK_THIRDPARTY_SATISFIED(Tetrad HDF5)
CHECK_THIRDPARTY_SATISFIED(UNIC HDF5)
CHECK_THIRDPARTY_SATISFIED(Velodyne HDF5)
CHECK_THIRDPARTY_SATISFIED(Vs HDF5)
CHECK_THIRDPARTY_SATISFIED(XGC HDF5)
CHECK_THIRDPARTY_SATISFIED(MOAB MOAB)
CHECK_THIRDPARTY_SATISFIED(Xolotl HDF5)

# if we don't have the xdmf lib, we can fall back to VisIt's xdmf reader
# that only uses hdf5
IF(NOT XDMF_FOUND)
    CHECK_THIRDPARTY_SATISFIED(VisItXdmf HDF5)
ENDIF(NOT XDMF_FOUND)


CHECK_THIRDPARTY_SATISFIED(ADIOS ADIOS)
CHECK_THIRDPARTY_SATISFIED(ADIOS2 ADIOS2)
CHECK_THIRDPARTY_SATISFIED(Adventure ADVIO)

CHECK_THIRDPARTY_SATISFIED(Boxlib2D BOXLIB)
CHECK_THIRDPARTY_SATISFIED(Boxlib3D BOXLIB)

CHECK_THIRDPARTY_SATISFIED(FITS CFITSIO)
CHECK_THIRDPARTY_SATISFIED(GDAL GDAL)

CHECK_THIRDPARTY_SATISFIED(Cosmos HDF4)
CHECK_THIRDPARTY_SATISFIED(RAGE   HDF4)
CHECK_THIRDPARTY_SATISFIED(ZeusMP HDF4)

CHECK_THIRDPARTY_SATISFIED(MDSplus MDSPLUS)
CHECK_THIRDPARTY_SATISFIED(Mili    MILI)
CHECK_THIRDPARTY_SATISFIED(Uintah  UINTAH)
IF(VISIT_PARALLEL)
  CHECK_THIRDPARTY_SATISFIED(IDX     PIDX)
ENDIF()
CHECK_THIRDPARTY_SATISFIED(MFEM    MFEM)

CHECK_THIRDPARTY_SATISFIED(OpenEXR OPENEXR)

# special cases for windows
IF(WIN32)
    CHECK_THIRDPARTY_SATISFIED(H5Part H5PART)
ELSE()
    IF(FASTBIT AND FASTQUERY)
        CHECK_THIRDPARTY_SATISFIED(H5Part H5PART FASTBIT FASTQUERY)
    ELSE()
        CHECK_THIRDPARTY_SATISFIED(H5Part H5PART)
    ENDIF()
    if(VISIT_PARADIS)
        CHECK_THIRDPARTY_SATISFIED(paraDIS BOOST)
        CHECK_THIRDPARTY_SATISFIED(paraDIS_tecplot BOOST)
    endif()
ENDIF()

# Save the list of all library dirs and libs so the engine can use it
IF(VISIT_STATIC)
    SET(ALL_THIRDPARTY_IO_LIBRARY_DIR ${TMP_THIRDPARTY_IO_LIBRARY_DIR} CACHE INTERNAL "List of all I/O lib dirs")
    SET(ALL_THIRDPARTY_IO_LIB ${TMP_THIRDPARTY_IO_LIB} CACHE INTERNAL "List of all I/O libs")
ENDIF(VISIT_STATIC)

# If we're not building statically, add SimV2.
# If we're not building statically, add XDB -- if we enabled it.
IF(NOT VISIT_STATIC)
    SET(SIMV2_PLUGIN SimV2)
ENDIF(NOT VISIT_STATIC)
IF(VISIT_ENABLE_XDB)
    SET(XDBPLUGIN FieldViewXDB)
ENDIF(VISIT_ENABLE_XDB)

# These are our required plugins (no external dependencies)
SET(REQUIRED_DATABASE_PLUGINS
AMRTest
ANALYZE
ANSYS
AUXFile
AugDecomp
BOV
CEAucd
Claw
CMAT
CTRL
Cube
Curve2D
DDCMD
Dyna3D
ExtrudedVol
Fluent
FT2
GGCM
GHOST
GMV
GULP
Gadget
Image
LAMMPS
Lines
MFIX
MM5
MatrixMarket
Miranda
NASTRAN
Nek5000
OVERFLOW
OpenFOAM
PATRAN
PuReMD
PLOT3D
PlainText
PLY
Point3D
ProteinDataBank
RAW
SAR
SAS
SPCTH
STL
Shapefile
${SIMV2_PLUGIN}
Spheral
TFT
TSurf
Tecplot
VASP
VTK
WPPImage
WavefrontOBJ
WellLogs
XSF
XYZ
Xmdv
ffp
lata
volimage
unv
${XDBPLUGIN}
)

IF(TARGET vtkIOEnSight)
    LIST(APPEND REQUIRED_DATABASE_PLUGINS EnSight)
ELSE()
    IF(DEFINED VISIT_SELECTED_DATABASE_PLUGINS)
        VISIT_SELECTED_PLUGIN_ERROR(DATABASE "EnSight" "EnSight plugin requires vtkIOEnSight")
    ELSE()
        MESSAGE(STATUS "EnSight plugin requires vtkIOEnSight")
    ENDIF()
ENDIF(TARGET vtkIOEnSight)

IF (NOT WIN32)
    LIST(APPEND REQUIRED_DATABASE_PLUGINS
        HDFS
        STAR
        TCGA
        Vis5D
        ZipWrapper
    )
    SET(OPTIONAL_DATABASE_PLUGINS
        Hex
        MRTest
        PICS_Tester
        Rect
        VLI
    )
ELSE(NOT WIN32)
    VISIT_SELECTED_PLUGIN_ERROR(DATABASE "Claw;STAR;TCGA;Vis5D;ZipWrapper;Hex;MRTest;PICS_Tester;Rect;VLI" "not currently buildable on Windows")
ENDIF (NOT WIN32)


SET(MINIMAL_DATABASE_PLUGINS
Curve2D
RAW
VTK
PICS_Tester
)

# Select the plugins to build
IF(DEFINED VISIT_SELECTED_DATABASE_PLUGINS)
    SET(BUILD_PLUGINS ${VISIT_SELECTED_DATABASE_PLUGINS})
ELSEIF(VISIT_BUILD_ALL_PLUGINS)
    SET(BUILD_PLUGINS ${REQUIRED_DATABASE_PLUGINS} ${THIRDPARTY_DEPENDENT_PLUGINS} ${OPTIONAL_DATABASE_PLUGINS})
ELSE()
    IF(VISIT_BUILD_MINIMAL_PLUGINS)
        SET(BUILD_PLUGINS ${MINIMAL_DATABASE_PLUGINS})
    ELSE(VISIT_BUILD_MINIMAL_PLUGINS)
        SET(BUILD_PLUGINS ${REQUIRED_DATABASE_PLUGINS} ${THIRDPARTY_DEPENDENT_PLUGINS})
    ENDIF(VISIT_BUILD_MINIMAL_PLUGINS)
ENDIF()

# Add the plugin directories
FOREACH(db ${BUILD_PLUGINS})
    # EXISTS test requires full path
    IF(EXISTS ${VISIT_SOURCE_DIR}/databases/${db})
        ADD_SUBDIRECTORY(${db})
    ELSE()
        IF(DEFINED VISIT_SELECTED_DATABASE_PLUGINS)
            Message(FATAL_ERROR "Selected  database (${db}) does not exist.")
        ELSEIF(VISIT_STATIC)
            # for code below, remove anything we aren't actually building
            Message("${db} removed from build because it code does not exist")
            LIST(REMOVE_ITEM BUILD_PLUGINS ${db})
        ENDIF()
    ENDIF()
ENDFOREACH(db)

# If we're building statically, make various main programs depend on plugins being built
IF(VISIT_STATIC)
    # Write a file containing the list of plugins such that we can include it into C++ code.
    FOREACH(db ${BUILD_PLUGINS})
        SET(dbCode "${dbCode}PLUGIN_VERB(${db})\n")
    ENDFOREACH(db)
    FILE(WRITE ${VISIT_BINARY_DIR}/include/enabled_databases.h "${dbCode}")

    IF(NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)
        CREATE_PLUGIN_DEPENDENCIES(mdserver "I" "Database" ${BUILD_PLUGINS})
        CREATE_PLUGIN_DEPENDENCIES(mdserver "M" "Database" ${BUILD_PLUGINS})
    ENDIF(NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)
    CREATE_PLUGIN_DEPENDENCIES(engine_ser_exe "I" "Database" ${BUILD_PLUGINS})
    CREATE_PLUGIN_DEPENDENCIES(engine_ser_exe "E" "Database_ser" ${BUILD_PLUGINS})
    IF(VISIT_PARALLEL)
        CREATE_PLUGIN_DEPENDENCIES(engine_par_exe "I" "Database" ${BUILD_PLUGINS})
        CREATE_PLUGIN_DEPENDENCIES(engine_par_exe "E" "Database_par" ${BUILD_PLUGINS})
    ENDIF(VISIT_PARALLEL)
ENDIF(VISIT_STATIC)
