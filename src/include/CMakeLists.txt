# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

INSTALL(FILES ${VISIT_BINARY_DIR}/include/visit-config.h 
              ${VISIT_BINARY_DIR}/include/visit-python-config.h
              ${VISIT_BINARY_DIR}/include/visit-version.h
              visit-hdf5.h
    DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}/visit/include
    PERMISSIONS OWNER_WRITE OWNER_READ GROUP_WRITE GROUP_READ WORLD_READ
)
