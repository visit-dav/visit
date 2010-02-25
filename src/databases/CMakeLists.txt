#*****************************************************************************
#
# Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
# Produced at the Lawrence Livermore National Laboratory
# LLNL-CODE-400142
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
#    I temporalily disabled the paraDIS and paraDIS_tecplot plugins since
#    they don't build.
#
#****************************************************************************/
IF(NOT WIN32)
    SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${VISIT_PLUGIN_DIR}/databases)
ELSE(NOT WIN32)
    SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${VISIT_PLUGIN_DIR}/databases)
ENDIF(NOT WIN32)

SET(THIRDPARTY_DEPENDENT_PLUGINS)

MACRO(THIRDPARTY_DEPENDENT_PLUGINS libname targets)
    SET(alltargets ${targets})
    FOREACH (X ${ARGN})
       SET(alltargets ${alltargets} ${X})
    ENDFOREACH (X ${ARGN})
    SET(libname2 "${libname}_FOUND")
    IF(${libname2})
        IF(VISIT_STATIC)
            SET(libdir "${libname}_LIBRARY_DIR")
            SET(libs "${libname}_LIB")
            IF(${libdir})
                SET(TMP_THIRDPARTY_IO_LIBRARY_DIR ${TMP_THIRDPARTY_IO_LIBRARY_DIR} ${${libdir}})
            ENDIF(${libdir})
            IF(${libs})
                FOREACH(lib ${${libs}})
                    LIST(FIND TMP_THIRDPARTY_IO_LIB ${lib} findVar)
                    IF(findVar EQUAL -1)
                        SET(TMP_THIRDPARTY_IO_LIB ${TMP_THIRDPARTY_IO_LIB} ${lib})
                    ENDIF(findVar EQUAL -1)
                ENDFOREACH(lib)
            ENDIF(${libs})
        ENDIF(VISIT_STATIC)
        SET(THIRDPARTY_DEPENDENT_PLUGINS ${THIRDPARTY_DEPENDENT_PLUGINS} ${alltargets})
    ENDIF(${libname2})
ENDMACRO(THIRDPARTY_DEPENDENT_PLUGINS libname targets)

# These are the various targets that are optional since they depend on 3rd-
# party I/O libraries to be present. We pass the name of the 3rd party library
# to THIRDPARTY_DEPENDENT_PLUGINS and it checks whether there is a HAVE_XXXX
# symbol defined for that library. If so, the targets that use that 3rd party
# library are added to the mix.
UNSET(TMP_THIRDPARTY_IO_LIBRARY_DIR)
UNSET(TMP_THIRDPARTY_IO_LIB)
THIRDPARTY_DEPENDENT_PLUGINS(ADIOS    ADIOS)
THIRDPARTY_DEPENDENT_PLUGINS(ADVIO    Adventure)
THIRDPARTY_DEPENDENT_PLUGINS(BOXLIB2D Boxlib2D)
THIRDPARTY_DEPENDENT_PLUGINS(BOXLIB3D Boxlib3D)
THIRDPARTY_DEPENDENT_PLUGINS(CCMIO    CCM)
THIRDPARTY_DEPENDENT_PLUGINS(CFITSIO  FITS)
THIRDPARTY_DEPENDENT_PLUGINS(CGNS     CGNS)
THIRDPARTY_DEPENDENT_PLUGINS(EXODUSII Exodus TimeVaryingExodus)
THIRDPARTY_DEPENDENT_PLUGINS(FASTBIT  HDF_UC)
THIRDPARTY_DEPENDENT_PLUGINS(GDAL     GDAL)
THIRDPARTY_DEPENDENT_PLUGINS(HDF4     Cosmos ZeusMP)
THIRDPARTY_DEPENDENT_PLUGINS(HDF5     
Chombo
CosmosPP
Enzo
FLASH
GTC
H5Nimrod
M3D
M3DC1
PFLOTRAN
Pixie
SAMRAI
Tetrad
UNIC
Velodyne
Vs
XDMF
)
THIRDPARTY_DEPENDENT_PLUGINS(H5PART   H5Part)
THIRDPARTY_DEPENDENT_PLUGINS(MILI     Mili)
THIRDPARTY_DEPENDENT_PLUGINS(NETCDF   NETCDF PlasmaState S3D)
IF (NOT WIN32)
    THIRDPARTY_DEPENDENT_PLUGINS(SILO     Cale KullLite PDB Silo SAMI Vista)
