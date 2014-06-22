# Configure for Qt5..
IF(NOT DEFINED VISIT_QT_DIR)
    MESSAGE(FATAL_ERROR "Qt5 installation directory not specified")
ENDIF()

set(QT_MOC_EXECUTABLE ${VISIT_QT_DIR}/bin/moc)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
#set(QT5_INCLUDE_DIRS "")
set(QT5_LIBRARIES "")

set(visit_qt_modules Core Gui Widgets OpenGL Network PrintSupport Xml UiTools)
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${VISIT_QT_DIR}/lib/cmake)
find_package (Qt5 REQUIRED ${visit_qt_modules})


foreach(mod ${visit_qt_modules})
  string(TOUPPER ${mod} upper_mod)
  if(NOT VISIT_QT_SKIP_INSTALL)
    if(WIN32 AND EXISTS ${VISIT_QT_DIR}/lib/Qt5${mod}.lib)
      THIRD_PARTY_INSTALL_LIBRARY(${VISIT_QT_DIR}/lib/Qt5${mod}.lib)
    endif()
    # headers
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
  endif()
endforeach()


set(QT_QTUITOOLS_LIBRARY ${Qt5UiTools_LIBRARIES})
set(QT_QTOPENGL_LIBRARY ${Qt5OpenGL_LIBRARIES})

# if/when we drop support for qt 4, perhaps leave these split and
# add Widgets or PrintSupport only where needed
set(QT_QTGUI_LIBRARY ${Qt5Gui_LIBRARIES} 
                     ${Qt5Widgets_LIBRARIES} 
                     ${Qt5PrintSupport_LIBRARIES})
set(QT_QTNETWORK_LIBRARY ${Qt5Network_LIBRARIES})
set(QT_QTXML_LIBRARY ${Qt5Xml_LIBRARIES})

# why is core not named the same as the others?
set(QT_CORE_LIBRARY ${Qt5Core_LIBRARIES})

if(NOT VISIT_QT_SKIP_INSTALL)
  # moc
  get_target_property(moc_location Qt5::moc LOCATION)
  MESSAGE("moc location: ${moc_location}")
  install(PROGRAMS ${moc_location}
          DESTINATION ${VISIT_INSTALLED_VERSION_BIN}
          PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE
          GROUP_WRITE GROUP_READ GROUP_EXECUTE
          WORLD_READ WORLD_EXECUTE
  )
endif()
