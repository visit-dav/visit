CONFIG      += designer plugin debug_and_release
 
TARGET      = $$qtLibraryTarget($$TARGET)
TEMPLATE    = lib
QTDIR_build:DESTDIR     = $$QT_BUILD_TREE/plugins/designer

INCLUDEPATH += ../..

DEFINES += "DESIGNER_PLUGIN"

HEADERS     = QvisDialogLineEdit.h \
              QvisDialogLineEditPlugin.h 
SOURCES     = QvisDialogLineEdit.cpp \
              QvisDialogLineEditPlugin.cpp 

# install
target.path = $$[QT_INSTALL_PLUGINS]/designer
sources.files = $$SOURCES $$HEADERS *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/designer/QvisDialogLineEditPlugin
INSTALLS += target sources
