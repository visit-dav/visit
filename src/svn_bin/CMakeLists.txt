# Install the visit config files
IF(WIN32)
    INSTALL(FILES 
        visit-config-anl.ini
        visit-config-llnl_closed.ini
        visit-config-llnl_open.ini
        visit-config-nersc.ini
        visit-config-ornl.ini
        visit-config-princeton.ini
        visit-config-tacc.ini
        DESTINATION ${VISIT_INSTALLED_VERSION_BIN}
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
    )
ELSE(WIN32)
    INSTALL(FILES 
        visit-config-anag
        visit-config-anl
        visit-config-llnl_closed
        visit-config-llnl_open
        visit-config-nersc
        visit-config-ornl
        visit-config-princeton
        visit-config-tacc
        visit-guiconfig-anag
        visit-visitrc-anag
        DESTINATION ${VISIT_VERSION}/.visit
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
    )
ENDIF(WIN32)
