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
Module:    vtkOSMesaGLRenderWindow.cxx

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

//*****************************************************************************
// Adapted from vtkOSOpenGLRenderWindow to provide
// OS Mesa support for VisIt, even if a VTK dist does not.
//*****************************************************************************

#include "vtkOSMesaGLRenderWindow.h"
#include "vtkOpenGLRenderer.h"
#include "vtkOpenGLProperty.h"
#include "vtkOpenGLTexture.h"
#include "vtkOpenGLCamera.h"
#include "vtkOpenGLLight.h"
#include "vtkOpenGLActor.h"
#include "vtkOpenGLPolyDataMapper.h"
#include <GL/gl.h>
#include "vtkgl.h"

#include <GL/osmesa.h>

#include "vtkCommand.h"
#include "vtkIdList.h"
#include "vtkObjectFactory.h"
#include "vtkRendererCollection.h"
#include "vtkOpenGLExtensionManager.h"

#include "vtksys/SystemTools.hxx"
#include "vtksys/ios/sstream"

class vtkOSMesaGLRenderWindow;
class vtkRenderWindow;

class vtkOSMesaGLRenderWindowInternal
{
  friend class vtkOSMesaGLRenderWindow;
private:
  vtkOSMesaGLRenderWindowInternal(vtkRenderWindow*);

  // store previous settings of on screen window
  int ScreenDoubleBuffer;
  int ScreenMapped;

  // OffScreen stuff
  OSMesaContext OffScreenContextId;
  void *OffScreenWindow;
};

vtkOSMesaGLRenderWindowInternal::vtkOSMesaGLRenderWindowInternal(
  vtkRenderWindow *rw)
{

  this->ScreenMapped = rw->GetMapped();
  this->ScreenDoubleBuffer = rw->GetDoubleBuffer();

  // OpenGL specific
  this->OffScreenContextId = NULL;
  this->OffScreenWindow = NULL;
}


vtkStandardNewMacro(vtkOSMesaGLRenderWindow);

#define MAX_LIGHTS 8

// a couple of routines for offscreen rendering
static void vtkOSMesaDestroyWindow(void *Window)
{
  free(Window);
}

static void *vtkOSMesaCreateWindow(int width, int height)
{
  return malloc(width*height*4);
}

vtkOSMesaGLRenderWindow::vtkOSMesaGLRenderWindow()
{
//   this->ParentId = (Window)NULL;
  this->ScreenSize[0] = 1280;
  this->ScreenSize[1] = 1024;
  this->OwnDisplay = 0;
  this->CursorHidden = 0;
  this->ForceMakeCurrent = 0;
  this->OwnWindow = 0;

  this->Internal = new vtkOSMesaGLRenderWindowInternal(this);

  this->Capabilities = 0;

}

// free up memory & close the window
vtkOSMesaGLRenderWindow::~vtkOSMesaGLRenderWindow()
{
  // close-down all system-specific drawing resources
  this->Finalize();

  vtkRenderer *ren;
  vtkCollectionSimpleIterator rit;
  this->Renderers->InitTraversal(rit);
  while ( (ren = this->Renderers->GetNextRenderer(rit)) )
    {
    ren->SetRenderWindow(NULL);
    }

  delete this->Internal;
}

// End the rendering process and display the image.
void vtkOSMesaGLRenderWindow::Frame()
{
  this->MakeCurrent();
  glFlush();
}

//
// Set the variable that indicates that we want a stereo capable window
// be created. This method can only be called before a window is realized.
//
void vtkOSMesaGLRenderWindow::SetStereoCapableWindow(int capable)
{
  if (!this->Internal->OffScreenContextId)
    {
    vtkOpenGLRenderWindow::SetStereoCapableWindow(capable);
    }
  else
    {
    vtkWarningMacro(<< "Requesting a StereoCapableWindow must be performed "
                    << "before the window is realized, i.e. before a render.");
    }
}

void vtkOSMesaGLRenderWindow::CreateAWindow()
{
  this->CreateOffScreenWindow(this->ScreenSize[0], this->ScreenSize[1]);
}

