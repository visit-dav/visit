#/autonfs/home/brugger/visit/cmake/2.8.0/linux-i686-gcc-3.4.6/bin/cmake
##
## ./build_visit generated host.cmake
## created: Tue Mar 23 12:31:50 CDT 2010
## system: Linux ellipse.uchicago.edu 2.6.9-89.0.7.ELsmp #1 SMP Wed Aug 5 14:12:18 EDT 2009 i686 i686 i386 GNU/Linux
## by: brugger

##
## Setup VISITHOME & VISITARCH variables.
##
SET(VISITHOME /autonfs/home/brugger/visit)
SET(VISITARCH linux-i686-gcc-3.4.6)

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
VISIT_OPTION_DEFAULT(VISIT_C_FLAGS "-O2")
VISIT_OPTION_DEFAULT(VISIT_CXX_FLAGS "-O2")

##
## Add parallel arguments.
##
VISIT_OPTION_DEFAULT(VISIT_PARALLEL ON)
VISIT_OPTION_DEFAULT(VISIT_MPI_CXX_FLAGS -I/soft/mpich-gm-1.2.7p1.2-intel-r1/include)
VISIT_OPTION_DEFAULT(VISIT_MPI_C_FLAGS   -I/soft/mpich-gm-1.2.7p1.2-intel-r1/include)
VISIT_OPTION_DEFAULT(VISIT_MPI_LD_FLAGS  "-L/soft/mpich-gm-1.2.7p1.2-intel-r1/lib -Wl,-rpath,/soft/gm-2.1.29_Linux-r1/lib -L/soft/gm-2.1.29_Linux-r1/lib -L/soft/pvfs-2.6.2-r1/lib -L/soft/pvfs-1.6.3-r1/lib -L/soft/intel-c-9.1.042-f-9.1.036-r1/lib")
VISIT_OPTION_DEFAULT(VISIT_MPI_LIBS     mpich gm pvfs2 pvfs pthread rt irc)

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
## H5Part
##
VISIT_OPTION_DEFAULT(VISIT_H5PART_DIR ${VISITHOME}/h5part/1.6.0/${VISITARCH})

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

