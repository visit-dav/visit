/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkOSMesaRenderWindow.cxx,v $
  Language:  C++
  Date:      $Date: 2003/08/26 19:51:59 $
  Version:   $Revision: 1.48 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#define USE_MGL_NAMESPACE
#include "MangleMesaInclude/osmesa.h"
#include <MangleMesaInclude/gl.h>

#include "vtkOSMesaRenderWindow.h"

#include "vtkMesaRenderer.h"

#include "vtkToolkits.h"

#include "vtkCommand.h"
#include "vtkIdList.h"
#include "vtkObjectFactory.h"
#include "vtkRendererCollection.h"

class vtkOSMesaRenderWindow;
class vtkOSMesaRenderWindowInternal
{
  friend class vtkOSMesaRenderWindow;
private:
  vtkOSMesaRenderWindowInternal()
  {
     this->OffScreenContextId = NULL;
     this->OffScreenWindow = NULL;
  }

  // OffScreen stuff
  OSMesaContext OffScreenContextId;
  void         *OffScreenWindow;
};


vtkCxxRevisionMacro(vtkOSMesaRenderWindow, "$Revision: 1.48 $");
vtkStandardNewMacro(vtkOSMesaRenderWindow);

#define MAX_LIGHTS 8


#ifdef WRITE_TRACE_FILE
static int trace_indent = 0;
static FILE *trace_file = 0;

static void trace_init()
{
    trace_file = fopen("trace.txt", "wt");
}

static void trace_finalize()
{
    if(trace_file)
    {
        fclose(trace_file);
        trace_file = 0;
    }
}

static void trace_print(const char *s, const char *f, const int line)
{
    if(s != 0)
    {
         for(int i = 0; i < trace_indent * 4; ++i)
             fprintf(trace_file, " ");
         fprintf(trace_file, "%s   (%s:%d)\n", s, f, line);
    }
}

static void trace_start(const char *s, const char *f, const int line)
{
    trace_print(s, f, line);
    ++trace_indent;
}

static void trace_end(const char *s, const char *f, const int line)
{
    --trace_indent;
    trace_print(s, f, line);
}

#define TraceInit() trace_init()
#define TraceFinalize() trace_finalize()
#define TraceBegin(S) trace_start(S, __FILE__, __LINE__)
#define TraceEnd(S)   trace_end(S, __FILE__, __LINE__)
#define TraceBeginEnd(S) trace_print(S, __FILE__, __LINE__)
#else
#define TraceInit()
#define TraceFinalize()
#define TraceBegin(S)
#define TraceEnd(S)
#define TraceBeginEnd(S) 
#endif


// a couple of routines for offscreen rendering
void vtkOSMesaDestroyWindow(void *Window) 
{
  TraceBegin("vtkOSMesaDestroyWindow");
  free(Window);
  TraceEnd("vtkOSMesaDestroyWindow");
}

void *vtkOSMesaCreateWindow(int width, int height) 
{
  TraceBegin("vtkOSMesaCreateWindow");
  void *retval = malloc(width*height*4);
  TraceEnd("vtkOSMesaCreateWindow");
  return retval;
}

vtkOSMesaRenderWindow::vtkOSMesaRenderWindow() : vtkMesaRenderWindow()
{
  TraceInit();
  TraceBegin("vtkOSMesaRenderWindow::vtkOSMesaRenderWindow");
  this->Internal = new vtkOSMesaRenderWindowInternal;
  TraceEnd("vtkOSMesaRenderWindow::vtkOSMesaRenderWindow");
}

// free up memory & close the window
vtkOSMesaRenderWindow::~vtkOSMesaRenderWindow()
{
  TraceBegin("vtkOSMesaRenderWindow::~vtkOSMesaRenderWindow");
  // close-down all system-specific drawing resources
  this->Finalize();

  delete this->Internal;

  TraceEnd("vtkOSMesaRenderWindow::~vtkOSMesaRenderWindow");
  TraceFinalize();
}

// End the rendering process and display the image.
void vtkOSMesaRenderWindow::Frame(void)
{
    TraceBegin("vtkOSMesaRenderWindow::Frame");

  this->MakeCurrent();
  glFlush();

    TraceEnd("vtkOSMesaRenderWindow::Frame");
}
 
// Initialize the window for rendering.
void vtkOSMesaRenderWindow::WindowInitialize (void)
{
  TraceBegin("vtkOSMesaRenderWindow::WindowInitialize");

  int width = this->Size[0];
  int height = this->Size[1];
  this->DoubleBuffer = 0;
  if (!this->Internal->OffScreenWindow)
    {
    this->Internal->OffScreenWindow = vtkOSMesaCreateWindow(width,height);
    this->Size[0] = width;
    this->Size[1] = height;      
    }    
  this->Internal->OffScreenContextId = OSMesaCreateContext(GL_RGBA, NULL);

  this->MakeCurrent();
  this->Mapped = 0;

  // tell our renderers about us
  vtkRenderer* ren;
  for (this->Renderers->InitTraversal(); 
       (ren = this->Renderers->GetNextItem());)
    {
    ren->SetRenderWindow(0);
    ren->SetRenderWindow(this);
    }

  this->OpenGLInit();
  glAlphaFunc(GL_GREATER,0);

  TraceEnd("vtkOSMesaRenderWindow::WindowInitialize");
}

