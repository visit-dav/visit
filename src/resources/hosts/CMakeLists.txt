# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#   Brad Whitlock, Thu Jun 14 15:28:13 PDT 2012
#   Install customlauncher files. Remove installation of all the other stuff
#   because it will be handled by wholesale installation of the hosts directory
#   into resources/hosts.
#
#   Kathleen Biagas, Wed Sep 12 14:34:04 PDT 2012
#   Added config, guiconfig and visitrc files, use OPTIONAL tag in case
#   files don't exist.
#
#*****************************************************************************

## provide proper install structure windows vs other:
IF(WIN32)
    SET(LPROF_DEST ${VISIT_INSTALLED_VERSION_BIN}/hosts)
    SET(BIN_DEST   ${VISIT_INSTALLED_VERSION_BIN})
    SET(CONFIG_DEST ${VISIT_INSTALLED_VERSION_BIN})
ELSE(WIN32)
    SET(LPROF_DEST ${VISIT_VERSION}/.visit/hosts)
    SET(BIN_DEST   ${VISIT_VERSION}/bin) # The internal bin directory
    SET(CONFIG_DEST ${VISIT_VERSION}/.visit)
ENDIF(WIN32)

# Install local host profiles if specified
IF (VISIT_INSTALL_PROFILES_TO_HOSTS)
    FILE(GLOB LPROF_FILES
         "${CMAKE_CURRENT_SOURCE_DIR}/${VISIT_INSTALL_PROFILES_TO_HOSTS}/*.xml")
    INSTALL(FILES ${LPROF_FILES}
            DESTINATION ${LPROF_DEST}
            PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
    )
    INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/${VISIT_INSTALL_PROFILES_TO_HOSTS}/customlauncher"
            DESTINATION ${BIN_DEST}
            PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
            OPTIONAL 
    ) 
    INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/${VISIT_INSTALL_PROFILES_TO_HOSTS}/config"
            DESTINATION ${CONFIG_DEST}
            PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
            OPTIONAL 
    ) 
    INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/${VISIT_INSTALL_PROFILES_TO_HOSTS}/guiconfig"
            DESTINATION ${CONFIG_DEST}
            PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
            OPTIONAL 
    ) 
    INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/${VISIT_INSTALL_PROFILES_TO_HOSTS}/visitrc"
            DESTINATION ${CONFIG_DEST}
            PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
            OPTIONAL 
    ) 
ENDIF (VISIT_INSTALL_PROFILES_TO_HOSTS)
