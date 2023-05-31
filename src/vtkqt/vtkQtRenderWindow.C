// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <visit-config.h> // For LIB_VERSION_LE

#if LIB_VERSION_LE(VTK, 8,1,0)
#include "vtkOpenGL.h"
#else
#include <vtk_glew.h>
#endif

#include <vtkQtRenderWindow.h>

#if LIB_VERSION_LE(VTK, 8,1,0)
#include "QVTKOpenGLWidget.h"
#else
#include <QVTKOpenGLNativeWidget.h>
#endif

#include <vtkGenericOpenGLRenderWindow.h>
#include <QVTKInteractor.h>
#include <vtkRenderWindow.h>

#include <QApplication>
#include <vtkUnsignedCharArray.h>

#ifdef __linux__
#define GLX_GLXEXT_LEGACY
#include <GL/glx.h>
#endif

#if LIB_VERSION_LE(VTK, 8,1,0)
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2);
#endif


#ifdef Q_OS_MAC
#include "osxHelper.h"
#include <QSysInfo>
#endif

// ****************************************************************************
// Class: vtkQtRenderWindowPrivate
//
// Purpose:
//   Private data storage for the vtkQtRenderWindow class.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   2010
//
// Modifications:
//   Brad Whitlock, Wed Jan 25 23:13:09 PST 2012
//   I added code to pass the QGLFormat to the widget so we explicitly request
//   an alpha channel. This fixes transparency on my Linux box. Maybe certain
//   systems don't have alpha channel defaulted to enabled.
//
//   Brad Whitlock, Fri Apr  6 11:39:00 PDT 2012
//   Add path for stereo initialization.
//   
//   Kevin Griffin, Thu Apr 27 11:32:52 PDT 2017
//   Disabled High DPI (Retina) for QT5 on OSX until the bug that causes the 
//   viewer to display random content is fixed.
//
//   Kevin Griffin, Tue Feb 27 14:23:36 PST 2018
//   Added additional checks before disabling High DPI since it causes
//   problems with the viewer window in OSX 10.13 and Qt 5.10.0 appears to
//   have fixed the underlying issue. This fixes Bug #3020.
//
//   Kevin Griffin, Thu May 17 08:16:16 PDT 2018
//   Commented-out the disable HiDPI work-around for the trunk since we've 
//   upgraded to VTK-8 and are no longer using QVTKWidget/2 which was causing 
//   the initial issues. I will investigate whether another form of work-around
//   is needed, if not I will remove the uncommented code.
//
//   Kevin Griffin, Thu Aug 30 18:55:01 PDT 2018
//   Removed the setting of the Qt::AA_ImmediateWidgetCreation attribute
//   for OSX since it was creating a black screen during zooming and lineouts
//   on OSX. Eliminated the call to setWindowFlags for OSX since it causes
//   the toolbar window to not render correctly.
//
// ****************************************************************************

class VTKQT_API vtkQtRenderWindowPrivate
{
public:
    vtkQtRenderWindowPrivate(vtkQtRenderWindow *w, bool stereo)
    {
        resizeEventCallback = NULL;
        resizeEventData = NULL;
        closeEventCallback = NULL;
        closeEventCallbackData = NULL;
        hideEventCallback = NULL;
        hideEventCallbackData = NULL;
        showEventCallback = NULL;
        showEventCallbackData = NULL;

        // With Qt5 there an issue with QVTKWidget2 asking for an
        // alpha channel (at least for OS X). However, not asking for
        // the channel seems to be benign. In addition, the 2D view
        // bounds and picking are off.
#if LIB_VERSION_LE(VTK, 8,1,0)
        gl = new QVTKOpenGLWidget(w);
        if (!gl->GetRenderWindow())
        {
            vtkGenericOpenGLRenderWindow *renWin = vtkGenericOpenGLRenderWindow::New();
            gl->SetRenderWindow(renWin);
            renWin->Delete();
        }
        gl->GetRenderWindow()->AlphaBitPlanesOn();
        gl->GetRenderWindow()->SetStereoRender( stereo );
#else
        // VTK seems to recommend using NativeWidget, so give it a try, but we
        // may need to use QVTKOpenGLStereoWidget which is the old
        // QVTKOpenGLWidget renamed
        gl = new QVTKOpenGLNativeWidget(w);
        if (!gl->renderWindow())
        {
            vtkGenericOpenGLRenderWindow *renWin = vtkGenericOpenGLRenderWindow::New();
            gl->setRenderWindow(renWin);
            renWin->Delete();
        }
        gl->renderWindow()->AlphaBitPlanesOn();
        gl->renderWindow()->SetStereoRender( stereo );
#endif
    }

