#/what/is/the/path/to/bin/cmake

SET(VISITHOME /home/whitlock2/visit)
SET(VISIT_VERBOSE_MAKEFILE TRUE)
##
## Set the VISITHOME environment variable.
##

##
## Specify the location of the mesa include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/5.0/linux_gcc_3.4.6)

##
## Specify the location of the vtk include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0c/linux_gcc_3.4.6/lib/vtk-5.0)

##
## Specify the location of the qt include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/3.3.2/linux_gcc_3.4.6/bin)

##
## Specify the location of the python include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/linux_gcc_3.4.6)

##
## Database reader plugin support libraries
##
###############################################################################

##
## Boxlib
##
##DEFAULT_BOXLIB2D_INCLUDE=$VISITHOME/boxlib/linux_gcc_3.4.6/include/2D
##DEFAULT_BOXLIB2D_LIBS=$VISITHOME/boxlib/linux_gcc_3.4.6/lib
##DEFAULT_BOXLIB3D_INCLUDE=$VISITHOME/boxlib/linux_gcc_3.4.6/include/3D
##DEFAULT_BOXLIB3D_LIBS=$VISITHOME/boxlib/linux_gcc_3.4.6/lib
##
## CFITSIO
##
VISIT_OPTION_DEFAULT(VISIT_CFITSIO_DIR ${VISITHOME}/cfitsio/3006/linux_gcc_3.4.6)

##
## CGNS
##
VISIT_OPTION_DEFAULT(VISIT_CGNS_DIR ${VISITHOME}/cgns/2.4/linux_gcc_3.4.6)

##
## GDAL
##
VISIT_OPTION_DEFAULT(VISIT_GDAL_DIR ${VISITHOME}/gdal/1.3.0/linux_gcc_3.4.6)

##
## HDF4
##
VISIT_OPTION_DEFAULT(VISIT_HDF4_DIR ${VISITHOME}/hdf4/2.0/linux_gcc_3.4.6)
VISIT_OPTION_DEFAULT(VISIT_HDF4_LIBDEP /usr/lib jpeg)

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.6.5/linux_gcc_3.4.6)
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)

##
## netCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR ${VISITHOME}/netcdf/3.6.0/linux_gcc_3.4.6)

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.5.1/linux_gcc_3.4.6)
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})
