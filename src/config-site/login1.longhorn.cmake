#/home/00382/tg456630/visit/cmake/2.6.4/linux-x86_64_gcc-4.1/bin/cmake
##
## ./build_visit generated host.cmake
## created: Tue Feb  9 12:58:36 CST 2010
## system: Linux login1.longhorn 2.6.18-128.7.1_lustre_perfctr_TACC #3 SMP Wed Oct 28 10:48:56 CDT 2009 x86_64 x86_64 x86_64 GNU/Linux
## by: tg456630

##
## Setup VISITHOME & VISITARCH variables.
##
SET(VISITHOME /home/00382/tg456630/visit)
SET(VISITARCH linux-x86_64_gcc-4.1)

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
VISIT_OPTION_DEFAULT(VISIT_C_COMPILER mpicc)
VISIT_OPTION_DEFAULT(VISIT_CXX_COMPILER mpiCC)
VISIT_OPTION_DEFAULT(VISIT_C_FLAGS " -m64 -O2")
VISIT_OPTION_DEFAULT(VISIT_CXX_FLAGS " -m64 -O2")
VISIT_OPTION_DEFAULT(VISIT_MPI_LD_FLAGS "-L/opt/apps/intel11_1/openmpi/1.3.3/lib")
VISIT_OPTION_DEFAULT(VISIT_MPI_LIBS mpi_cxx mpi)
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

##
## Ice-T
##
#VISIT_OPTION_DEFAULT(VISIT_ICET_DIR ${VISITHOME}/icet/0.5.4/${VISITARCH})
