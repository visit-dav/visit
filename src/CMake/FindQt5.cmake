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
#*****************************************************************************


# Configure for Qt5..
IF(NOT DEFINED VISIT_QT_DIR)
    MESSAGE(FATAL_ERROR "Qt5 installation directory not specified")
ENDIF()

if(WIN32)
    set(QT_MOC_EXECUTABLE ${VISIT_QT_DIR}/bin/moc.exe)
else()
    set(QT_MOC_EXECUTABLE ${VISIT_QT_DIR}/bin/moc)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
#set(QT5_INCLUDE_DIRS "")
set(QT5_LIBRARIES "")

set(visit_qt_modules Core Gui Widgets OpenGL Network PrintSupport Qml Svg Xml UiTools)

if(LINUX)
    set (visit_qt_modules ${visit_qt_modules} Concurrent X11Extras)
endif()

if(APPLE)
    set (visit_qt_modules ${visit_qt_modules} Concurrent)
endif()

set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${VISIT_QT_DIR}/lib/cmake)
find_package (Qt5 REQUIRED ${visit_qt_modules})


foreach(mod ${visit_qt_modules})
  string(TOUPPER ${mod} upper_mod)
  if(NOT VISIT_QT_SKIP_INSTALL)
    if(WIN32 AND EXISTS ${VISIT_QT_DIR}/lib/Qt5${mod}.lib)
      THIRD_PARTY_INSTALL_LIBRARY(${VISIT_QT_DIR}/lib/Qt5${mod}.lib)
    elseif(APPLE)
      if(EXISTS ${VISIT_QT_DIR}/lib/Qt${mod}.framework)
          THIRD_PARTY_INSTALL_LIBRARY(${VISIT_QT_DIR}/lib/Qt${mod}.framework)
      else()
          get_target_property(lib_loc Qt5::${mod} LOCATION)
          THIRD_PARTY_INSTALL_LIBRARY(${lib_loc})
      endif()
    endif()
    # headers
    if(NOT APPLE)
    foreach(H ${Qt5${mod}_INCLUDE_DIRS})
      if(${H} MATCHES "/include/Qt")
        INSTALL(DIRECTORY ${H}
                DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}/qt/include
                FILE_PERMISSIONS OWNER_WRITE OWNER_READ
                                   GROUP_WRITE GROUP_READ
                                   WORLD_READ
                DIRECTORY_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE
                                        GROUP_WRITE GROUP_READ GROUP_EXECUTE
                                        WORLD_READ WORLD_EXECUTE
                PATTERN ".svn" EXCLUDE
        )
      endif()
    endforeach()
    endif(NOT APPLE)
  endif()
endforeach()


set(QT_QTUITOOLS_LIBRARY ${Qt5UiTools_LIBRARIES})
set(QT_QTOPENGL_LIBRARY ${Qt5OpenGL_LIBRARIES})

set(QT_QTGUI_LIBRARY ${Qt5Gui_LIBRARIES} ${OPENGL_LIBRARIES})
set(QT_QTWIDGETS_LIBRARY ${Qt5Widgets_LIBRARIES})
set(QT_QTPRINTSUPPORT_LIBRARY ${Qt5PrintSupport_LIBRARIES})

set(QT_QTNETWORK_LIBRARY ${Qt5Network_LIBRARIES})
set(QT_QTXML_LIBRARY ${Qt5Xml_LIBRARIES})

set(QT_QTCORE_LIBRARY ${Qt5Core_LIBRARIES})

if (LINUX)
    set(QT_QTX11EXTRAS_LIBRARY ${Qt5X11Extras_LIBRARIES})
endif()

