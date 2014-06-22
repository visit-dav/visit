#include "vtkOpenGL.h"
#include "QVTKWidget2.h"
#include <vtkQtRenderWindow.h>

#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QX11Info>
#endif
#endif
#include "vtkGenericOpenGLRenderWindow.h"
#include "QVTKInteractor.h"
#include <vtkRenderWindow.h>

#include <QApplication>
#include <QDesktopWidget>
#include <vtkUnsignedCharArray.h>

#ifdef __linux__
# include <GL/glx.h>
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

        // Create the VTK widget and force our custom render window into it.
        if(stereo)
            gl = new QVTKWidget2(QGLFormat(QGL::DepthBuffer | QGL::AlphaChannel | QGL::StereoBuffers), w);
        else
            gl = new QVTKWidget2(QGLFormat(QGL::DepthBuffer | QGL::AlphaChannel), w);
        if (!gl->format().alpha())
            qWarning("Could not get alpha channel; results will be suboptimal");
    }

    virtual ~vtkQtRenderWindowPrivate()
    {
    }

    QVTKWidget2    *gl;

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

    setAnimated(false);

    setWindowFlags(f);
    setCentralWidget(d->gl);
}

vtkQtRenderWindow::vtkQtRenderWindow(bool stereo, QWidget *parent, Qt::WindowFlags f) : QMainWindow(parent, f)
{
    d = new vtkQtRenderWindowPrivate(this, stereo);
    setIconSize(QSize(20,20));

    setAnimated(false);

    setWindowFlags(f);
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
    return d->gl->GetRenderWindow();
}

    // Description:
    // Get the Qt/vtk interactor that was either created by default or set by the user
vtkRenderWindowInteractor * 
vtkQtRenderWindow::GetInteractor()
{
    return d->gl->GetInteractor();
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

