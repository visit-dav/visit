#/g/g19/whitlocb/Development/visit/cmake/2.6.4/"linux-ppc64_bgp_gcc-4.1.2"/bin/cmake

# 1. YOU MUST UPGRADE TO cmake 2.6.4 !!!! This script assumes 2.6.4 but it is unknown whether it exists on your system.
##
## Set the VISITHOME environment variable.
##
SET(VISITHOME /g/g19/whitlocb/Development/visit)
SET(VISIT_VERBOSE_MAKEFILE TRUE)

##
## Compiler flags.
##
SET(VISIT_C_COMPILER /bgsys/drivers/ppcfloor/gnu-linux/powerpc-bgp-linux/bin/gcc)
SET(VISIT_CXX_COMPILER /bgsys/drivers/ppcfloor/gnu-linux/powerpc-bgp-linux/bin/g++)
    # My current configure line is:

#    CFLAGS="-O2 -fPIC"
#    CXXFLAGS="-O2 -fPIC"
SET(VISIT_C_FLAGS "-g -fPIC")
SET(VISIT_CXX_FLAGS "-g -fPIC")


    # Set the parallel options

    # Enable Ice-T
else
# These are the options that are for xlc. They are right but there's an
# error somewhere that prevents libvisit_vtk from loading.
#    CC="xlc"
#    CXX="xlC"
#    CFLAGS="-O2 -qpic=large"
#    CXXFLAGS="-O2 -qpic=large"
#    VISITARCH="linux-ppc64_xlc"

# In the meantime, use gcc
SET(VISIT_C_COMPILER gcc)
SET(VISIT_C_FLAGS "-O2 -fPIC")
SET(VISIT_CXX_COMPILER g++)
SET(VISIT_CXX_FLAGS "-O2 -fPIC")

#fi

##
## Specify the location of the mesa include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/$MESA_VERSION/${VISITARCH})

##
## Specify the location of the vtk include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0c/${VISITARCH}/lib/vtk-5.0)

##
## Specify the location of cmake.
##


##
## Specify the location of the qt include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/3.3.8/$QTARCH/bin)

##
## Specify the location of the python include and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/2.5/${VISITARCH})

##
## Database reader plugin support libraries
##
##############################################################

##
## Boxlib
##

##
## CCMIO
##

##
## CFITSIO
##

##
## CGNS
##
VISIT_OPTION_DEFAULT(VISIT_CGNS_DIR ${VISITHOME}/cgns/2.4/${VISITARCH})

##
## Exodus
##

##
## FastBit
##

##
## GDAL
##

##
## H5Part
##

##
## HDF4
##

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.8.1/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)

##
## Mili
##

##
## NetCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR ${VISITHOME}/netcdf/3.6.3/${VISITARCH})

##
## SZIP
##
SET(SZIP_DIR ${VISITHOME}/szip/2.1/${VISITARCH})

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/$SILO_VERSION/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})
