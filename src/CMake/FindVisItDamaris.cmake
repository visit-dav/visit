# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#
# Modifications:
#   Kathleen Biagas, Mon Jun 16, 2025
#   Use visit_import_third_party instead of SET_UP_THIRD_PARTY
#

# Use the DAMARIS_DIR hint from the config-site .cmake file

visit_import_third_party(DAMARIS LIBS damaris)
visit_import_third_party(XERCESC LIBS xerces-c)
visit_import_third_party(XSD LIBS NO_LIBS)

if(DAMARIS_FOUND AND XERCESC_FOUND AND XSD_FOUND)

    set(BOOST_LIBS
        boost_date_time
        boost_filesystem
        boost_system)
    visit_import_third_party(BOOST LIBS ${BOOST_LIBS})

    set(dam_file_perms OWNER_WRITE OWNER_READ
                       GROUP_WRITE GROUP_READ
                                   WORLD_READ)
    set(dam_dir_perms OWNER_WRITE OWNER_READ OWNER_EXECUTE
                      GROUP_WRITE GROUP_READ GROUP_EXECUTE
                                  WORLD_READ WORKD_EXECUTE)

    install(DIRECTORY ${DAMARIS_INCLUDE_DIR}
            DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}/../damaris
            FILE_PERMISSIONS ${dam_file_perms}
            DIRECTORY_PERMISSIONS ${dam_dir_perms})

    install(FILES ${DAMARIS_LIBRARY_DIR}/libdamaris.a
            DESTINATION ${VISIT_INSTALLED_VERSION_LIB}/../damaris/lib
            PERMISSIONS ${dam_file_perms})

    install(DIRECTORY ${XERCESC_INCLUDE_DIR}
            DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}/../damaris
            FILE_PERMISSIONS ${dam_file_perms}
            DIRECTORY_PERMISSIONS ${dam_dir_perms})

    install(FILES ${XERCESC_LIBRARY_DIR}/libxerces-c.a
            DESTINATION ${VISIT_INSTALLED_VERSION_LIB}/../damaris/lib
            PERMISSIONS ${dam_file_perms})

    install(DIRECTORY ${XSD_INCLUDE_DIR}
            DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}/../damaris
            FILE_PERMISSIONS ${dam_file_perms}
            DIRECTORY_PERMISSIONS ${dam_dir_perms})

    install(DIRECTORY ${BOOST_INCLUDE_DIR}
            DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}/../damaris
            FILE_PERMISSIONS ${dam_file_perms}
            DIRECTORY_PERMISSIONS ${dam_dir_perms})

    install(FILES ${BOOST_LIBRARY_DIR}/libboost_date_time.a
                  ${BOOST_LIBRARY_DIR}/libboost_filesystem.a
                  ${BOOST_LIBRARY_DIR}/libboost_system.a
            DESTINATION ${VISIT_INSTALLED_VERSION_LIB}/../damaris/lib
            PERMISSIONS ${dam_file_perms})

    unset(dam_file_perms)
    unset(dam_dir_perms)
endif()

