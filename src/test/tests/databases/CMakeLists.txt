#*****************************************************************************
#
# Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
# Produced at the Lawrence Livermore National Laboratory
# LLNL-CODE-442911
# All rights reserved.
#
# This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
# full copyright notice is contained in the file COPYRIGHT located at the root
# of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
#
# Redistribution  and  use  in  source  and  binary  forms,  with  or  without
# modification, are permitted provided that the following conditions are met:
#
#  - Redistributions of  source code must  retain the above  copyright notice,
#    this list of conditions and the disclaimer below.
#  - Redistributions in binary form must reproduce the above copyright notice,
#    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
#    documentation and/or other materials provided with the distribution.
#  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
#    be used to endorse or promote products derived from this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
# ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
# LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
# DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
# SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
# CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
# LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
# OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.
#
#*****************************************************************************
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