void vtkOSMesaGLRenderWindow::DestroyWindow()
{
  this->MakeCurrent();

  // tell each of the renderers that this render window/graphics context
  // is being removed (the RendererCollection is removed by vtkRenderWindow's
  // destructor)
  vtkRenderer* ren;
  this->Renderers->InitTraversal();
  for ( ren = vtkOpenGLRenderer::SafeDownCast(this->Renderers->GetNextItemAsObject());
        ren != NULL;
        ren = vtkOpenGLRenderer::SafeDownCast(this->Renderers->GetNextItemAsObject())  )
    {
    ren->SetRenderWindow(NULL);
    ren->SetRenderWindow(this);
    }


  if (this->Capabilities)
    {
    delete[] this->Capabilities;
    this->Capabilities = 0;
    }

  this->DestroyOffScreenWindow();

  // make sure all other code knows we're not mapped anymore
  this->Mapped = 0;

}

void vtkOSMesaGLRenderWindow::CreateOffScreenWindow(int width, int height)
{

  this->DoubleBuffer = 0;

  if (!this->Internal->OffScreenWindow)
    {
    this->Internal->OffScreenWindow = vtkOSMesaCreateWindow(width,height);
    this->OwnWindow = 1;
    }
  if (!this->Internal->OffScreenContextId)
    {
    this->Internal->OffScreenContextId = OSMesaCreateContext(GL_RGBA, NULL);
    }
  this->MakeCurrent();

  this->Mapped = 0;
  this->Size[0] = width;
  this->Size[1] = height;

  this->MakeCurrent();

  // tell our renderers about us
  vtkRenderer* ren;
  for (this->Renderers->InitTraversal();
       (ren = this->Renderers->GetNextItem());)
    {
    ren->SetRenderWindow(0);
    ren->SetRenderWindow(this);
    }

  this->OpenGLInit();
}

void vtkOSMesaGLRenderWindow::DestroyOffScreenWindow()
{

  // release graphic resources.
  vtkRenderer *ren;
  vtkCollectionSimpleIterator rit;
  this->Renderers->InitTraversal(rit);
  while ( (ren = this->Renderers->GetNextRenderer(rit)) )
    {
    ren->SetRenderWindow(NULL);
    ren->SetRenderWindow(this);
    }


  if (this->Internal->OffScreenContextId)
    {
    OSMesaDestroyContext(this->Internal->OffScreenContextId);
    this->Internal->OffScreenContextId = NULL;
    vtkOSMesaDestroyWindow(this->Internal->OffScreenWindow);
    this->Internal->OffScreenWindow = NULL;
    }
}

void vtkOSMesaGLRenderWindow::ResizeOffScreenWindow(int width, int height)
{
  if(this->Internal->OffScreenContextId)
    {
    this->DestroyOffScreenWindow();
    this->CreateOffScreenWindow(width, height);
    }
}


// Initialize the window for rendering.
void vtkOSMesaGLRenderWindow::WindowInitialize (void)
{
  this->CreateAWindow();

  this->MakeCurrent();

  // tell our renderers about us
  vtkRenderer* ren;
  for (this->Renderers->InitTraversal();
       (ren = this->Renderers->GetNextItem());)
    {
    ren->SetRenderWindow(0);
    ren->SetRenderWindow(this);
    }

  this->OpenGLInit();
}

// Initialize the rendering window.
void vtkOSMesaGLRenderWindow::Initialize (void)
{
  if(! (this->Internal->OffScreenContextId))
    {
    // initialize offscreen window
    int width = ((this->Size[0] > 0) ? this->Size[0] : 300);
    int height = ((this->Size[1] > 0) ? this->Size[1] : 300);
    this->CreateOffScreenWindow(width, height);
    }
}

void vtkOSMesaGLRenderWindow::Finalize (void)
{

  // clean up offscreen stuff
  this->SetOffScreenRendering(0);

  // clean and destroy window
  this->DestroyWindow();

}

