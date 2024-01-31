# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Thu Mar 24 16:18:19 PDT 2016
#   Added install for qt libraries, and necessary plugins (eg platform
#   specific).  Create a qt.conf file to be installed in the bin dir that
#   tells Qt where to find the plugins.
#
#   Kevin Griffin, Wed Nov 2 10:04:28 PDT 2016
#   Added logic to install the correct OSX frameworks and static library.
#
#   Kevin Griffin, Wed May 17 13:23:24 PDT 2017
#   Installed the platform plugins directory in the gui.app and viewer.app
#   directories containing the gui and viewer executeables for OSX. Also
#   added missing frameworks, archives, and includes.
#
#   Eric Brugger, Thu Oct  5 15:11:39 PDT 2017
#   Added Svg to the visit_qt_modules for all platforms. Previously, there
#   was separate logic for adding it to Windows and Mac, but Linux also
#   needs it.
#
#   Eric Brugger, Tue Oct 10 12:33:50 PDT 2017
#   Added Concurrent to the visit_qt_modules for Linux. Previously,
#   it was only adding it for Mac, but Linux also needs it.
#
#   Kathleen Biagas, Thu Sep 27 11:33:43 PDT 2018
#   Add UiTools to qt_libs_install.
#
#   Kevin Griffin, Mon Aug 26 12:15:21 PDT 2019
#   Installed the plugins/platforms and plugins/styles directories in the
#   xmledit.app and mcurvit.app directories.
#
#   Kathleen Biagas, Wed Jan 22 14:46:17 MST 2020
#   Add ssl dlls to install for Windows.
#
#   Kathleen Biagas, Tue Jan 31, 2023
#   Change how qtplugin files are copied on Windows.
#
#   Kathleen Biagas, Wed May 10, 2023
#   Add support for Qt6. 
#
#   Kathleen Biagas, Wed Dec 16, 20223
#   Add Concurrent to core list of qt modules to ensure it gets installed.
#
#*****************************************************************************

#[====[
 If the config-site file did not set a QT version, then set a
 default QT version of the minimum required.
#]====]

if(NOT QT_VERSION)
  set(QT_MINIMUM_VERSION "5.10.1")
  if(VISIT_QT_VERSION)
      if(${VISIT_QT_VERSION} VERSION_LESS ${QT_MINIMUM_VERSION})
          message(FATAL_ERROR "QT version must be at least ${QT_MINIMUM_VERSION}")
      endif()
      SETUP_APP_VERSION(QT ${VISIT_QT_VERSION})
  else()
      message(STATUS "VISIT_QT_VERSION NOT FOUND assuming ${QT_MINIMUM_VERSION}")
      SETUP_APP_VERSION(QT ${QT_MINIMUM_VERSION})
  endif()
endif()

if(NOT DEFINED VISIT_QT_DIR)
    message(FATAL_ERROR "Qt installation directory not specified")
endif()

set(visit_qt_modules Core Concurrent Gui Widgets OpenGL Network PrintSupport Svg Xml UiTools)

if(${QT_MAJOR_VERSION} EQUAL 5)
    list(APPEND visit_qt_modules Qml)

    if(LINUX)
        list(APPEND visit_qt_modules X11Extras)
    endif()
else()
    list(APPEND visit_qt_modules OpenGLWidgets)
endif()


set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${VISIT_QT_DIR}/lib/cmake)
find_package (Qt${QT_MAJOR_VERSION} REQUIRED ${visit_qt_modules})

# set up some alias so we don't have to worry about versioning elsewhere
set(QT_QTCORE_LIBRARY         ${Qt${QT_MAJOR_VERSION}Core_LIBRARIES})
set(QT_QTGUI_LIBRARY          ${Qt${QT_MAJOR_VERSION}Gui_LIBRARIES} ${OPENGL_LIBRARIES})
set(QT_QTNETWORK_LIBRARY      ${Qt${QT_MAJOR_VERSION}Network_LIBRARIES})
set(QT_QTOPENGL_LIBRARY       ${Qt${QT_MAJOR_VERSION}OpenGL_LIBRARIES})
set(QT_QTPRINTSUPPORT_LIBRARY ${Qt${QT_MAJOR_VERSION}PrintSupport_LIBRARIES})
set(QT_QTUITOOLS_LIBRARY      ${Qt${QT_MAJOR_VERSION}UiTools_LIBRARIES})
set(QT_QTWIDGETS_LIBRARY      ${Qt${QT_MAJOR_VERSION}Widgets_LIBRARIES})
set(QT_QTXML_LIBRARY          ${Qt${QT_MAJOR_VERSION}Xml_LIBRARIES})

