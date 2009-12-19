#/what/is/the/path/to/bin/cmake

##
## Set the VISITHOME environment variable.
##
SET(VISITHOME /usr/gapps/visit)
SET(VISIT_VERBOSE_MAKEFILE TRUE)

##
## Use VisIt's 32 bit mesa.
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/5.0/aix_5_xlc_32)

##
## Use VisIt's 32 bit vtk.
##
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0c/aix_5_ll_xlc_32/lib/vtk-5.0)

##
## Use VisIt's 32 bit Qt.
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/4.4.3/aix_5_xlc_32/bin)

##
## Use VisIt's 32 bit Python
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/2.5/aix_5_xlc_32)

##
## Database reader plugin support libraries
##
###############################################################################

##
## Boxlib
##
VISIT_OPTION_DEFAULT(VISIT_BOXLIB2D_DIR /usr/gapps/visit/boxlib/aix_5_ll_xlc_32)
VISIT_OPTION_DEFAULT(VISIT_BOXLIB3D_DIR /usr/gapps/visit/boxlib/aix_5_ll_xlc_32)

##
## CFITSIO
##
VISIT_OPTION_DEFAULT(VISIT_CFITSIO_DIR /usr/gapps/visit/cfitsio/3006/aix_5_xlc_32)

##
## CGNS
##
VISIT_OPTION_DEFAULT(VISIT_CGNS_DIR /usr/gapps/visit/cgns/2.4/aix_5_xlc_32)

##
## Exodus
##
VISIT_OPTION_DEFAULT(VISIT_EXODUSII_DIR ${VISITHOME}/exodus/4.46/aix_5_xlc_32)

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR /usr/gapps/silo/hdf5/1.8.1/aix_5_ll)
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)

##
## Mili
##
VISIT_OPTION_DEFAULT(VISIT_MILI_DIR /usr/gapps/visit/mili/current/aix_5_ll_xlc_32)

##
## netCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR /usr/gapps/visit/netcdf/3.6.0/aix_5_xlc_32)

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR /usr/gapps/silo/4.7/aix_5_ll)
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

##
## ViSUS
##
VISIT_OPTION_DEFAULT(VISIT_VISUS_DIR /usr/gapps/visus/1.0a5/aix_5_32_fed)
