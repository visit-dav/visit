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
#   Kathleen Biagas, Mon May  1 09:09:20 PDT 2023
#   Use separate modules for finding Qt5 amd Qt6.
#
#*****************************************************************************

#[=[
 If the config-site file did not set a QT version, then set a
 default QT version of the minimum required.
#]=]

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

if(QT_VERSION VERSION_LESS "6.0.0")
    include(${VISIT_SOURCE_DIR}/CMake/FindQt5.cmake)
else()
    include(${VISIT_SOURCE_DIR}/CMake/FindQt6.cmake)
endif()

