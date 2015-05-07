#/usr/gapps/visit/thirdparty_static/2.7.0/cmake/2.8.10.2/linux-ppc64_gcc-4.4_BGQ/bin/cmake
##
## ./build_visit generated host.cmake
## created: Fri May 31 14:41:03 PDT 2013
## system: Linux rzuseqlac2 2.6.32-358.6chaos.bgq62.ppc64 #1 SMP Thu Apr 4 11:21:55 PDT 2013 ppc64 ppc64 ppc64 GNU/Linux
## by: whitlocb

##
## Setup VISITHOME & VISITARCH variables.
##
SET(VISITHOME /usr/gapps/visit/thirdparty_static/2.9.0)
SET(VISITARCH linux-ppc64_gcc-4.4_BGQ)
MESSAGE(status "************* compute node build")

## Compiler flags.
##
VISIT_OPTION_DEFAULT(VISIT_C_COMPILER bgxlc TYPE FILEPATH)
VISIT_OPTION_DEFAULT(VISIT_CXX_COMPILER bgxlC TYPE FILEPATH)

##
## BG/Q-specific settings
##
SET(CMAKE_CROSSCOMPILING    ON)
VISIT_OPTION_DEFAULT(VISIT_USE_X            OFF)
VISIT_OPTION_DEFAULT(VISIT_USE_GLEW         OFF)
VISIT_OPTION_DEFAULT(VISIT_SLIVR            OFF)
VISIT_OPTION_DEFAULT(VISIT_USE_BOOST        OFF)
VISIT_OPTION_DEFAULT(VISIT_DISABLE_SELECT   ON)
VISIT_OPTION_DEFAULT(VISIT_USE_NOSPIN_BCAST OFF)
VISIT_OPTION_DEFAULT(VISIT_OPENGL_DIR       ${VISITHOME}/mesa/7.8.2/${VISITARCH})

## engine only
VISIT_OPTION_DEFAULT(VISIT_ENGINE_ONLY ON TYPE BOOL)

##
## Parallel Build Setup.
##
VISIT_OPTION_DEFAULT(VISIT_PARALLEL ON TYPE BOOL)
## (inserted by build_visit for BG/Q. Some adjustment may be needed)
SET(BLUEGENEQ /bgsys/drivers/V1R2M0/ppc64)
VISIT_OPTION_DEFAULT(VISIT_PARALLEL ON TYPE BOOL)
VISIT_OPTION_DEFAULT(VISIT_MPI_CXX_FLAGS "-I${BLUEGENEQ} -I${BLUEGENEQ}/comm/sys/include -I${BLUEGENEQ}/spi/include -I${BLUEGENEQ}/spi/include/kernel/cnk -I${BLUEGENEQ}/comm/xl/include" TYPE STRING)
VISIT_OPTION_DEFAULT(VISIT_MPI_C_FLAGS   "-I${BLUEGENEQ} -I${BLUEGENEQ}/comm/sys/include -I${BLUEGENEQ}/spi/include -I${BLUEGENEQ}/spi/include/kernel/cnk -I${BLUEGENEQ}/comm/xl/include" TYPE STRING)
VISIT_OPTION_DEFAULT(VISIT_MPI_LD_FLAGS  "-L${BLUEGENEQ}/spi/lib -L${BLUEGENEQ}/comm/sys/lib -L${BLUEGENEQ}/spi/lib -L${BLUEGENEQ}/comm/sys/lib -L${BLUEGENEQ}/spi/lib -L${BLUEGENEQ}/comm/xl/lib -R/opt/ibmcmp/lib64/bg" TYPE STRING)
VISIT_OPTION_DEFAULT(VISIT_MPI_LIBS     mpich opa mpl pami SPI SPI_cnk rt pthread stdc++ pthread TYPE STRING)

##
## Static build
##
VISIT_OPTION_DEFAULT(VISIT_STATIC ON TYPE BOOL)

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

##
## Mesa
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/7.8.2/${VISITARCH})

##
## VTK
##
SETUP_APP_VERSION(VTK 6.1.0)
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/6.1.0/${VISITARCH})

##
## SZIP
##
VISIT_OPTION_DEFAULT(VISIT_SZIP_DIR ${VISITHOME}/szip/2.1/${VISITARCH})

##
## ZLIB
##
VISIT_OPTION_DEFAULT(VISIT_ZLIB_DIR ${VISITHOME}/zlib/1.2.8/${VISITARCH})

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.8.7/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz ${VISITHOME}/zlib/1.2.8/${VISITARCH}/lib z TYPE STRING)

##
## Ice-T
##
VISIT_OPTION_DEFAULT(VISIT_ICET_DIR ${VISITHOME}/icet/1.0.0/${VISITARCH})

##
## Mili
##
#VISIT_OPTION_DEFAULT(VISIT_MILI_DIR ${VISITHOME}/mili/1.11.1/${VISITARCH})

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.10.1/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP} TYPE STRING)

