#/usr/common/graphics/installs/visit_3rdparty/cmake/2.8.0/linux-x86_64_gcc-4.4/bin/cmake

##
## build_vist generated host.conf
## created: Mon Dec  8 17:15:07 PST 2008
## system: Linux nid04111 2.6.16.54-0.2.8_1.0000.3800.0-ss #1 SMP Tue Oct 21 16:10:08 PDT 2008 x86_64 x86_64 x86_64 GNU/Linux
## by: ghweber

##
## Set the VISITHOME environment variable.
##
SET(VISITHOME /usr/common/graphics/installs/visit_3rdparty)
SET(VISITARCH linux-x86_64_gcc-4.4)
SET(VISIT_VERBOSE_MAKEFILE TRUE)
VISIT_OPTION_DEFAULT(CMAKE_INSTALL_PREFIX /usr/common/graphics/visit)
VISIT_OPTION_DEFAULT(VISIT_INSTALL_THIRD_PARTY ON)
VISIT_OPTION_DEFAULT(CMAKE_BUILD_TYPE Release)

##
## Do not build Tuvok
##
VISIT_OPTION_DEFAULT(VISIT_TUVOK OFF)

##
## Specify the location of the mesa include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/7.5/${VISITARCH})

##
## Specify the location of the vtk include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0d/${VISITARCH}/lib/vtk-5.0)

##
## Specify the location of the qt include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/4.6.1/${VISITARCH}/bin)

##
## Specify the location of the python include and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/2.6.4/${VISITARCH})

## Compiler flags.
##
SET(VISIT_C_COMPILER gcc)
SET(VISIT_C_FLAGS "-march=barcelona -fPIC -DVIZSCHEMA_DECOMPOSE_DOMAINS")
SET(VISIT_MPI_C_FLAGS "-I/opt/mpt/default/xt/mpich2-gnu/include")
SET(VISIT_CXX_COMPILER g++)
SET(VISIT_CXX_FLAGS "-march=barcelona -fPIC -DVIZSCHEMA_DECOMPOSE_DOMAINS")
# Get these via CC -v
SET(VISIT_MPI_CXX_FLAGS "-I/opt/mpt/default/xt/mpich2-gnu/include")
SET(VISIT_MPI_LD_FLAGS "-L/opt/mpt/default/xt/mpich2-gnu/lib -L/opt/mpt/default/xt/pmi/lib -L/opt/mpt/default/xt/util/lib -L/opt/xt-pe/default/lib -Wl,--rpath,/opt/mpt/default/xt/mpich2-gnu/lib,--rpath,/opt/mpt/default/xt/pmi/lib,--rpath,/opt/mpt/default/xt/util/lib,--rpath,/opt/xt-pe/default/lib")
VISIT_OPTION_DEFAULT(VISIT_PARALLEL_RPATH "/opt/mpt/default/xt/mpich2-gnu/lib;/opt/mpt/default/xt/pmi/lib;/opt/mpt/default/xt/util/lib;/opt/xt-pe/default/lib")
SET(VISIT_MPI_LIBS mpich pmi alpslli alpsutil portals pthread rt)
VISIT_OPTION_DEFAULT(VISIT_PARALLEL ON)
# Options for static MPI libraries
#SET(VISIT_MPI_LIBS /opt/mpt/default/xt/mpich2-gnu/lib/libmpich.a /opt/mpt/default/xt/pmi/lib/libpmi.a /opt/mpt/default/xt/util/lib/libalpslli.a /opt/mpt/default/xt/util/lib/libalpsutil.a /opt/xt-service/default/lib/snos64/libportals.a pthread rt)
#VISIT_OPTION_DEFAULT(VISIT_NOLINK_MPI_WITH_LIBRARIES ON)

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
## CFITSIO
##
VISIT_OPTION_DEFAULT(VISIT_CFITSIO_DIR ${VISITHOME}/cfitsio/3006/${VISITARCH})

##
## CGNS
##
VISIT_OPTION_DEFAULT(VISIT_CGNS_DIR ${VISITHOME}/cgns/2.4/${VISITARCH})

##
## Exodus
##
#VISIT_OPTION_DEFAULT(VISIT_EXODUSII_DIR ${VISITHOME}/exodus/4.46/${VISITARCH})

##
## GDAL
##
VISIT_OPTION_DEFAULT(VISIT_GDAL_DIR ${VISITHOME}/gdal/1.6.2/${VISITARCH})

##
## H5Part
##
VISIT_OPTION_DEFAULT(VISIT_H5PART_DIR ${VISITHOME}/h5part/1.6.0/${VISITARCH})

##
## FastBit
##
VISIT_OPTION_DEFAULT(VISIT_FASTBIT_DIR ${VISITHOME}/fastbit/1.0.9/${VISITARCH})

##
## HDF4
##
VISIT_OPTION_DEFAULT(VISIT_HDF4_DIR ${VISITHOME}/hdf4/4.2.1/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_HDF4_LIBDEP /usr/lib jpeg)

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.8.4/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.7.2/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

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
SET(SZIP_DIR ${VISITHOME}/szip/2.1/${VISITARCH})

##
## CCMIO
##
VISIT_OPTION_DEFAULT(VISIT_CCMIO_DIR ${VISITHOME}/ccmio/2.6.1/${VISITARCH})

##
## Ice-T
##
VISIT_OPTION_DEFAULT(VISIT_ICET_DIR ${VISITHOME}/icet/0.5.4/${VISITARCH})

