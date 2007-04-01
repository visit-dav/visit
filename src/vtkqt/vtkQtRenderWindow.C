/*=========================================================================
  vtkQtRenderWindow.cpp - copyright 2000 Matthias Koenig 
  koenig@isg.cs.uni-magdeburg.de
  http://wwwisg.cs.uni-magdeburg.de/~makoenig
  =========================================================================*/
/* Portions of this code marked with "LLNL" have been modified from the
 * original sources by Matthias Koenig.  Contact VisIt@llnl.gov.  */
/*=========================================================================
  This module is an extension of the "Visualization Toolkit 
  ( copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen )".
  and combines it with "Qt (copyright (C) 1992-2000 Troll Tech AS)".
  =========================================================================*/
/*=========================================================================

  Module:    $RCSfile: vtkQtRenderWindow.cpp,v $
  Date:      $Date: 2000/04/15 18:58:51 $
  Version:   $Revision: 1.6 $

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
  
  * Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.
   
  * Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.
   
  * Modified source versions must be plainly marked as such, and must not be
  misrepresented as being the original software.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  
  =========================================================================*/
#include "vtkQtRenderWindow.h"
#include "vtkQtRenderWindowInteractor.h"
#include "vtkQtGLWidget.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

#include <vtkFloatArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkRendererCollection.h>

#include <qcursor.h>

#define MAX_LIGHTS 8

#ifdef Q_WS_WIN
static QWidget *parentForAllWindows = 0;
#endif

void vtkQtRenderWindow::initApp() {
    if (qApp)
        return;

    int ac = 0; char **av = NULL;
    new QApplication(ac, av);
}

// Modified by LLNL
vtkQtRenderWindow* vtkQtRenderWindow::New() {
    initApp();

#ifdef Q_WS_WIN
    // Create a parent widget for the render window and don't ever show
    // it so we do not get viewer windows in the taskbar.
    if(parentForAllWindows == 0)
    {
#if 0
        // This used to work but now it doesn't and I don't know why. Disable
        // it for now so the windows come up.
        parentForAllWindows = new QWidget(0, "VisIt");
        parentForAllWindows->resize(0,0);
#endif
    }

    // Create the render window as a child of the parent widget.
    vtkQtRenderWindow *tmp = new vtkQtRenderWindow(parentForAllWindows,
        0, 0, WType_TopLevel);
#else
    vtkQtRenderWindow *tmp = new vtkQtRenderWindow;
#endif

    return tmp;
}

// Initialize static member that controls global maximum number of multisamples.
static int vtkQtRenderWindowGlobalMaximumNumberOfMultiSamples = 8;

void vtkQtRenderWindow::SetGlobalMaximumNumberOfMultiSamples(int val) {
    if (val == vtkQtRenderWindowGlobalMaximumNumberOfMultiSamples) return;
    vtkQtRenderWindowGlobalMaximumNumberOfMultiSamples = val;
}

int vtkQtRenderWindow::GetGlobalMaximumNumberOfMultiSamples() {
    return vtkQtRenderWindowGlobalMaximumNumberOfMultiSamples;
}

// ****************************************************************************
// Method: vtkQtRenderWindow::vtkQtRenderWindow
//
// Purpose: 
//   Constructor for the vtkQtRenderWindow class.
//
// Arguments:
//
// Programmer: Matthias Koenig 
// Creation:   2000
//
// Modifications:
//   ... lots of changes by LLNL ...
//
//   Brad Whitlock, Mon Jan 27 17:22:45 PST 2003
//   I made it inherit from QMainWindow so we can have toolbars. I also made
//   it contain my vtkQtGLWidget.
//
//   Brad Whitlock, Wed Mar 12 10:04:38 PDT 2003
//   I added hide and show callbacks.
//
// ****************************************************************************

