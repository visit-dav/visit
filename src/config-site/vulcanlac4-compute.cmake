#/usr/gapps/visit/thirdparty_static/2.10.0/cmake/3.0.2/linux-ppc64_gcc-4.4_BGQ/bin/cmake
##
## ./build_visit generated host.cmake
## created: Tue Oct 27 15:52:20 PDT 2015
## system: Linux vulcanlac4 2.6.32-504.16.2.1chaos.1blueos_small_pages.bl2.2.ppc64 #1 SMP Thu Jul 16 10:43:19 PDT 2015 ppc64 ppc64 ppc64 GNU/Linux
## by: kbonnell

##
## Setup VISITHOME & VISITARCH variables.
##
SET(VISITHOME /usr/gapps/visit/thirdparty_static/2.10.0)
SET(VISITARCH linux-ppc64_gcc-4.4_BGQ)

## Compiler flags.
##
VISIT_OPTION_DEFAULT(VISIT_C_COMPILER bgxlc_r TYPE FILEPATH)
VISIT_OPTION_DEFAULT(VISIT_CXX_COMPILER bgxlC_r TYPE FILEPATH)

##
## BG/Q-specific settings
##
SET(CMAKE_CROSSCOMPILING    ON)
VISIT_OPTION_DEFAULT(VISIT_ENGINE_ONLY ON TYPE BOOL)
SET(VISIT_SELECTED_DATABASE_PLUGINS "BOV;Cale;CaleHDF5;Curve2D;EnSight;Miranda;PDB;PlainText;SAMRAI;Silo;VTK")
VISIT_OPTION_DEFAULT(VISIT_USE_X            OFF TYPE BOOL)
VISIT_OPTION_DEFAULT(VISIT_USE_GLEW         OFF TYPE BOOL)
VISIT_OPTION_DEFAULT(VISIT_SLIVR            OFF TYPE BOOL)
VISIT_OPTION_DEFAULT(VISIT_DISABLE_SELECT   ON  TYPE BOOL)
VISIT_OPTION_DEFAULT(VISIT_USE_NOSPIN_BCAST OFF TYPE BOOL)
VISIT_OPTION_DEFAULT(VISIT_PYTHON_FILTERS   OFF TYPE BOOL)
VISIT_OPTION_DEFAULT(VISIT_PYTHON_SCRIPTING OFF TYPE BOOL)

VISIT_OPTION_DEFAULT(VISIT_OPENGL_DIR       ${VISITHOME}/mesa/7.8.2/${VISITARCH})
ADD_DEFINITIONS(-DVISIT_BLUE_GENE_Q)


##
## Parallel Build Setup.
##
SET(BLUEGENEQ /bgsys/drivers/V1R2M0/ppc64)
VISIT_OPTION_DEFAULT(VISIT_PARALLEL ON TYPE BOOL)
VISIT_OPTION_DEFAULT(VISIT_MPI_CXX_FLAGS "-I${BLUEGENEQ} -I${BLUEGENEQ}/comm/sys/include -I${BLUEGENEQ}/spi/include -I${BLUEGENEQ}/spi/include/kernel/cnk -I${BLUEGENEQ}/comm/xl/include" TYPE STRING)
VISIT_OPTION_DEFAULT(VISIT_MPI_C_FLAGS   "-I${BLUEGENEQ} -I${BLUEGENEQ}/comm/sys/include -I${BLUEGENEQ}/spi/include -I${BLUEGENEQ}/spi/include/kernel/cnk -I${BLUEGENEQ}/comm/xl/include" TYPE STRING)
VISIT_OPTION_DEFAULT(VISIT_MPI_LD_FLAGS  "-L${BLUEGENEQ}/spi/lib -L${BLUEGENEQ}/comm/sys/lib -L${BLUEGENEQ}/spi/lib -L${BLUEGENEQ}/comm/xl/lib -R/opt/ibmcmp/lib64/bg/bglib64" TYPE STRING)
VISIT_OPTION_DEFAULT(VISIT_MPI_LIBS     mpich opa mpl pami SPI SPI_cnk rt pthread stdc++ pthread TYPE STRING)

##
## Static build
##
VISIT_OPTION_DEFAULT(VISIT_STATIC ON TYPE BOOL)

##
## VisIt Thread Option
##
VISIT_OPTION_DEFAULT(VISIT_THREAD OFF TYPE BOOL)

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
## VTK
##
SETUP_APP_VERSION(VTK 6.1.0)
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/${VTK_VERSION}/${VISITARCH})
##

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
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.8.14/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz ${VISITHOME}/zlib/1.2.8/${VISITARCH}/lib z TYPE STRING)

##
## Ice-T
##
VISIT_OPTION_DEFAULT(VISIT_ICET_DIR ${VISITHOME}/icet/1.0.0/${VISITARCH})

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.10.1/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP} TYPE STRING)

