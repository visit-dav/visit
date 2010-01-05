##
## ./build_visit generated host.cmake
## ignatius
##   SUSE 9.3 / x86_64 / gcc 3.3.5
##   Home machine of Hank Childs

##
## Setup VISITHOME & VISITARCH variables.
##
SET(VISITHOME /home/childs/visit)
SET(VISITARCH linux-x86_64_gcc-3.3)
SET(VISIT_VERBOSE_MAKEFILE TRUE)

##
## Specify the location of the mesa.
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/7.5/${VISITARCH})

##
## Specify the location of the vtk.
##
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0d/${VISITARCH}/lib/vtk-5.0/)

##
## Specify the Qt4 binary dir. 
## (qmake us used to locate & setup Qt4 dependencies)
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/4.4.3/${VISITARCH}/bin)

##
## Specify the location of the python.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/2.5/${VISITARCH})

##
## Compiler flags.
##
VISIT_OPTION_DEFAULT(VISIT_C_COMPILER gcc)
VISIT_OPTION_DEFAULT(VISIT_CXX_COMPILER g++)
VISIT_OPTION_DEFAULT(VISIT_C_FLAGS " -m64 -fPIC")
VISIT_OPTION_DEFAULT(VISIT_CXX_FLAGS " -m64 -fPIC")

##############################################################
##
## Database reader plugin support libraries
##
##############################################################

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.7/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

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