vtkQtRenderWindow::vtkQtRenderWindow(const QGLFormat &format, QWidget *parent, const char *name, const QGLWidget *shareWidget, WFlags f)
    : vtkRenderWindow(), QMainWindow(parent, name, f)
{
    vtkDebugMacro(<< " vtkQtRenderWindow constructor\n");
    this->WindowName = 0;
    this->Interactor = NULL;
    this->MultiSamples = vtkQtRenderWindowGlobalMaximumNumberOfMultiSamples;

    // Initialize some callback function pointers/data.
    this->resizeEventCallback = 0;
    this->resizeEventData = 0;
    this->closeEventCallback = 0;
    this->closeEventCallbackData = 0;
    this->hideEventCallback = 0;
    this->hideEventCallbackData = 0;
    this->showEventCallback = 0;
    this->showEventCallbackData = 0;

    // Set the window name
    SetWindowName("window 1");

    setFocusPolicy(QWidget:: WheelFocus);
    setBackgroundMode(QWidget::NoBackground);

    // Create the GL widget.
    gl = new vtkQtGLWidget(this, "vtkQtGLWidget");
    setCentralWidget(gl);
}

#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qwhatsthis.h>
#include <qpainter.h>

// ****************************************************************************
// Method: vtkQtRenderWindow::vtkQtRenderWindow
//
// Purpose: 
//   Constructor for the vtkQtRenderWindow class.
//
// Arguments:
//
// Programmer: Matthias Koenig 
// Creation:   2000
//
// Modifications:
//   ... lots of changes by LLNL ...
//
//   Brad Whitlock, Mon Jan 27 17:22:45 PST 2003
//   I made it inherit from QMainWindow so we can have toolbars. I also made
//   it contain my vtkQtGLWidget.
//
//   Brad Whitlock, Wed Mar 12 10:04:38 PDT 2003
//   I added hide and show callbacks.
//
// ****************************************************************************

vtkQtRenderWindow::vtkQtRenderWindow(QWidget *parent, const char *name, const QGLWidget *shareWidget, WFlags f)
    : vtkRenderWindow(), QMainWindow(parent, name, f)
{
    vtkDebugMacro(<< " vtkQtRenderWindow constructor\n");
    this->Interactor = NULL;
    this->WindowName = 0;
    this->MultiSamples = vtkQtRenderWindowGlobalMaximumNumberOfMultiSamples;

    // Initialize some callback function pointers/data.
    this->resizeEventCallback = 0;
    this->resizeEventData = 0;
    this->closeEventCallback = 0;
    this->closeEventCallbackData = 0;
    this->hideEventCallback = 0;
    this->hideEventCallbackData = 0;
    this->showEventCallback = 0;
    this->showEventCallbackData = 0;

    // Set the window name
    SetWindowName("window 1");

    setFocusPolicy(QWidget:: WheelFocus);
    setBackgroundMode(QWidget::NoBackground);

    // Create the GL widget.
    gl = new vtkQtGLWidget(this, "vtkQtGLWidget");
    setCentralWidget(gl);

/*
cout << "vtkQtRenderWindow::vtkQtRenderWindow: Size[0]="
     << this->Size[0] << ", Size[1]=" << this->Size[1] << endl;
*/
}

// Modified by LLNL
vtkQtRenderWindow::~vtkQtRenderWindow()
{
    short cur_light;
    vtkOpenGLRenderer *ren;

    // If we set the window name, delete the string.
    if(this->WindowName != 0)
    {
        delete [] this->WindowName;
        this->WindowName = 0;
    }

    ReferenceCount = 0;
  
    MakeCurrent();
    for (cur_light = GL_LIGHT0; cur_light < GL_LIGHT0+MAX_LIGHTS; cur_light++) {
        glDisable((GLenum)cur_light);
    }

    // tell each of the renderers that this render window/graphics context
    // is being removed (the RendererCollection is removed by vtkRenderWindow's
    // destructor)
    this->Renderers->InitTraversal();
    for ( ren = (vtkOpenGLRenderer *) this->Renderers->GetNextItemAsObject();
          ren != NULL;
          ren = (vtkOpenGLRenderer *) this->Renderers->GetNextItemAsObject() ) {
        ren->SetRenderWindow(NULL);
    }
    glFinish();
}

