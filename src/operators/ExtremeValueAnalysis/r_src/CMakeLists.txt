
#copy files to build directory..
CONFIGURE_FILE(auxil.r
              ${CMAKE_BINARY_DIR}/lib/r_support/Rscripts/auxil.r @ONLY)
CONFIGURE_FILE(gev.fit2.r
              ${CMAKE_BINARY_DIR}/lib/r_support/Rscripts/gev.fit2.r @ONLY)
CONFIGURE_FILE(gevVisit.r
              ${CMAKE_BINARY_DIR}/lib/r_support/Rscripts/gevVisit.r @ONLY)

#install all files..
MESSAGE(STATUS "Installing R scripts: ${VISIT_INSTALLED_VERSION_LIB} ")
INSTALL(FILES auxil.r gev.fit2.r gevVisit.r
        DESTINATION ${VISIT_INSTALLED_VERSION_LIB}/r_support/Rscripts/
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ)
