#/home/brad/Development/thirdparty_shared/cmake/2.8.3/linux-x86_64_gcc-4.4/bin/cmake

IF(WIN32)
   MESSAGE(STATUS "Skipping Linux configs")
   INCLUDE(config-site/windows.cmake)
ELSE(WIN32)
##
## ./build_visit generated host.cmake
## created: Sat Jun  2 01:11:06 PDT 2012
## system: Linux coruscant 2.6.31-17-generic #54-Ubuntu SMP Thu Dec 10 17:01:44 UTC 2009 x86_64 GNU/Linux
## by: brad

##
## Setup VISITHOME & VISITARCH variables.
##
SET(VISITHOME /home/brad/Development/thirdparty_shared)
SET(VISITARCH linux-x86_64_gcc-4.6)

##
## Specify the location of the mesa.
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/7.8.2/${VISITARCH})

##
## Specify the location of the vtk.
##
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.8.0/${VISITARCH})

##
## Specify the Qt4 binary dir. 
## (qmake us used to locate & setup Qt4 dependencies)
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/4.7.4/${VISITARCH}/bin)

##
## Specify the location of the python.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/2.6.4/${VISITARCH})

##
## Compiler flags.
##
VISIT_OPTION_DEFAULT(VISIT_C_COMPILER gcc)
VISIT_OPTION_DEFAULT(VISIT_CXX_COMPILER g++)
VISIT_OPTION_DEFAULT(VISIT_FORTRAN_COMPILER gfortran)
VISIT_OPTION_DEFAULT(VISIT_C_FLAGS " -Wno-deprecated -fvisibility=hidden -m64 -fPIC")
VISIT_OPTION_DEFAULT(VISIT_CXX_FLAGS " -Wno-deprecated -fvisibility=hidden -m64 -fPIC")
VISIT_OPTION_DEFAULT(VISIT_FORTRAN ON)

##
## Parallel Build Setup.
##
VISIT_OPTION_DEFAULT(VISIT_PARALLEL ON)
## (configured w/ mpi compiler wrapper)
VISIT_OPTION_DEFAULT(VISIT_MPI_COMPILER /usr/bin/mpic++)
VISIT_OPTION_DEFAULT(VISIT_MPI_FORTRAN_COMPILER /usr/bin/mpif77)

##
## Ice-T
##
#VISIT_OPTION_DEFAULT(VISIT_ICET_DIR ${VISITHOME}/icet/0.5.4/${VISITARCH})

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
## SZIP
##
VISIT_OPTION_DEFAULT(VISIT_SZIP_DIR ${VISITHOME}/szip/2.1/${VISITARCH})

##
## HDF4
##
VISIT_OPTION_DEFAULT(VISIT_HDF4_DIR ${VISITHOME}/hdf4/4.2.5/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_HDF4_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz /usr/lib/x86_64-linux-gnu jpeg)


##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.8.7/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz /usr/lib/x86_64-linux-gnu z)

##
## NetCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR ${VISITHOME}/netcdf/4.1.1/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_NETCDF_LIBDEP HDF5_LIBRARY_DIR hdf5_hl HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

##############################################################
##
## Database reader plugin support libraries group 2
##
##############################################################

##
## AdvIO
##
VISIT_OPTION_DEFAULT(VISIT_ADVIO_DIR ${VISITHOME}/AdvIO/1.2/${VISITARCH}/)

##
## Boxlib
##
#VISIT_OPTION_DEFAULT(VISIT_BOXLIB_DIR ${VISITHOME}/boxlib/2010.10.01/${VISITARCH})

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
VISIT_OPTION_DEFAULT(VISIT_CGNS_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

##
## Exodus
##
VISIT_OPTION_DEFAULT(VISIT_EXODUSII_DIR ${VISITHOME}/exodus/4.98/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_EXODUSII_LIBDEP NETCDF_LIBRARY_DIR netcdf ${VISIT_NETCDF_LIBDEP})

##
## GDAL
##
VISIT_OPTION_DEFAULT(VISIT_GDAL_DIR ${VISITHOME}/gdal/1.7.1/${VISITARCH})

##
## H5Part
##
#VISIT_OPTION_DEFAULT(VISIT_H5PART_DIR ${VISITHOME}/h5part/1.6.2/${VISITARCH})
#VISIT_OPTION_DEFAULT(VISIT_H5PART_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

##
## Mili
##
#VISIT_OPTION_DEFAULT(VISIT_MILI_DIR ${VISITHOME}/mili/1.10.0/${VISITARCH})

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.8/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

##
## Xdmf
##
VISIT_OPTION_DEFAULT(VISIT_XDMF_DIR ${VISITHOME}/Xdmf/2.1.1/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_XDMF_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})
ENDIF(WIN32)
