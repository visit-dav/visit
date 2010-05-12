#/soft/apps/visit/cmake/2.8.0/linux-sles10-x86_64_gcc-4.1/bin/cmake
##
## ./build_visit generated host.cmake
## created: Fri May  7 08:37:44 CDT 2010
## system: Linux login1 2.6.16.60-0.42.8-smp #1 SMP Tue Dec 15 17:28:00 UTC 2009 x86_64 x86_64 x86_64 GNU/Linux
## by: childs

##
## Setup VISITHOME & VISITARCH variables.
##
SET(VISITHOME /soft/apps/visit)
SET(VISITARCH linux-sles10-x86_64_gcc-4.1)

##
## Specify the location of the mesa.
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/7.5/${VISITARCH})

##
## Specify the location of the vtk.
##
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0e/${VISITARCH}/lib/vtk-5.0/)

##
## Specify the Qt4 binary dir. 
## (qmake us used to locate & setup Qt4 dependencies)
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/4.6.1/${VISITARCH}/bin)

##
## Specify the location of the python.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/2.6.4/${VISITARCH})

##
## Compiler flags.
##
VISIT_OPTION_DEFAULT(VISIT_C_COMPILER gcc)
VISIT_OPTION_DEFAULT(VISIT_CXX_COMPILER g++)
VISIT_OPTION_DEFAULT(VISIT_C_FLAGS " -m64 -fPIC -fvisibility=hidden")
VISIT_OPTION_DEFAULT(VISIT_CXX_FLAGS " -m64 -fPIC -fvisibility=hidden")
SET(VISIT_VERBOSE_MAKEFILE TRUE)
SET(VISIT_MPI_C_FLAGS "-I/soft/apps/mpich-mx-1.2.7..7-1/include")
SET(VISIT_MPI_CXX_FLAGS "-I/soft/apps/mpich-mx-1.2.7..7-1/include")
SET(VISIT_MPI_LD_FLAGS "-L/soft/apps/mpich-mx-1.2.7..7-1/lib -L/opt/mx/lib")
SET(VISIT_MPI_LIBS mpich myriexpress)
VISIT_OPTION_DEFAULT(VISIT_NOLINK_MPI_WITH_LIBRARIES ON)
VISIT_OPTION_DEFAULT(VISIT_ICET_DIR /soft/apps/visit/icet/0.5.4/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_PARALLEL ON)

##############################################################
##
## Database reader plugin support libraries
##
##############################################################

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.8.4/${VISITARCH})

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.7.2/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

