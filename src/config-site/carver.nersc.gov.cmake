#/global/common/carver_sl6/usg/cmake/3.0.1/bin/cmake
##
## it generated host.cmake
## created: Wed Jan 15 18:56:09 PST 2014
## system: Linux cvrsvc01 2.6.18-238.12.1.el5.nersc #1 SMP Mon Jul 11 16:15:58 PDT 2011 x86_64 x86_64 x86_64 GNU/Linux
## updated: Fri Aug 29 17:41:26 PDT 2014
## system: Linux cvrsvc03 2.6.32-358.18.1.el6.nersc.x86_64 #1 SMP Wed Aug 28 02:17:42 PDT 2013 x86_64 x86_64 x86_64 GNU/Linux
## by: ghweber

##
## Setup VISITHOME & VISITARCH variables.
##
SET(VISITHOME /usr/common/graphics/installs/visit_3rdparty)
SET(VISITARCH linux-x86_64_gcc-4.4)

VISIT_OPTION_DEFAULT(VISIT_INSTALL_PROFILES_TO_HOSTS "nersc" TYPE STRING)
VISIT_OPTION_DEFAULT(CMAKE_INSTALL_PREFIX /usr/common/graphics/visit)
VISIT_OPTION_DEFAULT(VISIT_INSTALL_THIRD_PARTY ON)

##
## Compiler flags.
##
VISIT_OPTION_DEFAULT(VISIT_C_COMPILER gcc TYPE FILEPATH)
VISIT_OPTION_DEFAULT(VISIT_CXX_COMPILER g++ TYPE FILEPATH)
VISIT_OPTION_DEFAULT(VISIT_C_FLAGS "-m64 -fPIC -fvisibility=hidden" TYPE STRING)
VISIT_OPTION_DEFAULT(VISIT_CXX_FLAGS "-m64 -fPIC -fvisibility=hidden" TYPE STRING)

##
## Parallel Options
##
VISIT_OPTION_DEFAULT(VISIT_MPI_C_FLAGS "-DOMPI_SKIP_MPICXX -I/usr/common/usg/openmpi/1.8.1/gcc4.8.3/include/openmpi/opal/mca/hwloc/hwloc172/hwloc/include -I/usr/common/usg/openmpi/1.8.1/gcc4.8.3/include/openmpi/opal/mca/event/libevent2021/libevent -I/usr/common/usg/openmpi/1.8.1/gcc4.8.3/include/openmpi/opal/mca/event/libevent2021/libevent/include -I/usr/common/usg/openmpi/1.8.1/gcc4.8.3/include -I/usr/common/usg/openmpi/1.8.1/gcc4.8.3/include/openmpi")
VISIT_OPTION_DEFAULT(VISIT_MPI_CXX_FLAGS "-DOMPI_SKIP_MPICXX -I/usr/common/usg/openmpi/1.8.1/gcc4.8.3/include/openmpi/opal/mca/hwloc/hwloc172/hwloc/include -I/usr/common/usg/openmpi/1.8.1/gcc4.8.3/include/openmpi/opal/mca/event/libevent2021/libevent -I/usr/common/usg/openmpi/1.8.1/gcc4.8.3/include/openmpi/opal/mca/event/libevent2021/libevent/include -I/usr/common/usg/openmpi/1.8.1/gcc4.8.3/include -I/usr/common/usg/openmpi/1.8.1/gcc4.8.3/include/openmpi")
VISIT_OPTION_DEFAULT(VISIT_PARALLEL_LINKER_FLAGS "-L/usr/syscom/opt/torque/4.2.7.h1/lib -L/usr/common/usg/OFED/gcc4.8.3/2.0-3.0.0-Mellanox/lib -Wl,--enable-new-dtags -L/usr/common/usg/openmpi/1.8.1/gcc4.8.3/lib")
VISIT_OPTION_DEFAULT(VISIT_MPI_LIBS "/usr/common/usg/openmpi/1.8.1/gcc4.8.3/lib/libmpi.so")
VISIT_OPTION_DEFAULT(VISIT_PARALLEL ON TYPE BOOL)
VISIT_OPTION_DEFAULT(VISIT_PARALLEL_RPATH "/usr/common/usg/openmpi/1.8.1/gcc4.8.3/lib;/usr/syscom/opt/torque/4.2.7.h1/lib;/usr/syscom/opt/torque/4.2.7.h1/lib;/usr/syscom/opt/torque/4.2.7.h1/lib;/usr/syscom/opt/torque/4.2.7.h1/lib;/usr/common/usg/OFED/gcc4.8.3/2.0-3.0.0-Mellanox/lib")