// Initialize the rendering window.
void vtkOSMesaRenderWindow::Initialize (void)
{
  TraceBegin("vtkOSMesaRenderWindow::Initialize");
  // make sure we havent already been initialized 

  if (this->Internal->OffScreenContextId)
    {
    TraceEnd("vtkOSMesaRenderWindow::Initialize");
    return;
    }

  // now initialize the window 
  this->WindowInitialize();

  TraceEnd("vtkOSMesaRenderWindow::Initialize");
}

void vtkOSMesaRenderWindow::Finalize (void)
{
  vtkRenderer *ren;
  GLuint id;
  short cur_light;
   
  TraceBegin("vtkOSMesaRenderWindow::Finalize");

  // make sure we have been initialized 
  if (this->Internal->OffScreenContextId)
    {
    this->MakeCurrent();

    // tell each of the renderers that this render window/graphics context
    // is being removed (the RendererCollection is removed by vtkRenderWindow's
    // destructor)
    this->Renderers->InitTraversal();
    for ( ren = vtkMesaRenderer::SafeDownCast(this->Renderers->GetNextItemAsObject());
          ren != NULL;
          ren = vtkMesaRenderer::SafeDownCast(this->Renderers->GetNextItemAsObject())  )
      {
      ren->SetRenderWindow(NULL);
      }
        
    /* first delete all the old lights */
    for (cur_light = GL_LIGHT0; cur_light < GL_LIGHT0+MAX_LIGHTS; cur_light++)
      {
      glDisable((GLenum)cur_light);
      }

    /* now delete all textures */
    glDisable(GL_TEXTURE_2D);
    for (int i = 1; i < this->TextureResourceIds->GetNumberOfIds(); i++)
      {
      id = (GLuint) this->TextureResourceIds->GetId(i);
#ifdef GL_VERSION_1_1
      if (glIsTexture(id))
        {
        glDeleteTextures(1, &id);
        }
#else
      if (glIsList(id))
        {
        glDeleteLists(id,1);
        }
#endif
      }

    glFinish();

    if (this->Internal->OffScreenContextId)
      {
      OSMesaDestroyContext(this->Internal->OffScreenContextId);
      this->Internal->OffScreenContextId = NULL;
      vtkOSMesaDestroyWindow(this->Internal->OffScreenWindow);
      this->Internal->OffScreenWindow = NULL;
      }
    }

  TraceEnd("vtkOSMesaRenderWindow::Finalize");
}


// Resize the window.
void
vtkOSMesaRenderWindow::WindowRemap(void)
{
  TraceBegin("vtkOSMesaRenderWindow::WindowRemap");

  // shut everything down
  this->Finalize();

  // set everything up again 
  this->Initialize();

  TraceEnd("vtkOSMesaRenderWindow::WindowRemap");
}

// Begin the rendering process.
void vtkOSMesaRenderWindow::Start(void)
{
  TraceBegin("vtkOSMesaRenderWindow::Start");

  // if the renderer has not been initialized, do so now
  if (!this->Internal->OffScreenContextId)
    {
    this->Initialize();
    }

  // set the current window 
  this->MakeCurrent();

  TraceEnd("vtkOSMesaRenderWindow::Start");
}


// Specify the size of the rendering window.
void vtkOSMesaRenderWindow::SetSize(int x,int y)
{
  TraceBegin("vtkOSMesaRenderWindow::SetSize");

  if ((this->Size[0] != x)||(this->Size[1] != y))
    {
    this->Modified();
    this->Size[0] = x;
    this->Size[1] = y;
    }

  if (this->Internal->OffScreenWindow)
    {
    vtkRenderer *ren;
    // Disconnect renderers from this render window.
    vtkRendererCollection *renderers = this->Renderers;
    renderers->Register(this);
    this->Renderers->Delete();
    this->Renderers = vtkRendererCollection::New();
    renderers->InitTraversal();
    while ( (ren = renderers->GetNextItem()) )
      {
      ren->SetRenderWindow(NULL);
      }
    
    // Destroy the offscreen context and memory
    OSMesaDestroyContext(this->Internal->OffScreenContextId);
    this->Internal->OffScreenContextId = NULL;
    vtkOSMesaDestroyWindow(this->Internal->OffScreenWindow);
    this->Internal->OffScreenWindow = NULL;
    // Recreate the offscreen context and memory
    this->WindowInitialize();
    
    // Add the renders back into the render window.
    renderers->InitTraversal();
    while ( (ren = renderers->GetNextItem()) )
      {
      this->AddRenderer(ren);
      }
    renderers->Delete();
    }

  TraceEnd("vtkOSMesaRenderWindow::SetSize");
}

