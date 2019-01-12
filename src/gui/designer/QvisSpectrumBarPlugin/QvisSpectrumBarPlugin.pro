CONFIG      += designer plugin debug_and_release
 
TARGET      = $$qtLibraryTarget($$TARGET)
TEMPLATE    = lib
QTDIR_build:DESTDIR     = $$QT_BUILD_TREE/plugins/designer

INCLUDEPATH += ../..

HEADERS     = QvisSpectrumBar.h \
              QvisSpectrumBarPlugin.h 
SOURCES     = QvisSpectrumBar.cpp \
              QvisSpectrumBarPlugin.cpp 

# install
target.path = $$[QT_INSTALL_PLUGINS]/designer
sources.files = $$SOURCES $$HEADERS *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/designer/QvisSpectrumBarPlugin
INSTALLS += target sources
