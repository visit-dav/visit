IF(VISIT_QT4)
    INCLUDE(CMake/FindVisItQt4.cmake)
ELSE()
    INCLUDE(CMake/FindVisItQt5.cmake)
ENDIF()


