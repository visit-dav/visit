#include "vtkOpenGL.h"
#include "QVTKWidget2.h"
#include <vtkQtRenderWindow.h>

#ifdef Q_WS_X11
#include <QX11Info>
#endif
#include "vtkGenericOpenGLRenderWindow.h"
#include "QVTKInteractor.h"
#include <vtkRenderWindow.h>

#include <QApplication>
#include <QDesktopWidget>
#include <vtkUnsignedCharArray.h>


//#ifdef __linux__
//# include <GL/glxew.h>
//#endif

#ifdef __linux__
# include <GL/glx.h>
#endif

//
// Subclass vtkGenericOpenGLRenderWindow so that certain VTK method calls act 
// on the main window that contains the render window.
//
class VTKQT_API vtkQtRenderWindowProxy : public vtkGenericOpenGLRenderWindow
{
public:
    vtkTypeMacro(vtkQtRenderWindowProxy,vtkGenericOpenGLRenderWindow);

    static vtkQtRenderWindowProxy *New()
    {
        return new vtkQtRenderWindowProxy;
    }

    virtual void HideCursor()
    {
        qApp->setOverrideCursor(Qt::BlankCursor);
    }

    virtual void ShowCursor()
    {
        qApp->restoreOverrideCursor();
    }

    virtual int IsDirect()
    {
#if defined(Q_GLX) || (defined(Q_WS_X11) && defined(QT_MODULE_OPENGL))
        MakeCurrent();
        GLXContext ctx = glXGetCurrentContext();
        return glXIsDirect(QX11Info::display(), ctx);
#elif defined(Q_WS_WIN)
        // If we are running on Windows, we are almost certainly on someone's
        // local machine, so declare direct.
        return 1;
#elif defined(Q_WS_MACX)
        // If we are running on Mac, we are almost certainly on someone's
        // local machine, so declare direct.
        return 1;
#else
        // Its not a Mac and its not Windows and it didn't like the test at the
        // top.  Declare it as "not direct", so that we will still use display
        // lists.
        return 0;
#endif
    }

    virtual void * GetGenericDisplayId()
    {
#if defined(Q_WS_WIN)
        return (void *)qt_win_display_dc();
#elif defined(Q_WS_X11)
        return (void *)QX11Info::display();
#elif defined(Q_WS_MACX)
        // Return the information about the GL widget so we can create a
        // transparent window over it.
        typedef struct { int x,y,w,h; } OverlayInfo;
        OverlayInfo *info = new OverlayInfo;
        QPoint tl(gl->mapToGlobal(QPoint(0,0)));
        info->x = tl.x();
        info->y = tl.y();
        info->w = gl->width();
        info->h = gl->height();
        return (void *)info;
#endif
    }

    virtual void *GetGenericDrawable() { return (void*)gl->winId(); }
    virtual void *GetGenericWindowId() { return (void*)gl->winId(); }

    void SetWidget(QVTKWidget2 *w) {gl = w;}

private:
    vtkQtRenderWindowProxy()
    {
    }
    QVTKWidget2              *gl;
};


//
// Private data storage.
//
class VTKQT_API vtkQtRenderWindowPrivate
{
public:
    vtkQtRenderWindowPrivate(vtkQtRenderWindow *w)
    {
        resizeEventCallback = NULL;
        resizeEventData = NULL;
        closeEventCallback = NULL;
        closeEventCallbackData = NULL;
        hideEventCallback = NULL;
        hideEventCallbackData = NULL;
        showEventCallback = NULL;
        showEventCallbackData = NULL;

        // Create a render window.
        proxy = vtkQtRenderWindowProxy::New();

        // Create the VTK widget and force our custom render window into it.
        gl = new QVTKWidget2(w);
        gl->SetRenderWindow(proxy);
        proxy->SetWidget(gl);
    }

    virtual ~vtkQtRenderWindowPrivate()
    {
        proxy->Delete();
    }

    QVTKWidget2              *gl;
    vtkQtRenderWindowProxy   *proxy;

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
    d = new vtkQtRenderWindowPrivate(this);
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

