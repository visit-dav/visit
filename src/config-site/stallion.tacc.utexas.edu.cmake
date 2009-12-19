#/work/pnav/visit/visit/cmake/2.6.4/linux-x86_64_gcc-4.2.4/bin/cmake

# 1. YOU MUST UPGRADE TO cmake 2.6.4 !!!! This script assumes 2.6.4 but it is unknown whether it exists on your system.
##
## build_visit generated host.conf
## created: Tue May  5 10:47:32 CDT 2009
## system: Linux stallion 2.6.24-23-generic #1 SMP Wed Apr 1 21:43:24 UTC 2009 x86_64 GNU/Linux
## by: pnav

##
## Set the VISITHOME environment variable.
##
SET(VISITHOME /work/pnav/visit/visit)
SET(VISITARCH linux-x86_64_gcc-4.2.4)
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
#VISIT_QT_BIN=$VISITHOME/qt/3.3.8/$VISITARCH/bin
#QT_INCLUDE=$VISITHOME/qt/3.3.8/$VISITARCH/include
#QT_LIB=$VISITHOME/qt/3.3.8/$VISITARCH/lib
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/4.5.1/${VISITARCH}/bin)

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
VISIT_OPTION_DEFAULT(VISIT_CFITSIO_DIR ${VISITHOME}/cfitsio/3006/${VISITARCH})

##
## CGNS
##
VISIT_OPTION_DEFAULT(VISIT_CGNS_DIR ${VISITHOME}/cgns/2.4/${VISITARCH})

##
## Exodus
##
VISIT_OPTION_DEFAULT(VISIT_EXODUSII_DIR ${VISITHOME}/exodus/4.46/${VISITARCH})

##
## GDAL
##

##
## H5Part
##

##
## FastBit
##

##
## HDF4
##

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.8.1/${VISITARCH})
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
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.7/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

##
