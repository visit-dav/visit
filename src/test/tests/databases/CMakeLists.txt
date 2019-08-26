# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

SET(PY_TESTS
    ANALYZE ANSYS bov chgcar claw curve ddcMD Fluent image
    imgvol lata leos miranda NASTRAN OpenFOAM overlink
    PATRAN PLOT3D RAW rect scale_mesh shapefile singlemulti tecplot
    unv vtk Xmdv xyz)
IF(RUN_BROKEN_TESTS)
  LIST(APPEND PY_TESTS diff zipwrapper)
ENDIF()
PY_3RDPARTY_TEST(PY_TESTS BOXLIB boxlib)
PY_3RDPARTY_TEST(PY_TESTS CGNS CGNS)
PY_3RDPARTY_TEST(PY_TESTS XDMF xdmf)
PY_3RDPARTY_TEST(PY_TESTS NETCDF exodus netcdf)
PY_3RDPARTY_TEST(PY_TESTS CFITSIO FITS)
PY_3RDPARTY_TEST(PY_TESTS GDAL GDAL)
PY_3RDPARTY_TEST(PY_TESTS MILI mili)
PY_3RDPARTY_TEST(PY_TESTS H5PART h5part)
PY_3RDPARTY_TEST(PY_TESTS SILO
        activesource correlation Cale closedatabase empty_db
        export_db global_node_ids history kulllite metadata
        multi_format multi_format2 oldsilo pdbdatabase
        ProteinDataBank reopen sami silo_altdriver
        silo_datatypes silo timesliders xform_precision
        wave_tv)
IF(NOT WIN32)
    PY_3RDPARTY_TEST(PY_TESTS SILO)
ENDIF()
PY_3RDPARTY_TEST(PY_TESTS HDF5 Chombo flash pixie samrai Velodyne Xolotl)
IF(TARGET vtkIOEnSight)
    LIST(APPEND PY_TESTS EnSight)
ENDIF()
SET(TEST_EXTRA_ARGS)
IF (NOT HAVE_OSMESA)
    SET(TEST_EXTRA_ARGS "--threshold-error=ansys2_0:24,lata_11:29,lata_12:32")
    SET(TEST_EXTRA_ARGS "${TEST_EXTRA_ARGS},lata_13:15,miranda_raw_reader4:27")
    SET(TEST_EXTRA_ARGS "${TEST_EXTRA_ARGS},miranda_raw_reader5:17,nastran2_0:22")
    SET(TEST_EXTRA_ARGS "${TEST_EXTRA_ARGS},nastran4_0:21,nastran4_1:21,PATRAN2_0:21")
    SET(TEST_EXTRA_ARGS "${TEST_EXTRA_ARGS},netcdf_0_00:15,samrai_12:34,samrai_15:12")
ENDIF()
ADD_VISIT_PY_TESTS(Database "${PY_TESTS}" ${TEST_EXTRA_ARGS})
