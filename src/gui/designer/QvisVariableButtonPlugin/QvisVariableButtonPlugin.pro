CONFIG      += designer plugin debug_and_release
 
TARGET      = $$qtLibraryTarget($$TARGET)
TEMPLATE    = lib
QTDIR_build:DESTDIR     = $$QT_BUILD_TREE/plugins/designer

INCLUDEPATH += ../../../include/visit

DEFINES += "DESIGNER_PLUGIN"

HEADERS     = QvisVariableButton.h \
              QvisVariableButtonPlugin.h 
SOURCES     = QvisVariableButton.cpp \
              QvisVariableButtonPlugin.cpp 

# install
target.path = $$[QT_INSTALL_PLUGINS]/designer
sources.files = $$SOURCES $$HEADERS *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/designer/QvisVariableButtonPlugin
INSTALLS += target sources
