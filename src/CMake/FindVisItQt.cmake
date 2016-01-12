IF(VISIT_QT5)
    INCLUDE(CMake/FindVisItQt5.cmake)
ELSE()
    INCLUDE(CMake/FindVisItQt4.cmake)
ENDIF()


