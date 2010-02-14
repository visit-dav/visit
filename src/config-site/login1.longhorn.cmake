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
VISIT_OPTION_DEFAULT(VISIT_C_COMPILER gcc)
VISIT_OPTION_DEFAULT(VISIT_CXX_COMPILER g++)
VISIT_OPTION_DEFAULT(VISIT_C_FLAGS " -m64 -fPIC -m64 -fPIC -DUSE_STDARG -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_UNISTD_H=1 -DHAVE_STDARG_H=1 -DUSE_STDARG=1 -DMALLOC_RET_VOID=1 -I/opt/apps/intel11_1/mvapich2/1.4/include")
VISIT_OPTION_DEFAULT(VISIT_CXX_FLAGS " -m64 -fPIC -m64 -fPIC -DUSE_STDARG -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_UNISTD_H=1 -DHAVE_STDARG_H=1 -DUSE_STDARG=1 -DMALLOC_RET_VOID=1 -I/opt/apps/intel11_1/mvapich2/1.4/include")
VISIT_OPTION_DEFAULT(VISIT_MPI_LD_FLAGS "-Wl,-rpath,/opt/apps/intel/11.1/lib/intel64 -Wl,-rpath,/opt/apps/intel/11.1/lib/intel64 -L/opt/ofed/lib64/ -L/opt/apps/intel11_1/mvapich2/1.4/lib -Wl,-rpath -Wl,/opt/apps/intel11_1/mvapich2/1.4/lib -L/opt/apps/intel/11.1/lib/intel64/")
VISIT_OPTION_DEFAULT(VISIT_MPI_LIBS imf mpichcxx mpich pthread rdmacm ibverbs ibumad rt)
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
VISIT_OPTION_DEFAULT(VISIT_ICET_DIR ${VISITHOME}/icet/0.5.4/${VISITARCH})
