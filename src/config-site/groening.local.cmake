#/scratch/build/visit_3rdparty/cmake/2.8.0/i386-apple-darwin9_gcc-4.0/bin/cmake
##
## visit_svn/2.0RC_trunk//src/svn_bin/build_visit generated host.cmake
## created: Thu May 20 08:53:22 PDT 2010
## system: Darwin groening.local 9.8.0 Darwin Kernel Version 9.8.0: Wed Jul 15 16:55:01 PDT 2009; root:xnu-1228.15.4~1/RELEASE_I386 i386
## by: ghweber

##
## Setup VISITHOME & VISITARCH variables.
##
SET(VISITHOME /scratch/build/visit_3rdparty)
SET(VISITARCH i386-apple-darwin9_gcc-4.0)
SET(VISIT_VERBOSE_MAKEFILE TRUE)
VISIT_OPTION_DEFAULT(CMAKE_INSTALL_PREFIX /usr/local/visit)
VISIT_OPTION_DEFAULT(CMAKE_BUILD_TYPE Release)

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
VISIT_OPTION_DEFAULT(VISIT_C_FLAGS "-O2 -fno-common -fexceptions -fvisibility=hidden")
VISIT_OPTION_DEFAULT(VISIT_CXX_FLAGS "-O2 -fno-common -fexceptions -fvisibility=hidden")

##
##  Parallel flags
##
SET(VISIT_MPI_C_FLAGS "-D_REENTRANT -DOMPI_SKIP_MPICXX")
SET(VISIT_MPI_CXX_FLAGS "-D_REENTRANT -DOMPI_SKIP_MPICXX")
SET(VISIT_PARALLEL_LINKER_FLAGS "-Wl,-undefined,dynamic_lookup")
SET(VISIT_MPI_LIBS "-Wl,-u,_munmap -Wl,-multiply_defined,suppress -lmpi -lopen-rte -lopen-pal -lutil")
VISIT_OPTION_DEFAULT(VISIT_PARALLEL ON)


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
## GDAL
##
#VISIT_OPTION_DEFAULT(VISIT_GDAL_DIR ${VISITHOME}/gdal/1.7.1/${VISITARCH})

##
## FastBit
##
VISIT_OPTION_DEFAULT(VISIT_FASTBIT_DIR ${VISITHOME}/fastbit/1.0.9/${VISITARCH})

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

##
## Xdmf
##
VISIT_OPTION_DEFAULT(VISIT_XDMF_DIR ${VISITHOME}/Xdmf/2.1/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_XDMF_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