void vtkOSMesaRenderWindow::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "OffScreenContextId: " << this->Internal->OffScreenContextId << "\n";
}

void vtkOSMesaRenderWindow::MakeCurrent()
{
  TraceBegin("vtkOSMesaRenderWindow::MakeCurrent");

  if (this->Internal->OffScreenContextId) 
    {
    if (OSMesaMakeCurrent(this->Internal->OffScreenContextId, 
                          this->Internal->OffScreenWindow, GL_UNSIGNED_BYTE, 
                          this->Size[0], this->Size[1]) != GL_TRUE) 
      {
      vtkWarningMacro("failed call to OSMesaMakeCurrent");
      }
    }

  TraceEnd("vtkOSMesaRenderWindow::MakeCurrent");
}

// Get the size of the screen in pixels
int *vtkOSMesaRenderWindow::GetScreenSize()
{
    TraceBeginEnd("vtkOSMesaRenderWindow::GetScreenSize");
    return this->Size;
}

// This probably has been moved to superclass.
void *vtkOSMesaRenderWindow::GetGenericWindowId()
{
    TraceBeginEnd("vtkOSMesaRenderWindow::GetGenericWindowId");

    return (void *)this->Internal->OffScreenWindow;
}

void *vtkOSMesaRenderWindow::GetGenericContext()
{
    TraceBeginEnd("vtkOSMesaRenderWindow::GetGenericContext");

    return (void *)&(this->Internal->OffScreenContextId);
}

void vtkOSMesaRenderWindow::SetDisplayId(void*)
{
    TraceBeginEnd("vtkOSMesaRenderWindow::SetDisplayId");
}

void vtkOSMesaRenderWindow::SetWindowId(void*)
{
    TraceBeginEnd("vtkOSMesaRenderWindow::SetWindowId");
}

void vtkOSMesaRenderWindow::SetParentId(void*)
{
    TraceBeginEnd("vtkOSMesaRenderWindow::SetParentId");
}

void* vtkOSMesaRenderWindow::GetGenericDisplayId()
{
    TraceBeginEnd("vtkOSMesaRenderWindow::GetGenericDisplayId");
    return 0;
}

void* vtkOSMesaRenderWindow::GetGenericParentId()
{
    TraceBeginEnd("vtkOSMesaRenderWindow::GetGenericParentId");
    return 0;
}

void* vtkOSMesaRenderWindow::GetGenericDrawable()
{
    TraceBeginEnd("vtkOSMesaRenderWindow::GetGenericDrawable");
    return (void *)&(this->Internal->OffScreenContextId);
}

void vtkOSMesaRenderWindow::SetWindowInfo(char*)
{
    TraceBegin("vtkOSMesaRenderWindow::SetWindowInfo");
}

void vtkOSMesaRenderWindow::SetParentInfo(char*)
{
    TraceBeginEnd("vtkOSMesaRenderWindow::SetParentInfo");
}

void vtkOSMesaRenderWindow::HideCursor()
{
    TraceBeginEnd("vtkOSMesaRenderWindow::HideCursor");
}

void vtkOSMesaRenderWindow::ShowCursor()
{
    TraceBeginEnd("vtkOSMesaRenderWindow::ShowCursor");
}

void vtkOSMesaRenderWindow::SetFullScreen(int)
{
    TraceBeginEnd("vtkOSMesaRenderWindow::SetFullScreen");
}

int vtkOSMesaRenderWindow::GetEventPending()
{
    TraceBeginEnd("vtkOSMesaRenderWindow::GetEventPending");
    return 0;
}

void vtkOSMesaRenderWindow::SetNextWindowId(void*)
{
    TraceBeginEnd("vtkOSMesaRenderWindow::SetNextWindowId");
}

void vtkOSMesaRenderWindow::SetNextWindowInfo(char*)
{
    TraceBeginEnd("vtkOSMesaRenderWindow::SetNextWindowInfo");
}


void
vtkOSMesaRenderWindow::SavePPM(const char *filename)
{
    if(this->Internal->OffScreenWindow)
    {
        FILE *fp = fopen(filename, "wb");
        if(fp)
        {
            fprintf(fp, "P6\n%d %d\n255\n", this->Size[0], this->Size[1]);
            unsigned char *pix = (unsigned char *)this->Internal->OffScreenWindow;
            int npix = this->Size[0] * this->Size[1];
            for(int i = 0; i < npix; ++i)
            {
                fwrite((void *)pix, 3, 1, fp);
                pix += 4;
            }
            fclose(fp);
        }
    }
}