void vtkQtRenderWindow::PrintSelf(ostream& os, vtkIndent indent) {
    this->vtkRenderWindow::PrintSelf(os,indent);
    os << indent << "MultiSamples: " << this->MultiSamples << "\n";
}


vtkRenderWindowInteractor* vtkQtRenderWindow::MakeRenderWindowInteractor()
{
    vtkQtRenderWindowInteractor *interactor = vtkQtRenderWindowInteractor::New();
    this->Interactor = interactor;
    interactor->SetRenderWindow(this);
    // Set the interactor into the GL widget.
    gl->SetInteractor(interactor);
    return interactor;
}

void vtkQtRenderWindow::SetInteractor(vtkQtRenderWindowInteractor *interactor)
{
    gl->SetInteractor(interactor);
    vtkRenderWindow::SetInteractor(interactor);
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
//   Hank Childs, Thu Oct 25 14:58:39 PDT 2001
//   If a window is resized, then all of its renderers should consider
//   it modified.
//
//   Brad Whitlock, Mon Nov 5 15:04:30 PST 2001
//   Added code to save the size into the Size array.
//
//   Brad Whitlock, Mon Jan 27 11:29:53 PDT 2003
//   I modified the method so it uses QMainWindow methods.
//
// ****************************************************************************

void
vtkQtRenderWindow::resizeEvent(QResizeEvent *re)
{
    // Handle the resize and then record the size of the GL widget since that's
    // the size that we care about.
    QMainWindow::resizeEvent(re);
    int w = gl->width();
    int h = gl->height();

    this->Modified();
    this->Size[0] = w;
    this->Size[1] = h;

    vtkRenderer *aren;
    for (Renderers->InitTraversal() ; (aren = Renderers->GetNextItem()) ; )
    {
        aren->Modified();
    }

    if (resizeEventCallback)
        resizeEventCallback(resizeEventData);
}

// ****************************************************************************
// Method: vtkQtRenderWindow::moveEvent
//
// Purpose: 
//   This method is called when the window is moved.
//
// Arguments:
//   me : The move event.
//
// Note:       We update the internal Position array using the new position.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 5 15:03:36 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
vtkQtRenderWindow::moveEvent(QMoveEvent *me)
{
    QMainWindow::moveEvent(me);
    this->Position[0] = me->pos().x();
    this->Position[1] = me->pos().y();
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
    if(closeEventCallback != NULL)
        (*closeEventCallback)(closeEventCallbackData);
    else
        QMainWindow::closeEvent(e);
}

// ****************************************************************************
// Method: vtkQtRenderWindow::paintEvent
//
// Purpose: 
//   This method handles paint events for the window.
//
// Arguments:
//   pe : The paint event to process.
//
// Returns:    
//
// Note:       We have to draw the toolbar background ourselves because we've
//             told the main window (this widget) not to erase in the background
//             color before calling this method. We told it not to erase so
//             it does not erase the area behind the GL widget and cause flicker.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jan 27 17:09:02 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
vtkQtRenderWindow::paintEvent(QPaintEvent *pe)
{
    // Determine the area of the window occupied by the toolbars.
    QRegion whole(QRect(0, 0, width(), height()));
    QRegion glRegion(QRect(gl->x(), gl->y(), gl->width(), gl->height()));
    QRegion toolArea = whole.subtract(glRegion);
    QRegion updateArea(pe->region().intersect(toolArea));

    if(!updateArea.isEmpty() && !updateArea.isNull())
    {
        QPainter paint(this);
        // Only allow updates where the update area intersects the toolbar area.
        paint.setClipRegion(updateArea);
        // Draw a filled rectangle over the entire window. Most should be
        // clipped away because the toolbars only take up a smaller portion
        // of the window.
        QBrush brush(colorGroup().button());
        paint.fillRect(rect(), brush);
    }
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
    if(hideEventCallback)
        (*hideEventCallback)(hideEventCallbackData);
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
    if(showEventCallback)
        (*showEventCallback)(showEventCallbackData);
}

// ****************************************************************************
// Method: vtkQtRenderWindow::show
//
// Purpose: 
//   Tells the window to show itself.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jan 27 17:23:46 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
vtkQtRenderWindow::show()
{
    this->Mapped = 1;
    QMainWindow::show();
}

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

QPoint
vtkQtRenderWindow::mapFromGlobal(const QPoint &pos) const
{
    return gl->mapFromGlobal(pos);
}

// ****************************************************************************
// Method: vtkQtRenderWindow::CreateToolbar
//
// Purpose: 
//   Tells the render window to create a new toolbar widget and return a
//   pointer to it.
//
// Arguments:
//   name : The name of the toolbar.
//
// Returns:    A void pointer to the toolbar.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 29 14:19:41 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void *
vtkQtRenderWindow::CreateToolbar(const char *name)
{
// Maybe we should do dock windows instead...
    QToolBar *tools = new QToolBar(this, name);
    if(tools)
    {
        tools->setBackgroundMode(QWidget::PaletteButton);
        tools->setLabel(name);
    }

    return (void *)tools;
}

// ****************************************************************************
// Method: vtkQtRenderWindow::SetLargeIcons
//
// Purpose: 
//   Tells the window whether it should use large icons.
//
// Arguments:
//   val : The new large icon value.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 16 09:31:47 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
vtkQtRenderWindow::SetLargeIcons(bool val)
{
    if(usesBigPixmaps() != val)
        setUsesBigPixmaps(val);
}

// ****************************************************************************
// Method: vtkQtRenderWindow::GetGenericContext
//
// Purpose: 
//   Returns a graphics context for the platform under which Qt was
//   compiled.
//
// Arguments:
//
// Returns:    
//
// Note:       
//   It's done this way so the QtRenderWindow can use the various
//   X11, WIN32 vtk components that are already out there. This frees
//   us from having to code many more new vtk components that use Qt.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 31 15:29:50 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Jul 1 16:58:23 PST 2002
//   Added proper Win32 implementation.
//
//   Brad Whitlock, Mon Jan 27 11:31:33 PDT 2003
//   I made it draw on the GL widget.
//
//   Brad Whitlock, Fri May 16 9:23:23 PDT 2003
//   I added a MacOS X implementation.
//
// ****************************************************************************

void *
vtkQtRenderWindow::GetGenericContext()
{
#if defined(Q_WS_X11)
    static GC gc = (GC) NULL; 

    if (!gc) gc = XCreateGC(gl->x11Display(), gl->winId(), 0, 0);

    return (void *) gc;
#elif defined(Q_WS_WIN)
    return (void *)qt_display_dc();
#elif defined(Q_WS_MACX)
    cerr << "vtkQtRenderWindow::GetGenericContext(): Mac version not implemented!" <<  endl;
    return 0;
#else
    return 0;
#endif
}

// ****************************************************************************
// Method: vtkQtRenderWindow::SetWindowName
//
// Purpose: 
//   Sets vtkWindow::WindowName and sets the caption in the Qt window.
//
// Arguments:
//   name : The name of the window.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 31 16:44:06 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
vtkQtRenderWindow::SetWindowName(const char *name)
{
    if(name == 0)
        return;

    int len = strlen(name);
    if(len > 0)
    {
        if(this->WindowName) 
            delete [] this->WindowName;

        this->WindowName = new char[len + 1];
        strcpy(this->WindowName, name);
        setCaption(this->WindowName);
    }
}

// ****************************************************************************
// Method: vtkQtRenderWindow::GetPosition
//
// Purpose: 
//   Returns the position of the window.
//
// Arguments:
//
// Returns:    
//   Returns the position of the window in an int array of length 2.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 2 10:06:02 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

int *
vtkQtRenderWindow::GetPosition()
{
    return this->Position;
}

// ****************************************************************************
// Method: vtkQtRenderWindow::SetPosition
//
// Purpose: 
//   Sets the x,y screen coordinates of the window.
//
// Arguments:
//   x : The x screen location.
//   y : The y screen location.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 1 11:27:12 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
vtkQtRenderWindow::SetPosition(int x, int y)
{
/*
cout << "vtkQtRenderWindow::SetPosition: x=" << x << ", y=" << y << endl;
*/
    if((this->Position[0] != x) || (this->Position[1] != y))
    {
        this->Modified();
        this->Position[0] = x;
        this->Position[1] = y;
    }

    // Move the Qt window
    move(x, y);
}

// ****************************************************************************
// Method: vtkQtRenderWindow::SetPosition
//
// Purpose: 
//   Sets the x,y screen coordinates of the window.
//
// Arguments:
//   a : An array containing the x,y screen locations.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 1 11:27:12 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
vtkQtRenderWindow::SetPosition(int a[2])
{
    this->SetPosition(a[0], a[1]);
}

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
    resizeEventCallback = callback;
    resizeEventData     = data;
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
    closeEventCallback = callback;
    closeEventCallbackData = data;
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
    hideEventCallback = callback;
    hideEventCallbackData = data;
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
    showEventCallback = callback;
    showEventCallbackData = data;
}

