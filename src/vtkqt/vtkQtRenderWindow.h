/////////////////////////////////////////////////////////////////////////////
//
// This source was not originally written by LLNL but LLNL has made
// modifications.
//
// Modifications:
//    Brad Whitlock, Wed Nov 1 11:37:58 PDT 2000
//    I added vtk get methods for the window id, display, graphics context.
//    I also overrode methods to move the window and set its caption.
//
//    Eric Brugger, Fri Aug 17 08:37:21 PDT 2001
//    I added the method SetResizeEvent.
//
//    Brad Whitlock, Wed Aug 22 10:53:45 PDT 2001
//    I added a callback function to be called when the window closes.
//
//    Hank Childs, Fri May 28 13:47:39 PDT 2004
//    Better test for Q_GLX, compliments of Brad Whitlock.
//
/////////////////////////////////////////////////////////////////////////////

/*=========================================================================
  vtkQtRenderWindow.h - copyright 2000 Matthias Koenig 
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

  Module:    $RCSfile: vtkQtRenderWindow.h,v $
  Date:      $Date: 2000/04/15 18:58:19 $
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

#ifndef _vtkQtRenderWindow_h
#define _vtkQtRenderWindow_h
#include <vtkqt_exports.h>

#include <qapplication.h>
#include <qmainwindow.h>
#include <vtkRenderWindow.h>
#include <vtkOpenGLRenderer.h>

#include <qgl.h>
#if defined(Q_GLX) || (defined(Q_WS_X11) && defined(QT_MODULE_OPENGL))
#include <qintdict.h>
#define INT8  dummy_INT8
#define INT32 dummy_INT32
#include <GL/glx.h>
#undef  INT8
#undef  INT32
#endif

class vtkQtRenderWindowInteractor;
class vtkFloatArray;
class vtkUnsignedCharArray;
class vtkQtGLWidget;

class VTKQT_API vtkQtRenderWindow : public vtkRenderWindow, public QMainWindow
{
// ****************************************************************************
// Modifications:
//
//  Hank Childs, Sun May  9 15:38:40 PDT 2004
//  Added IsDirect method.
//
// ****************************************************************************

public:
    void PrintSelf(ostream& os, vtkIndent indent);

    // Overridden by LLNL
    // Description:
    // Set the position in screen coordinates of the rendering window.
    virtual int* GetPosition();
    virtual void SetPosition(int,int);
    virtual void SetPosition(int a[2]);

    // Description:
    // Specify the size of the rendering window.
    virtual int* GetSize(); //inline
    virtual void SetSize(int w,int h); //inline
    virtual void SetSize(int a[2]); //inline

    // Overridden by LLNL
    // Description:
    // Set name of rendering window
    virtual void SetWindowName(const char *);

    vtkQtRenderWindow(QWidget *parent = 0, const char* name = 0, const QGLWidget *shareWidget = 0, WFlags f = 0);
    vtkQtRenderWindow(const QGLFormat &format, QWidget *parent = 0, const char *name = 0, const QGLWidget *shareWidget = 0, WFlags f = 0);
    virtual ~vtkQtRenderWindow();
    static vtkQtRenderWindow *New();

    // Description
    // use updateGL, not Render
    virtual void Render();

    // Overridden by LLNL for vtk 4.0
    virtual void Start() { };
    virtual void Frame() { };
    virtual void HideCursor();
    virtual void ShowCursor();
    virtual void SetFullScreen(int);
    virtual void WindowRemap() { };

    virtual void SetLineSmoothing(int);
    virtual vtkRenderWindowInteractor* MakeRenderWindowInteractor();
    void SetInteractor(vtkQtRenderWindowInteractor*);

    // Description:
    // Set/Get the pixel data of an image, transmitted as RGBRGB... 
    virtual unsigned char *GetPixelData(int x1, int y1,int x2, int y2, int front);
    virtual int SetPixelData(int x,int y,int x2,int y2,unsigned char *, int front);
    virtual int SetPixelData(int x,int y,int x2,int y2, vtkUnsignedCharArray *buffer,
                             int front);
    virtual int GetPixelData(int x1, int y1,int x2, int y2, int blend,
                             vtkUnsignedCharArray *buffer);

    // Description:
    // Set/Get the pixel data of an image, transmitted as RGBARGBA... 
    virtual float *GetRGBAPixelData(int x,int y,int x2,int y2,int front);
    virtual int    GetRGBAPixelData(int, int, int, int, int, vtkFloatArray* );

    virtual int    SetRGBAPixelData(int x,int y,int x2,int y2,float *,int front,
                                    int blend=0);
    virtual int    SetRGBAPixelData(int, int, int, int, vtkFloatArray*,
                        int, int blend=0);
    // Description:
    // Set/Get the pixel data of an image, transmitted as unsigned char RGBA, RGBA...
    /*NOT IMPLEMENTED YET*/
    virtual unsigned char *GetRGBACharPixelData(int ,int ,int ,int ,int ) { return 0; };
    virtual int GetRGBACharPixelData(int ,int, int, int, int,
                   vtkUnsignedCharArray*) { return 0; };
    virtual int SetRGBACharPixelData(int ,int ,int ,int ,unsigned char *, int,
                    int blend=0) { return 0; };
    virtual int SetRGBACharPixelData(int, int, int, int,
                   vtkUnsignedCharArray *,
                   int, int blend=0) { return 0; };
    virtual void ReleaseRGBAPixelData(float * data) { delete [] data; }; 

    // Description:
    // Set/Get the zbuffer data from an image
    virtual float *GetZbufferData( int x1, int y1, int x2, int y2 );
    virtual int    GetZbufferData( int x1, int y1, int x2, int y2, vtkFloatArray *data );
    virtual int    SetZbufferData( int x1, int y1, int x2, int y2, float *buffer );
    virtual int    SetZbufferData( int x1, int y1, int x2, int y2, vtkFloatArray *data );

    // Overridden by LLNL
    // Description:
    // Qt window get set functions
    virtual void *GetGenericDisplayId();
    virtual void *GetGenericWindowId();
    virtual void *GetGenericParentId();
    virtual void *GetGenericContext();
    virtual void *GetGenericDrawable();

    // Overridden by LLNL
    virtual void SetDisplayId(void *)  { };
    virtual void SetWindowId(void *)   { };
    virtual void SetNextWindowId(void *)   { };
    virtual void SetParentId(void *)   { };
    virtual void SetWindowInfo(char *) { };
    virtual void SetNextWindowInfo(char *) { };
    virtual void SetParentInfo(char *) { };
    virtual int  GetEventPending();
    virtual int  GetDepthBufferSize();

    // Description:
    // Make this window the current OpenGL context.
    virtual void MakeCurrent();

    // Description:
    // Set/Get the maximum number of multisamples
    static void SetGlobalMaximumNumberOfMultiSamples(int val);
    static int  GetGlobalMaximumNumberOfMultiSamples();

    // Description:
    // Set / Get the number of multisamples to use for hardware antialiasing.
    vtkSetMacro(MultiSamples,int);
    vtkGetMacro(MultiSamples,int);

    // qt functions
    virtual void show();
    virtual QPoint mapFromGlobal(const QPoint &pos) const;

    // Added by LLNL
    // Description:
    // Callback for a window resize event.
    void SetResizeCallback(void (*callback)(void *), void*);

    // Added by LLNL
    // Description:
    // Sets callback function pointer that is called when window closes.
    void SetCloseCallback(void (*callback)(void *), void*);

    // Added by LLNL
    // Description:
    // Sets callback function pointer that is called when window hides.
    void SetHideCallback(void (*callback)(void *), void*);

    // Added by LLNL
    // Description:
    // Sets callback function pointer that is called when window shows.
    void SetShowCallback(void (*callback)(void *), void*);

    // Added by LLNL
    // Description:
    // Forces the render window to render its actors.
    void TriggerRender();

    // Added by LLNL
    // Description:
    // Allows the GL widget to tell the window that its size changed. This
    // is called mostly when toolbars are repositioned in such a way that the
    // size of the GL widget changes.
    void TellGLSize(int w, int h);

    // Added by LLNL
    // Description:
    // Forces the render window to create a new toolbar with the given name.
    void *CreateToolbar(const char *name);
    void SetLargeIcons(bool);

    // Added by LLNL
    // Description:
    // Needed to set the format
    virtual void SetStereoCapableWindow(int capable);

    // Added by LLNL
    // Description:
    // Determines whether or not we are rendering to the GPU.  If we are not,
    // it is advisable that we use display lists.
    virtual int IsDirect(void);

    // Added by LLNL
    // Description:
    // Set the cursor for the GL part of the vis window.
    virtual void setCursor(const QCursor &);

protected:
    virtual void resizeEvent(QResizeEvent *);
    virtual void moveEvent(QMoveEvent *);
    virtual void closeEvent(QCloseEvent *);
    virtual void paintEvent(QPaintEvent *);
    virtual void hideEvent(QHideEvent *);
    virtual void showEvent(QShowEvent *);

    static void initApp();

    int MultiSamples;
    long OldMonitorSetting;
private:
    // Added by LLNL
    vtkQtGLWidget  *gl;
    void          (*resizeEventCallback)(void *);
    void           *resizeEventData;
    void          (*closeEventCallback)(void *);
    void           *closeEventCallbackData;
    void          (*hideEventCallback)(void *);
    void           *hideEventCallbackData;
    void          (*showEventCallback)(void *);
    void           *showEventCallbackData;
};

#endif