if(${QT_MAJOR_VERSION} EQUAL 5 AND LINUX)
    set(QT_QTX11EXTRAS_LIBRARY ${Qt5X11Extras_LIBRARIES})
elseif(${QT_MAJOR_VERSION} EQUAL 6)
    set(QT_QTOPENGLWIDGETS_LIBRARY ${Qt${QT_MAJOR_VERSION}OpenGLWidgets_LIBRARIES})
endif()

if(NOT VISIT_QT_SKIP_INSTALL)
    # moc
    get_target_property(moc_location Qt${QT_MAJOR_VERSION}::moc LOCATION)
    install(PROGRAMS ${moc_location}
            DESTINATION ${VISIT_INSTALLED_VERSION_BIN}
            PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE
                        GROUP_WRITE GROUP_READ GROUP_EXECUTE
                                    WORLD_READ WORLD_EXECUTE)

    foreach(mod ${visit_qt_modules})
        list(APPEND qt_libs_install Qt${QT_MAJOR_VERSION}::${mod})

        if(APPLE)
            if(EXISTS ${VISIT_QT_DIR}/lib/Qt${mod}.framework)
                THIRD_PARTY_INSTALL_LIBRARY(${VISIT_QT_DIR}/lib/Qt${mod}.framework)
            else()
                get_target_property(lib_loc Qt${QT_MAJOR_VERSION}::${mod} LOCATION)
                THIRD_PARTY_INSTALL_LIBRARY(${lib_loc})
            endif()
        else()
            # headers
            foreach(H ${Qt${QT_MAJOR_VERSION}${mod}_INCLUDE_DIRS})
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
            install(FILES ${T}
                DESTINATION ${VISIT_INSTALLED_VERSION_ARCHIVES}
                PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE
                            GROUP_WRITE GROUP_READ GROUP_EXECUTE
                                        WORLD_READ WORLD_EXECUTE)
        endforeach()
    else()
        foreach(qtlib ${qt_libs_install})
            if(${QT_MAJOR_VERSION} EQUAL 5)
                get_target_property(qtlib_location ${qtlib} LOCATION)
            elseif(WIN32)
                get_target_property(qtlib_location ${qtlib} IMPORTED_IMPLIB)
            else()
                get_target_property(qtlib_location ${qtlib} IMPORTED_LOCATION)
            endif()
            # On Linux, the library names are Qtxxx.so.${QT_VERSION}
            # We need to remove the version part so that
            # THIRD_PARTY_INSTALL_LIBRARY will work correctly.
            if (LINUX)
                string(REPLACE ".${Qt${QT_MAJOR_VERSION}Core_VERSION}" ""
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
    if(WIN32)
        install(DIRECTORY ${VISIT_QT_DIR}/plugins/platforms
                DESTINATION ${VISIT_INSTALLED_VERSION_BIN}/qtplugins)
      if(${QT_MAJOR_VERSION} EQUAL 5)
          install(DIRECTORY ${VISIT_QT_DIR}/plugins/printsupport
                  DESTINATION ${VISIT_INSTALLED_VERSION_BIN}/qtplugins)
      endif()

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
        if(${QT_MAJOR_VERSION} EQUAL 5)
            add_custom_command(TARGET copy_qt_plugins
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${VISIT_QT_DIR}/plugins/printsupport
                    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ThirdParty/qtplugins/printsupport)
        endif()

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
         THIRD_PARTY_INSTALL_LIBRARY(${VISIT_QT_DIR}/lib/libQt${QT_MAJOR_VERSION}XcbQpa.so)
    elseif(WIN32)
        # Need the ssl dll's too.
        if(NOT OPENSSL_ROOT_DIR)
          set(OPENSSL_ROOT_DIR "${VISIT_QT_DIR}")
        endif()
        if(${QT_MAJOR_VERSION} EQUAL 5)
            set(ssldlls ${OPENSSL_ROOT_DIR}/bin/libeay32.dll
                        ${OPENSSL_ROOT_DIR}/bin/ssleay32.dll)
        else()
            set(ssldlls ${OPENSSL_ROOT_DIR}/bin/libcrypto-1_1-x64.dll
                        ${OPENSSL_ROOT_DIR}/bin/libssl-1_1-x64.dll)
        endif()
        file(COPY ${ssldlls} 
             DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ThirdParty
             FILE_PERMISSIONS OWNER_READ OWNER_WRITE
                              GROUP_READ GROUP_WRITE
                              WORLD_READ)
        install(FILES ${ssldlls}
                DESTINATION ${VISIT_INSTALLED_VERSION_BIN}
                PERMISSIONS OWNER_READ OWNER_WRITE
                            GROUP_READ GROUP_WRITE
                            WORLD_READ)
    endif()
endif()

