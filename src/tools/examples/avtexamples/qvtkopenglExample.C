// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <GUIWindow.h>

#include <QApplication>

#include <QVTKOpenGLWidget.h>

#include <iostream>

#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkInteractionStyle)
VTK_MODULE_INIT(vtkRenderingOpenGL2)

int
main(int argc, char **argv)
{
    int retval = 0;

    //
    // Setting the default QSurfaceFormat required with QVTKOpenGLwidget.
    // This causes Qt to create an OpenGL 3.2 context.
    //
    auto surfaceFormat = QVTKOpenGLWidget::defaultFormat();
    surfaceFormat.setSamples(0);
    surfaceFormat.setAlphaBufferSize(0);
    QSurfaceFormat::setDefaultFormat(surfaceFormat);

    //
    // Create the QApplication. This sets the qApp pointer.
    //
    QApplication *mainApp = new QApplication(argc, argv, true);

    GUIWindow *guiWin = new GUIWindow();

    guiWin->show();
    guiWin->raise();

    retval = mainApp->exec();

    return retval;
}
