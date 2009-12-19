#/home/ahern/work/VisIt/svn/trunk/visit/cmake/2.6.4/linux-x86_64_gcc-4.1.2/bin/cmake

# 1. YOU MUST UPGRADE TO cmake 2.6.4 !!!! This script assumes 2.6.4 but it is unknown whether it exists on your system.
##
## build_visit generated host.conf
## created: Tue Dec 30 15:20:36 EST 2008
## system: Linux lije.ornl.gov 2.6.18-92.1.18.el5 #1 SMP Wed Nov 5 09:00:19 EST 2008 x86_64 x86_64 x86_64 GNU/Linux
## by: ahern

##
## Set the VISITHOME environment variable.
##
SET(VISITHOME /home/ahern/work/VisIt/svn/trunk/visit)
SET(VISITARCH linux-x86_64_gcc-4.1.2)
SET(VISIT_VERBOSE_MAKEFILE TRUE)

##
## Specify the location of the mesa include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/5.0/${VISITARCH})

##
## Specify the location of the vtk include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0c/${VISITARCH}/lib/vtk-5.0)

##
## Specify the location of cmake.
##


##
## Specify the location of the qt include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/4.4.3/${VISITARCH}/bin)

##
## Specify the location of the python include and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/2.5/${VISITARCH})


## Compiler flags.
##
SET(VISIT_C_COMPILER gcc)
SET(VISIT_C_FLAGS "-m64 -fPIC")
SET(VISIT_CXX_COMPILER g++)
SET(VISIT_CXX_FLAGS "-m64 -fPIC")

##
## Database reader plugin support libraries
##
##############################################################

##
## Boxlib
##

##
## CFITSIO
##

##
## CGNS
##

##
## Exodus
##

##
## GDAL
##

##
## H5Part
##

##
## HDF4
##

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.6.5/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)

##
## Mili
##

##
## NetCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR ${VISITHOME}/netcdf/3.6.0/${VISITARCH})
##
## SZIP
##
##
## CCMIO
##
##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.6.2/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

##
