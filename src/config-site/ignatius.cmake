#/home/childs/build_visit/cmake-2.6.4/bin/cmake

# 1. YOU MUST UPGRADE TO cmake 2.6.4 !!!! This script assumes 2.6.4 but it is unknown whether it exists on your system.
##
## ignatius
##   SUSE 9.3 / x86_64 / gcc 3.3.5
##   Home machine of Hank Childs
##

SET(VISITHOME /home/childs/visit)
SET(VISITARCH linux-x86_64_gcc-3.3)
SET(VISIT_VERBOSE_MAKEFILE TRUE)

##
## Set the VISITHOME environment variable.
##

##
## Specify the location of the mesa include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/7.5/linux-x86_64_gcc-3.3)

##
## Specify the location of the vtk include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0c/linux_gcc-3.3.5/lib/vtk-5.0)

##
## Specify the location of the qt include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/4.4.3/linux-x86_64_gcc-3.3.5/bin)

##
## Specify the location of the python include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/linux_gcc-3.3.5)

##
## Tell VisIt where to find cmake in case we want to build slivr.
##


##
## Database reader plugin support libraries
##
###############################################################################

##
## Boxlib
##
VISIT_OPTION_DEFAULT(VISIT_BOXLIB2D_DIR ${VISITHOME}/boxlib/linux_gcc-3.3.5)
VISIT_OPTION_DEFAULT(VISIT_BOXLIB3D_DIR ${VISITHOME}/boxlib/linux_gcc-3.3.5)

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.8.2/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)


##
## Silo
##
SET(VISIT_C_COMPILER gcc)
SET(VISIT_C_FLAGS "-O2 -m64 -fPIC")
SET(VISIT_CXX_COMPILER g++)
SET(VISIT_CXX_FLAGS "-O2 -m64 -fPIC")

