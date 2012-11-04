# this section should go in its own "prepare windows type cmake file"
IF (MSVC_VERSION)
  SET(ERRMSG "You have chosen a compiler version not currently supported.  You will need to build all dependent 3rd party libraries with this compiler, and provide the correct locations for them.")
  MESSAGE(STATUS "MSVC_VERSION=${MSVC_VERSION}")
  SET(PYTHONDIR "Python26")
  SET(JPEGDIR "jpeg-6b")
  SET(GDALDIR "gdal-1.7.1")
  IF (NOT MSVC_VERSION LESS 1600)
      SET(VISIT_MSVC_VERSION "MSVC2010")
      SET(PYTHONDIR "python-2.7.2")
      SET(JPEGDIR "jpeg-8c")
      SET(GDALDIR "gdal-1.9.0")
  ELSEIF (NOT MSVC_VERSION LESS 1500)
      SET(VISIT_MSVC_VERSION "MSVC90")
  ELSEIF (NOT MSVC_VERSION LESS 1400)
      MESSAGE(SEND_ERROR "${ERRMSG}")
  ELSE (NOT MSVC_VERSION LESS 1600)
      MESSAGE(SEND_ERROR "${ERRMSG}")
  ENDIF (NOT MSVC_VERSION LESS 1600)

ELSE (MSVC_VERSION)
  MESSAGE(SEND_ERROR "${ERRMSG}")
ENDIF(MSVC_VERSION)

# this can be left in generic_windows.cmake, but allow user to
# have their own config-site file

IF (CMAKE_CL_64)
    SET(VISIT_MSVC_VERSION "${VISIT_MSVC_VERSION}-x64")
ENDIF (CMAKE_CL_64)
MESSAGE(STATUS "Prebuilt library directory name: ${VISIT_MSVC_VERSION}")

SET(VISIT_WINDOWS_DIR ${VISIT_SOURCE_DIR}/../windowsbuild)
SET(VISITHOME ${VISIT_WINDOWS_DIR}/ThirdParty)


# Create the NSIS installer package (override default from root CMakeLists.txt
OPTION(VISIT_MAKE_NSIS_INSTALLER "Create an installer package using NSIS." ON)


##############################################################
##
## Required libraries
##
##############################################################


##
## VTK
##
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR  ${VISITHOME}/vtk-5.8.0a/${VISIT_MSVC_VERSION})

##
## QT
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN     ${VISITHOME}/Qt-4.8.3/lib/${VISIT_MSVC_VERSION})
VISIT_OPTION_DEFAULT(QT_DIR     ${VISITHOME}/Qt-4.8.3)
VISIT_OPTION_DEFAULT(QT_QMAKE_EXECUTABLE ${VISIT_QT_BIN}/qmake.exe TYPE FILEPATH)

##
## PYTHON
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR 
    ${VISITHOME}/${PYTHONDIR}/${VISIT_MSVC_VERSION})

##
## PYSIDE
##
VISIT_OPTION_DEFAULT(VISIT_PYSIDE_DIR 
    ${VISITHOME}/PySide-1.1.1/${VISIT_MSVC_VERSION})

##############################################################
##
## Some misc libraries Database readers may depend upon
##
##############################################################

##
## JPEG
##
VISIT_OPTION_DEFAULT(VISIT_JPEG_DIR     ${VISITHOME}/${JPEGDIR})

##
## SZIP
##
VISIT_OPTION_DEFAULT(VISIT_SZIP_DIR     ${VISITHOME}/szip)

##
## ZLIB
##
VISIT_OPTION_DEFAULT(VISIT_ZLIB_DIR     ${VISITHOME}/zlib)

##############################################################
##
## Database reader plugin support libraries
##
## For libraries with LIBDEP settings, order matters.
## Libraries with LIBDEP settings that depend on other
## Library's LIBDEP settings must come after them.
##############################################################

