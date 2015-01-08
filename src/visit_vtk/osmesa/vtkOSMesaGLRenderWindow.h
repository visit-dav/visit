/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/


/*****************************************************************************
* Adapted from VTK6/Rendering/OpenGL/vtkOSOpenGLRenderWindow
* Provides OS Mesa support for VisIt in cases where a VTK build does not.
*****************************************************************************/

/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOSMesaGLRenderWindow.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkOSMesaGLRenderWindow - OffScreen Mesa rendering window
// .SECTION Description
// vtkOSMesaGLRenderWindow is a concrete implementation of the abstract class
// vtkOpenGLRenderWindow. vtkOSMesaGLRenderWindow interfaces to the OffScreen
// Mesa software implementation of the OpenGL library. The framebuffer resides
// on host memory. The framebuffer is the collection of logical buffers
// (color buffer(s), depth buffer, stencil buffer, accumulation buffer,
// multisample buffer) defining where the output of GL rendering is directed.
// Application programmers should normally use vtkRenderWindow instead of the
// OpenGL specific version.


#ifndef __vtkOSMesaGLRenderWindow_h
#define __vtkOSMesaGLRenderWindow_h

#include "vtkRenderingOpenGLModule.h" // For export macro
#include "vtkOpenGLRenderWindow.h"

class vtkIdList;
class vtkOSMesaGLRenderWindowInternal;

class VTKRENDERINGOPENGL_EXPORT vtkOSMesaGLRenderWindow : public vtkOpenGLRenderWindow
{
public:
  static vtkOSMesaGLRenderWindow *New();
  vtkTypeMacro(vtkOSMesaGLRenderWindow,vtkOpenGLRenderWindow);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Begin the rendering process.
  virtual void Start(void);

  // Description:
  // End the rendering process and display the image.
  virtual void Frame(void);

  // Description:
  // Initialize the window for rendering.
  virtual void WindowInitialize(void);

  // Description:
  // Initialize the rendering window.  This will setup all system-specific
  // resources.  This method and Finalize() must be symmetric and it
  // should be possible to call them multiple times, even changing WindowId
  // in-between.  This is what WindowRemap does.
  virtual void Initialize(void);

  // Description:
  // "Deinitialize" the rendering window.  This will shutdown all system-specific
  // resources.  After having called this, it should be possible to destroy
  // a window that was used for a SetWindowId() call without any ill effects.
  virtual void Finalize(void);

  // Description:
  // Change the window to fill the entire screen.
  virtual void SetFullScreen(int);

  // Description:
  // Specify the size of the rendering window in pixels.
  virtual void SetSize(int x,int y);
  virtual void SetSize(int a[2]) {this->SetSize(a[0], a[1]);};

  // Description:
  // Get the current size of the screen in pixels.
  virtual int     *GetScreenSize();

  // Description:
  // Get the position in screen coordinates (pixels) of the window.
  virtual int     *GetPosition();

  // Description:
  // Move the window to a new position on the display.
  void     SetPosition(int x, int y);
  void     SetPosition(int a[2]) {this->SetPosition(a[0], a[1]);};

  // Description:
  // Prescribe that the window be created in a stereo-capable mode. This
  // method must be called before the window is realized. This method
  // overrides the superclass method since this class can actually check
  // whether the window has been realized yet.
  virtual void SetStereoCapableWindow(int capable);

  // Description:
  // Make this window the current OpenGL context.
  void MakeCurrent();

  // Description:
  // Tells if this window is the current OpenGL context for the calling thread.
  virtual bool IsCurrent();

  // Description:
  // If called, allow MakeCurrent() to skip cache-check when called.
  // MakeCurrent() reverts to original behavior of cache-checking
  // on the next render.
  void SetForceMakeCurrent();

  // Description:
  // Get report of capabilities for the render window
  const char *ReportCapabilities();

  // Description:
  // Does this render window support OpenGL? 0-false, 1-true
  int SupportsOpenGL();

  // Description:
  // Is this render window using hardware acceleration? 0-false, 1-true
  int IsDirect();

  // Description:
  // Resize the window.
  virtual void WindowRemap(void);

  // Description:
  // Xwindow get set functions
  virtual void *GetGenericDisplayId() {return 0;}
  virtual void *GetGenericWindowId();
  virtual void *GetGenericParentId()  {return 0;}
  virtual void *GetGenericContext();
  virtual void *GetGenericDrawable()  {return 0;}

  // Description:
  // Set the X display id for this RenderWindow to use to a pre-existing
  // X display id.
  void     SetDisplayId(void *) {}

  // Description:
  // Sets the parent of the window that WILL BE created.
  void     SetParentId(void *);

  // Description:
  // Set this RenderWindow's X window id to a pre-existing window.
  void     SetWindowId(void *);

  // Description:
  // Set the window id of the new window once a WindowRemap is done.
  // This is the generic prototype as required by the vtkRenderWindow
  // parent.
  void     SetNextWindowId(void *);

  void     SetWindowName(const char *);

  // Description:
  // Hide or Show the mouse cursor, it is nice to be able to hide the
  // default cursor if you want VTK to display a 3D cursor instead.
  void HideCursor() {}
  void ShowCursor() {}

  // Description:
  // Change the shape of the cursor
  virtual void SetCurrentCursor(int);

  // Description:
  // Check to see if a mouse button has been pressed.
  // All other events are ignored by this method.
  // This is a useful check to abort a long render.
  virtual  int GetEventPending();

  // Description:
  // Set this RenderWindow's X window id to a pre-existing window.
  void     SetWindowInfo(char *info);

  // Description:
  // Set the window info that will be used after WindowRemap()
  void     SetNextWindowInfo(char *info);

  // Description:
  // Sets the X window id of the window that WILL BE created.
  void     SetParentInfo(char *info);

  // Description:
  // Render without displaying the window.
  void SetOffScreenRendering(int i);

protected:
  vtkOSMesaGLRenderWindow();
  ~vtkOSMesaGLRenderWindow();

  vtkOSMesaGLRenderWindowInternal *Internal;

  int      OwnWindow;
  int      OwnDisplay;
  int      ScreenSize[2];
  int      CursorHidden;
  int      ForceMakeCurrent;
  char    *Capabilities;

  void CreateAWindow();
  void DestroyWindow();
  void CreateOffScreenWindow(int width, int height);
  void DestroyOffScreenWindow();
  void ResizeOffScreenWindow(int width, int height);


private:
  vtkOSMesaGLRenderWindow(const vtkOSMesaGLRenderWindow&);  // Not implemented.
  void operator=(const vtkOSMesaGLRenderWindow&);  // Not implemented.
};



#endif
