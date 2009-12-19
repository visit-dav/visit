#/what/is/the/path/to/bin/cmake

##
## build_vist generated host.conf
## created: Tue Oct  9 17:08:36 EDT 2007
## system: Linux turnip 2.6.9-55.0.9.ELsmp #1 SMP Tue Sep 25 02:16:15 EDT 2007 x86_64 x86_64 x86_64 GNU/Linux
## by: dpn

##
## Set the VISITHOME environment variable.
##
SET(VISITHOME /apps/visit/thirdparty/visit)
SET(VISIT_VERBOSE_MAKEFILE TRUE)

##
## Specify the location of the mesa include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/5.0/linux-x86_64_gcc-3.4.6)

##
## Specify the location of the vtk include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0c/linux-x86_64_gcc-3.4.6/lib/vtk-5.0)

##
## Specify the location of the qt include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/4.4.3/linux-x86_64_gcc-3.4.6/bin)

##
## Specify the location of the python include and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/2.5/linux-x86_64_gcc-3.4.6)

##
## If enable_viewer_mesa_stub in not set, then disable it.
##
#if test -z "$enable_viewer_mesa_stub"; then
#not done -- enable_viewer_mesa_stub = no
#fi

## Compiler flags.
##
SET(VISIT_C_COMPILER gcc)
SET(VISIT_C_FLAGS "-g -m64 -fPIC")
SET(VISIT_CXX_COMPILER g++)
SET(VISIT_CXX_FLAGS "-g -m64 -Wno-deprecated -fPIC")
SET(VISIT_MPI_CXX_FLAGS "-DMPICH_SKIP_MPICXX")

## MPI
SET(VISIT_MPI_LD_FLAGS "-L/usr/local/lib")
SET(VISIT_MPI_LIBS mpich)

##
## Database reader plugin support libraries
##
##############################################################

##
## ADIOS
##
VISIT_OPTION_DEFAULT(VISIT_ADIOS_DIR /apps/adios)

##
## Boxlib
##
VISIT_OPTION_DEFAULT(VISIT_BOXLIB2D_DIR ${VISITHOME}/boxlib/linux-x86_64_gcc-3.4.6)
VISIT_OPTION_DEFAULT(VISIT_BOXLIB3D_DIR ${VISITHOME}/boxlib/linux-x86_64_gcc-3.4.6)

##
## CFITSIO
##
VISIT_OPTION_DEFAULT(VISIT_CFITSIO_DIR ${VISITHOME}/cfitsio/3006/linux-x86_64_gcc-3.4.6)

##
## CGNS
##
VISIT_OPTION_DEFAULT(VISIT_CGNS_DIR ${VISITHOME}/cgns/2.4/linux-x86_64_gcc-3.4.6)

##
## Exodus
##
VISIT_OPTION_DEFAULT(VISIT_EXODUSII_DIR ${VISITHOME}/exodus/4.46/linux-x86_64_gcc-3.4.6)

##
## GDAL
##
VISIT_OPTION_DEFAULT(VISIT_GDAL_DIR ${VISITHOME}/gdal/1.3.2/linux-x86_64_gcc-3.4.6)

##
## H5Part
##
VISIT_OPTION_DEFAULT(VISIT_H5PART_DIR ${VISITHOME}/h5part/1.4.0/linux-x86_64_gcc-3.4.6)

##
## HDF4
##
VISIT_OPTION_DEFAULT(VISIT_HDF4_DIR ${VISITHOME}/hdf4/4.2.1/linux-x86_64_gcc-3.4.6)
VISIT_OPTION_DEFAULT(VISIT_HDF4_LIBDEP /usr/lib jpeg)

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.6.5/linux-x86_64_gcc-3.4.6)
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)

##
## Mili
##

##
## NetCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR ${VISITHOME}/netcdf/3.6.0/linux-x86_64_gcc-3.4.6)

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.6.1/linux-x86_64_gcc-3.4.6)
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})


