#/share/home/00382/tg456630/visit/cmake/2.6.4/linux-x86_64_gcc/bin/cmake

# 1. YOU MUST UPGRADE TO cmake 2.6.4 !!!! This script assumes 2.6.4 but it is unknown whether it exists on your system.
##
## build_visit generated host.conf
## created: Fri Mar 20 19:01:05 CDT 2009
## system: Linux spur.tacc.utexas.edu 2.6.9-67.0.22.EL_lustre.1.6.7smp #1 SMP Mon Mar 16 15:37:03 CDT 2009 x86_64 x86_64 x86_64 GNU/Linux
## by: tg456630

##
## Set the VISITHOME environment variable.
##
SET(VISITHOME /share/home/00382/tg456630/visit)
SET(VISITARCH linux-x86_64_gcc)
SET(VISIT_VERBOSE_MAKEFILE TRUE)

##
## Specify the location of the mesa include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/7.5/linux-x86_64_gcc-3.4)

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
SET(VISIT_C_FLAGS "-m64 -fPIC -I/opt/apps/intel10_1/mvapich/1.0.1/include")
SET(VISIT_CXX_COMPILER g++)
SET(VISIT_CXX_FLAGS "-m64 -fPIC -I/opt/apps/intel10_1/mvapich/1.0.1/include")

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
##
## SZIP
##
##
## CCMIO
##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.7/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

##
## Ice-T
##
#not done -- ICET_ENABLE = "yes"
#not done -- DEFAULT_ICET_LIB = $VISITHOME/icet/0.5.4/linux-x86_64_gcc-3.4.6/lib
#not done -- DEFAULT_ICET_INCLUDE = $VISITHOME/icet/0.5.4/linux-x86_64_gcc-3.4.6/include


##
