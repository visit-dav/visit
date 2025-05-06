# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Bonnell, Wed April 16, 2025
#   Utilize visit_import_library.
#
#****************************************************************************

# Uses the UINTAH_DIR hint from the config-site .cmake file


if(${VISIT_USE_SYSTEM_UINTAH})
  set(uintah_INCDIR ../src)
else()
  set(uintah_INCDIR include)
endif()

# Note UINTAH_INTERFACES_LIB is called in the reader using dlopen
# so find the library in order to set the necessary compile definitions

find_library(_interfaces_lib
    NAMES VisIt_interfaces
    PATHS ${UINTAH_DIR}
    PATH_SUFFIXES lib lib64)

if(_interfaces_lib)
    cmake_path(SET ilib ${_interfaces_lib})
    cmake_path(GET ilib FILENAME iname)
    cmake_path(GET ilib PARENT_PATH idir)
    set(uintah_DEFINES 
        UINTAH_LIBRARY_DIR=\"${idir}\"
        UINTAH_INTERFACES_LIB=\"${iname}\"
        UINTAH_VERSION_HEX=${UINTAH_VERSION_HEX})
    unset(_interfaces_lib)
endif()

visit_import_third_party(UINTAH
    INCDIR  ${uintah_INCDIR}
    DEFINES ${uintah_DEFINES}
    LIBS    VisIt_interfaces
            CCA_Components_DataArchiver
            CCA_Components_LoadBalancers
            CCA_Components_ProblemSpecification
            CCA_Components_Schedulers
            CCA_Ports
            Core_Containers
            Core_DataArchive
            Core_Datatypes
            Core_Disclosure
            Core_Exceptions
            Core_Geometry
            Core_GeometryPiece
            Core_Grid
            Core_IO
            Core_Malloc
            Core_Math
            Core_OS
            Core_Parallel
            Core_ProblemSpec
            Core_Util)

