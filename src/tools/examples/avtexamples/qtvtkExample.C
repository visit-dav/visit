// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <GUIWindow2.h>

#include <QApplication>

#include <visit-config.h> // For LIB_VERSION_LE
#if LIB_VERSION_LE(VTK,8,1,0)
#include <QVTKOpenGLWidget.h>
#else
#include <QVTKOpenGLNativeWidget.h>
#endif

#include <iostream>

#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkInteractionStyle)

int
main(int argc, char **argv)
{
    int retval = 0;

    //
    // Setting the default QSurfaceFormat required with QVTKOpenGLwidget.
    // This causes Qt to create an OpenGL 3.2 context.
    //
#if LIB_VERSION_LE(VTK,8,1,0)
    auto surfaceFormat = QVTKOpenGLWidget::defaultFormat();
#else
    auto surfaceFormat = QVTKOpenGLNativeWidget::defaultFormat();
#endif
    surfaceFormat.setSamples(0);
    surfaceFormat.setAlphaBufferSize(0);
    QSurfaceFormat::setDefaultFormat(surfaceFormat);

    //
    // Create the QApplication. This sets the qApp pointer.
    //
    QApplication *mainApp = new QApplication(argc, argv, true);

    GUIWindow2 *guiWin = new GUIWindow2();

    guiWin->show();
    guiWin->raise();

    retval = mainApp->exec();

    return retval;
}
