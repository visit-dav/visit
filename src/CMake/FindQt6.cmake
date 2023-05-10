# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#[============================================================================[
  Modifications:
#]============================================================================]


if(NOT DEFINED VISIT_QT_DIR)
    message(FATAL_ERROR "Qt6 installation directory not specified")
endif()

set(visit_qt_modules Core Gui Widgets OpenGL OpenGLWidgets Network PrintSupport Svg Xml UiTools)

# is this needed with Qt6 ?
#if(APPLE)
#    list(APPEND visit_qt_modules Concurrent)
#endif()

set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${VISIT_QT_DIR}/lib/cmake)

find_package(Qt6 REQUIRED ${visit_qt_modules})


set(QT_QTUITOOLS_LIBRARY ${Qt6UiTools_LIBRARIES})
set(QT_QTOPENGL_LIBRARY ${Qt6OpenGL_LIBRARIES})

set(QT_QTGUI_LIBRARY ${Qt6Gui_LIBRARIES} ${OPENGL_LIBRARIES})
set(QT_QTWIDGETS_LIBRARY ${Qt6Widgets_LIBRARIES})
set(QT_QTOPENGLWIDGETS_LIBRARY ${Qt6OpenGLWidgets_LIBRARIES})
set(QT_QTPRINTSUPPORT_LIBRARY ${Qt6PrintSupport_LIBRARIES})


set(QT_QTNETWORK_LIBRARY ${Qt6Network_LIBRARIES})
set(QT_QTXML_LIBRARY ${Qt6Xml_LIBRARIES})

set(QT_QTCORE_LIBRARY ${Qt6Core_LIBRARIES})

