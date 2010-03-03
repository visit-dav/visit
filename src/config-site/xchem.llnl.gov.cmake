#/usr/gapps/visit/cmake/2.6.4/linux-x86_64-gcc-4.0.2/bin/cmake
##
## ./build_visit generated host.cmake
## created: Fri Feb 26 13:16:05 PST 2010
## system: Linux xchem.llnl.gov 2.6.17-1.2142_FC4smp #1 SMP Tue Jul 11 22:59:20 EDT 2006 x86_64 x86_64 x86_64 GNU/Linux
## by: brugger

##
## Setup VISITHOME & VISITARCH variables.
##
SET(VISITHOME /usr/gapps/visit)
SET(VISITARCH linux-x86_64-gcc-4.0.2)

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
VISIT_OPTION_DEFAULT(VISIT_CXX_FLAGS "-Wno-deprecated -m64 -fPIC")
VISIT_OPTION_DEFAULT(VISIT_EXE_LINKER_FLAGS "-Wl,-rpath=/usr/lib64/tls")

##
## Add parallel arguments.
##
VISIT_OPTION_DEFAULT(VISIT_PARALLEL ON)
VISIT_OPTION_DEFAULT(VISIT_MPI_CXX_FLAGS -DMPICH_IGNORE_CXX_SEEK -I/opt/llnl/mpich2_shared/ch3-ssm/include)
VISIT_OPTION_DEFAULT(VISIT_MPI_C_FLAGS   -DMPICH_IGNORE_CXX_SEEK -I/opt/llnl/mpich2_shared/ch3-ssm/include)
VISIT_OPTION_DEFAULT(VISIT_MPI_LD_FLAGS  "-L/opt/llnl/mpich2_shared/ch3-ssm/lib -Wl,-rpath=/opt/llnl/mpich2_shared/ch3-ssm/lib -Wl,-rpath=/usr/lib64/tls")
VISIT_OPTION_DEFAULT(VISIT_MPI_LIBS     mpich rt)

##############################################################
##
## Database reader plugin support libraries
##
##############################################################

##
## Boxlib
##
VISIT_OPTION_DEFAULT(VISIT_BOXLIB2D_DIR ${VISITHOME}/boxlib/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_BOXLIB3D_DIR ${VISITHOME}/boxlib/${VISITARCH})

##
## CCMIO
##
VISIT_OPTION_DEFAULT(VISIT_CCMIO_DIR ${VISITHOME}/ccmio/2.6.1/${VISITARCH})

##
## CFITSIO
##
VISIT_OPTION_DEFAULT(VISIT_CFITSIO_DIR ${VISITHOME}/cfitsio/3006/${VISITARCH})

##
## CGNS
##
VISIT_OPTION_DEFAULT(VISIT_CGNS_DIR ${VISITHOME}/cgns/2.4/${VISITARCH})

##
## HDF4
##
VISIT_OPTION_DEFAULT(VISIT_HDF4_DIR ${VISITHOME}/hdf4/4.2.1/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_HDF4_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz /usr/lib jpeg)

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.8.4/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)

##
## Mili
##
VISIT_OPTION_DEFAULT(VISIT_MILI_DIR ${VISITHOME}/mili/1.10.0/${VISITARCH})

##
## NetCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR ${VISITHOME}/netcdf/3.6.3/${VISITARCH})

##
## SZIP
##
VISIT_OPTION_DEFAULT(VISIT_SZIP_DIR ${VISITHOME}/szip/2.1/${VISITARCH})

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.7.2/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