if(NOT VISIT_QT_SKIP_INSTALL)
  # moc
  get_target_property(moc_location Qt5::moc LOCATION)
  MESSAGE(STATUS "moc location: ${moc_location}")
  install(PROGRAMS ${moc_location}
          DESTINATION ${VISIT_INSTALLED_VERSION_BIN}
          PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE
          GROUP_WRITE GROUP_READ GROUP_EXECUTE
          WORLD_READ WORLD_EXECUTE
  )

  set(qt_libs_install
        Qt5::Core
        Qt5::Gui
        Qt5::Network
        Qt5::OpenGL
        Qt5::PrintSupport
        Qt5::Widgets
        Qt5::Qml
        Qt5::Svg
        Qt5::Xml
        Qt5::UiTools
  )
  if(LINUX)
      set(qt_libs_install ${qt_libs_install} Qt5::Concurrent Qt5::X11Extras)
  endif()
  if(APPLE)
      set(qt_libs_install ${qt_libs_install} Qt5::Concurrent)
  endif()

  IF(APPLE)
      file(GLOB QT_INCLUDES "${VISIT_QT_DIR}/include/Qt*")
      FOREACH(H ${QT_INCLUDES})
          INSTALL(DIRECTORY ${H}
                  DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}/qt/include
                  FILE_PERMISSIONS OWNER_WRITE OWNER_READ
                                     GROUP_WRITE GROUP_READ
                                     WORLD_READ
                  DIRECTORY_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE
                                          GROUP_WRITE GROUP_READ GROUP_EXECUTE
                                          WORLD_READ WORLD_EXECUTE
                  PATTERN ".svn" EXCLUDE
          )
      ENDFOREACH(H)

      # Add Qt archives (lib*.a)
      file(GLOB QT_ARCHIVES "${VISIT_QT_DIR}/lib/*.a")
      FOREACH(T ${QT_ARCHIVES})
          INSTALL(FILES ${T}
              DESTINATION ${VISIT_INSTALLED_VERSION_ARCHIVES}
              PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                  GROUP_READ GROUP_WRITE GROUP_EXECUTE
                  WORLD_READ             WORLD_EXECUTE
              CONFIGURATIONS "" None Debug Release RelWithDebInfo MinSizeRel
          )
      ENDFOREACH(T)
  ENDIF(APPLE)

  if(NOT APPLE)
      foreach(qtlib ${qt_libs_install})
          get_target_property(qtlib_location ${qtlib} LOCATION)
          # On Linux, the library names are Qt5xxx.so.${QT_VERSION}
          # We need to remove the version part so that THIRD_PARTY_INSTALL_LIBRARY
          # will work correctly.
          if (LINUX)
              string(REPLACE ".${Qt5Core_VERSION}" ""
                     qtlib_location ${qtlib_location})
          endif()
          THIRD_PARTY_INSTALL_LIBRARY(${qtlib_location})
      endforeach()
  endif(NOT APPLE)


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
      install(DIRECTORY ${VISIT_QT_DIR}/plugins/printsupport
              DESTINATION ${VISIT_INSTALLED_VERSION_BIN}/qtplugins)

      # We also need the platforms, print support and qt.conf in the build dir.
      file(WRITE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/qt.conf
               "[Paths]\nPlugins=../ThirdParty/qtplugins\n")

      add_custom_target(copy_qt_plugins ALL
          COMMAND ${CMAKE_COMMAND} -E copy_if_different
              ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/qt.conf
              ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/qt.conf
          COMMAND ${CMAKE_COMMAND} -E copy_directory
              ${VISIT_QT_DIR}/plugins/platforms
              ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ThirdParty/qtplugins/platforms
          COMMAND ${CMAKE_COMMAND} -E copy_directory
              ${VISIT_QT_DIR}/plugins/printsupport
              ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ThirdParty/qtplugins/printsupport)

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

  if (LINUX)
      # Xcb related plugins
      install(DIRECTORY ${VISIT_QT_DIR}/plugins/xcbglintegrations
              DESTINATION ${VISIT_INSTALLED_VERSION_LIB}/qtplugins)

          # there is also a platform-plugin related library that
          # needs to be installed, but there doesn't seem to be
          # a way to find this via Qt's cmake mechanisms, hence this
          # hard-coded extra step
          THIRD_PARTY_INSTALL_LIBRARY(${VISIT_QT_DIR}/lib/libQt5XcbQpa.so)
  endif()

  if(WIN32)
      # Need the ssl dll's too.
      if (NOT OPENSSL_ROOT_DIR)
        set(OPENSSL_ROOT_DIR "${VISIT_QT_DIR}")
      endif ()
      file(COPY ${OPENSSL_ROOT_DIR}/bin/libeay32.dll
                ${OPENSSL_ROOT_DIR}/bin/ssleay32.dll
           DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ThirdParty
           FILE_PERMISSIONS OWNER_READ OWNER_WRITE
                            GROUP_READ GROUP_WRITE
                            WORLD_READ
      )
      install(FILES ${OPENSSL_ROOT_DIR}/bin/libeay32.dll
                    ${OPENSSL_ROOT_DIR}/bin/ssleay32.dll
              DESTINATION ${VISIT_INSTALLED_VERSION_BIN}
              PERMISSIONS OWNER_READ OWNER_WRITE
                          GROUP_READ GROUP_WRITE
                          WORLD_READ
              CONFIGURATIONS "" None Debug Release RelWithDebInfo MinSizeRel
      )
  endif()
endif()
