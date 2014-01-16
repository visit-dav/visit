#/usr/common/graphics/installs/visit_3rdparty/cmake/2.8.8/linux-x86_64_gcc-4.7/bin/cmake
##
## /global/common/shared/graphics/installs/visit_svn/2.6RC_trunk/src/svn_bin/build_visit generated host.cmake
## created: Fri Jan 11 10:38:51 PST 2013
## system: Linux edison02 3.0.38-0.5-default #1 SMP Fri Aug 3 09:02:17 UTC 2012 (358029e) x86_64 x86_64 x86_64 GNU/Linux
## by: ghweber

## FIXME: Update to VTK 6 and install VisIt 2.7 on Edison

##
## Setup VISITHOME & VISITARCH variables.
##
SET(VISITHOME /usr/common/graphics/installs/visit_3rdparty)
SET(VISITARCH linux-x86_64_gcc-4.7)
#SET(VISIT_VERBOSE_MAKEFILE TRUE)
VISIT_OPTION_DEFAULT(VISIT_INSTALL_PROFILES_TO_HOSTS "nersc" TYPE STRING)
VISIT_OPTION_DEFAULT(CMAKE_INSTALL_PREFIX /usr/common/graphics/visit)
VISIT_OPTION_DEFAULT(VISIT_INSTALL_THIRD_PARTY ON)
VISIT_OPTION_DEFAULT(CMAKE_BUILD_TYPE Release)


##
## Compiler flags.
##
VISIT_OPTION_DEFAULT(VISIT_C_COMPILER gcc TYPE FILEPATH)
VISIT_OPTION_DEFAULT(VISIT_CXX_COMPILER g++ TYPE FILEPATH)
VISIT_OPTION_DEFAULT(VISIT_C_FLAGS " -m64 -fvisibility=hidden" TYPE STRING)
VISIT_OPTION_DEFAULT(VISIT_CXX_FLAGS " -m64 -fvisibility=hidden" TYPE STRING)
VISIT_OPTION_DEFAULT(VISIT_C_FLAGS "-m64 -fvisibility=hidden -DVIZSCHEMA_DECOMPOSE_DOMAINS" TYPE STRING)
VISIT_OPTION_DEFAULT(VISIT_MPI_C_FLAGS "-I/opt/cray/mpt/default/gni/mpich2-gnu/47/include" TYPE STRING)
VISIT_OPTION_DEFAULT(VISIT_CXX_FLAGS "-m64 -fvisibility=hidden -DVIZSCHEMA_DECOMPOSE_DOMAINS")
#SET(VISIT_EXE_LINKER_FLAGS "-Wl,--rpath,/opt/gcc/4.3.3/snos/lib64")
# Get these via CC -v
VISIT_OPTION_DEFAULT(VISIT_MPI_CXX_FLAGS "-I/opt/cray/mpt/default/gni/mpich2-gnu/47/include" TYPE STRING)
VISIT_OPTION_DEFAULT(VISIT_MPI_LD_FLAGS "-L/opt/cray/mpt/default/gni/mpich2-gnu/47/lib -L/opt/cray/pmi/default/lib64 -L/opt/cray/alps/default/lib64" TYPE STRING)
VISIT_OPTION_DEFAULT(VISIT_PARALLEL_RPATH "/opt/cray/mpt/default/gni/mpich2-gnu/47/lib;/opt/cray/pmi/default/lib64;/opt/cray/alps/default/lib64" TYPE STRING)
VISIT_OPTION_DEFAULT(VISIT_MPI_LIBS mpich pmi alpslli alpsutil pthread rt)
VISIT_OPTION_DEFAULT(VISIT_PARALLEL ON)
VISIT_OPTION_DEFAULT(VISIT_CREATE_SOCKET_RELAY_EXECUTABLE ON)

##############################################################
##
## Database reader plugin support libraries
##
##############################################################
##

##
## Mesa
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/7.8.2/${VISITARCH})

##
## Python
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR /usr/common/graphics/installs/visit_3rdparty/python/2.6.4/linux-x86_64_gcc-4.7)

##
## Qt
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/4.8.3/${VISITARCH}/bin)

##
## R
##
VISIT_OPTION_DEFAULT(VISIT_R_DIR ${VISITHOME}/R/2.13.2/${VISITARCH}/lib/R)

##
## VTK
##
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk-r/5.8.0.a/${VISITARCH})

## The HDF4, HDF5 and NetCDF libraries must be first so that
## their libdeps are defined for any plugins that need them.
##
## For libraries with LIBDEP settings, order matters.
## Libraries with LIBDEP settings that depend on other
## Library's LIBDEP settings must come after them.

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.8.7/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz /usr/lib z TYPE STRING)

##
## NetCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR ${VISITHOME}/netcdf/4.1.1/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_NETCDF_LIBDEP HDF5_LIBRARY_DIR hdf5_hl HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP} TYPE STRING)

##
## AdvIO
##
VISIT_OPTION_DEFAULT(VISIT_ADVIO_DIR ${VISITHOME}/AdvIO/1.2/${VISITARCH}/)

##
## Boxlib
##
VISIT_OPTION_DEFAULT(VISIT_BOXLIB_DIR ${VISITHOME}/boxlib/1.0.7/${VISITARCH})

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
VISIT_OPTION_DEFAULT(VISIT_CGNS_DIR ${VISITHOME}/cgns/3.0.8/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_CGNS_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP} TYPE STRING)

##
## FastBit
##
VISIT_OPTION_DEFAULT(VISIT_FASTBIT_DIR ${VISITHOME}/fastbit/1.2.0/${VISITARCH})

##
## GDAL
##
VISIT_OPTION_DEFAULT(VISIT_GDAL_DIR ${VISITHOME}/gdal/1.7.1/${VISITARCH})

##
## H5Part
##
VISIT_OPTION_DEFAULT(VISIT_H5PART_DIR ${VISITHOME}/h5part/1.6.6/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_H5PART_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP} TYPE STRING)

##
## PySide
##
VISIT_OPTION_DEFAULT(VISIT_PYSIDE_DIR ${VISITHOME}/pyside/1.1.1/${VISITARCH}/)

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.9/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP} TYPE STRING)

##
## SZIP
##
VISIT_OPTION_DEFAULT(VISIT_SZIP_DIR ${VISITHOME}/szip/2.1/${VISITARCH})

##
## Xdmf
##
VISIT_OPTION_DEFAULT(VISIT_XDMF_DIR ${VISITHOME}/Xdmf/2.1.1/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_XDMF_LIBDEP HDF5_LIBRARY_DIR hdf5 VTK_LIBRARY_DIRS vtklibxml2 TYPE STRING)