##
## HDF4
##
VISIT_OPTION_DEFAULT(VISIT_HDF4_DIR     ${VISITHOME}/hdf4.2.6/${VISIT_MSVC_VERSION})
VISIT_OPTION_DEFAULT(VISIT_HDF4_LIBDEP  
    JPEG_LIBRARY_DIR libjpeg 
    SZIP_LIBRARY_DIR szlibdll 
    ZLIB_LIBRARY_DIR zlib1
    TYPE STRING
)

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR     ${VISITHOME}/hdf5-1.8.7/${VISIT_MSVC_VERSION})
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP  
    SZIP_LIBRARY_DIR szlibdll 
    ZLIB_LIBRARY_DIR zlib1
    TYPE STRING
)

##
## NETCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR   ${VISITHOME}/netcdf-4.1.1)
VISIT_OPTION_DEFAULT(VISIT_NETCDF_LIBDEP 
    HDF5_LIBRARY_DIR hdf5dll ${VISIT_HDF5_LIBDEP}
    TYPE STRING
)

##
## BOXLIB
##
VISIT_OPTION_DEFAULT(VISIT_BOXLIB_DIR ${VISITHOME}/BoxLib-0.1.8)

##
## CCMIO
##
VISIT_OPTION_DEFAULT(VISIT_CCMIO_DIR    ${VISITHOME}/ccmio)

##
## CFITSIO
##
VISIT_OPTION_DEFAULT(VISIT_CFITSIO_DIR  ${VISITHOME}/cfitsio)

##
## CGNS
##
VISIT_OPTION_DEFAULT(VISIT_CGNS_DIR     ${VISITHOME}/cgns-3.1.3/${VISIT_MSVC_VERSION})
VISIT_OPTION_DEFAULT(VISIT_CGNS_LIBDEP 
    HDF5_LIBRARY_DIR hdf5dll ${VISIT_HDF5_LIBDEP}
    TYPE STRING
)

##
## EXODUS
##
VISIT_OPTION_DEFAULT(VISIT_EXODUSII_DIR ${VISITHOME}/exodus-4.98)
VISIT_OPTION_DEFAULT(VISIT_EXODUSII_LIBDEP 
    NETCDF_LIBRARY_DIR netcdf ${VISIT_NETCDF_LIBDEP}
    TYPE STRING
)

##
## GDAL
##
VISIT_OPTION_DEFAULT(VISIT_GDAL_DIR     ${VISITHOME}/${GDALDIR})

##
## H5PART
##
VISIT_OPTION_DEFAULT(VISIT_H5PART_DIR   ${VISITHOME}/h5part-1.6.5/${VISIT_MSVC_VERSION})
VISIT_OPTION_DEFAULT(VISIT_H5PART_LIBDEP 
    HDF5_LIBRARY_DIR hdf5dll ${VISIT_HDF5_LIBDEP}
    TYPE STRING
)


##
## MILI
##
VISIT_OPTION_DEFAULT(VISIT_MILI_DIR     ${VISITHOME}/Mili-11.1.1)

##
## SILO
##
IF (NOT CMAKE_CL_64)
    VISIT_OPTION_DEFAULT(VISIT_SILO_DIR     ${VISITHOME}/silo)
ELSE (NOT CMAKE_CL_64)
    VISIT_OPTION_DEFAULT(VISIT_SILO_DIR     ${VISITHOME}/silo_x64)
ENDIF (NOT CMAKE_CL_64)

VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP  
    HDF5_LIBRARY_DIR hdf5dll ${VISIT_HDF5_LIBDEP}
    TYPE STRING
)

##
## XDMF
##
VISIT_OPTION_DEFAULT(VISIT_XDMF_DIR ${VISITHOME}/Xdmf-2.1.1/${VISIT_MSVC_VERSION})
VISIT_OPTION_DEFAULT(VISIT_XDMF_LIBDEP 
    HDF5_LIBRARY_DIR hdf5dll ${VISIT_HDF5_LIBDEP}
    TYPE STRING
)
