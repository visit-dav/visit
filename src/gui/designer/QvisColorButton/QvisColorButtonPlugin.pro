CONFIG      += designer plugin debug_and_release
 
TARGET      = $$qtLibraryTarget($$TARGET)
TEMPLATE    = lib
QTDIR_build:DESTDIR     = $$QT_BUILD_TREE/plugins/designer

INCLUDEPATH += ../..

HEADERS     = QvisColorButton.h \
              QvisColorButtonPlugin.h \
              QvisColorSelectionWidget.h \
              QvisGridWidget.h \
              QvisColorGridWidget.h
SOURCES     = QvisColorButton.cpp \
              QvisColorButtonPlugin.cpp \
              QvisColorSelectionWidget.cpp \
              QvisGridWidget.cpp \
              QvisColorGridWidget.cpp
# install
target.path = $$[QT_INSTALL_PLUGINS]/designer
sources.files = $$SOURCES $$HEADERS *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/designer/QvisColorButtonPlugin
INSTALLS += target sources