// Change the window to fill the entire screen.
void vtkOSMesaGLRenderWindow::SetFullScreen(int arg)
{
  this->Modified();
}

// Resize the window.
void vtkOSMesaGLRenderWindow::WindowRemap()
{
  // shut everything down
  this->Finalize();

  // set the default windowid
//   this->WindowId = this->NextWindowId;
//   this->NextWindowId = (Window)NULL;

  // set everything up again
  this->Initialize();
}

// Begin the rendering process.
void vtkOSMesaGLRenderWindow::Start(void)
{
  this->Initialize();

  // set the current window
  this->MakeCurrent();
}


// Specify the size of the rendering window.
void vtkOSMesaGLRenderWindow::SetSize(int width,int height)
{
  if ((this->Size[0] != width)||(this->Size[1] != height))
    {
    this->Size[0] = width;
    this->Size[1] = height;
    this->ResizeOffScreenWindow(width,height);
    this->Modified();
    }
}

void vtkOSMesaGLRenderWindow::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "OffScreenContextId: " << this->Internal->OffScreenContextId << "\n";
//   os << indent << "Color Map: " << this->ColorMap << "\n";
//   os << indent << "Next Window Id: " << this->NextWindowId << "\n";
//   os << indent << "Window Id: " << this->GetWindowId() << "\n";
}

void vtkOSMesaGLRenderWindow::MakeCurrent()
{
  // set the current window
  if (this->Internal->OffScreenContextId)
    {
    if (OSMesaMakeCurrent(this->Internal->OffScreenContextId,
                          this->Internal->OffScreenWindow, GL_UNSIGNED_BYTE,
                          this->Size[0], this->Size[1]) != GL_TRUE)
      {
      vtkWarningMacro("failed call to OSMesaMakeCurrent");
      }
    }
}

// ----------------------------------------------------------------------------
// Description:
// Tells if this window is the current OpenGL context for the calling thread.
bool vtkOSMesaGLRenderWindow::IsCurrent()
{
  bool result=false;
  if(this->Internal->OffScreenContextId)
    {
    result=this->Internal->OffScreenContextId==OSMesaGetCurrentContext();
    }
  return result;
}


void vtkOSMesaGLRenderWindow::SetForceMakeCurrent()
{
  this->ForceMakeCurrent = 1;
}

void *vtkOSMesaGLRenderWindow::GetGenericContext()
{
  return (void *)this->Internal->OffScreenContextId;
}

int vtkOSMesaGLRenderWindow::GetEventPending()
{
  return 0;
}

// Get the size of the screen in pixels
int *vtkOSMesaGLRenderWindow::GetScreenSize()
{

  this->ScreenSize[0] = 1280;
  this->ScreenSize[1] = 1024;
  return this->ScreenSize;
}

// Get the position in screen coordinates (pixels) of the window.
int *vtkOSMesaGLRenderWindow::GetPosition(void)
{
  return this->Position;
}

// Move the window to a new position on the display.
void vtkOSMesaGLRenderWindow::SetPosition(int x, int y)
{
  if ((this->Position[0] != x)||(this->Position[1] != y))
    {
    this->Modified();
    }
  this->Position[0] = x;
  this->Position[1] = y;
}

// Set this RenderWindow's X window id to a pre-existing window.
void vtkOSMesaGLRenderWindow::SetWindowInfo(char *info)
{
  int tmp;

  this->OwnDisplay = 1;

  sscanf(info,"%i",&tmp);

}

// Set this RenderWindow's X window id to a pre-existing window.
void vtkOSMesaGLRenderWindow::SetNextWindowInfo(char *info)
{
  int tmp;
  sscanf(info,"%i",&tmp);

//   this->SetNextWindowId((Window)tmp);
}

// Sets the X window id of the window that WILL BE created.
void vtkOSMesaGLRenderWindow::SetParentInfo(char *info)
{
  int tmp;

  // get the default display connection
  this->OwnDisplay = 1;

  sscanf(info,"%i",&tmp);

//   this->SetParentId(tmp);
}

