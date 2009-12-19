#/apps/visit/cmake/2.6.4/linux-x86_64_gcc-4.2.4/bin/cmake

# 1. YOU MUST UPGRADE TO cmake 2.6.4 !!!! This script assumes 2.6.4 but it is unknown whether it exists on your system.
# 2. This script does not use ${VISITARCH}. You should rerun build_visit to produce more uniformly installed libraries and a better config file.

## config-site file for pion.ornl.gov,
##    an 8-Core, 8GB Xeon workstation

## -fPIC
## -Wno-deprecated because VTK's header files are 
SET(VISIT_C_FLAGS "-fPIC")
SET(VISIT_CXX_FLAGS "-fPIC -Wall -Wno-deprecated")

##
## Set the VISITHOME environment variable.
##
SET(VISITHOME /apps/visit)
SET(VISIT_VERBOSE_MAKEFILE TRUE)

##
## Specify the location of the mesa include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/7.5/linux-x86_64_gcc-4.2.4)

##
## Specify the location of the vtk include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0c/linux-x86_64_gcc-4.2.4/lib/vtk-5.0)

##
## Specify the location of cmake.
##


##
## Specify the location of the qt include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN /usr/bin)

##
## Specify the location of the python include and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/2.5/linux-x86_64_gcc-4.2.3)

# use openmpi for parallel
SET(VISIT_MPI_CXX_FLAGS "-DOMPI_SKIP_MPICXX -I/usr/lib/openmpi/include")
SET(VISIT_MPI_LIBS mpi)

##
## If enable_viewer_mesa_stub in not set, then disable it.
##
#if test -z "$enable_viewer_mesa_stub"; then
#not done -- enable_viewer_mesa_stub = no
#fi

##
## Database reader plugin support libraries
##
##############################################################

##
## Boxlib
##
VISIT_OPTION_DEFAULT(VISIT_BOXLIB2D_DIR ${VISITHOME}/boxlib/linux-x86_64_gcc-4.2.4)
VISIT_OPTION_DEFAULT(VISIT_BOXLIB3D_DIR ${VISITHOME}/boxlib/linux-x86_64_gcc-4.2.4)

##
## CFITSIO
##
VISIT_OPTION_DEFAULT(VISIT_CFITSIO_DIR ${VISITHOME}/cfitsio/3006/linux-x86_64_gcc-4.2.3)

##
## CGNS
##
VISIT_OPTION_DEFAULT(VISIT_CGNS_DIR ${VISITHOME}/cgns/2.4/linux-x86_64_gcc-4.2.3)

##
## Exodus
##
VISIT_OPTION_DEFAULT(VISIT_EXODUSII_DIR ${VISITHOME}/exodus/4.46/linux-x86_64_gcc-4.2.3)

##
## GDAL
##
VISIT_OPTION_DEFAULT(VISIT_GDAL_DIR ${VISITHOME}/gdal/1.3.2/linux-x86_64_gcc-4.2.3)

##
## H5Part
##
VISIT_OPTION_DEFAULT(VISIT_H5PART_DIR ${VISITHOME}/h5part/1.4.2/linux-x86_64_gcc-4.2.4)

##
## HDF4
##
VISIT_OPTION_DEFAULT(VISIT_HDF4_DIR ${VISITHOME}/hdf4/4.2.1/linux-x86_64_gcc-4.2.3)
VISIT_OPTION_DEFAULT(VISIT_HDF4_LIBDEP /usr/lib jpeg)

##
## SZIP
##
SET(SZIP_DIR ${VISITHOME}/szip/2.1/linux-x86_64_gcc-4.2.4)

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.8.2/linux-x86_64_gcc-4.2.4)
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)

##
## Mili
##
VISIT_OPTION_DEFAULT(VISIT_MILI_DIR ${VISITHOME}/mili/1.10.0/linux-x86_64_gcc-4.2.3)

##
## NetCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR ${VISITHOME}/netcdf/3.6.0/linux-x86_64_gcc-4.2.3)

##
## CCMIO
##
VISIT_OPTION_DEFAULT(VISIT_CCMIO_DIR ${VISITHOME}/ccmio/2.6.1/linux-x86_64_gcc-4.2.3)

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.7/linux-x86_64_gcc-4.2.4)
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

##
## FastBit
##
VISIT_OPTION_DEFAULT(VISIT_FASTBIT_DIR ${VISITHOME}/fastbit/1.0.9/linux-x86_64_gcc-4.2.4)
