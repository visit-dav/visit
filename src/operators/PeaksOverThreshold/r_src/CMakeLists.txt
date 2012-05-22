
#copy files to build directory..
CONFIGURE_FILE(potVisit.r
              ${CMAKE_BINARY_DIR}/lib/r_support/Rscripts/potVisit.r @ONLY)
CONFIGURE_FILE(pp.fit2.r
              ${CMAKE_BINARY_DIR}/lib/r_support/Rscripts/potVisit.r @ONLY)

#install all files..
MESSAGE(STATUS "Installing R scripts: ${VISIT_INSTALLED_VERSION_LIB} ")
INSTALL(FILES  potVisit.r pp.fit2.r
        DESTINATION ${VISIT_INSTALLED_VERSION_LIB}/r_support/Rscripts/
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ)