##
## VisIt Thread Option
##
VISIT_OPTION_DEFAULT(VISIT_THREAD OFF TYPE BOOL)

##
## VisIt Boost Option.
##
VISIT_OPTION_DEFAULT(VISIT_USE_BOOST ON TYPE BOOL)

##############################################################
##
## Database reader plugin support libraries
##
## The HDF4, HDF5 and NetCDF libraries must be first so that
## their libdeps are defined for any plugins that need them.
##
## For libraries with LIBDEP settings, order matters.
## Libraries with LIBDEP settings that depend on other
## Library's LIBDEP settings must come after them.
##############################################################

##
## Mesa
##
#VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/7.10.2/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/6.5.3/${VISITARCH})

##
## Python
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/2.7.6/${VISITARCH})

##
## Qt
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/4.8.3/${VISITARCH}/bin)

##
## VTK
##
SETUP_APP_VERSION(VTK 6.1.0)
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk-r/${VTK_VERSION}/${VISITARCH})

##
## AdvIO
##
VISIT_OPTION_DEFAULT(VISIT_ADVIO_DIR ${VISITHOME}/AdvIO/1.2/${VISITARCH})

##
## Boxlib
##
VISIT_OPTION_DEFAULT(VISIT_BOXLIB_DIR ${VISITHOME}/boxlib/1.0.7/${VISITARCH})

##
## CFITSIO
##
VISIT_OPTION_DEFAULT(VISIT_CFITSIO_DIR ${VISITHOME}/cfitsio/3006/${VISITARCH})

##
## SZIP
##
VISIT_OPTION_DEFAULT(VISIT_SZIP_DIR ${VISITHOME}/szip/2.1/${VISITARCH})

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.8.7/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz /usr/lib z TYPE STRING)

##
## CGNS
##
VISIT_OPTION_DEFAULT(VISIT_CGNS_DIR ${VISITHOME}/cgns/3.2.1/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_CGNS_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP} TYPE STRING)

##
## FastBit
##
VISIT_OPTION_DEFAULT(VISIT_FASTBIT_DIR ${VISITHOME}/fastbit/1.2.0/${VISITARCH})

##
## GDAL
##
VISIT_OPTION_DEFAULT(VISIT_GDAL_DIR ${VISITHOME}/gdal/1.10.0/${VISITARCH})

##
## H5Part
##
VISIT_OPTION_DEFAULT(VISIT_H5PART_DIR ${VISITHOME}/h5part/1.6.6/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_H5PART_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP} TYPE STRING)

##
## HDF4
##
VISIT_OPTION_DEFAULT(VISIT_HDF4_DIR ${VISITHOME}/hdf4/4.2.5/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_HDF4_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz ${VISITHOME}/vtk-r/${VTK_VERSION}/${VISITARCH}/lib vtkjpeg-${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION} TYPE STRING)

##
## Ice-T
##
VISIT_OPTION_DEFAULT(VISIT_ICET_DIR ${VISITHOME}/icet/1.0.0/${VISITARCH})

##
## NetCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR ${VISITHOME}/netcdf/4.1.1/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_NETCDF_LIBDEP HDF5_LIBRARY_DIR hdf5_hl HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP} TYPE STRING)

##
## PySide
##
VISIT_OPTION_DEFAULT(VISIT_PYSIDE_DIR ${VISITHOME}/pyside/1.2.2/${VISITARCH}/)

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.10.1/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP} TYPE STRING)

##
## Xdmf
##
VISIT_OPTION_DEFAULT(VISIT_XDMF_DIR ${VISITHOME}/Xdmf/2.1.1/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_XDMF_LIBDEP HDF5_LIBRARY_DIR hdf5  VTK_LIBRARY_DIRS vtklibxml2-${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}  TYPE STRING)