if(NOT VISIT_QT_SKIP_INSTALL)
    # moc
    get_target_property(moc_location Qt6::moc LOCATION)
    install(PROGRAMS ${moc_location}
            DESTINATION ${VISIT_INSTALLED_VERSION_BIN}
            PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE
            GROUP_WRITE GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE)

    foreach(mod ${visit_qt_modules})
        list(APPEND qt_libs_install Qt6::${mod})  

        if(APPLE)
            if(EXISTS ${VISIT_QT_DIR}/lib/Qt${mod}.framework)
                THIRD_PARTY_INSTALL_LIBRARY(${VISIT_QT_DIR}/lib/Qt${mod}.framework)
            else()
                get_target_property(lib_loc Qt6::${mod} LOCATION)
                THIRD_PARTY_INSTALL_LIBRARY(${lib_loc})
            endif()
        else()
            # headers
            foreach(H ${Qt6${mod}_INCLUDE_DIRS})
                if(${H} MATCHES "/include/Qt")
                    INSTALL(DIRECTORY ${H}
                            DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}/qt/include
                            FILE_PERMISSIONS OWNER_WRITE OWNER_READ
                                             GROUP_WRITE GROUP_READ
                                             WORLD_READ
                            DIRECTORY_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE
                                                  GROUP_WRITE GROUP_READ GROUP_EXECUTE
                                                  WORLD_READ WORLD_EXECUTE)
                endif()
            endforeach()
        endif()
    endforeach()
 
    if(APPLE)
        file(GLOB QT_INCLUDES "${VISIT_QT_DIR}/include/Qt*")
        foreach(H ${QT_INCLUDES})
            install(DIRECTORY ${H}
                    DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}/qt/include
                    FILE_PERMISSIONS OWNER_WRITE OWNER_READ
                                       GROUP_WRITE GROUP_READ
                                       WORLD_READ
                    DIRECTORY_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE
                                            GROUP_WRITE GROUP_READ GROUP_EXECUTE
                                            WORLD_READ WORLD_EXECUTE)
        endforeach()

        # Add Qt archives (lib*.a)
        file(GLOB QT_ARCHIVES "${VISIT_QT_DIR}/lib/*.a")
        foreach(T ${QT_ARCHIVES})
            INSTALL(FILES ${T}
                DESTINATION ${VISIT_INSTALLED_VERSION_ARCHIVES}
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_WRITE GROUP_EXECUTE
                    WORLD_READ             WORLD_EXECUTE)
        endforeach()
    else()
        foreach(qtlib ${qt_libs_install})
            if(WIN32)
                get_target_property(qtlib_location ${qtlib} IMPORTED_IMPLIB)
            else()
                get_target_property(qtlib_location ${qtlib} IMPORTED_LOCATION)
            endif()
            # On Linux, the library names are Qt6xxx.so.${QT_VERSION}
            # We need to remove the version part so that THIRD_PARTY_INSTALL_LIBRARY
            # will work correctly.
            if (LINUX)
                string(REPLACE ".${Qt6Core_VERSION}" ""
                       qtlib_location ${qtlib_location})
            endif()
            THIRD_PARTY_INSTALL_LIBRARY(${qtlib_location})
        endforeach()
    endif()

    # We need a qt.conf file telling qt where to find the plugins
    if(WIN32)
        file(WRITE ${VISIT_BINARY_DIR}/qt.conf "[Paths]\nPlugins=./qtplugins\n")
    else()
        file(WRITE ${VISIT_BINARY_DIR}/qt.conf "[Paths]\nPlugins=../lib/qtplugins\n")
    endif()

    install(FILES ${VISIT_BINARY_DIR}/qt.conf
            DESTINATION ${VISIT_INSTALLED_VERSION_BIN}
            PERMISSIONS OWNER_READ OWNER_WRITE
                        GROUP_READ GROUP_WRITE
                        WORLD_READ)

    # Platform plugins
    if (WIN32)
        install(DIRECTORY ${VISIT_QT_DIR}/plugins/platforms
                DESTINATION ${VISIT_INSTALLED_VERSION_BIN}/qtplugins)

        # We also need platforms and qt.conf in the build dir.
        file(WRITE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/qt.conf
                 "[Paths]\nPlugins=../ThirdParty/qtplugins\n")

        add_custom_target(copy_qt_plugins ALL
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/qt.conf
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/qt.conf
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${VISIT_QT_DIR}/plugins/platforms
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ThirdParty/qtplugins/platforms)

         visit_add_to_util_builds(copy_qt_plugins)

    elseif(APPLE)
        install(DIRECTORY ${VISIT_QT_DIR}/plugins/platforms
                DESTINATION ${VISIT_INSTALLED_VERSION_BIN}/gui.app/Contents/MacOS)

        install(DIRECTORY ${VISIT_QT_DIR}/plugins/platforms
                DESTINATION ${VISIT_INSTALLED_VERSION_BIN}/viewer.app/Contents/MacOS)

        install(DIRECTORY ${VISIT_QT_DIR}/plugins/platforms
                DESTINATION ${VISIT_INSTALLED_VERSION_BIN}/xmledit.app/Contents/MacOS)

        install(DIRECTORY ${VISIT_QT_DIR}/plugins/platforms
                DESTINATION ${VISIT_INSTALLED_VERSION_BIN}/mcurvit.app/Contents/MacOS)

        install(DIRECTORY ${VISIT_QT_DIR}/plugins/styles
                DESTINATION ${VISIT_INSTALLED_VERSION_BIN}/gui.app/Contents/MacOS)

        install(DIRECTORY ${VISIT_QT_DIR}/plugins/styles
                DESTINATION ${VISIT_INSTALLED_VERSION_BIN}/viewer.app/Contents/MacOS)

        install(DIRECTORY ${VISIT_QT_DIR}/plugins/styles
                DESTINATION ${VISIT_INSTALLED_VERSION_BIN}/xmledit.app/Contents/MacOS)

        install(DIRECTORY ${VISIT_QT_DIR}/plugins/styles
                DESTINATION ${VISIT_INSTALLED_VERSION_BIN}/mcurvit.app/Contents/MacOS)
    else()
        install(DIRECTORY ${VISIT_QT_DIR}/plugins/platforms
                DESTINATION ${VISIT_INSTALLED_VERSION_LIB}/qtplugins)
    endif()

    if(LINUX)
        # Xcb related plugins
        install(DIRECTORY ${VISIT_QT_DIR}/plugins/xcbglintegrations
                DESTINATION ${VISIT_INSTALLED_VERSION_LIB}/qtplugins)

         # there is also a platform-plugin related library that
         # needs to be installed, but there doesn't seem to be
         # a way to find this via Qt's cmake mechanisms, hence this
         # hard-coded extra step
         THIRD_PARTY_INSTALL_LIBRARY(${VISIT_QT_DIR}/lib/libQt6XcbQpa.so)
    elseif(WIN32)
        # Need the ssl dll's too.
        if(NOT OPENSSL_ROOT_DIR)
          set(OPENSSL_ROOT_DIR "${VISIT_QT_DIR}")
        endif()
        file(COPY ${OPENSSL_ROOT_DIR}/bin/libcrypto-1_1-x64.dll
                  ${OPENSSL_ROOT_DIR}/bin/libssl-1_1-x64.dll
             DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ThirdParty
             FILE_PERMISSIONS OWNER_READ OWNER_WRITE
                              GROUP_READ GROUP_WRITE
                              WORLD_READ)
        install(FILES ${OPENSSL_ROOT_DIR}/bin/libcrypto-1_1-x64.dll
                      ${OPENSSL_ROOT_DIR}/bin/libssl-1_1-x64.dll
                DESTINATION ${VISIT_INSTALLED_VERSION_BIN}
                PERMISSIONS OWNER_READ OWNER_WRITE
                            GROUP_READ GROUP_WRITE
                            WORLD_READ)
    endif()
endif()