// ****************************************************************************
// Method: vtkQtRenderWindow::TriggerRender
//
// Purpose: 
//   Causes all of the VTK objects to render themselves.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jan 27 17:22:45 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
vtkQtRenderWindow::TriggerRender()
{
    vtkRenderWindow::Render();
}

// ****************************************************************************
// Method: vtkQtRenderWindow::IsDirect
//
// Purpose:
//     Indicates whether or not this window is rendering directly to the GPU
//     or whether it is going through graphics hardware.
//
// Programmer: Hank Childs
// Creation:   May 9, 2004
//
// Modifications:
//
//   Hank Childs, Fri May 28 13:38:47 PDT 2004
//   Q_GLX is not used in newer version of Qt, so improve the test.  Also make
//   sure that we are using the right graphics context.
//
// **************************************************************************** 

int
vtkQtRenderWindow::IsDirect(void)
{
#if defined(Q_GLX) || (defined(Q_WS_X11) && defined(QT_MODULE_OPENGL))
    MakeCurrent();
    GLXContext ctx = glXGetCurrentContext();
    return glXIsDirect((Display *) GetGenericDisplayId(), ctx);
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


// ****************************************************************************
// Method: vtkQtRenderWindow::TellGLSize
//
// Purpose: 
//   Lets the window know the size of the GL widget.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jan 27 17:22:45 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
vtkQtRenderWindow::TellGLSize(int w, int h)
{
    if ((this->Size[0] != w) || (this->Size[1] != h))
    {
        this->Modified();
        this->Size[0] = w;
        this->Size[1] = h;
    }
}

//const char *vtkQtRenderWindow::GetClassName() { return "vtkQtRenderWindow"; }


void vtkQtRenderWindow::Render()      { gl->updateGL(); }
void vtkQtRenderWindow::MakeCurrent() { gl->makeCurrent(); }

void vtkQtRenderWindow::SetSize(int w,int h)    { resize(w, h); }
void vtkQtRenderWindow::SetSize(int a[2])       { this->SetSize(a[0], a[1]); }
int* vtkQtRenderWindow::GetSize()               { return this->Size; }

// Added by LLNL
void *vtkQtRenderWindow::GetGenericDisplayId()
{
#if defined(Q_WS_WIN)
    return (void *)qt_display_dc();
#elif defined(Q_WS_X11)
    return (void *)this->x11Display();
#elif defined(Q_WS_MACX)
    cerr << "vtkQtRenderWindow::GetGenericDisplayId(): Mac version not implemented!" << endl;
    return 0;
#endif
};

void *vtkQtRenderWindow::GetGenericWindowId()  { return (void *)gl->winId();};
void *vtkQtRenderWindow::GetGenericParentId()  { return this->parentWidget() ? (void *)this->parentWidget()->winId() : (void *)0;};
void *vtkQtRenderWindow::GetGenericDrawable()  { return (void *)gl->winId();};

// Added by LLNL
// ****************************************************************************
//  Method:  vtkQtRenderWindow::SetStereoCapableWindow
//
//  Purpose:
//    Virtual function, called when vtkRenderWindow is told to enable stereo.
//
//  Arguments:
//    capable    true to enable stereo for this window
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 19, 2002
//
// ****************************************************************************
void
vtkQtRenderWindow::SetStereoCapableWindow(int capable)
{
    vtkRenderWindow::SetStereoCapableWindow(capable);
    QGLFormat form = gl->format();
    form.setStereo(capable);
    gl->setFormat(form);
}

//
// Added by LLNL
//

// ****************************************************************************
// Method: vtkQtRenderWindow::HideCursor
//
// Purpose: 
//   Hides the cursor.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 15 15:16:55 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
vtkQtRenderWindow::HideCursor()
{
    qApp->setOverrideCursor(BlankCursor);
}

// ****************************************************************************
// Method: vtkQtRenderWindow::ShowCursor
//
// Purpose: 
//   Restores the last cursor (shows the cursor)
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 15 15:17:10 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
vtkQtRenderWindow::ShowCursor()
{
    qApp->restoreOverrideCursor();
}

// ****************************************************************************
// Method: vtkQtRenderWindow::SetFullScreen
//
// Purpose: 
//   Sets the renderwindow's full screen mode.
//
// Arguments:
//   val : The full screen mode.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 15 14:58:54 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
vtkQtRenderWindow::SetFullScreen(int val)
{
    if(this->FullScreen == val)
        return;

    this->FullScreen = val;
    if(this->FullScreen)
        showFullScreen();
    else
        showNormal();
}

// ****************************************************************************
// Method: vtkQtRenderWindow::GetEventPending
//
// Purpose: 
//   Returns 1 if there are pending events, 0 otherwise.
//
// Returns:    1 if there are pending events, 0 otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 15 15:28:55 PST 2002
//
// Modifications:
//   
// ****************************************************************************

int
vtkQtRenderWindow::GetEventPending()
{
    //return qApp->hasPendingEvents() ? 1 : 0;
    return 0;
}

// ****************************************************************************
// Method: vtkQtRenderWindow::GetDepthBufferSize
//
// Purpose: 
//   Returns the depth of the z-buffer.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 15 15:26:25 PST 2002
//
// Modifications:
//   
// ****************************************************************************

int
vtkQtRenderWindow::GetDepthBufferSize()
{
    int bits;
    glGetIntegerv(GL_DEPTH_BITS, &bits);

    return bits;
}

// ****************************************************************************
// Method: vtkQtRenderWindow::SetLineSmoothing
//
// Purpose: 
//   Sets the renderwindow's line smoothing mode.
//
// Arguments:
//   val : The line smoothing mode.
//
// Programmer: Kathleen Bonnell
// Creation:   December 4, 2002 
//
// Modifications:
//   Brad Whitlock, Mon Jan 27 17:28:03 PST 2003
//   I made it use the vtkQtGLWidget.
//
// ****************************************************************************

void
vtkQtRenderWindow::SetLineSmoothing(int val)
{
    if(this->LineSmoothing == val)
        return;

    this->LineSmoothing = val;

    //
    //  Make sure the changes take effect.
    //
    gl->setLineSmoothing(val != 0);
    gl->initializeGL();
}

// ****************************************************************************
// Method: vtkQtRenderWindow::setCursor
//
// Purpose: 
//   Sets the cursor for the GL widget.
//
// Arguments:
//   c : The cursor to use.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 7 14:54:27 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
vtkQtRenderWindow::setCursor(const QCursor &c)
{
    gl->setCursor(c);
}

//
// Include more source code for vtkQtRenderWindow that deals with getting/setting
// pixels to and from the GL widget. This code, though needed by VTK, is boilerplate
// and we don't need to change it too often.
//
#include "vtkQtRenderWindow_pixelops.C"