    virtual ~vtkQtRenderWindowPrivate()
    {
    }
#if LIB_VERSION_LE(VTK, 8,1,0)
    QVTKOpenGLWidget     *gl;
#else
    QVTKOpenGLNativeWidget     *gl;
#endif

    void          (*resizeEventCallback)(void *);
    void           *resizeEventData;
    void          (*closeEventCallback)(void *);
    void           *closeEventCallbackData;
    void          (*hideEventCallback)(void *);
    void           *hideEventCallbackData;
    void          (*showEventCallback)(void *);
    void           *showEventCallbackData;
};


vtkQtRenderWindow::vtkQtRenderWindow(QWidget *parent, Qt::WindowFlags f) : QMainWindow(parent, f)
{
    d = new vtkQtRenderWindowPrivate(this, false);
    setIconSize(QSize(20,20));
    setUnifiedTitleAndToolBarOnMac(false);
    setAnimated(false);
    setWindowFlags(f);

    // With the call to setCentralWidget() vtkQtRenderWindow takes
    // ownership of the gl widget pointer and deletes it at the
    // appropriate time.
    setCentralWidget(d->gl);
}

vtkQtRenderWindow::vtkQtRenderWindow(bool stereo, QWidget *parent, Qt::WindowFlags f) : QMainWindow(parent, f)
{
    d = new vtkQtRenderWindowPrivate(this, stereo);
    setIconSize(QSize(20,20));
    setUnifiedTitleAndToolBarOnMac(false);
    setAnimated(false);
    setWindowFlags(f);
    
    // With the call to setCentralWidget() vtkQtRenderWindow takes
    // ownership of the gl widget pointer and deletes it at the
    // appropriate time.
    setCentralWidget(d->gl);
}


vtkQtRenderWindow::~vtkQtRenderWindow()
{
    delete d;
}

vtkQtRenderWindow *
vtkQtRenderWindow::New()
{
    return new vtkQtRenderWindow();
}

vtkQtRenderWindow *
vtkQtRenderWindow::New(bool stereo)
{
    return new vtkQtRenderWindow(stereo);
}

void
vtkQtRenderWindow::Delete()
{
    deleteLater();
}

    // Description:
    // Get the vtk render window.
vtkRenderWindow *
vtkQtRenderWindow::GetRenderWindow()
{
#if LIB_VERSION_LE(VTK, 8,1,0)
    return d->gl->GetRenderWindow();
#else
    return d->gl->renderWindow();
#endif
}

    // Description:
    // Get the Qt/vtk interactor that was either created by default or set by the user
vtkRenderWindowInteractor * 
vtkQtRenderWindow::GetInteractor()
{
#if LIB_VERSION_LE(VTK, 8,1,0)
    return d->gl->GetInteractor();
#else
    return d->gl->interactor();
#endif
}

QWidget *
vtkQtRenderWindow::GetGLWidget()
{
    return d->gl;
}

//
// Added by LLNL
//

// ****************************************************************************
// Method: vtkQtRenderWindow::mapFromGlobal
//
// Purpose: 
//   Since most of the window is occupied by the gl widget, let's transform
//   points relative to the gl widget instead of the main window.
//
// Arguments:
//   pos : The point to be transformed.
//
// Returns:    A transformed point.
//
// Note:       This makes lineout work correctly with the new widget scheme.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 28 08:01:53 PDT 2003
//
// Modifications:
//   
// ****************************************************************************
/*
QPoint
vtkQtRenderWindow::mapFromGlobal(const QPoint &pos) const
{
    return d->gl->mapFromGlobal(pos);
}
*/

// ****************************************************************************
// Method: vtkQtRenderWindow::SetResizeCallback
//
// Purpose: 
//   Set a callback for a window resize event.
//
// Arguments:
//   callback  The callback routine.
//   data      A pointer to the data to pass the callback routine.
//
// Programmer: Eric Brugger
// Creation:   August 17, 2001
//
// ****************************************************************************

