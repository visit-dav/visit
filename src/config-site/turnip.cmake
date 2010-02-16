#/apps/visit/thirdparty/visit/cmake/2.6.4/linux-x86_64_gcc-3.4/bin/cmake
##
## ./build_visit generated host.cmake
## created: Wed Jan  6 14:16:58 EST 2010
## system: Linux turnip 2.6.9-89.0.16.ELsmp #1 SMP Tue Oct 27 03:51:52 EDT 2009 x86_64 x86_64 x86_64 GNU/Linux
## by: dpn

##
## Setup VISITHOME & VISITARCH variables.
##
SET(VISITHOME /apps/visit/thirdparty/visit)
SET(VISITARCH linux-x86_64_gcc-3.4)
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

##
## Parallel Build Setup.
##
VISIT_OPTION_DEFAULT(VISIT_PARALLEL ON)
## (configured w/ mpi compiler wrapper)
#VISIT_OPTION_DEFAULT(VISIT_MPI_COMPILER g++)
VISIT_OPTION_DEFAULT(VISIT_MPI_CXX_FLAGS "-DMPICH_SKIP_MPICXX")
VISIT_OPTION_DEFAULT(VISIT_MPI_LD_FLAGS  "-L/usr/local/lib")
VISIT_OPTION_DEFAULT(VISIT_MPI_LIBS mpichcxx mpich pthread rt)

#VISIT_OPTION_DEFAULT(VISIT_MPI_LD_FLAGS  "-pthread -L/home/harrison37/apps/openmpi/1.3.3/lib -Wl,--export-dynamic ")
#VISIT_OPTION_DEFAULT(VISIT_MPI_LIBS mpi_cxx mpi open-rte open-pal dl nsl util m dl)


##############################################################
##
## Database reader plugin support libraries
##
##############################################################

##
## CGNS
##
VISIT_OPTION_DEFAULT(VISIT_CGNS_DIR ${VISITHOME}/cgns/2.4/${VISITARCH})

##
## H5Part
##
VISIT_OPTION_DEFAULT(VISIT_H5PART_DIR ${VISITHOME}/h5part/1.4.2/${VISITARCH})

##
## FastBit
##
VISIT_OPTION_DEFAULT(VISIT_FASTBIT_DIR ${VISITHOME}/fastbit/1.0.9/${VISITARCH})

##
## HDF4
##
VISIT_OPTION_DEFAULT(VISIT_HDF4_DIR ${VISITHOME}/hdf4/4.2.1/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_HDF4_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz /usr/lib jpeg)

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.8.2/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)

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
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.7/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

##
## Adios
##
VISIT_OPTION_DEFAULT(VISIT_ADIOS_DIR /apps/adios)


