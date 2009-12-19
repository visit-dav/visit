#/what/is/the/path/to/bin/cmake

##
## Set the VISITHOME environment variable.
##
SET(VISITHOME /Users/whitlocb/VisItBuild/visit)
SET(VISIT_VERBOSE_MAKEFILE TRUE)

##
## Use VisIt's mesa.
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa)

##
## Use VisIt's vtk.
##
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/lib/vtk-5.0)

##
## If QT is not set, use VisIt's Qt.
##
#if test -z "$VISIT_QT_BIN"; then
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/bin)
#fi
#if test -z "$QT_INCLUDE"; then
#fi
#if test -z "$QT_LIB"; then
#fi

##
## Set the PYDIR variable that tells us where to find Python.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python)

##
## Database reader plugin support libraries
##
###############################################################################

##
## CFITSIO
##
VISIT_OPTION_DEFAULT(VISIT_CFITSIO_DIR ${VISITHOME}/cfitsio)

##
## CGNS
##
VISIT_OPTION_DEFAULT(VISIT_CGNS_DIR ${VISITHOME}/cgns/2.4)

##
## Exodus
##
VISIT_OPTION_DEFAULT(VISIT_EXODUSII_DIR ${VISITHOME}/exodusII)

##
## GDAL
##
VISIT_OPTION_DEFAULT(VISIT_GDAL_DIR ${VISITHOME}/gdal/1.3.0)

##
## HDF4
##
VISIT_OPTION_DEFAULT(VISIT_HDF4_DIR ${VISITHOME}/hdf4)
VISIT_OPTION_DEFAULT(VISIT_HDF4_LIBDEP /usr/lib jpeg)

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5)
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)

##
## Mili
##
VISIT_OPTION_DEFAULT(VISIT_MILI_DIR ${VISITHOME}/mili)

##
## NETCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR ${VISITHOME}/netcdf/3.6.0)

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo)
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})