void
vtkQtRenderWindow::SetResizeCallback(void (*callback)(void *), void *data)
{
    d->resizeEventCallback = callback;
    d->resizeEventData     = data;
}

// ****************************************************************************
// Method: vtkQtRenderWindow::SetCloseCallback
//
// Purpose: 
//   Sets the callback function to be called when the window gets a closeEvent
//   from the window manager.
//
// Arguments:
//   Callback  The callback function to be called when the window is
//             closed.
//   data      The data that is passed to the CloseCallback.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 22 12:37:29 PDT 2001
//
// ****************************************************************************

void
vtkQtRenderWindow::SetCloseCallback(void (*callback)(void *), void *data)
{
    d->closeEventCallback = callback;
    d->closeEventCallbackData = data;
}

// ****************************************************************************
// Method: vtkQtRenderWindow::SetHideCallback
//
// Purpose: 
//   Sets the callback function to be called when the window gets a hideEvent
//   from the window manager.
//
// Arguments:
//   Callback  The callback function to be called 
//   data      The data that is passed to the callback.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 12 10:12:46 PDT 2003
//
// ****************************************************************************

void
vtkQtRenderWindow::SetHideCallback(void (*callback)(void *), void *data)
{
    d->hideEventCallback = callback;
    d->hideEventCallbackData = data;
}

// ****************************************************************************
// Method: vtkQtRenderWindow::SetShowCallback
//
// Purpose: 
//   Sets the callback function to be called when the window gets a showEvent
//   from the window manager.
//
// Arguments:
//   Callback  The callback function to be called 
//   data      The data that is passed to the callback.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 12 10:12:46 PDT 2003
//
// ****************************************************************************

void
vtkQtRenderWindow::SetShowCallback(void (*callback)(void *), void *data)
{
    d->showEventCallback = callback;
    d->showEventCallbackData = data;
}

// ****************************************************************************
// Method: vtkQtRenderWindow::closeEvent
//
// Purpose: 
//   This method is called when the window wants to close.
//
// Note:
//   Since we are not calling the accept() method on the close
//   event, it is the responsibility of the callback function to
//   make sure that the window is hidden or deleted.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 22 12:37:24 PDT 2001
//
// Modifications:
//    Brad Whitlock, Mon Jul 1 16:25:15 PST 2002
//    I made it call the superclass's closeEvent method if no callback is set.
//
// ****************************************************************************

void
vtkQtRenderWindow::closeEvent(QCloseEvent *e)
{
    // Calls the client's callback function if one was set up.
    if(d->closeEventCallback != NULL)
        (*d->closeEventCallback)(d->closeEventCallbackData);
    else
        QMainWindow::closeEvent(e);
}

// ****************************************************************************
// Method: vtkQtRenderWindow::hideEvent
//
// Purpose: 
//   Handles a hide event and calls a callback function.
//
// Arguments:
//   e : The hide event.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 12 10:09:16 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
vtkQtRenderWindow::hideEvent(QHideEvent *e)
{
    QMainWindow::hideEvent(e);
    if(d->hideEventCallback)
        (*d->hideEventCallback)(d->hideEventCallbackData);
}

// ****************************************************************************
// Method: vtkQtRenderWindow::showEvent
//
// Purpose: 
//   Handles a show event and calls a callback function.
//
// Arguments:
//   e : The show event.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 12 10:09:16 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
vtkQtRenderWindow::showEvent(QShowEvent *e)
{
    QMainWindow::showEvent(e);
    if(d->showEventCallback)
        (*d->showEventCallback)(d->showEventCallbackData);
}

// ****************************************************************************
// Method: vtkQtRenderWindow::resizeEvent
//
// Purpose: 
//   Method that gets called by qt when a resize event occurs.
//
// Arguments:
//   re        A pointer to the Qt resize event structure.
//
// Programmer: Eric Brugger
// Creation:   August 17, 2001
//
// Modifications:
//
// ****************************************************************************

void
vtkQtRenderWindow::resizeEvent(QResizeEvent *re)
{
    // Handle the resize and then record the size of the GL widget since that's
    // the size that we care about.
    QMainWindow::resizeEvent(re);
    if (d->resizeEventCallback)
        d->resizeEventCallback(d->resizeEventData);
}

