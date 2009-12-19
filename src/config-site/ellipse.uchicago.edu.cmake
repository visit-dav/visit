#/what/is/the/path/to/bin/cmake

##
## Set the VISITHOME environment variable.
##
SET(VISITHOME /autonfs/home/brugger/visit)
SET(VISIT_VERBOSE_MAKEFILE TRUE)

##
## Use the icc 9.1 compiler.
##
#if test -z "$CXX"; then
SET(VISIT_C_COMPILER icc)
SET(VISIT_CXX_COMPILER icpc)
#fi
#if test -z "$CC"; then
#fi

##
## Specify the location of the mesa include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/5.0/linux_rhel3_gcc_3.4.6)

##
## Specify the location of the vtk include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0c/linux_rhel3_gcc_3.4.6/lib/vtk-5.0)

##
## Specify the location of the qt include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/3.3.2/linux_rhel3_gcc_3.4.6/bin)

##
## Specify the location of the python include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/linux_rhel3_gcc_3.4.6)

##
## Add parallel arguments.
##
SET(VISIT_MPI_CXX_FLAGS "-I/soft/mpich-gm-1.2.7p1.2-intel-r1/include")

##
## Database reader plugin support libraries
##
###############################################################################

##
## Boxlib
##
VISIT_OPTION_DEFAULT(VISIT_BOXLIB2D_DIR ${VISITHOME}/boxlib/linux_rhel3_gcc_3.4.6)
VISIT_OPTION_DEFAULT(VISIT_BOXLIB3D_DIR ${VISITHOME}/boxlib/linux_rhel3_gcc_3.4.6)

##
## CFITSIO
##
VISIT_OPTION_DEFAULT(VISIT_CFITSIO_DIR ${VISITHOME}/cfitsio/3006/linux_rhel3_gcc_3.4.6)

##
## CGNS
##
VISIT_OPTION_DEFAULT(VISIT_CGNS_DIR ${VISITHOME}/cgns/2.4/linux_rhel3_gcc_3.4.6)

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.8.1/linux_rhel3_gcc_3.4.6)
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)

##
## GDAL
##
VISIT_OPTION_DEFAULT(VISIT_GDAL_DIR ${VISITHOME}/gdal/1.3.2/linux_rhel3_gcc_3.4.6)

##
## netCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR ${VISITHOME}/netcdf/3.6.0/linux_rhel3_gcc_3.4.6)

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.6.2/linux_rhel3_gcc_3.4.6)
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})