void vtkOSMesaGLRenderWindow::SetWindowId(void *arg)
{
//   this->SetWindowId((Window)arg);
}
void vtkOSMesaGLRenderWindow::SetParentId(void *arg)
{
//   this->SetParentId((Window)arg);
}

const char* vtkOSMesaGLRenderWindow::ReportCapabilities()
{
  MakeCurrent();

//   int scrnum = DefaultScreen(this->DisplayId);

  const char *glVendor = (const char *) glGetString(GL_VENDOR);
  const char *glRenderer = (const char *) glGetString(GL_RENDERER);
  const char *glVersion = (const char *) glGetString(GL_VERSION);
  const char *glExtensions = (const char *) glGetString(GL_EXTENSIONS);

  vtksys_ios::ostringstream strm;
  strm << "OpenGL vendor string:  " << glVendor << endl;
  strm << "OpenGL renderer string:  " << glRenderer << endl;
  strm << "OpenGL version string:  " << glVersion << endl;
  strm << "OpenGL extensions:  " << glExtensions << endl;
  delete[] this->Capabilities;
  size_t len = strm.str().length();
  this->Capabilities = new char[len + 1];
  strncpy(this->Capabilities, strm.str().c_str(), len);
  this->Capabilities[len] = 0;
  return this->Capabilities;
}

int vtkOSMesaGLRenderWindow::SupportsOpenGL()
{
  MakeCurrent();
  return 1;
}


int vtkOSMesaGLRenderWindow::IsDirect()
{
  MakeCurrent();
  return 0;
}


void vtkOSMesaGLRenderWindow::SetWindowName(const char * cname)
{
  char *name = new char[ strlen(cname)+1 ];
  strcpy(name, cname);
  vtkOpenGLRenderWindow::SetWindowName( name );
  delete [] name;
}

// Specify the X window id to use if a WindowRemap is done.
/*void vtkOSMesaGLRenderWindow::SetNextWindowId(Window arg)
{
  vtkDebugMacro(<< "Setting NextWindowId to " << (void *)arg << "\n");

  this->NextWindowId = arg;
}*/

void vtkOSMesaGLRenderWindow::SetNextWindowId(void *arg)
{
//   this->SetNextWindowId((Window)arg);
}


// Set the X display id for this RenderWindow to use to a pre-existing
// X display id.
/*void vtkOSMesaGLRenderWindow::SetDisplayId(Display  *arg)
{
  vtkDebugMacro(<< "Setting DisplayId to " << (void *)arg << "\n");

  this->DisplayId = arg;
  this->OwnDisplay = 0;

}*/

/*void vtkOSMesaGLRenderWindow::SetDisplayId(void *arg)
{
  this->SetDisplayId((Display *)arg);
  this->OwnDisplay = 0;
}*/

//============================================================================
// Stuff above this is almost a mirror of vtkOSMesaGLRenderWindow.
// The code specific to OpenGL Off-Screen stuff may eventually be
// put in a supper class so this whole file could just be included
// (mangled) from vtkOSMesaGLRenderWindow like the other OpenGL classes.
//============================================================================

void vtkOSMesaGLRenderWindow::SetOffScreenRendering(int i)
{
  if (this->OffScreenRendering == i)
    {
    return;
    }

  // invoke super
  this->vtkRenderWindow::SetOffScreenRendering(i);

  this->Internal->ScreenDoubleBuffer = this->DoubleBuffer;
  this->DoubleBuffer = 0;
  if(this->Mapped)
    {
    this->DestroyWindow();
    }

  // delay initialization until Render
}

// This probably has been moved to superclass.
void *vtkOSMesaGLRenderWindow::GetGenericWindowId()
{
  return (void *)this->Internal->OffScreenWindow;
}

void vtkOSMesaGLRenderWindow::SetCurrentCursor(int shape)
{
  if ( this->InvokeEvent(vtkCommand::CursorChangedEvent,&shape) )
    {
    return;
    }
  this->Superclass::SetCurrentCursor(shape);
}