ELSE (NOT WIN32)
    THIRDPARTY_DEPENDENT_PLUGINS(SILO     Cale KullLite PDB Silo SAMI)
ENDIF (NOT WIN32)
THIRDPARTY_DEPENDENT_PLUGINS(VISUS    ViSUS)

# Add ITAPS
IF(ITAPS_DIRS)
    SET(THIRDPARTY_DEPENDENT_PLUGINS ${THIRDPARTY_DEPENDENT_PLUGINS} ${ITAPS_DIRS})
    IF(VISIT_STATIC)
        FOREACH(impl ${ITAPS_DIRS})
            SET(itapslib "${impl}_LIB")
            IF(${itapslib})
                SET(TMP_THIRDPARTY_IO_LIB ${TMP_THIRDPARTY_IO_LIB} ${${itapslib}})
            ENDIF(${itapslib})
            SET(itapslibdir "${impl}_LIBRARY_DIR")
            IF(${itapslibdir})
                SET(TMP_THIRDPARTY_IO_LIBRARY_DIR ${TMP_THIRDPARTY_IO_LIBRARY_DIR} ${${itapslibdir}})
            ENDIF(${itapslibdir})
        ENDFOREACH(impl)
    ENDIF(VISIT_STATIC)
ENDIF(ITAPS_DIRS)

# Save the list of all library dirs and libs so the engine can use it
IF(VISIT_STATIC)
    SET(ALL_THIRDPARTY_IO_LIBRARY_DIR ${TMP_THIRDPARTY_IO_LIBRARY_DIR} CACHE INTERNAL "List of all I/O lib dirs")
    SET(ALL_THIRDPARTY_IO_LIB ${TMP_THIRDPARTY_IO_LIB} CACHE INTERNAL "List of all I/O libs")
ENDIF(VISIT_STATIC)

# These are our required plugins (no external dependencies)
SET(REQUIRED_DATABASE_PLUGINS
ANALYZE
ANSYS
AUXFile
AugDecomp
BOV
BOW
CEAucd
CMAT
CTRL
Curve2D
DDCMD
Dyna3D
EnSight
ExtrudedVol
Fluent
GGCM
Gadget
Image
LAMMPS
Lines
MFIX
MM5
Miranda
NASTRAN
Nek5000
OVERFLOW
OpenFOAM
PATRAN
PLOT2D
PLOT3D
PlainText
Point3D
ProteinDataBank
RAW
SAR
SAS
STL
Shapefile
Spheral
TFT
TSurf
Tecplot
VTK
WavefrontOBJ
XYZ
Xmdv
)

IF (NOT WIN32)
    LIST(APPEND REQUIRED_DATABASE_PLUGINS 
        Claw
        VASP
        Vis5D
        ZipWrapper
    )
    IF(NOT VISIT_STATIC)
        LIST(APPEND REQUIRED_DATABASE_PLUGINS SimV1 SimV1Writer SimV2)
    ENDIF(NOT VISIT_STATIC)
    SET(OPTIONAL_DATABASE_PLUGINS
        Hex
        Rect
        VLI
    )
ENDIF (NOT WIN32)


SET(MINIMAL_DATABASE_PLUGINS
Curve2D
RAW
VTK
)

# Select the plugins to build
IF(VISIT_BUILD_ALL_PLUGINS)
    SET(BUILD_PLUGINS ${REQUIRED_DATABASE_PLUGINS} ${THIRDPARTY_DEPENDENT_PLUGINS} ${OPTIONAL_DATABASE_PLUGINS})
ELSE(VISIT_BUILD_ALL_PLUGINS)
    IF(VISIT_BUILD_MINIMAL_PLUGINS)
        SET(BUILD_PLUGINS ${MINIMAL_DATABASE_PLUGINS})
    ELSE(VISIT_BUILD_MINIMAL_PLUGINS)
        SET(BUILD_PLUGINS ${REQUIRED_DATABASE_PLUGINS} ${THIRDPARTY_DEPENDENT_PLUGINS})
    ENDIF(VISIT_BUILD_MINIMAL_PLUGINS)
ENDIF(VISIT_BUILD_ALL_PLUGINS)

# Add the plugin directories
FOREACH(db ${BUILD_PLUGINS})
    ADD_SUBDIRECTORY(${db})
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
