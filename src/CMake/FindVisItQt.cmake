IF(NOT DEFINED VISIT_QT5)
    INCLUDE(CMake/FindVisItQt4.cmake)
ELSE()
    INCLUDE(CMake/FindVisItQt5.cmake)
ENDIF()


