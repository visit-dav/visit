#/what/is/the/path/to/bin/cmake

##
## Set the VISITHOME environment variable.
##
SET(VISITHOME /usr/gapps/visit)
SET(VISIT_VERBOSE_MAKEFILE TRUE)

##
## If MESA is not set, use VisIt's mesa.
##
#if test -z "$MESA"; then
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/5.0/linux_rhel3_gcc_3.2.3)
#fi

##
## If VTK is not set, use VisIt's vtk.
##
#if test -z "$VTK"; then
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0c/linux_rhel3_gcc_3.2.3/lib/vtk-5.0)
#fi

##
## If QT is not set, use VisIt's Qt.
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/4.4.3/linux_rhel3_gcc_3.4.6/bin)

##
## Use VisIt's Python.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/2.5/linux_rhel3_gcc_3.2.3)

##
## Turn off warnings for deprecated features.
##
SET(VISIT_CXX_FLAGS "-Wno-deprecated")

##
## Database reader plugin support libraries
##
###############################################################################

##
## Boxlib
##
VISIT_OPTION_DEFAULT(VISIT_BOXLIB2D_DIR /usr/gapps/visit/boxlib/linux_rhel3_gcc_3.2.3)
VISIT_OPTION_DEFAULT(VISIT_BOXLIB3D_DIR /usr/gapps/visit/boxlib/linux_rhel3_gcc_3.2.3)

##
## CFITSIO
##
VISIT_OPTION_DEFAULT(VISIT_CFITSIO_DIR ${VISITHOME}/cfitsio/3006/linux_rhel3_gcc_3.2.3)

##
## CGNS
##
VISIT_OPTION_DEFAULT(VISIT_CGNS_DIR /usr/gapps/visit/cgns/2.4/linux_rhel3_gcc_3.2.3)

##
## Exodus
##
VISIT_OPTION_DEFAULT(VISIT_EXODUSII_DIR ${VISITHOME}/exodus/4.46/linux_rhel3_gcc_3.2.3)

##
## GDAL
##
VISIT_OPTION_DEFAULT(VISIT_GDAL_DIR /usr/gapps/visit/gdal/1.3.0/linux_rhel3_gcc_3.2.3)

##
## HDF4
##
VISIT_OPTION_DEFAULT(VISIT_HDF4_DIR /usr/gapps/visit/hdf4/2.0/linux_rhel3_gcc_3.2.3)
VISIT_OPTION_DEFAULT(VISIT_HDF4_LIBDEP /usr/lib jpeg)

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR /usr/gapps/silo/hdf5/1.8.1/i686_Linux_ELsmp)
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)

##
## Mili
##
VISIT_OPTION_DEFAULT(VISIT_MILI_DIR /usr/gapps/visit/mili/current/linux_rhel3_gcc_3.2.3)

##
## netCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR /usr/gapps/visit/netcdf/3.6.0/linux_rhel3_gcc_3.2.3)

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR /usr/gapps/silo/4.7/i686_Linux_ELsmp)
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

##
## ViSUS 
##
VISIT_OPTION_DEFAULT(VISIT_VISUS_DIR /usr/gapps/visit/visus/linux_rhel3_gcc_3.2.3_new)
