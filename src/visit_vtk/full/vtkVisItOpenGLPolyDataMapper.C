/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItOpenGLPolyDataMapper.cxx,v $
  Language:  C++
  Date:      $Date: 2003/04/28 19:13:10 $
  Version:   $Revision: 1.78 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkVisItOpenGLPolyDataMapper.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCommand.h"
#include "vtkDataArray.h"
#include "vtkMatrix4x4.h"
#include "vtkObjectFactory.h"
#include "vtkOpenGLRenderer.h"
#include "vtkPlane.h"
#include "vtkPlaneCollection.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkPolygon.h"
#include "vtkProperty.h"
#include "vtkTimerLog.h"
#include "vtkTriangle.h"
#include "vtkRenderWindow.h"

#include "vtkLookupTable.h"

static const int dlSize = 8192;


#ifndef VTK_IMPLEMENT_MESA_CXX
  #include <visit-config.h>
  #ifdef HAVE_LIBGLEW
    #include <GL/glew.h>
  #endif
  #if defined(__APPLE__) && (defined(VTK_USE_CARBON) || defined(VTK_USE_COCOA))
    #include <OpenGL/gl.h>
  #else
    #if defined(_WIN32)
       #include <windows.h>
    #endif
    #include <GL/gl.h>
  #endif
#endif

#include <math.h>

#ifndef VTK_IMPLEMENT_MESA_CXX
vtkCxxRevisionMacro(vtkVisItOpenGLPolyDataMapper, "$Revision: 1.78 $");
vtkStandardNewMacro(vtkVisItOpenGLPolyDataMapper);
#endif

static float vtk1Over255[] = {
0.f, 0.00392157f, 0.00784314f, 0.0117647f, 0.0156863f, 0.0196078f, 0.0235294f, 
0.027451f, 0.0313725f, 0.0352941f, 0.0392157f, 0.0431373f, 0.0470588f, 0.0509804f, 
0.054902f, 0.0588235f, 0.0627451f, 0.0666667f, 0.0705882f, 0.0745098f, 0.0784314f, 
0.0823529f, 0.0862745f, 0.0901961f, 0.0941176f, 0.0980392f, 0.101961f, 0.105882f, 
0.109804f, 0.113725f, 0.117647f, 0.121569f, 0.12549f, 0.129412f, 0.133333f, 
0.137255f, 0.141176f, 0.145098f, 0.14902f, 0.152941f, 0.156863f, 0.160784f, 
0.164706f, 0.168627f, 0.172549f, 0.176471f, 0.180392f, 0.184314f, 0.188235f, 
0.192157f, 0.196078f, 0.2f, 0.203922f, 0.207843f, 0.211765f, 0.215686f, 0.219608f, 
0.223529f, 0.227451f, 0.231373f, 0.235294f, 0.239216f, 0.243137f, 0.247059f, 
0.25098f, 0.254902f, 0.258824f, 0.262745f, 0.266667f, 0.270588f, 0.27451f, 
0.278431f, 0.282353f, 0.286275f, 0.290196f, 0.294118f, 0.298039f, 0.301961f, 
0.305882f, 0.309804f, 0.313725f, 0.317647f, 0.321569f, 0.32549f, 0.329412f, 
0.333333f, 0.337255f, 0.341176f, 0.345098f, 0.34902f, 0.352941f, 0.356863f, 
0.360784f, 0.364706f, 0.368627f, 0.372549f, 0.376471f, 0.380392f, 0.384314f, 
0.388235f, 0.392157f, 0.396078f, 0.4f, 0.403922f, 0.407843f, 0.411765f, 0.415686f, 
0.419608f, 0.423529f, 0.427451f, 0.431373f, 0.435294f, 0.439216f, 0.443137f, 
0.447059f, 0.45098f, 0.454902f, 0.458824f, 0.462745f, 0.466667f, 0.470588f, 
0.47451f, 0.478431f, 0.482353f, 0.486275f, 0.490196f, 0.494118f, 0.498039f, 
0.501961f, 0.505882f, 0.509804f, 0.513725f, 0.517647f, 0.521569f, 0.52549f, 
0.529412f, 0.533333f, 0.537255f, 0.541176f, 0.545098f, 0.54902f, 0.552941f, 
0.556863f, 0.560784f, 0.564706f, 0.568627f, 0.572549f, 0.576471f, 0.580392f, 
0.584314f, 0.588235f, 0.592157f, 0.596078f, 0.6f, 0.603922f, 0.607843f, 0.611765f, 
0.615686f, 0.619608f, 0.623529f, 0.627451f, 0.631373f, 0.635294f, 0.639216f, 
0.643137f, 0.647059f, 0.65098f, 0.654902f, 0.658824f, 0.662745f, 0.666667f, 
0.670588f, 0.67451f, 0.678431f, 0.682353f, 0.686275f, 0.690196f, 0.694118f, 
0.698039f, 0.701961f, 0.705882f, 0.709804f, 0.713725f, 0.717647f, 0.721569f, 
0.72549f, 0.729412f, 0.733333f, 0.737255f, 0.741176f, 0.745098f, 0.74902f, 
0.752941f, 0.756863f, 0.760784f, 0.764706f, 0.768627f, 0.772549f, 0.776471f, 
0.780392f, 0.784314f, 0.788235f, 0.792157f, 0.796078f, 0.8f, 0.803922f, 0.807843f, 
0.811765f, 0.815686f, 0.819608f, 0.823529f, 0.827451f, 0.831373f, 0.835294f, 
0.839216f, 0.843137f, 0.847059f, 0.85098f, 0.854902f, 0.858824f, 0.862745f, 
0.866667f, 0.870588f, 0.87451f, 0.878431f, 0.882353f, 0.886275f, 0.890196f, 
0.894118f, 0.898039f, 0.901961f, 0.905882f, 0.909804f, 0.913725f, 0.917647f, 
0.921569f, 0.92549f, 0.929412f, 0.933333f, 0.937255f, 0.941176f, 0.945098f, 
0.94902f, 0.952941f, 0.956863f, 0.960784f, 0.964706f, 0.968627f, 0.972549f, 
0.976471f, 0.980392f, 0.984314f, 0.988235f, 0.992157f, 0.996078f, 1.f
};

// Construct empty object.
// ****************************************************************************
//  Modifications:
//    Hank Childs, Tue May 25 10:25:46 PDT 2004
//    Initialize new members for display lists.
//
//    Brad Whitlock, Thu Aug 25 14:54:29 PST 2005
//    Initialized new point texturing members.
//
//    Brad Whitlock, Tue Dec 6 13:37:21 PST 2005
//    Changed to 1-pass texturing. Also added check for point sprite extension.
//
//    Brad Whitlock, Thu Aug 24 15:43:19 PST 2006
//    I added color texture members.
//
// ****************************************************************************
vtkVisItOpenGLPolyDataMapper::vtkVisItOpenGLPolyDataMapper()
{
  this->ListStart = 0;
  this->doingDisplayLists = false;
  this->primsInCurrentList = 0;
  this->nLists = 0;
  this->CurrentList = 0;
  this->PointTextureMethod = TEXTURE_NO_POINTS;
  this->SphereTexturesDataCreated = false;
  this->SphereTexturesLoaded = false;
  this->TextureName = 0;
#ifndef VTK_IMPLEMENT_MESA_CXX
  this->PointSpriteSupported = -1;  // OpenGL
#else
  this->PointSpriteSupported = 1;   // Mesa
#endif

  this->EnableColorTexturing = false;
  this->ColorTexturingAllowed = false;
  this->ColorTextureLoaded = false;
  this->ColorTextureName = 0;
  this->ColorTexture = 0;
  this->ColorTextureSize = 0;
  this->ColorTextureLooksDiscrete = false;
  this->OpenGLSupportsVersion1_2 = false;
#ifndef VTK_IMPLEMENT_MESA_CXX
  this->GLEW_initialized = false;
#endif
}

// Destructor (don't call ReleaseGraphicsResources() since it is virtual
vtkVisItOpenGLPolyDataMapper::~vtkVisItOpenGLPolyDataMapper()
{
  if (this->LastWindow)
    {
    this->ReleaseGraphicsResources(this->LastWindow);
    }

  if (this->ColorTexture != 0)
      delete [] this->ColorTexture;
}

// ****************************************************************************
// Release the graphics resources used by this mapper.  In this case, release
// the display list if any.
//
//  Modifications:
//    Brad Whitlock, Fri Aug 26 11:05:35 PDT 2005
//    Added code to free textures if they have been loaded.
//
// ****************************************************************************

void vtkVisItOpenGLPolyDataMapper::ReleaseGraphicsResources(vtkWindow *win)
{
  if (this->ListStart && win)
    {
    win->MakeCurrent();
    glDeleteLists(this->ListStart,nLists);
    this->ListStart = 0;
    }
  this->LastWindow = NULL; 

  // Free the textures if they have been loaded.
  if (this->SphereTexturesLoaded)
    {
        win->MakeCurrent();
        glDeleteTextures(1, &this->TextureName);
        this->SphereTexturesLoaded = false;
    }

  if (this->ColorTextureLoaded)
    {
        win->MakeCurrent();
        glDeleteTextures(1, &this->ColorTextureName);
        this->ColorTextureLoaded = false;        
    }
}

//
// Receives from Actor -> maps data to primitives
//
// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Tue May 25 10:04:36 PDT 2004
//    Break big display lists into groups of smaller display lists.
//
// ****************************************************************************
 
void vtkVisItOpenGLPolyDataMapper::RenderPiece(vtkRenderer *ren, vtkActor *act)
{
  vtkIdType numPts;
  vtkPolyData *input= this->GetInput();
  vtkPlaneCollection *clipPlanes;
  vtkPlane *plane;
  int i, numClipPlanes;
  double planeEquation[4];

  //
  // make sure that we've been properly initialized
  //
  if (ren->GetRenderWindow()->CheckAbortStatus())
    {
    return;
    }
  
  if ( input == NULL ) 
    {
    vtkErrorMacro(<< "No input!");
    return;
    }
  else
    {
    this->InvokeEvent(vtkCommand::StartEvent,NULL);
    input->Update();
    this->InvokeEvent(vtkCommand::EndEvent,NULL);

    numPts = input->GetNumberOfPoints();
    } 

  if (numPts == 0)
    {
    vtkDebugMacro(<< "No points!");
    return;
    }
  
  if ( this->LookupTable == NULL )
    {
    this->CreateDefaultLookupTable();
    }

// make sure our window is current
  ren->GetRenderWindow()->MakeCurrent();

  clipPlanes = this->ClippingPlanes;

  if (clipPlanes == NULL)
    {
    numClipPlanes = 0;
    }
  else
    {
    numClipPlanes = clipPlanes->GetNumberOfItems();
    if (numClipPlanes > 6)
      {
      vtkErrorMacro(<< "OpenGL guarantees at most 6 additional clipping planes");
      }
    }

  for (i = 0; i < numClipPlanes; i++)
    {
     glEnable((GLenum)(GL_CLIP_PLANE0+i));
    }

  if ( clipPlanes )
    {
    vtkMatrix4x4 *actorMatrix = vtkMatrix4x4::New();
    act->GetMatrix( actorMatrix );
    actorMatrix->Invert();
    
    double origin[4], normal[3], point[4];
    
    for (i = 0; i < numClipPlanes; i++)
      {    
      plane = (vtkPlane *)clipPlanes->GetItemAsObject(i);
      
      plane->GetOrigin(origin);
      plane->GetNormal(normal);
      
      point[0] = origin[0] + normal[0];
      point[1] = origin[1] + normal[1];
      point[2] = origin[2] + normal[2];
      
      origin[3] = point[3] = 1.0;
      
      actorMatrix->MultiplyPoint( origin, origin );
      actorMatrix->MultiplyPoint( point, point );
      
      if ( origin[3] != 1.0 )
        {
        origin[0] /= origin[3];
        origin[1] /= origin[3];
        origin[2] /= origin[3];
        }
      
      if ( point[3] != 1.0 )
        {
        point[0] /= point[3];
        point[1] /= point[3];
        point[2] /= point[3];
        }
      
      normal[0] = point[0] - origin[0];
      normal[1] = point[1] - origin[1];
      normal[2] = point[2] - origin[2];
      
      planeEquation[0] = normal[0];
      planeEquation[1] = normal[1];
      planeEquation[2] = normal[2];
      planeEquation[3] = -(planeEquation[0]*origin[0]+
                           planeEquation[1]*origin[1]+
                           planeEquation[2]*origin[2]);
      glClipPlane((GLenum)(GL_CLIP_PLANE0+i),planeEquation);
      }
    
    actorMatrix->Delete();  
    }
  

  //
  // if something has changed regenerate colors and display lists
  // if required
  //
  int noAbort=1;
  if ( this->GetMTime() > this->BuildTime || 
       input->GetMTime() > this->BuildTime ||
       act->GetProperty()->GetMTime() > this->BuildTime ||
       ren->GetRenderWindow() != this->LastWindow)
    {
    // sets this->Colors as side effect
    this->ColorTexturingAllowed = this->MapScalarsWithTextureSupport(
        act->GetProperty()->GetOpacity());

    if (!this->ImmediateModeRendering && 
        !this->GetGlobalImmediateModeRendering())
      {
      vtkTimerLog::MarkStartEvent("Building display list");
      this->ReleaseGraphicsResources(ren->GetRenderWindow());
      this->LastWindow = ren->GetRenderWindow();

      // get a unique display list id
      int nCells = input->GetNumberOfCells();
      this->nLists = nCells / dlSize;
      if ((nCells % dlSize) != 0)
          this->nLists++;
      this->nLists += 1; // For the "uber display list"
      this->ListStart = glGenLists(this->nLists);

      this->CurrentList = this->ListStart+1;
      this->doingDisplayLists = true;
      this->primsInCurrentList = 0;
      noAbort = this->Draw(ren,act);

      // Now make an uber-display-list that calls all of the other display
      // lists.
      glNewList(this->ListStart,GL_COMPILE);

      // Note that lastList will almost always be ListStart+nLists.
      // However: not all the draw methods know how to break up DLs into
      // smaller ones.  So there is a chance that CurrentList is smaller...
      int lastList = this->CurrentList;

      for (int i = this->ListStart+1 ; i <= lastList ; i++)
        {
        glCallList(i);
        }
      glEndList();
      vtkTimerLog::MarkEndEvent("Building display list");

      // Time the actual drawing
      this->Timer->StartTimer();
      // Turn on color texturing if it's enabled.
      this->BeginColorTexturing();
      glCallList(this->ListStart);
      this->EndColorTexturing();
      this->Timer->StopTimer();
      }
    else
      {
      this->ReleaseGraphicsResources(ren->GetRenderWindow());
      this->LastWindow = ren->GetRenderWindow();
      }
    if (noAbort)
      {
      this->BuildTime.Modified();
      }
    }
  // if nothing changed but we are using display lists, draw it
  else
    {
    if (!this->ImmediateModeRendering && 
        !this->GetGlobalImmediateModeRendering())
      {
      // Time the actual drawing
      this->Timer->StartTimer();
      this->BeginColorTexturing();
      glCallList(this->ListStart);
      this->EndColorTexturing();
      this->Timer->StopTimer();      
      }
    }
   
  // if we are in immediate mode rendering we always
  // want to draw the primitives here
  if (this->ImmediateModeRendering ||
      this->GetGlobalImmediateModeRendering())
    {
    // sets this->Colors as side effect
    this->ColorTexturingAllowed = this->MapScalarsWithTextureSupport(
        act->GetProperty()->GetOpacity());

    // Time the actual drawing
    this->Timer->StartTimer();
    this->doingDisplayLists = false;
    this->BeginColorTexturing();
    this->Draw(ren,act);
    this->EndColorTexturing();
    this->Timer->StopTimer();      
    }

  this->TimeToDraw = (float)this->Timer->GetElapsedTime();

  // If the timer is not accurate enough, set it to a small
  // time so that it is not zero
  if ( this->TimeToDraw == 0.0 )
    {
    this->TimeToDraw = 0.0001;
    }

  for (i = 0; i < numClipPlanes; i++)
    {
    glDisable((GLenum)(GL_CLIP_PLANE0+i));
    }
}

//
// Helper routine which starts a poly, triangle or quad based upon
// the number of points in the polygon and whether triangles or quads
// were the last thing being drawn (we can get better performance if we
// can draw several triangles within a single glBegin(GL_TRIANGLES) or
// several quads within a single glBegin(GL_QUADS). 
//
static void vtkOpenGLBeginPolyTriangleOrQuad(GLenum aGlFunction,
                                             GLenum &previousGlFunction,
                                             int npts)
{
  if (aGlFunction == GL_POLYGON)
    {
    switch (npts)
      {
      case 3:  // Need to draw a triangle.
        if (previousGlFunction != GL_TRIANGLES)
          {
          // we were not already drawing triangles, were we drawing quads?
          if (previousGlFunction == GL_QUADS)
            {
            // we were previously drawing quads, close down the quads.
            glEnd();
            }
          // start drawing triangles
          previousGlFunction = GL_TRIANGLES;
          glBegin(GL_TRIANGLES);
          }
          break;
      case 4:  // Need to draw a quad
        if (previousGlFunction != GL_QUADS)
          {
          // we were not already drawing quads, were we drawing triangles?
          if (previousGlFunction == GL_TRIANGLES)
            {
            // we were previously drawing triangles, close down the triangles.
            glEnd();
            }
          // start drawing quads
          previousGlFunction = GL_QUADS;
          glBegin(GL_QUADS);
          }
        break;
      default:
        // if we were supposed to be drawing polygons but were really
        // drawing triangles or quads, then we need to close down the
        // triangles or quads and begin a polygon
        if (previousGlFunction != GL_INVALID_VALUE
            && previousGlFunction != GL_POLYGON)
          {
          glEnd();
          }
        previousGlFunction = GL_POLYGON;
        glBegin(aGlFunction);
        break;
      }
    }
  else if (aGlFunction == GL_POINTS)
    {
    // we are supposed to be drawing points
    if (previousGlFunction != GL_POINTS)
      {
      // We were not drawing points before this, switch to points.
      // We don't need to worry about switching from triangles or quads
      // since draw all points before drawing any polygons (i.e. in the polys
      // case we switch to triangles and quads as an optimization, there is
      // nothing to switch to that is below points).
      previousGlFunction = GL_POINTS;
      glBegin(GL_POINTS);
      }
    }
  else
    {
    previousGlFunction = aGlFunction;
    glBegin(aGlFunction);
    }
}

// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Added new arguments for display lists.  DID NOT MAKE USE OF.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument -- NOT USED HERE.
//
static void vtkOpenGLDraw01(vtkCellArray *aPrim, GLenum aGlFunction,
                            vtkIdType &, vtkPoints *p, vtkDataArray *, 
                            vtkUnsignedCharArray *, vtkDataArray *, 
                            vtkOpenGLRenderer *ren, int &noAbort,
                            bool doingDisplayLists,int &primsInCurrentList,
                            int &CurrentList, bool ColorTexturingAllowed)
{
  int i,j;
  vtkIdType npts = 0;
  int count = 0;

  const float *vertices = (float*)p->GetVoidPointer(0); 
  GLenum previousGlFunction=GL_INVALID_VALUE;

  vtkIdType *ids = aPrim->GetData()->GetPointer(0);
  int nCells = aPrim->GetNumberOfCells();
  for (i = 0; i < nCells; i++, count++)
    {  
    npts = *ids++;
    vtkOpenGLBeginPolyTriangleOrQuad(aGlFunction, previousGlFunction, npts);
    for (j = 0; j < npts; j++)
      {
      glVertex3fv(vertices + 3*(*ids));
      ids++;
      }

    if ((previousGlFunction != GL_TRIANGLES)
        && (previousGlFunction != GL_QUADS)
        && (previousGlFunction != GL_POINTS))
      {
      glEnd();
      }
    
    // check for abort condition
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    }
  if ((previousGlFunction == GL_TRIANGLES)
      || (previousGlFunction == GL_QUADS)
      || (previousGlFunction == GL_POINTS))
    {
    glEnd();
    }
}

// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Break display lists into smaller chunks.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument -- NOT USED HERE.
//
static void vtkOpenGLDrawN013(vtkCellArray *aPrim, GLenum aGlFunction,
                              vtkIdType &, vtkPoints *p, vtkDataArray *n, 
                              vtkUnsignedCharArray *, vtkDataArray *, 
                              vtkOpenGLRenderer *ren, int &noAbort,
                              bool doingDisplayLists, int &primsInCurrentList,
                              int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  int count = 0;

  GLenum previousGlFunction=GL_INVALID_VALUE;
  
  const float *normal = (float*)n->GetVoidPointer(0);
  const float *vertices = (float*)p->GetVoidPointer(0);

  int ncells = aPrim->GetNumberOfCells();
  vtkIdType *ids = aPrim->GetData()->GetPointer(0);
  for (int i = 0 ; i < ncells ; i++, count++)
    {
    int npts = *ids++;
    vtkOpenGLBeginPolyTriangleOrQuad( aGlFunction, previousGlFunction, npts );
    
    for (j = 0; j < npts; j++) 
      {
      glNormal3fv(normal + 3*(*ids));
      glVertex3fv(vertices + 3*(*ids));
      ids++;
      }

    if ((previousGlFunction != GL_TRIANGLES) 
        && (previousGlFunction != GL_QUADS)
        && (previousGlFunction != GL_POINTS))
      {
      glEnd();
      }
    
    // check for abort condition
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    if (doingDisplayLists)
      {
      primsInCurrentList++;
      if (primsInCurrentList >= dlSize)
        {
        glEnd();
        glEndList();
        CurrentList++;
        glNewList(CurrentList,GL_COMPILE);
        glBegin(previousGlFunction);
        primsInCurrentList = 0;
        }
      }
    }
  if ((previousGlFunction == GL_TRIANGLES)
      || (previousGlFunction == GL_QUADS)
      || (previousGlFunction == GL_POINTS))
    {
    glEnd();
    }
}

// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Break display lists into smaller chunks.
//
//   Hank Childs, Tue Jun  1 16:47:49 PDT 2004
//   Index using the cellNum argument.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument --NOT USED UERE.
//
static void vtkOpenGLDrawCN013(vtkCellArray *aPrim, GLenum aGlFunction,
                               vtkIdType &cellNum, vtkPoints *p, 
                               vtkDataArray *n, vtkUnsignedCharArray *, 
                               vtkDataArray *, vtkOpenGLRenderer *ren, 
                               int &noAbort,
                               bool doingDisplayLists, int &primsInCurrentList,
                               int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  int count = 0;
  
  GLenum previousGlFunction=GL_INVALID_VALUE;

  const float *normal = (float*)n->GetVoidPointer(0);
  const float *vertices = (float*)p->GetVoidPointer(0);

  int ncells = aPrim->GetNumberOfCells();
  vtkIdType *ids = aPrim->GetData()->GetPointer(0);
  for (int i = 0 ; i < ncells ; i++, count++, cellNum++)
    {
    int npts = *ids++;
    vtkOpenGLBeginPolyTriangleOrQuad( aGlFunction, previousGlFunction, npts );
    
    glNormal3fv(normal + 3*cellNum);
    
    for (j = 0; j < npts; j++) 
      {
      glVertex3fv(vertices + 3*(*ids));
      ids++;
      }

    if ((previousGlFunction != GL_TRIANGLES) 
        && (previousGlFunction != GL_QUADS)
        && (previousGlFunction != GL_POINTS))
      {
      glEnd();
      }
    
    // check for abort condition
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    if (doingDisplayLists)
      {
      primsInCurrentList++;
      if (primsInCurrentList >= dlSize)
        {
        glEnd();
        glEndList();
        CurrentList++;
        glNewList(CurrentList,GL_COMPILE);
        glBegin(previousGlFunction);
        primsInCurrentList = 0;
        }
      }
    }
  if ((previousGlFunction == GL_TRIANGLES)
      || (previousGlFunction == GL_QUADS)
      || (previousGlFunction == GL_POINTS))
    {
    glEnd();
    }
}

// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Added new arguments for display lists.  DID NOT MAKE USE OF.
//
//   Hank Childs, Tue Nov  9 15:40:05 PST 2004
//   Optimize this case since this comes up for pseudocolors of
//   point meshes with point glyphs.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument.
//
static void vtkOpenGLDrawS01(vtkCellArray *aPrim, GLenum aGlFunction,
                             vtkIdType &, vtkPoints *p, vtkDataArray *, 
                             vtkUnsignedCharArray *c, vtkDataArray *, 
                             vtkOpenGLRenderer *ren, int &noAbort,
                             bool doingDisplayLists,int &primsInCurrentList,
                             int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  int count = 0;
  
  GLenum previousGlFunction=GL_INVALID_VALUE;

  const float *vertices = (float*)p->GetVoidPointer(0);
  const unsigned char *colors = c->GetPointer(0);

  int ncells = aPrim->GetNumberOfCells();
  vtkIdType *ids = aPrim->GetData()->GetPointer(0);

  for (int i = 0 ; i < ncells ; i++, count++)
    { 
    vtkIdType npts = *ids++;
    vtkOpenGLBeginPolyTriangleOrQuad( aGlFunction, previousGlFunction, 
                                      npts );

    if (ColorTexturingAllowed)
      {
      // We have nodal data and texturing is allowed, so use it.
      for (j = 0; j < npts; j++) 
        {
        glTexCoord1f(vtk1Over255[(colors + 4*(*ids))[0]]);
        glVertex3fv(vertices + 3*(*ids));
        ids++;
        }
      }
    else
      {
      for (j = 0; j < npts; j++) 
        {
        glColor4ubv(colors + 4*(*ids));
        glVertex3fv(vertices + 3*(*ids));
        ids++;
        }
      }

    if ((previousGlFunction != GL_TRIANGLES) 
        && (previousGlFunction != GL_QUADS)
        && (previousGlFunction != GL_POINTS))
      {
      glEnd();
      }
    
    // check for abort condition
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    if (doingDisplayLists)
      {
      primsInCurrentList++;
      if (primsInCurrentList >= dlSize)
        {
        glEnd();
        glEndList();
        CurrentList++;
        glNewList(CurrentList,GL_COMPILE);
        glBegin(previousGlFunction);
        primsInCurrentList = 0;
        }
      }
    }
  if ((previousGlFunction == GL_TRIANGLES)
      || (previousGlFunction == GL_QUADS)
      || (previousGlFunction == GL_POINTS))
    {
    glEnd();
    }
}

// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Break display lists into smaller chunks.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument.
//
static void vtkOpenGLDrawNS013(vtkCellArray *aPrim, GLenum aGlFunction,
                               vtkIdType &, vtkPoints *p, vtkDataArray *n, 
                               vtkUnsignedCharArray *c, vtkDataArray *, 
                               vtkOpenGLRenderer *ren, int &noAbort,
                               bool doingDisplayLists, int &primsInCurrentList,
                               int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  int count = 0;
  
  GLenum previousGlFunction=GL_INVALID_VALUE;

  const float *normal = (float*)n->GetVoidPointer(0);
  const float *vertices = (float*)p->GetVoidPointer(0);
  const unsigned char *colors = c->GetPointer(0);

  int ncells = aPrim->GetNumberOfCells();
  vtkIdType *ids = aPrim->GetData()->GetPointer(0);
  for (int i = 0 ; i < ncells ; i++, count++)
    {
    int npts = *ids++;
    vtkOpenGLBeginPolyTriangleOrQuad( aGlFunction, previousGlFunction, npts );

    if (ColorTexturingAllowed)
      {
      // Color texturing is enabled so use it.
      for (j = 0; j < npts; j++) 
        {
        const unsigned char *rgba = colors + 4*(*ids);
//        unsigned char white[] = {255,255,255,255}; //rgba[3]};
//        glColor4ubv(white);
        glTexCoord1f(vtk1Over255[rgba[0]]);
        glNormal3fv(normal + 3*(*ids));
        glVertex3fv(vertices + 3*(*ids));
        ids++;
        }
      }
    else
      {
      for (j = 0; j < npts; j++) 
        {
        glColor4ubv(colors + 4*(*ids));
        glNormal3fv(normal + 3*(*ids));
        glVertex3fv(vertices + 3*(*ids));
        ids++;
        }
      }

    if ((previousGlFunction != GL_TRIANGLES) 
        && (previousGlFunction != GL_QUADS)
        && (previousGlFunction != GL_POINTS))
      {
      glEnd();
      }
    
    // check for abort condition
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    if (doingDisplayLists)
      {
      primsInCurrentList++;
      if (primsInCurrentList >= dlSize)
        {
        glEnd();
        glEndList();
        CurrentList++;
        glNewList(CurrentList,GL_COMPILE);
        glBegin(previousGlFunction);
        primsInCurrentList = 0;
        }
      }
    }
  if ((previousGlFunction == GL_TRIANGLES)
      || (previousGlFunction == GL_QUADS)
      || (previousGlFunction == GL_POINTS))
    {
    glEnd();
    }
}

// Modifications:
//
//   Hank Childs, Thu May  6 07:36:12 PDT 2004
//   Make sure color comes before vertex.
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Break display lists into smaller chunks.
//
//   Hank Childs, Tue Jun  1 16:47:49 PDT 2004
//   Index using the cellNum argument.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument.
//

static void vtkOpenGLDrawCNS013(vtkCellArray *aPrim, GLenum aGlFunction,
                                vtkIdType &cellNum, vtkPoints *p, 
                                vtkDataArray *n, vtkUnsignedCharArray *c, 
                                vtkDataArray *, vtkOpenGLRenderer *ren, 
                                int &noAbort,
                                bool doingDisplayLists,int &primsInCurrentList,
                                int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  int count = 0;
  
  GLenum previousGlFunction=GL_INVALID_VALUE;

  const float *normal = (float*)n->GetVoidPointer(0);
  const float *vertices = (float*)p->GetVoidPointer(0);
  const unsigned char *colors = c->GetPointer(0);

  int ncells = aPrim->GetNumberOfCells();
  vtkIdType *ids = aPrim->GetData()->GetPointer(0);

  for (int i = 0 ; i < ncells ; i++, count++, cellNum++)
    {
    int npts = *ids++;
    vtkOpenGLBeginPolyTriangleOrQuad( aGlFunction, previousGlFunction, npts );
    
    glNormal3fv(normal + 3*cellNum);
    
    if (ColorTexturingAllowed)
      {
      for (j = 0; j < npts; j++) 
        {
        glTexCoord1f(vtk1Over255[(colors + 4*(*ids))[0]]);
        glVertex3fv(vertices + 3*(*ids));
        ids++;
        }
      }
    else
      {
      for (j = 0; j < npts; j++) 
        {
        glColor4ubv(colors + 4*(*ids));
        glVertex3fv(vertices + 3*(*ids));
        ids++;
        }
      }

    if ((previousGlFunction != GL_TRIANGLES) 
        && (previousGlFunction != GL_QUADS)
        && (previousGlFunction != GL_POINTS))
      {
      glEnd();
      }
    
    // check for abort condition
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    if (doingDisplayLists)
      {
      primsInCurrentList++;
      if (primsInCurrentList >= dlSize)
        {
        glEnd();
        glEndList();
        CurrentList++;
        glNewList(CurrentList,GL_COMPILE);
        glBegin(previousGlFunction);
        primsInCurrentList = 0;
        }
      }
    }
  if ((previousGlFunction == GL_TRIANGLES)
      || (previousGlFunction == GL_QUADS)
      || (previousGlFunction == GL_POINTS))
    {
    glEnd();
    }
}

// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Added new arguments for display lists.  DID NOT MAKE USE OF.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument -- NOT USED HERE.
//
static void vtkOpenGLDrawT01(vtkCellArray *aPrim, GLenum aGlFunction,
                             vtkIdType &, vtkPoints *p, vtkDataArray *, 
                             vtkUnsignedCharArray *, vtkDataArray *t, 
                             vtkOpenGLRenderer *ren, int &noAbort,
                             bool doingDisplayLists,int &primsInCurrentList,
                             int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  vtkIdType *pts = 0;
  vtkIdType npts = 0;
  int count = 0;
  
  GLenum previousGlFunction=GL_INVALID_VALUE;

  for (aPrim->InitTraversal(); noAbort && aPrim->GetNextCell(npts,pts); 
       count++)
    { 
    vtkOpenGLBeginPolyTriangleOrQuad( aGlFunction, previousGlFunction, 
                                      npts );
    
    for (j = 0; j < npts; j++) 
      {
      glTexCoord2dv(t->GetTuple(pts[j]));
      glVertex3dv(p->GetPoint(pts[j]));
      }

    if ((previousGlFunction != GL_TRIANGLES) 
        && (previousGlFunction != GL_QUADS)
        && (previousGlFunction != GL_POINTS))
      {
      glEnd();
      }
    
    // check for abort condition
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    }
  if ((previousGlFunction == GL_TRIANGLES)
      || (previousGlFunction == GL_QUADS)
      || (previousGlFunction == GL_POINTS))
    {
    glEnd();
    }
}

// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Added new arguments for display lists.  DID NOT MAKE USE OF.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument -- NOT USED HERE.
//
static void vtkOpenGLDrawNT013(vtkCellArray *aPrim, GLenum aGlFunction,
                               vtkIdType &, vtkPoints *p, vtkDataArray *n, 
                               vtkUnsignedCharArray *, vtkDataArray *t, 
                               vtkOpenGLRenderer *ren, int &noAbort,
                               bool doingDisplayLists,int &primsInCurrentList,
                               int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  vtkIdType *pts = 0;
  vtkIdType npts = 0;
  int count = 0;
  
  GLenum previousGlFunction=GL_INVALID_VALUE;
  
  for (aPrim->InitTraversal(); noAbort && aPrim->GetNextCell(npts,pts); 
       count++)
    { 
    vtkOpenGLBeginPolyTriangleOrQuad( aGlFunction, previousGlFunction, 
                                      npts );
    
    for (j = 0; j < npts; j++) 
      {
      glTexCoord2dv(t->GetTuple(pts[j]));
      glNormal3dv(n->GetTuple(pts[j]));
      glVertex3dv(p->GetPoint(pts[j]));
      }

    if ((previousGlFunction != GL_TRIANGLES) 
        && (previousGlFunction != GL_QUADS)
        && (previousGlFunction != GL_POINTS))
      {
      glEnd();
      }
    
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    }
  if ((previousGlFunction == GL_TRIANGLES)
      || (previousGlFunction == GL_QUADS)
      || (previousGlFunction == GL_POINTS))
    {
    glEnd();
    }
}

// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Added new arguments for display lists.  DID NOT MAKE USE OF.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument -- NOT USED HERE.
//
static void vtkOpenGLDrawCNT013(vtkCellArray *aPrim, GLenum aGlFunction,
                                vtkIdType &cellNum, vtkPoints *p, 
                                vtkDataArray *n, vtkUnsignedCharArray *, 
                                vtkDataArray *t, vtkOpenGLRenderer *ren, 
                                int &noAbort,
                                bool doingDisplayLists,int &primsInCurrentList,
                                int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  vtkIdType *pts = 0;
  vtkIdType npts = 0;
  int count = 0;
  
  GLenum previousGlFunction=GL_INVALID_VALUE;

  for (aPrim->InitTraversal(); noAbort && aPrim->GetNextCell(npts,pts); 
       count++, cellNum++)
    { 
    vtkOpenGLBeginPolyTriangleOrQuad( aGlFunction, previousGlFunction, 
                                      npts );
    
    glNormal3dv(n->GetTuple(cellNum));
    
    for (j = 0; j < npts; j++) 
      {
      glTexCoord2dv(t->GetTuple(pts[j]));
      glVertex3dv(p->GetPoint(pts[j]));
      }

    if ((previousGlFunction != GL_TRIANGLES) 
        && (previousGlFunction != GL_QUADS)
        && (previousGlFunction != GL_POINTS))
      {
      glEnd();
      }
    
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    }
  if ((previousGlFunction == GL_TRIANGLES)
      || (previousGlFunction == GL_QUADS)
      || (previousGlFunction == GL_POINTS))
    {
    glEnd();
    }
}

// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Added new arguments for display lists.  DID NOT MAKE USE OF.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument.
//
static void vtkOpenGLDrawST01(vtkCellArray *aPrim, GLenum aGlFunction,
                              vtkIdType &, vtkPoints *p, vtkDataArray *, 
                              vtkUnsignedCharArray *c, vtkDataArray *t, 
                              vtkOpenGLRenderer *ren, int &noAbort,
                              bool doingDisplayLists,int &primsInCurrentList,
                              int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  vtkIdType *pts = 0;
  vtkIdType npts = 0;
  int count = 0;
  
  GLenum previousGlFunction=GL_INVALID_VALUE;

  for (aPrim->InitTraversal(); noAbort && aPrim->GetNextCell(npts,pts); 
       count++)
    { 
    vtkOpenGLBeginPolyTriangleOrQuad( aGlFunction, previousGlFunction, npts );
    
    if (ColorTexturingAllowed)
      {
      for (j = 0; j < npts; j++) 
        {
        glTexCoord1f(vtk1Over255[(c->GetPointer(4*pts[j]))[0]]);
        glVertex3dv(p->GetPoint(pts[j]));
        }
      }
    else
      {
      for (j = 0; j < npts; j++) 
        {
        glColor4ubv(c->GetPointer(4*pts[j]));
        glTexCoord2dv(t->GetTuple(pts[j]));
        glVertex3dv(p->GetPoint(pts[j]));
        }
      }

    if ((previousGlFunction != GL_TRIANGLES) 
        && (previousGlFunction != GL_QUADS)
        && (previousGlFunction != GL_POINTS))
      {
      glEnd();
      }
    
    // check for abort condition
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    }
  if ((previousGlFunction == GL_TRIANGLES)
      || (previousGlFunction == GL_QUADS)
      || (previousGlFunction == GL_POINTS))
    {
    glEnd();
    }
}

// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Added new arguments for display lists.  DID NOT MAKE USE OF.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument.
//
static void vtkOpenGLDrawNST013(vtkCellArray *aPrim, GLenum aGlFunction,
                                vtkIdType &, vtkPoints *p, vtkDataArray *n, 
                                vtkUnsignedCharArray *c, vtkDataArray *t, 
                                vtkOpenGLRenderer *ren, int &noAbort,
                                bool doingDisplayLists,int &primsInCurrentList,
                                int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  vtkIdType *pts = 0;
  vtkIdType npts = 0;
  int count = 0;
  
  GLenum previousGlFunction=GL_INVALID_VALUE;

  for (aPrim->InitTraversal(); noAbort && aPrim->GetNextCell(npts,pts); 
       count++)
    { 
    vtkOpenGLBeginPolyTriangleOrQuad( aGlFunction, previousGlFunction, npts );
    
    if (ColorTexturingAllowed)
      {
      for (j = 0; j < npts; j++) 
        {
        glTexCoord1f(vtk1Over255[(c->GetPointer(4*pts[j]))[0]]);
        glNormal3dv(n->GetTuple(pts[j]));
        glVertex3dv(p->GetPoint(pts[j]));
        }
      }
    else
      {
      for (j = 0; j < npts; j++) 
        {
        glColor4ubv(c->GetPointer(4*pts[j]));
        glTexCoord2dv(t->GetTuple(pts[j]));
        glNormal3dv(n->GetTuple(pts[j]));
        glVertex3dv(p->GetPoint(pts[j]));
        }
      }

    if ((previousGlFunction != GL_TRIANGLES) 
        && (previousGlFunction != GL_QUADS)
        && (previousGlFunction != GL_POINTS))
      {
      glEnd();
      }
    
    // check for abort condition
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    }
  if ((previousGlFunction == GL_TRIANGLES)
      || (previousGlFunction == GL_QUADS)
      || (previousGlFunction == GL_POINTS))
    {
    glEnd();
    }
}

// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Added new arguments for display lists.  DID NOT MAKE USE OF.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument.
//

static void vtkOpenGLDrawCNST013(vtkCellArray *aPrim, GLenum aGlFunction,
                                 vtkIdType &cellNum, vtkPoints *p, 
                                 vtkDataArray *n, vtkUnsignedCharArray *c, 
                                 vtkDataArray *t, vtkOpenGLRenderer *ren,
                                 int &noAbort,
                                 bool doingDisplayLists,int &primsInCurrentList,
                                 int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  vtkIdType *pts = 0;
  vtkIdType npts = 0;
  int count = 0;
  
  GLenum previousGlFunction=GL_INVALID_VALUE;

  for (aPrim->InitTraversal(); noAbort && aPrim->GetNextCell(npts,pts); 
       count++, cellNum++)
    {
    vtkOpenGLBeginPolyTriangleOrQuad( aGlFunction, previousGlFunction, npts );
    
    glNormal3dv(n->GetTuple(cellNum));

    if (ColorTexturingAllowed)
      {
      for (j = 0; j < npts; j++) 
        {
        glTexCoord1f(vtk1Over255[(c->GetPointer(4*pts[j]))[0]]);
        glVertex3dv(p->GetPoint(pts[j]));
        }
      }
    else
      {    
      for (j = 0; j < npts; j++) 
        {
        glColor4ubv(c->GetPointer(4*pts[j]));
        glTexCoord2dv(t->GetTuple(pts[j]));
        glVertex3dv(p->GetPoint(pts[j]));
        }
      }

    if ((previousGlFunction != GL_TRIANGLES) 
        && (previousGlFunction != GL_QUADS)
        && (previousGlFunction != GL_POINTS))
      {
      glEnd();
      }
    
    // check for abort condition
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    }
  if ((previousGlFunction == GL_TRIANGLES)
      || (previousGlFunction == GL_QUADS)
      || (previousGlFunction == GL_POINTS))
    {
    glEnd();
    }
}

// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Added new arguments for display lists.  DID NOT MAKE USE OF.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument.
//

static void vtkOpenGLDrawCS01(vtkCellArray *aPrim, GLenum aGlFunction,
                              vtkIdType &cellNum, vtkPoints *p, vtkDataArray *, 
                              vtkUnsignedCharArray *c, vtkDataArray *, 
                              vtkOpenGLRenderer *ren, int &noAbort,
                              bool doingDisplayLists,int &primsInCurrentList,
                              int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  vtkIdType *pts = 0;
  vtkIdType npts = 0;
  int count = 0;
  
  GLenum previousGlFunction=GL_INVALID_VALUE;

  for (aPrim->InitTraversal(); noAbort && aPrim->GetNextCell(npts,pts); 
       count++, cellNum++)
    { 
    vtkOpenGLBeginPolyTriangleOrQuad( aGlFunction, previousGlFunction, 
                                      npts );
    
    if (ColorTexturingAllowed)
      {
      for (j = 0; j < npts; j++) 
        {
        glTexCoord1f(vtk1Over255[c->GetPointer(4*cellNum)[0]]);
        glVertex3dv(p->GetPoint(pts[j]));
        }
      }
    else
      {
      for (j = 0; j < npts; j++) 
        {
        glColor4ubv(c->GetPointer(4*cellNum));
        glVertex3dv(p->GetPoint(pts[j]));
        }
      }

    if ((previousGlFunction != GL_TRIANGLES) 
        && (previousGlFunction != GL_QUADS)
        && (previousGlFunction != GL_POINTS))
      {
      glEnd();
      }
    
    // check for abort condition
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    }
  if ((previousGlFunction == GL_TRIANGLES)
      || (previousGlFunction == GL_QUADS)
      || (previousGlFunction == GL_POINTS))
    {
    glEnd();
    }
}

// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Break display lists into smaller chunks.
//
//   Hank Childs, Tue Jun  1 16:47:49 PDT 2004
//   Index using the cellNum argument.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument -- NOT USED HERE
//

static void vtkOpenGLDrawNCS013(vtkCellArray *aPrim, GLenum aGlFunction,
                                vtkIdType &cellNum, vtkPoints *p, 
                                vtkDataArray *n, vtkUnsignedCharArray *c, 
                                vtkDataArray *, vtkOpenGLRenderer *ren,
                                int &noAbort,
                                bool doingDisplayLists,int &primsInCurrentList,
                                int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  int count = 0;
  
  GLenum previousGlFunction=GL_INVALID_VALUE;

  const float *normal = (float*)n->GetVoidPointer(0);
  const float *vertices = (float*)p->GetVoidPointer(0);
  const unsigned char *colors = c->GetPointer(0);

  int ncells = aPrim->GetNumberOfCells();
  vtkIdType *ids = aPrim->GetData()->GetPointer(0);
  for (int i = 0 ; i < ncells ; i++, count++, cellNum++)
    {
    int npts = *ids++;
    vtkOpenGLBeginPolyTriangleOrQuad( aGlFunction, previousGlFunction, 
                                      npts );
    
    glColor4ubv(colors + 4*cellNum);
    
    for (j = 0; j < npts; j++) 
      {
      glNormal3fv(normal + 3*(*ids));
      glVertex3fv(vertices + 3*(*ids));
      ids++;
      }

    if ((previousGlFunction != GL_TRIANGLES) 
        && (previousGlFunction != GL_QUADS)
        && (previousGlFunction != GL_POINTS))
      {
      glEnd();
      }
    
    // check for abort condition
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    if (doingDisplayLists)
      {
      primsInCurrentList++;
      if (primsInCurrentList >= dlSize)
        {
        glEnd();
        glEndList();
        CurrentList++;
        glNewList(CurrentList,GL_COMPILE);
        glBegin(previousGlFunction);
        primsInCurrentList = 0;
        }
      }
    }
  if ((previousGlFunction == GL_TRIANGLES)
      || (previousGlFunction == GL_QUADS)
      || (previousGlFunction == GL_POINTS))
    {
    glEnd();
    }
}

// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Break display lists into smaller chunks.
//
//   Hank Childs, Tue Jun  1 16:47:49 PDT 2004
//   Index using the cellNum argument.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument -- NOT USED HERE.
//

static void vtkOpenGLDrawCNCS013(vtkCellArray *aPrim, GLenum aGlFunction,
                                 vtkIdType &cellNum, vtkPoints *p, 
                                 vtkDataArray *n, vtkUnsignedCharArray *c, 
                                 vtkDataArray *, vtkOpenGLRenderer *ren,
                                 int &noAbort,
                                 bool doingDisplayLists,int &primsInCurrentList,
                                 int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  int count = 0;
  
  GLenum previousGlFunction=GL_INVALID_VALUE;

  const float *normal = (float*)n->GetVoidPointer(0);
  const float *vertices = (float*)p->GetVoidPointer(0);
  const unsigned char *colors = c->GetPointer(0);

  int ncells = aPrim->GetNumberOfCells();
  vtkIdType *ids = aPrim->GetData()->GetPointer(0);

  for (int i = 0 ; i < ncells ; i++, count++, cellNum++)
    {
    int npts = *ids++;
    vtkOpenGLBeginPolyTriangleOrQuad( aGlFunction, previousGlFunction, 
                                      npts );
    
    glColor4ubv(colors + 4*cellNum);
    glNormal3fv(normal + 3*cellNum);
    
    for (j = 0; j < npts; j++) 
      {
      glVertex3fv(vertices + 3*(*ids));
      ids++;
      }

    if ((previousGlFunction != GL_TRIANGLES) 
        && (previousGlFunction != GL_QUADS)
        && (previousGlFunction != GL_POINTS))
      {
      glEnd();
      }
    
    // check for abort condition
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    if (doingDisplayLists)
      {
      primsInCurrentList++;
      if (primsInCurrentList >= dlSize)
        {
        glEnd();
        glEndList();
        CurrentList++;
        glNewList(CurrentList,GL_COMPILE);
        glBegin(previousGlFunction);
        primsInCurrentList = 0;
        }
      }
    }
  if ((previousGlFunction == GL_TRIANGLES)
      || (previousGlFunction == GL_QUADS)
      || (previousGlFunction == GL_POINTS))
    {
    glEnd();
    }
}

// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Added new arguments for display lists.  DID NOT MAKE USE OF.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument.
//

static void vtkOpenGLDrawCST01(vtkCellArray *aPrim, GLenum aGlFunction,
                               vtkIdType &cellNum, vtkPoints *p, vtkDataArray *, 
                               vtkUnsignedCharArray *c, vtkDataArray *t, 
                               vtkOpenGLRenderer *ren, int &noAbort,
                               bool doingDisplayLists,int &primsInCurrentList,
                               int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  vtkIdType *pts = 0;
  vtkIdType npts = 0;
  int count = 0;
  
  GLenum previousGlFunction=GL_INVALID_VALUE;

  for (aPrim->InitTraversal(); noAbort && aPrim->GetNextCell(npts,pts); 
       count++, cellNum++)
    { 
    vtkOpenGLBeginPolyTriangleOrQuad( aGlFunction, previousGlFunction, 
                                      npts );
    
    if (ColorTexturingAllowed)
      {
      for (j = 0; j < npts; j++) 
        {
        glTexCoord1f(vtk1Over255[c->GetPointer(4*cellNum)[0]]);
        glVertex3dv(p->GetPoint(pts[j]));
        }
      }
    else
      {
      for (j = 0; j < npts; j++) 
        {
        glColor4ubv(c->GetPointer(4*cellNum));
        glTexCoord2dv(t->GetTuple(pts[j]));
        glVertex3dv(p->GetPoint(pts[j]));
        }
      }

    if ((previousGlFunction != GL_TRIANGLES) 
        && (previousGlFunction != GL_QUADS)
        && (previousGlFunction != GL_POINTS))
      {
      glEnd();
      }
    
    // check for abort condition
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    }
  if ((previousGlFunction == GL_TRIANGLES)
      || (previousGlFunction == GL_QUADS)
      || (previousGlFunction == GL_POINTS))
    {
    glEnd();
    }
}

// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Added new arguments for display lists.  DID NOT MAKE USE OF.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument.
//

static void vtkOpenGLDrawNCST013(vtkCellArray *aPrim, GLenum aGlFunction,
                                 vtkIdType &cellNum, vtkPoints *p, 
                                 vtkDataArray *n, vtkUnsignedCharArray *c, 
                                 vtkDataArray *t, vtkOpenGLRenderer *ren,
                                 int &noAbort,
                                 bool doingDisplayLists,int &primsInCurrentList,
                                 int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  vtkIdType *pts = 0;
  vtkIdType npts = 0;
  int count = 0;
  
  GLenum previousGlFunction=GL_INVALID_VALUE;

  for (aPrim->InitTraversal(); noAbort && aPrim->GetNextCell(npts,pts); 
       count++, cellNum++)
    { 
    vtkOpenGLBeginPolyTriangleOrQuad( aGlFunction, previousGlFunction, 
                                      npts );
    
    if (ColorTexturingAllowed)
      {
      for (j = 0; j < npts; j++) 
        {
        glTexCoord1f(vtk1Over255[c->GetPointer(4*cellNum)[0]]);
        glNormal3dv(n->GetTuple(pts[j]));
        glVertex3dv(p->GetPoint(pts[j]));
        }
      }
    else
      {
      for (j = 0; j < npts; j++) 
        {
        glColor4ubv(c->GetPointer(4*cellNum));
        glTexCoord2dv(t->GetTuple(pts[j]));
        glNormal3dv(n->GetTuple(pts[j]));
        glVertex3dv(p->GetPoint(pts[j]));
        }
      }

    if ((previousGlFunction != GL_TRIANGLES) 
        && (previousGlFunction != GL_QUADS)
        && (previousGlFunction != GL_POINTS))
      {
      glEnd();
      }
    
    // check for abort condition
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    }
  if ((previousGlFunction == GL_TRIANGLES)
      || (previousGlFunction == GL_QUADS)
      || (previousGlFunction == GL_POINTS))
    {
    glEnd();
    }
}

// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Added new arguments for display lists.  DID NOT MAKE USE OF.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument -- NOT USED HERE.
//

static void vtkOpenGLDrawCNCST013(vtkCellArray *aPrim, GLenum aGlFunction,
                                  vtkIdType &cellNum, vtkPoints *p, 
                                  vtkDataArray *n, vtkUnsignedCharArray *c, 
                                  vtkDataArray *t, vtkOpenGLRenderer *ren, 
                                  int &noAbort,
                                  bool doingDisplayLists,int &primsInCurrentList,
                                  int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  vtkIdType *pts = 0;
  vtkIdType npts = 0;
  int count = 0;
  
  GLenum previousGlFunction=GL_INVALID_VALUE;

  for (aPrim->InitTraversal(); noAbort && aPrim->GetNextCell(npts,pts); 
       count++, cellNum++)
    { 
    vtkOpenGLBeginPolyTriangleOrQuad( aGlFunction, previousGlFunction, 
                                      npts );
    
    glColor4ubv(c->GetPointer(4*cellNum));
    glNormal3dv(n->GetTuple(cellNum));
      
    for (j = 0; j < npts; j++) 
      {
      glTexCoord2dv(t->GetTuple(pts[j]));
      glVertex3dv(p->GetPoint(pts[j]));
      }

    if ((previousGlFunction != GL_TRIANGLES) 
        && (previousGlFunction != GL_QUADS)
        && (previousGlFunction != GL_POINTS))
      {
      glEnd();
      }
    
    // check for abort condition
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    }
  if ((previousGlFunction == GL_TRIANGLES)
      || (previousGlFunction == GL_QUADS)
      || (previousGlFunction == GL_POINTS))
    {
    glEnd();
    }
}


// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Break display lists into smaller chunks.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument -- NOT USED HERE.
//
static void vtkOpenGLDraw3(vtkCellArray *aPrim, GLenum aGlFunction,
                           vtkIdType &, vtkPoints *p, vtkDataArray *, 
                           vtkUnsignedCharArray *, vtkDataArray *, 
                           vtkOpenGLRenderer *ren, int &noAbort,
                           bool doingDisplayLists,int &primsInCurrentList,
                           int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  int count = 0;
  
  GLenum previousGlFunction=GL_INVALID_VALUE;

  const float *vertices = (float*)p->GetVoidPointer(0);

  int ncells = aPrim->GetNumberOfCells();
  vtkIdType *ids = aPrim->GetData()->GetPointer(0);
  for (int i = 0 ; i < ncells ; i++, count++)
    {
    int npts = *ids++;
    vtkOpenGLBeginPolyTriangleOrQuad( aGlFunction, previousGlFunction, npts );
    
    for (j = 0; j < npts; j++) 
      {
      glVertex3fv(vertices + 3*(*ids));
      ids++;
      }

    if ((previousGlFunction != GL_TRIANGLES) 
        && (previousGlFunction != GL_QUADS)
        && (previousGlFunction != GL_POINTS))
      {
      glEnd();
      }
    
    // check for abort condition
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    if (doingDisplayLists)
      {
      primsInCurrentList++;
      if (primsInCurrentList >= dlSize)
        {
        glEnd();
        glEndList();
        CurrentList++;
        glNewList(CurrentList,GL_COMPILE);
        glBegin(previousGlFunction);
        primsInCurrentList = 0;
        }
      }
    }
  if ((previousGlFunction == GL_TRIANGLES)
      || (previousGlFunction == GL_QUADS)
      || (previousGlFunction == GL_POINTS))
    {
    glEnd();
    }
}

// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Break display lists into smaller chunks.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument.
//

static void vtkOpenGLDrawS3(vtkCellArray *aPrim, GLenum aGlFunction,
                            vtkIdType &, vtkPoints *p, vtkDataArray *, 
                            vtkUnsignedCharArray *c, vtkDataArray *, 
                            vtkOpenGLRenderer *ren, int &noAbort,
                            bool doingDisplayLists,int &primsInCurrentList,
                            int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  int count = 0;

  GLenum previousGlFunction=GL_INVALID_VALUE;

  const float *vertices = (float*)p->GetVoidPointer(0);
  const unsigned char *colors = c->GetPointer(0);

  int ncells = aPrim->GetNumberOfCells();
  vtkIdType *ids = aPrim->GetData()->GetPointer(0); 
  for (int i = 0 ; i < ncells ; i++, count++)
    {
    int npts = *ids++;
    vtkOpenGLBeginPolyTriangleOrQuad( aGlFunction, previousGlFunction, 
                                      npts );
    if (ColorTexturingAllowed)
      {
      // 1D texturing version.
      for (j = 0; j < npts; j++) 
        {
        glTexCoord1f(vtk1Over255[(colors + 4*(*ids))[0]]);
        glVertex3fv(vertices + 3*(*ids));
        ids++;
        }
      }
    else
      {
      for (j = 0; j < npts; j++) 
        {
        glColor4ubv(colors + 4*(*ids));
        glVertex3fv(vertices + 3*(*ids));
        ids++;
        }
      }

    if ((previousGlFunction != GL_TRIANGLES) 
        && (previousGlFunction != GL_QUADS)
        && (previousGlFunction != GL_POINTS))
      {
      glEnd();
      }
    
    // check for abort condition
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    if (doingDisplayLists)
      {
      primsInCurrentList++;
      if (primsInCurrentList >= dlSize)
        {
        glEnd();
        glEndList();
        CurrentList++;
        glNewList(CurrentList,GL_COMPILE);
        glBegin(previousGlFunction);
        primsInCurrentList = 0;
        }
      }
    }
  if ((previousGlFunction == GL_TRIANGLES)
      || (previousGlFunction == GL_QUADS)
      || (previousGlFunction == GL_POINTS))
    {
    glEnd();
    }
}

// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Added new arguments for display lists.  DID NOT MAKE USE OF.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument -- NOT USED HERE.
//
static void vtkOpenGLDrawT3(vtkCellArray *aPrim, GLenum aGlFunction,
                            vtkIdType &, vtkPoints *p, vtkDataArray *, 
                            vtkUnsignedCharArray *, vtkDataArray *t, 
                            vtkOpenGLRenderer *ren, int &noAbort,
                            bool doingDisplayLists,int &primsInCurrentList,
                            int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  vtkIdType *pts = 0;
  vtkIdType npts = 0;
  int count = 0;
  double polyNorm[3];
  
  GLenum previousGlFunction=GL_INVALID_VALUE;

  for (aPrim->InitTraversal(); noAbort && aPrim->GetNextCell(npts,pts); 
       count++)
    { 
    vtkOpenGLBeginPolyTriangleOrQuad( aGlFunction, previousGlFunction, npts );
    
    vtkPolygon::ComputeNormal(p,npts,pts,polyNorm);
    
    for (j = 0; j < npts; j++) 
      {
      glTexCoord2dv(t->GetTuple(pts[j]));
      glNormal3dv(polyNorm);
      glVertex3dv(p->GetPoint(pts[j]));
      }

    if ((previousGlFunction != GL_TRIANGLES) 
        && (previousGlFunction != GL_QUADS)
        && (previousGlFunction != GL_POINTS))
      {
      glEnd();
      }
    
    // check for abort condition
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    }
  if ((previousGlFunction == GL_TRIANGLES)
      || (previousGlFunction == GL_QUADS)
      || (previousGlFunction == GL_POINTS))
    {
    glEnd();
    }
}

// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Added new arguments for display lists.  DID NOT MAKE USE OF.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument.
//
static void vtkOpenGLDrawST3(vtkCellArray *aPrim, GLenum aGlFunction,
                             vtkIdType &, vtkPoints *p, vtkDataArray *, 
                             vtkUnsignedCharArray *c, vtkDataArray *t, 
                             vtkOpenGLRenderer *ren, int &noAbort,
                             bool doingDisplayLists,int &primsInCurrentList,
                             int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  vtkIdType *pts = 0;
  vtkIdType npts = 0;
  int count = 0;
  double polyNorm[3];
  
  GLenum previousGlFunction=GL_INVALID_VALUE;

  for (aPrim->InitTraversal(); noAbort && aPrim->GetNextCell(npts,pts); 
       count++)
    { 
    vtkOpenGLBeginPolyTriangleOrQuad( aGlFunction, previousGlFunction, npts );
    
    vtkPolygon::ComputeNormal(p,npts,pts,polyNorm);
    
    if (ColorTexturingAllowed)
      {
      for (j = 0; j < npts; j++) 
        {
        glTexCoord1f(vtk1Over255[c->GetPointer(4*pts[j])[0]]);
        glNormal3dv(polyNorm);
        glVertex3dv(p->GetPoint(pts[j]));
        }
      }
    else
      {
      for (j = 0; j < npts; j++) 
        {
        glColor4ubv(c->GetPointer(4*pts[j]));
        glTexCoord2dv(t->GetTuple(pts[j]));
        glNormal3dv(polyNorm);
        glVertex3dv(p->GetPoint(pts[j]));
        }
      }
    
    if ((previousGlFunction != GL_TRIANGLES)
        && (previousGlFunction != GL_QUADS)
        && (previousGlFunction != GL_POINTS))
      {
      glEnd();
      }
    
    // check for abort condition
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    }
  if ((previousGlFunction == GL_TRIANGLES)
      || (previousGlFunction == GL_QUADS)
      || (previousGlFunction == GL_POINTS))
    {
    glEnd();
    }
}

// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Break display lists into smaller chunks.
//
//   Hank Childs, Tue Jun  1 16:47:49 PDT 2004
//   Index using the cellNum argument.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument -- NOT USED HERE.
//
static void vtkOpenGLDrawCS3(vtkCellArray *aPrim, GLenum aGlFunction,
                             vtkIdType &cellNum, vtkPoints *p, vtkDataArray *, 
                             vtkUnsignedCharArray *c, vtkDataArray *, 
                             vtkOpenGLRenderer *ren, int &noAbort,
                             bool doingDisplayLists,int &primsInCurrentList,
                             int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  int count = 0;
  
  GLenum previousGlFunction=GL_INVALID_VALUE;

  const float *vertices = (float*)p->GetVoidPointer(0);
  const unsigned char *colors = c->GetPointer(0);

  int ncells = aPrim->GetNumberOfCells();
  vtkIdType *ids = aPrim->GetData()->GetPointer(0);
  for (int i = 0 ; i < ncells ; i++, count++, cellNum++)
    {
    int npts = *ids++;
    vtkOpenGLBeginPolyTriangleOrQuad( aGlFunction, previousGlFunction, npts );
    
    glColor4ubv(colors + 4*cellNum);
    for (j = 0; j < npts; j++) 
      {
      glVertex3fv(vertices + 3*(*ids));
      ids++;
      }

    if ((previousGlFunction != GL_TRIANGLES)
        && (previousGlFunction != GL_QUADS)
        && (previousGlFunction != GL_POINTS))
      {
      glEnd();
      }

    // check for abort condition
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    if (doingDisplayLists)
      {
      primsInCurrentList++;
      if (primsInCurrentList >= dlSize)
        {
        glEnd();
        glEndList();
        CurrentList++;
        glNewList(CurrentList,GL_COMPILE);
        glBegin(previousGlFunction);
        primsInCurrentList = 0;
        }
      }
    }
  if ((previousGlFunction == GL_TRIANGLES)
      || (previousGlFunction == GL_QUADS)
      || (previousGlFunction == GL_POINTS))
    {
    glEnd();
    }
}

// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Added new arguments for display lists.  DID NOT MAKE USE OF.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument.
//
static void vtkOpenGLDrawCST3(vtkCellArray *aPrim, GLenum aGlFunction,
                              vtkIdType &cellNum, vtkPoints *p, vtkDataArray *, 
                              vtkUnsignedCharArray *c, vtkDataArray *t, 
                              vtkOpenGLRenderer *ren, int &noAbort,
                              bool doingDisplayLists,int &primsInCurrentList,
                              int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  vtkIdType *pts = 0;
  vtkIdType npts = 0;
  int count = 0;
  double polyNorm[3];
  
  GLenum previousGlFunction=GL_INVALID_VALUE;

  for (aPrim->InitTraversal(); noAbort && aPrim->GetNextCell(npts,pts); 
       count++, cellNum++)
    { 
    vtkOpenGLBeginPolyTriangleOrQuad( aGlFunction, previousGlFunction, npts );
    
    vtkPolygon::ComputeNormal(p,npts,pts,polyNorm);
    
    if (ColorTexturingAllowed)
      {
      for (j = 0; j < npts; j++) 
        {
        glTexCoord1f(vtk1Over255[c->GetPointer(4*cellNum)[0]]);
        glNormal3dv(polyNorm);
        glVertex3dv(p->GetPoint(pts[j]));
        }
      }
    else
      {
      for (j = 0; j < npts; j++) 
        {
        glColor4ubv(c->GetPointer(4*cellNum));
        glTexCoord2dv(t->GetTuple(pts[j]));
        glNormal3dv(polyNorm);
        glVertex3dv(p->GetPoint(pts[j]));
        }
      }

    if ((previousGlFunction != GL_TRIANGLES) 
        && (previousGlFunction != GL_QUADS)
        && (previousGlFunction != GL_POINTS))
      {
      glEnd();
      }
    
    // check for abort condition
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    }
  if ((previousGlFunction == GL_TRIANGLES)
      || (previousGlFunction == GL_QUADS)
      || (previousGlFunction == GL_POINTS))
    {
    glEnd();
    }
}
  
// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Added new arguments for display lists.  DID NOT MAKE USE OF.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument -- NOT USED HERE.
//
static void vtkOpenGLDraw2(vtkCellArray *aPrim, GLenum aGlFunction,
                           vtkIdType &, vtkPoints *p, vtkDataArray *, 
                           vtkUnsignedCharArray *, vtkDataArray *, 
                           vtkOpenGLRenderer *ren, int &noAbort,
                           bool doingDisplayLists,int &primsInCurrentList,
                           int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  vtkIdType *pts = 0;
  vtkIdType npts = 0;
  vtkIdType idx[3];
  int count = 0;
  double polyNorm[3];
  
  GLenum previousGlFunction=GL_INVALID_VALUE;

  for (aPrim->InitTraversal(); noAbort && aPrim->GetNextCell(npts,pts); 
       count++)
    { 
    vtkOpenGLBeginPolyTriangleOrQuad( aGlFunction, previousGlFunction, npts );
    
    vtkTriangle::ComputeNormal(p,3,pts,polyNorm);
    
    for (j = 0; j < npts; j++) 
      {
      if ( j > 2)
        {
        if (j % 2)
          {
          idx[0] = pts[j-2]; idx[1] = pts[j]; idx[2] = pts[j-1]; 
          vtkTriangle::ComputeNormal(p, 3, idx, polyNorm);
          }
        else
          {
          idx[0] = pts[j-2]; idx[1] = pts[j-1]; idx[2] = pts[j]; 
          vtkTriangle::ComputeNormal(p, 3, idx, polyNorm);
          }
        }
      else if ( j == 0 )
        {
        vtkTriangle::ComputeNormal(p, 3, pts, polyNorm);
        }
      glNormal3dv(polyNorm);
      glVertex3dv(p->GetPoint(pts[j]));
      }

    if ((previousGlFunction != GL_TRIANGLES) 
        && (previousGlFunction != GL_QUADS)
        && (previousGlFunction != GL_POINTS))
      {
      glEnd();
      }
    
    // check for abort condition
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    }
  if ((previousGlFunction == GL_TRIANGLES)
      || (previousGlFunction == GL_QUADS)
      || (previousGlFunction == GL_POINTS))
    {
    glEnd();
    }
}

// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Added new arguments for display lists.  DID NOT MAKE USE OF.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument.
//
static void vtkOpenGLDrawS2(vtkCellArray *aPrim, GLenum aGlFunction,
                            vtkIdType &, vtkPoints *p, vtkDataArray *, 
                            vtkUnsignedCharArray *c, vtkDataArray *, 
                            vtkOpenGLRenderer *ren, int &noAbort,
                            bool doingDisplayLists,int &primsInCurrentList,
                            int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  vtkIdType *pts = 0;
  vtkIdType npts = 0;
  vtkIdType idx[3];
  int count = 0;
  double polyNorm[3];
  
  GLenum previousGlFunction=GL_INVALID_VALUE;

  for (aPrim->InitTraversal(); noAbort && aPrim->GetNextCell(npts,pts); 
       count++)
    { 
    vtkOpenGLBeginPolyTriangleOrQuad( aGlFunction, previousGlFunction, npts );
    
    vtkTriangle::ComputeNormal(p,3,pts,polyNorm);
    
    for (j = 0; j < npts; j++) 
      {
      if (ColorTexturingAllowed)
        {
        // 1D texturing version.
        glTexCoord1f(vtk1Over255[c->GetPointer(4*pts[j])[0]]);
        }
      else
        glColor4ubv(c->GetPointer(4*pts[j]));

      if ( j > 2)
        {
        if (j % 2)
          {
          idx[0] = pts[j-2]; idx[1] = pts[j]; idx[2] = pts[j-1]; 
          vtkTriangle::ComputeNormal(p, 3, idx, polyNorm);
          }
        else
          {
          idx[0] = pts[j-2]; idx[1] = pts[j-1]; idx[2] = pts[j]; 
          vtkTriangle::ComputeNormal(p, 3, idx, polyNorm);
          }
        }
      else if ( j == 0 )
        {
        vtkTriangle::ComputeNormal(p, 3, pts, polyNorm);
        }
      glNormal3dv(polyNorm);
      glVertex3dv(p->GetPoint(pts[j]));
      }

    if ((previousGlFunction != GL_TRIANGLES) 
        && (previousGlFunction != GL_QUADS)
        && (previousGlFunction != GL_POINTS))
      {
      glEnd();
      }

    // check for abort condition
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    }
  if ((previousGlFunction == GL_TRIANGLES)
      || (previousGlFunction == GL_QUADS)
      || (previousGlFunction == GL_POINTS))
    {
    glEnd();
    }
}

// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Added new arguments for display lists.  DID NOT MAKE USE OF.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument -- NOT USED HERE.
//
static void vtkOpenGLDrawT2(vtkCellArray *aPrim, GLenum aGlFunction,
                            vtkIdType &, vtkPoints *p, vtkDataArray *, 
                            vtkUnsignedCharArray *, vtkDataArray *t, 
                            vtkOpenGLRenderer *ren, int &noAbort,
                            bool doingDisplayLists,int &primsInCurrentList,
                            int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  vtkIdType idx[3];
  vtkIdType *pts = 0;
  vtkIdType npts = 0;
  int count = 0;
  double polyNorm[3];
  
  GLenum previousGlFunction=GL_INVALID_VALUE;

  for (aPrim->InitTraversal(); noAbort && aPrim->GetNextCell(npts,pts); 
       count++)
    { 
    vtkOpenGLBeginPolyTriangleOrQuad( aGlFunction, previousGlFunction, npts );
    
    vtkTriangle::ComputeNormal(p,3,pts,polyNorm);
    
    for (j = 0; j < npts; j++) 
      {
      glTexCoord2dv(t->GetTuple(pts[j]));
      if ( j > 2)
        {
        if (j % 2)
          {
          idx[0] = pts[j-2]; idx[1] = pts[j]; idx[2] = pts[j-1]; 
          vtkTriangle::ComputeNormal(p, 3, idx, polyNorm);
          }
        else
          {
          idx[0] = pts[j-2]; idx[1] = pts[j-1]; idx[2] = pts[j]; 
          vtkTriangle::ComputeNormal(p, 3, idx, polyNorm);
          }
        }
      else if ( j == 0 )
        {
        vtkTriangle::ComputeNormal(p, 3, pts, polyNorm);
        }
      glNormal3dv(polyNorm);
      glVertex3dv(p->GetPoint(pts[j]));
      }

    if ((previousGlFunction != GL_TRIANGLES) 
        && (previousGlFunction != GL_QUADS)
        && (previousGlFunction != GL_POINTS))
      {
      glEnd();
      }
    
    // check for abort condition
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    }
  if ((previousGlFunction == GL_TRIANGLES)
      || (previousGlFunction == GL_QUADS)
      || (previousGlFunction == GL_POINTS))
    {
    glEnd();
    }
}

// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Added new arguments for display lists.  DID NOT MAKE USE OF.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument.
//
static void vtkOpenGLDrawST2(vtkCellArray *aPrim, GLenum aGlFunction,
                             vtkIdType &, vtkPoints *p, vtkDataArray *, 
                             vtkUnsignedCharArray *c, vtkDataArray *t, 
                             vtkOpenGLRenderer *ren, int &noAbort,
                             bool doingDisplayLists,int &primsInCurrentList,
                             int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  vtkIdType idx[3];
  vtkIdType *pts = 0;
  vtkIdType npts = 0;
  int count = 0;
  double polyNorm[3];
  
  GLenum previousGlFunction=GL_INVALID_VALUE;

  for (aPrim->InitTraversal(); noAbort && aPrim->GetNextCell(npts,pts); 
       count++)
    { 
    vtkOpenGLBeginPolyTriangleOrQuad( aGlFunction, previousGlFunction, npts );
    
    vtkTriangle::ComputeNormal(p,3,pts,polyNorm);
    
    for (j = 0; j < npts; j++) 
      {
      if (ColorTexturingAllowed)
        {
        glTexCoord1f(vtk1Over255[c->GetPointer(4*pts[j])[0]]);
        }
      else
        {
        glColor4ubv(c->GetPointer(4*pts[j]));
        glTexCoord2dv(t->GetTuple(pts[j]));
        }
      if ( j > 2)
        {
        if (j % 2)
          {
          idx[0] = pts[j-2]; idx[1] = pts[j]; idx[2] = pts[j-1]; 
          vtkTriangle::ComputeNormal(p, 3, idx, polyNorm);
          }
        else
          {
          idx[0] = pts[j-2]; idx[1] = pts[j-1]; idx[2] = pts[j]; 
          vtkTriangle::ComputeNormal(p, 3, idx, polyNorm);
          }
        }
      else if ( j == 0 )
        {
        vtkTriangle::ComputeNormal(p, 3, pts, polyNorm);
        }
      glNormal3dv(polyNorm);
      glVertex3dv(p->GetPoint(pts[j]));
      }

    if ((previousGlFunction != GL_TRIANGLES) 
        && (previousGlFunction != GL_QUADS)
        && (previousGlFunction != GL_POINTS))
      {
      glEnd();
      }
    
    // check for abort condition
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    }
  if ((previousGlFunction == GL_TRIANGLES)
      || (previousGlFunction == GL_QUADS)
      || (previousGlFunction == GL_POINTS))
    {
    glEnd();
    }
}

// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Added new arguments for display lists.  DID NOT MAKE USE OF.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument.
//
static void vtkOpenGLDrawCS2(vtkCellArray *aPrim, GLenum aGlFunction,
                             vtkIdType &cellNum, vtkPoints *p, vtkDataArray *, 
                             vtkUnsignedCharArray *c, vtkDataArray *, 
                             vtkOpenGLRenderer *ren, int &noAbort,
                             bool doingDisplayLists,int &primsInCurrentList,
                             int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  vtkIdType idx[3];
  vtkIdType *pts = 0;
  vtkIdType npts = 0;
  int count = 0;
  double polyNorm[3];
  
  GLenum previousGlFunction=GL_INVALID_VALUE;

  for (aPrim->InitTraversal(); noAbort && aPrim->GetNextCell(npts,pts); 
       count++, cellNum++)
    { 
    vtkOpenGLBeginPolyTriangleOrQuad( aGlFunction, previousGlFunction, npts );
    
    vtkTriangle::ComputeNormal(p,3,pts,polyNorm);
    
    for (j = 0; j < npts; j++) 
      {
      if (ColorTexturingAllowed)
         glTexCoord1f(vtk1Over255[c->GetPointer(4*cellNum)[0]]);
      else
         glColor4ubv(c->GetPointer(4*cellNum));
      if ( j > 2)
        {
        if (j % 2)
          {
          idx[0] = pts[j-2]; idx[1] = pts[j]; idx[2] = pts[j-1]; 
          vtkTriangle::ComputeNormal(p, 3, idx, polyNorm);
          }
        else
          {
          idx[0] = pts[j-2]; idx[1] = pts[j-1]; idx[2] = pts[j]; 
          vtkTriangle::ComputeNormal(p, 3, idx, polyNorm);
          }
        }
      else if ( j == 0 )
        {
        vtkTriangle::ComputeNormal(p, 3, pts, polyNorm);
        }
      glNormal3dv(polyNorm);
      glVertex3dv(p->GetPoint(pts[j]));
      }

    if ((previousGlFunction != GL_TRIANGLES) 
        && (previousGlFunction != GL_QUADS)
        && (previousGlFunction != GL_POINTS))
      {
      glEnd();
      }
    
    // check for abort condition
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    }
  if ((previousGlFunction == GL_TRIANGLES)
      || (previousGlFunction == GL_QUADS)
      || (previousGlFunction == GL_POINTS))
    {
    glEnd();
    }
}

// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Added new arguments for display lists.  DID NOT MAKE USE OF.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument.
//
static void vtkOpenGLDrawCST2(vtkCellArray *aPrim, GLenum aGlFunction,
                              vtkIdType &cellNum, vtkPoints *p, vtkDataArray *, 
                              vtkUnsignedCharArray *c, vtkDataArray *t, 
                              vtkOpenGLRenderer *ren, int &noAbort,
                              bool doingDisplayLists,int &primsInCurrentList,
                              int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  vtkIdType idx[3];
  vtkIdType *pts = 0;
  vtkIdType npts = 0;
  int count = 0;
  double polyNorm[3];
  
  GLenum previousGlFunction=GL_INVALID_VALUE;

  for (aPrim->InitTraversal(); noAbort && aPrim->GetNextCell(npts,pts); 
       count++, cellNum++)
    { 
    vtkOpenGLBeginPolyTriangleOrQuad( aGlFunction, previousGlFunction, npts );
    
    vtkTriangle::ComputeNormal(p,3,pts,polyNorm);

    for (j = 0; j < npts; j++) 
      {
      if (ColorTexturingAllowed)
         glTexCoord1f(vtk1Over255[c->GetPointer(4*cellNum)[0]]);
      else
         {
         glColor4ubv(c->GetPointer(4*cellNum));
         glTexCoord2dv(t->GetTuple(pts[j]));
         }
      if ( j > 2)
        {
        if (j % 2)
          {
          idx[0] = pts[j-2]; idx[1] = pts[j]; idx[2] = pts[j-1]; 
          vtkTriangle::ComputeNormal(p, 3, idx, polyNorm);
          }
        else
          {
          idx[0] = pts[j-2]; idx[1] = pts[j-1]; idx[2] = pts[j]; 
          vtkTriangle::ComputeNormal(p, 3, idx, polyNorm);
          }
        }
      else if ( j == 0 )
        {
        vtkTriangle::ComputeNormal(p, 3, pts, polyNorm);
        }
      glNormal3dv(polyNorm);
      glVertex3dv(p->GetPoint(pts[j]));
      }

    if ((previousGlFunction != GL_TRIANGLES) 
        && (previousGlFunction != GL_QUADS)
        && (previousGlFunction != GL_POINTS))
      {
      glEnd();
      }
    
    // check for abort condition
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    }
  if ((previousGlFunction == GL_TRIANGLES)
      || (previousGlFunction == GL_QUADS)
      || (previousGlFunction == GL_POINTS))
    {
    glEnd();
    }
} 
  
// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Added new arguments for display lists.  DID NOT MAKE USE OF.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument -- NOT USED HERE.
//

static void vtkOpenGLDrawW(vtkCellArray *aPrim, GLenum,
                           vtkIdType &, vtkPoints *p, vtkDataArray *, 
                           vtkUnsignedCharArray *, vtkDataArray *, 
                           vtkOpenGLRenderer *ren, int &noAbort,
                           bool doingDisplayLists,int &primsInCurrentList,
                           int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  vtkIdType idx[3];
  vtkIdType *pts = 0;
  vtkIdType npts = 0;
  int count = 0;
  double polyNorm[3];
  
  for (aPrim->InitTraversal(); noAbort && aPrim->GetNextCell(npts,pts); 
       count++)
    { 
    // draw first line
    glBegin(GL_LINE_STRIP);
    for (j = 0; j < npts; j += 2) 
      {
      if ( j == 0 )
        {
        vtkTriangle::ComputeNormal(p, 3, pts, polyNorm);
        }
      else
        {
        idx[0] = pts[j-2]; idx[1] = pts[j-1]; idx[2] = pts[j]; 
        vtkTriangle::ComputeNormal(p, 3, idx, polyNorm);
        }
      glNormal3dv(polyNorm);
      glVertex3dv(p->GetPoint(pts[j]));
      }
    glEnd();
      
    // draw second line
    glBegin(GL_LINE_STRIP);
    for (j = 1; j < npts; j += 2) 
      {
      if (j == 1)
        {
        vtkTriangle::ComputeNormal(p, 3, pts, polyNorm);
        }
      else
        {
        idx[0] = pts[j-2]; idx[1] = pts[j]; idx[2] = pts[j-1]; 
        vtkTriangle::ComputeNormal(p, 3, idx, polyNorm);
        }
      glNormal3dv(polyNorm);
      glVertex3dv(p->GetPoint(pts[j]));
      }
    glEnd();
  
    // check for abort condition
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    }
}

// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Added new arguments for display lists.  DID NOT MAKE USE OF.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument -- NOT USED HERE.
//
static void vtkOpenGLDrawNW(vtkCellArray *aPrim, GLenum,
                            vtkIdType &, vtkPoints *p, vtkDataArray *n, 
                            vtkUnsignedCharArray *, vtkDataArray *, 
                            vtkOpenGLRenderer *ren, int &noAbort,
                            bool doingDisplayLists,int &primsInCurrentList,
                            int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  vtkIdType *pts = 0;
  vtkIdType npts = 0;
  int count = 0;
  
  for (aPrim->InitTraversal(); noAbort && aPrim->GetNextCell(npts,pts); 
       count++)
    { 
    // draw first line
    glBegin(GL_LINE_STRIP);
    for (j = 0; j < npts; j += 2) 
      {
      glNormal3dv(n->GetTuple(pts[j]));
      glVertex3dv(p->GetPoint(pts[j]));
      }
    glEnd();
    
    // draw second line
    glBegin(GL_LINE_STRIP);
    for (j = 1; j < npts; j += 2) 
      {
      glNormal3dv(n->GetTuple(pts[j]));
      glVertex3dv(p->GetPoint(pts[j]));
      }
    glEnd();
    
    // check for abort condition
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    }
}

// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Added new arguments for display lists.  DID NOT MAKE USE OF.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument.
//
static void vtkOpenGLDrawSW(vtkCellArray *aPrim, GLenum,
                            vtkIdType &, vtkPoints *p, vtkDataArray *, 
                            vtkUnsignedCharArray *c, vtkDataArray *, 
                            vtkOpenGLRenderer *ren, int &noAbort,
                            bool doingDisplayLists,int &primsInCurrentList,
                            int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  vtkIdType *pts = 0;
  vtkIdType idx[3];
  vtkIdType npts = 0;
  int count = 0;
  double polyNorm[3];
  
  for (aPrim->InitTraversal(); noAbort && aPrim->GetNextCell(npts,pts); 
       count++)
    { 
    // draw first line
    glBegin(GL_LINE_STRIP);
    for (j = 0; j < npts; j += 2) 
      {
      if (ColorTexturingAllowed)
         glTexCoord1f(vtk1Over255[c->GetPointer(4*pts[j])[0]]);
      else
         glColor4ubv(c->GetPointer(4*pts[j]));
      if ( j == 0 )
        {
        vtkTriangle::ComputeNormal(p, 3, pts, polyNorm);
        }
      else
        {
        idx[0] = pts[j-2]; idx[1] = pts[j-1]; idx[2] = pts[j]; 
        vtkTriangle::ComputeNormal(p, 3, idx, polyNorm);
        }
      glNormal3dv(polyNorm);
      glVertex3dv(p->GetPoint(pts[j]));
      }
    glEnd();
    
    // draw second line
    glBegin(GL_LINE_STRIP);
    for (j = 1; j < npts; j += 2) 
      {
      if (ColorTexturingAllowed)
        glTexCoord1f(vtk1Over255[c->GetPointer(4*pts[j])[0]]);
      else
        glColor4ubv(c->GetPointer(4*pts[j]));
      if (j == 1)
        {
        vtkTriangle::ComputeNormal(p, 3, pts, polyNorm);
        }
      else
        {
        idx[0] = pts[j-2]; idx[1] = pts[j]; idx[2] = pts[j-1]; 
        vtkTriangle::ComputeNormal(p, 3, idx, polyNorm);
        }
      glNormal3dv(polyNorm);
      glVertex3dv(p->GetPoint(pts[j]));
      }
    glEnd();
    
    // check for abort condition
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    }
}

// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Added new arguments for display lists.  DID NOT MAKE USE OF.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument.
//
static void vtkOpenGLDrawNSW(vtkCellArray *aPrim, GLenum,
                             vtkIdType &, vtkPoints *p, vtkDataArray *n, 
                             vtkUnsignedCharArray *c, vtkDataArray *, 
                             vtkOpenGLRenderer *ren, int &noAbort,
                             bool doingDisplayLists,int &primsInCurrentList,
                             int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  vtkIdType *pts = 0;
  vtkIdType npts = 0;
  int count = 0;
  
  for (aPrim->InitTraversal(); noAbort && aPrim->GetNextCell(npts,pts); 
       count++)
    {
    if (ColorTexturingAllowed)
      {
      // draw first line
      glBegin(GL_LINE_STRIP);
      for (j = 0; j < npts; j += 2) 
        {
        glTexCoord1f(vtk1Over255[c->GetPointer(4*pts[j])[0]]);
        glNormal3dv(n->GetTuple(pts[j]));
        glVertex3dv(p->GetPoint(pts[j]));
        }
      glEnd();
    
      // draw second line
      glBegin(GL_LINE_STRIP);
      for (j = 1; j < npts; j += 2) 
        {
        glTexCoord1f(vtk1Over255[c->GetPointer(4*pts[j])[0]]);
        glNormal3dv(n->GetTuple(pts[j]));
        glVertex3dv(p->GetPoint(pts[j]));
        }
      glEnd();
      }
    else
      {
      // draw first line
      glBegin(GL_LINE_STRIP);
      for (j = 0; j < npts; j += 2) 
        {
        glColor4ubv(c->GetPointer(4*pts[j]));
        glNormal3dv(n->GetTuple(pts[j]));
        glVertex3dv(p->GetPoint(pts[j]));
        }
      glEnd();
    
      // draw second line
      glBegin(GL_LINE_STRIP);
      for (j = 1; j < npts; j += 2) 
        {
        glColor4ubv(c->GetPointer(4*pts[j]));
        glNormal3dv(n->GetTuple(pts[j]));
        glVertex3dv(p->GetPoint(pts[j]));
        }
      glEnd();
      }
    
    // check for abort condition
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    }
}

// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Added new arguments for display lists.  DID NOT MAKE USE OF.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument -- NOT USED HERE.
//
static void vtkOpenGLDrawTW(vtkCellArray *aPrim, GLenum,
                            vtkIdType &, vtkPoints *p, vtkDataArray *, 
                            vtkUnsignedCharArray *, vtkDataArray *t, 
                            vtkOpenGLRenderer *ren, int &noAbort,
                            bool doingDisplayLists,int &primsInCurrentList,
                            int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  vtkIdType *pts = 0;
  vtkIdType idx[3];
  vtkIdType npts = 0;
  int count = 0;
  double polyNorm[3];
  
  for (aPrim->InitTraversal(); noAbort && aPrim->GetNextCell(npts,pts); 
       count++)
    { 
    // draw first line
    glBegin(GL_LINE_STRIP);
    for (j = 0; j < npts; j += 2) 
      {
      if ( j == 0 )
        {
        vtkTriangle::ComputeNormal(p, 3, pts, polyNorm);
        }
      else
        {
        idx[0] = pts[j-2]; idx[1] = pts[j-1]; idx[2] = pts[j]; 
        vtkTriangle::ComputeNormal(p, 3, idx, polyNorm);
        }
      glNormal3dv(polyNorm);
      glTexCoord2dv(t->GetTuple(pts[j]));
      glVertex3dv(p->GetPoint(pts[j]));
      }
    glEnd();
    
    // draw second line
    glBegin(GL_LINE_STRIP);
    for (j = 1; j < npts; j += 2) 
      {
      if (j == 1)
        {
        vtkTriangle::ComputeNormal(p, 3, pts, polyNorm);
        }
      else
        {
        idx[0] = pts[j-2]; idx[1] = pts[j]; idx[2] = pts[j-1]; 
        vtkTriangle::ComputeNormal(p, 3, idx, polyNorm);
        }
      glNormal3dv(polyNorm);
      glTexCoord2dv(t->GetTuple(pts[j]));
      glVertex3dv(p->GetPoint(pts[j]));
      }
    glEnd();
    
    // check for abort condition
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    }
}

// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Added new arguments for display lists.  DID NOT MAKE USE OF.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument -- NOT USED HERE.
//
static void vtkOpenGLDrawNTW(vtkCellArray *aPrim, GLenum,
                             vtkIdType &, vtkPoints *p, vtkDataArray *n, 
                             vtkUnsignedCharArray *, vtkDataArray *t, 
                             vtkOpenGLRenderer *ren, int &noAbort,
                             bool doingDisplayLists,int &primsInCurrentList,
                             int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  vtkIdType *pts = 0;
  vtkIdType npts = 0;
  int count = 0;
  
  for (aPrim->InitTraversal(); noAbort && aPrim->GetNextCell(npts,pts); 
       count++)
    { 
    glBegin(GL_LINE_STRIP);
    for (j = 0; j < npts; j += 2) 
      {
      glNormal3dv(n->GetTuple(pts[j]));
      glTexCoord2dv(t->GetTuple(pts[j]));
      glVertex3dv(p->GetPoint(pts[j]));
      }
    glEnd();
    
    // draw second line
    glBegin(GL_LINE_STRIP);
    for (j = 1; j < npts; j += 2) 
      {
      glNormal3dv(n->GetTuple(pts[j]));
      glTexCoord2dv(t->GetTuple(pts[j]));
      glVertex3dv(p->GetPoint(pts[j]));
      }
    glEnd();
    
    // check for abort condition
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    }
}

// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Added new arguments for display lists.  DID NOT MAKE USE OF.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument.
//
static void vtkOpenGLDrawSTW(vtkCellArray *aPrim, GLenum,
                             vtkIdType &, vtkPoints *p, vtkDataArray *, 
                             vtkUnsignedCharArray *c, vtkDataArray *t, 
                             vtkOpenGLRenderer *ren, int &noAbort,
                             bool doingDisplayLists,int &primsInCurrentList,
                             int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  vtkIdType *pts = 0;
  vtkIdType idx[3];
  vtkIdType npts = 0;
  int count = 0;
  double polyNorm[3];
  
  for (aPrim->InitTraversal(); noAbort && aPrim->GetNextCell(npts,pts); 
       count++)
    { 
    // draw first line
    glBegin(GL_LINE_STRIP);
    for (j = 0; j < npts; j += 2) 
      {
      if (ColorTexturingAllowed)
        glTexCoord1f(vtk1Over255[c->GetPointer(4*pts[j])[0]]);
      else
        glColor4ubv(c->GetPointer(4*pts[j]));
      if ( j == 0 )
        {
        vtkTriangle::ComputeNormal(p, 3, pts, polyNorm);
        }
      else
        {
        idx[0] = pts[j-2]; idx[1] = pts[j-1]; idx[2] = pts[j]; 
        vtkTriangle::ComputeNormal(p, 3, idx, polyNorm);
        }
      glNormal3dv(polyNorm);
      if (!ColorTexturingAllowed)
         glTexCoord2dv(t->GetTuple(pts[j]));
      glVertex3dv(p->GetPoint(pts[j]));
      }
    glEnd();
      
    // draw second line
    glBegin(GL_LINE_STRIP);
    for (j = 1; j < npts; j += 2) 
      {
      if (ColorTexturingAllowed)
        glTexCoord1f(vtk1Over255[c->GetPointer(4*pts[j])[0]]);
      else
        glColor4ubv(c->GetPointer(4*pts[j]));
      if (j == 1)
        {
        vtkTriangle::ComputeNormal(p, 3, pts, polyNorm);
        }
      else
        {
        idx[0] = pts[j-2]; idx[1] = pts[j]; idx[2] = pts[j-1]; 
        vtkTriangle::ComputeNormal(p, 3, idx, polyNorm);
        }
      glNormal3dv(polyNorm);
      if (!ColorTexturingAllowed)
        glTexCoord2dv(t->GetTuple(pts[j]));
      glVertex3dv(p->GetPoint(pts[j]));
      }
    glEnd();
    
    // check for abort condition
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    }
}

// Modifications:
//
//   Hank Childs, Tue May 25 10:51:06 PDT 2004
//   Added new arguments for display lists.  DID NOT MAKE USE OF.
//
//   Brad Whitlock, Wed Aug 23 11:03:01 PDT 2006
//   Added ColorTexturingAllowed argument.
//
static void vtkOpenGLDrawNSTW(vtkCellArray *aPrim, GLenum,
                              vtkIdType &, vtkPoints *p, vtkDataArray *n, 
                              vtkUnsignedCharArray *c, vtkDataArray *t, 
                              vtkOpenGLRenderer *ren, int &noAbort,
                              bool doingDisplayLists,int &primsInCurrentList,
                              int &CurrentList, bool ColorTexturingAllowed)
{
  int j;
  vtkIdType *pts = 0;
  vtkIdType npts = 0;
  int count = 0;
  
  for (aPrim->InitTraversal(); noAbort && aPrim->GetNextCell(npts,pts); 
       count++)
    { 
    if (ColorTexturingAllowed)
      {
      // draw first line
      glBegin(GL_LINE_STRIP);
      for (j = 0; j < npts; j += 2) 
        {
        glTexCoord1f(vtk1Over255[c->GetPointer(4*pts[j])[0]]);
        glNormal3dv(n->GetTuple(pts[j]));
        glVertex3dv(p->GetPoint(pts[j]));
        }
      glEnd();
    
      // draw second line
      glBegin(GL_LINE_STRIP);
      for (j = 1; j < npts; j += 2) 
        {
        glTexCoord1f(vtk1Over255[c->GetPointer(4*pts[j])[0]]);
        glNormal3dv(n->GetTuple(pts[j]));
        glVertex3dv(p->GetPoint(pts[j]));
        }
      glEnd();
      }
    else
      {
      // draw first line
      glBegin(GL_LINE_STRIP);
      for (j = 0; j < npts; j += 2) 
        {
        glColor4ubv(c->GetPointer(4*pts[j]));
        glNormal3dv(n->GetTuple(pts[j]));
        glTexCoord2dv(t->GetTuple(pts[j]));
        glVertex3dv(p->GetPoint(pts[j]));
        }
      glEnd();
    
      // draw second line
      glBegin(GL_LINE_STRIP);
      for (j = 1; j < npts; j += 2) 
        {
        glColor4ubv(c->GetPointer(4*pts[j]));
        glNormal3dv(n->GetTuple(pts[j]));
        glTexCoord2dv(t->GetTuple(pts[j]));
        glVertex3dv(p->GetPoint(pts[j]));
        }
      glEnd();
      }
  
    // check for abort condition
    if (count == 100)
      {
      count = 0;
      if (ren->GetRenderWindow()->CheckAbortStatus())
        {
        noAbort = 0;
        }
      }
    }
}

// ****************************************************************************
//  Method: vtkVisItOpenGLPolyDataMapper::Draw
//
//  Purpose:
//     Draw method for OpenGL.
//
//  Modifications:
//
//     Hank Childs, Tue May 25 09:05:02 PDT 2004
//     Use flat shading when possible as a Mesa optimization.  Also break
//     one big display list into groups of smaller display lists, since that
//     is a big performance win with graphics hardware.
//
//     Hank Childs, Wed Jun  2 07:31:33 PDT 2004
//     Put in a check to make sure that the normals array is of proper size.
//
//     Brad Whitlock, Thu Aug 25 14:57:38 PST 2005
//     Added code to set up point texturing.
//
// ****************************************************************************

int vtkVisItOpenGLPolyDataMapper::Draw(vtkRenderer *aren, vtkActor *act)
{
  vtkOpenGLRenderer *ren = (vtkOpenGLRenderer *)aren;
  int rep, interpolation;
  float tran;
  GLenum glFunction[4], aGlFunction;
  vtkProperty *prop;
  vtkPoints *p;
  vtkCellArray *prims[4], *aPrim;
  vtkUnsignedCharArray *c=NULL;
  vtkDataArray *n;
  vtkDataArray *t;
  int tDim;
  int noAbort = 1;
  vtkPolyData *input = this->GetInput();
  int cellScalars = 0;
  vtkIdType cellNum = 0;
  int cellNormals = 0;
  int resolve=0, zResolve=0;
  double zRes = 0.0;
  
  // Do our glGet calls before we start the display list, since they cannot
  // be issued from within a display list.
  int  lastShadeModel = -1;
  glGetIntegerv(GL_SHADE_MODEL, &lastShadeModel);

  if (this->doingDisplayLists)
    {
    glNewList(this->CurrentList,GL_COMPILE);
    this->primsInCurrentList = 0;
    }

  // get the property 
  prop = act->GetProperty();

  // get the transparency 
  tran = prop->GetOpacity();
  
  // if the primitives are invisable then get out of here 
  if (tran <= 0.0)
    {
    return noAbort;
    }

  // get the representation (e.g., surface / wireframe / points)
  rep = prop->GetRepresentation();

  switch (rep) 
    {
    case VTK_POINTS:
      glFunction[0]  = GL_POINTS;
      glFunction[1]  = GL_POINTS;
      glFunction[2]  = GL_POINTS;
      glFunction[3]  = GL_POINTS;
      break;
    case VTK_WIREFRAME:
      glFunction[0] = GL_POINTS;
      glFunction[1] = GL_LINE_STRIP;
      glFunction[2] = GL_LINE_STRIP;
      glFunction[3] = GL_LINE_LOOP;
      break;
    case VTK_SURFACE:
      glFunction[0] = GL_POINTS;
      glFunction[1] = GL_LINE_STRIP;
      glFunction[2] = GL_TRIANGLE_STRIP;
      glFunction[3] = GL_POLYGON;
      break;
    default: 
      vtkErrorMacro(<< "Bad representation sent\n");
      glFunction[0] = GL_POINTS;
      glFunction[1] = GL_LINE_STRIP;
      glFunction[2] = GL_TRIANGLE_STRIP;
      glFunction[3] = GL_POLYGON;
      break;
    }

  // get the shading interpolation 
  interpolation = prop->GetInterpolation();

  // and draw the display list
  p = input->GetPoints();
  
  // are they cell or point scalars
  if ( this->Colors )
    {
    c = this->Colors;
    if ( (this->ScalarMode == VTK_SCALAR_MODE_USE_CELL_DATA ||
          this->ScalarMode == VTK_SCALAR_MODE_USE_CELL_FIELD_DATA ||
          !input->GetPointData()->GetScalars() )
         && this->ScalarMode != VTK_SCALAR_MODE_USE_POINT_FIELD_DATA)
      {
      cellScalars = 1;
      }
    }
    
  prims[0] = input->GetVerts();
  prims[1] = input->GetLines();
  prims[2] = input->GetPolys();
  prims[3] = input->GetStrips();

  t = input->GetPointData()->GetTCoords();
  if ( t ) 
    {
    tDim = t->GetNumberOfComponents();
    if (tDim != 2)
      {
      vtkDebugMacro(<< "Currently only 2d textures are supported.\n");
      t = NULL;
      }
    }

  n = input->GetPointData()->GetNormals();

  // Make sure we have a normal for every point.
  if (n != NULL)
    {
    if (n->GetNumberOfTuples() != input->GetNumberOfPoints())
        n = NULL;
    }
  if (interpolation == VTK_FLAT)
    {
    n = NULL;
    }
  
  cellNormals = 0;
  if (input->GetCellData()->GetNormals())
    {
    cellNormals = 1;
    n = input->GetCellData()->GetNormals();

    // Make sure we have a normal for every cell.
    if (n->GetNumberOfTuples() != input->GetNumberOfCells())
      {
      n = NULL;
      cellNormals = 0;
      }
    }
  
  // if we are doing vertex colors then set lmcolor to adjust 
  // the current materials ambient and diffuse values using   
  // vertex color commands otherwise tell it not to.          
  glDisable( GL_COLOR_MATERIAL );
  if (c)
    {
    GLenum lmcolorMode;
    if (this->ScalarMaterialMode == VTK_MATERIALMODE_DEFAULT)
      {
      if (prop->GetAmbient() > prop->GetDiffuse())
        {
        lmcolorMode = GL_AMBIENT;
        }
      else
        {
        lmcolorMode = GL_DIFFUSE;
        }
      }
    else if (this->ScalarMaterialMode == VTK_MATERIALMODE_AMBIENT_AND_DIFFUSE)
      {
      lmcolorMode = GL_AMBIENT_AND_DIFFUSE;
      }
    else if (this->ScalarMaterialMode == VTK_MATERIALMODE_AMBIENT)
      {
      lmcolorMode = GL_AMBIENT;
      }
    else // if (this->ScalarMaterialMode == VTK_MATERIALMODE_DIFFUSE)
      {
      lmcolorMode = GL_DIFFUSE;
      } 
    glColorMaterial( GL_FRONT_AND_BACK, lmcolorMode);
    glEnable( GL_COLOR_MATERIAL );
    }

 
  // how do we draw points
  void (*draw0)(vtkCellArray *, GLenum, vtkIdType &, vtkPoints *, 
                vtkDataArray *,
                vtkUnsignedCharArray *, vtkDataArray *, vtkOpenGLRenderer *, 
                int &, bool, int &, int &, bool);

  int idx;
  if (n && !cellNormals)
    {
    idx = 1;
    }
  else
    {
    idx = 0;
    }
  if (c)
    {
    idx += 2;
    }
  if (t)
    {
    idx += 4;
    }
  if (cellScalars)
    {
    idx += 8;
    }
  if (cellNormals)
    {
    idx += 16;
    }

  bool havePointNormals = n && !cellNormals;
  bool havePointColors  = c && !cellScalars;
  bool canDoFlat = !havePointNormals && !havePointColors;
  bool switchedShadingModel = false;
  if (canDoFlat)
  {
    if (lastShadeModel != GL_FLAT)
    {
      glShadeModel(GL_FLAT);
      switchedShadingModel = true;
    }
  }

  switch (idx) 
    {
    case 0: draw0 = vtkOpenGLDraw01; break;
    case 1: draw0 = vtkOpenGLDrawN013; break;
    case 2: draw0 = vtkOpenGLDrawS01; break;
    case 3: draw0 = vtkOpenGLDrawNS013; break;
    case 4: draw0 = vtkOpenGLDrawT01; break;
    case 5: draw0 = vtkOpenGLDrawNT013; break;
    case 6: draw0 = vtkOpenGLDrawST01; break;
    case 7: draw0 = vtkOpenGLDrawNST013; break;
    case 10: draw0 = vtkOpenGLDrawCS01; break;
    case 11: draw0 = vtkOpenGLDrawNCS013; break;
    case 14: draw0 = vtkOpenGLDrawCST01; break;
    case 15: draw0 = vtkOpenGLDrawNCST013; break;

    case 16: draw0 = vtkOpenGLDrawCN013; break;
    case 18: draw0 = vtkOpenGLDrawCNS013; break;
    case 20: draw0 = vtkOpenGLDrawCNT013; break;
    case 22: draw0 = vtkOpenGLDrawCNST013; break;
    case 26: draw0 = vtkOpenGLDrawCNCS013; break;
    case 30: draw0 = vtkOpenGLDrawCNCST013; break;
    default:
      // This case should never be reached.
      return noAbort;
    }

  // how do we draw lines
  void (*draw1)(vtkCellArray *, GLenum, vtkIdType &, vtkPoints *, vtkDataArray *,
                vtkUnsignedCharArray *, vtkDataArray *, vtkOpenGLRenderer *, 
                int &, bool, int &, int &, bool);
  switch (idx) 
    {
    case 0: draw1 = vtkOpenGLDraw01; break;
    case 1: draw1 = vtkOpenGLDrawN013; break;
    case 2: draw1 = vtkOpenGLDrawS01; break;
    case 3: draw1 = vtkOpenGLDrawNS013; break;
    case 4: draw1 = vtkOpenGLDrawT01; break;
    case 5: draw1 = vtkOpenGLDrawNT013; break;
    case 6: draw1 = vtkOpenGLDrawST01; break;
    case 7: draw1 = vtkOpenGLDrawNST013; break;
    case 10: draw1 = vtkOpenGLDrawCS01; break;
    case 11: draw1 = vtkOpenGLDrawNCS013; break;
    case 14: draw1 = vtkOpenGLDrawCST01; break;
    case 15: draw1 = vtkOpenGLDrawNCST013; break;
    case 16: draw1 = vtkOpenGLDrawCN013; break;
    case 18: draw1 = vtkOpenGLDrawCNS013; break;
    case 20: draw1 = vtkOpenGLDrawCNT013; break;
    case 22: draw1 = vtkOpenGLDrawCNST013; break;
    case 26: draw1 = vtkOpenGLDrawCNCS013; break;
    case 30: draw1 = vtkOpenGLDrawCNCST013; break;
    default:
      // This case should never be reached.
      return noAbort;
    }

  // how do we draw tstrips
  void (*draw2)(vtkCellArray *, GLenum, vtkIdType &, vtkPoints *, 
                vtkDataArray *,
                vtkUnsignedCharArray *, vtkDataArray *, vtkOpenGLRenderer *, 
                int &, bool, int &, int &, bool);
  void (*draw2W)(vtkCellArray *, GLenum, vtkIdType &, vtkPoints *,
                 vtkDataArray *, vtkUnsignedCharArray *, vtkDataArray *, 
                 vtkOpenGLRenderer *, int &, bool, int &, int &, bool);
  switch (idx) 
    {
    case 0: draw2 = vtkOpenGLDraw2; break;
    case 1: draw2 = vtkOpenGLDrawN013; break;
    case 2: draw2 = vtkOpenGLDrawS2; break;
    case 3: draw2 = vtkOpenGLDrawNS013; break;
    case 4: draw2 = vtkOpenGLDrawT2; break;
    case 5: draw2 = vtkOpenGLDrawNT013; break;
    case 6: draw2 = vtkOpenGLDrawST2; break;
    case 7: draw2 = vtkOpenGLDrawNST013; break;
    case 10: draw2 = vtkOpenGLDrawCS2; break;
    case 11: draw2 = vtkOpenGLDrawNCS013; break;
    case 14: draw2 = vtkOpenGLDrawCST2; break;
    case 15: draw2 = vtkOpenGLDrawNCST013; break;
    case 16: draw2 = vtkOpenGLDraw2; break;
    case 18: draw2 = vtkOpenGLDrawS2; break;
    case 20: draw2 = vtkOpenGLDrawT2; break;
    case 22: draw2 = vtkOpenGLDrawST2; break;
    case 26: draw2 = vtkOpenGLDrawCS2; break;
    case 30: draw2 = vtkOpenGLDrawCST2; break;
    default:
      // This case should never be reached.
      return noAbort;
    }
  switch (idx)
    {
    case 0: draw2W = vtkOpenGLDrawW; break;
    case 1: draw2W = vtkOpenGLDrawNW; break;
    case 2: draw2W = vtkOpenGLDrawSW; break;
    case 3: draw2W = vtkOpenGLDrawNSW; break;
    case 4: draw2W = vtkOpenGLDrawTW; break;
    case 5: draw2W = vtkOpenGLDrawNTW; break;
    case 6: draw2W = vtkOpenGLDrawSTW; break;
    case 7: draw2W = vtkOpenGLDrawNSTW; break;
    case 10: draw2W = vtkOpenGLDrawW; break;
    case 11: draw2W = vtkOpenGLDrawNW; break;
    case 14: draw2W = vtkOpenGLDrawTW; break;
    case 15: draw2W = vtkOpenGLDrawNTW; break;
    case 16: draw2W = vtkOpenGLDrawW; break;
    case 18: draw2W = vtkOpenGLDrawSW; break;
    case 20: draw2W = vtkOpenGLDrawTW; break;
    case 22: draw2W = vtkOpenGLDrawSTW; break;
    case 26: draw2W = vtkOpenGLDrawW; break;
    case 30: draw2W = vtkOpenGLDrawTW; break;
    default:
      // This case should never be reached.
      return noAbort;
    }
  
  // how do we draw polys
  void (*draw3)(vtkCellArray *, GLenum, vtkIdType &, vtkPoints *, vtkDataArray *,
                vtkUnsignedCharArray *, vtkDataArray *, 
                vtkOpenGLRenderer *, int &, bool, int &, int &, bool);
  switch (idx) 
    {
    case 0: draw3 = vtkOpenGLDraw3; break;
    case 1: draw3 = vtkOpenGLDrawN013; break;
    case 2: draw3 = vtkOpenGLDrawS3; break;
    case 3: draw3 = vtkOpenGLDrawNS013; break;
    case 4: draw3 = vtkOpenGLDrawT3; break;
    case 5: draw3 = vtkOpenGLDrawNT013; break;
    case 6: draw3 = vtkOpenGLDrawST3; break;
    case 7: draw3 = vtkOpenGLDrawNST013; break;
    case 10: draw3 = vtkOpenGLDrawCS3; break;
    case 11: draw3 = vtkOpenGLDrawNCS013; break;
    case 14: draw3 = vtkOpenGLDrawCST3; break;
    case 15: draw3 = vtkOpenGLDrawNCST013; break;
    case 16: draw3 = vtkOpenGLDrawCN013; break;
    case 18: draw3 = vtkOpenGLDrawCNS013; break;
    case 20: draw3 = vtkOpenGLDrawCNT013; break;
    case 22: draw3 = vtkOpenGLDrawCNST013; break;
    case 26: draw3 = vtkOpenGLDrawCNCS013; break;
    case 30: draw3 = vtkOpenGLDrawCNCST013; break;
    default:
      // This case should never be reached.
      return noAbort;
    }

  if ( this->GetResolveCoincidentTopology() )
    {
    resolve = 1;
    if ( this->GetResolveCoincidentTopology() == VTK_RESOLVE_SHIFT_ZBUFFER )
      {
      zResolve = 1;
      zRes = this->GetResolveCoincidentTopologyZShift();
      }
    else
      {
#ifdef GL_VERSION_1_1
      double f, u;
      glEnable(GL_POLYGON_OFFSET_FILL);
      this->GetResolveCoincidentTopologyPolygonOffsetParameters(f,u);
      glPolygonOffset(f,u);
#endif      
      }
    }

  // do verts
  aPrim = prims[0];
  aGlFunction = glFunction[0];

  // For verts or lines that have no normals, disable shading.
  // This will fall back on the color set in the glColor4fv() 
  // call in vtkOpenGLProperty::Render() - the color returned
  // by vtkProperty::GetColor() with alpha set to 1.0.
  glDisable( GL_LIGHTING);

  TextureState atts;
  this->StartFancyPoints(atts);

  // draw all the elements
  draw0(aPrim, aGlFunction, cellNum, p, n, c, t, ren, noAbort,
        this->doingDisplayLists, this->primsInCurrentList, this->CurrentList,
        this->ColorTexturingAllowed);

  this->EndFancyPoints(atts);
  
  // do lines
  if ( zResolve )
    {
    glDepthRange(zRes, 1.);
    }
  aPrim = prims[1];
  aGlFunction = glFunction[1];
  
  // draw all the elements
  draw1(aPrim, aGlFunction, cellNum, p, n, c, t, ren, noAbort,
        this->doingDisplayLists, this->primsInCurrentList, this->CurrentList,
        this->ColorTexturingAllowed);
  
  // turn the lighting on if we have normals.
  if (n)
    {
    glEnable( GL_LIGHTING);
    }
  
  // do polys
  aPrim = prims[2];
  aGlFunction = glFunction[3];

  draw3(aPrim, aGlFunction, cellNum, p, n, c, t, ren, noAbort,
        this->doingDisplayLists, this->primsInCurrentList, this->CurrentList,
        this->ColorTexturingAllowed);


  // do tstrips
  if ( zResolve )
    {
    glDepthRange(2*zRes, 1.);
    }
  aPrim = prims[3];
  aGlFunction = glFunction[2];
  draw2(aPrim, aGlFunction, cellNum, p, n, c, t, ren, noAbort,
        this->doingDisplayLists, this->primsInCurrentList, this->CurrentList,
        this->ColorTexturingAllowed);
  if (rep == VTK_WIREFRAME)   
    {
    draw2W(aPrim, aGlFunction, cellNum, p, n, c, t, ren, noAbort,
        this->doingDisplayLists, this->primsInCurrentList, this->CurrentList,
        this->ColorTexturingAllowed);
    }

  // enable lighting again if necessary
  //if (!n && rep == VTK_POINTS)
  if (!n)
    {
    glEnable( GL_LIGHTING);
    }

  if (switchedShadingModel)
    {
    glShadeModel(lastShadeModel);
    }

  if (resolve)
    {
    if ( zResolve )
      {
      glDepthRange(0., 1.);
      }
    else
      {
#ifdef GL_VERSION_1_1
      glDisable(GL_POLYGON_OFFSET_FILL);
#endif
      }
    }

  if (this->doingDisplayLists)
    glEndList();

  return noAbort;
}

void vtkVisItOpenGLPolyDataMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

// ****************************************************************************
// Method: vtkVisItOpenGLPolyDataMapper::StartFancyPoints
//
// Purpose: 
//   Sets up point texturing.
//
// Arguments:
//   atts : The GL state before setting up point texturing.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 25 14:58:09 PST 2005
//
// Modifications:
//   Brad Whitlock, Thu Nov 3 13:21:08 PST 2005
//   Added conditional compilation.
//
//   Brad Whitlock, Tue Dec 6 13:37:58 PST 2005
//   Changed to 1-pass texturing.
//
// ****************************************************************************

void
vtkVisItOpenGLPolyDataMapper::StartFancyPoints(
    vtkVisItOpenGLPolyDataMapper::TextureState &atts)
{
    if(this->PointTextureMethod == TEXTURE_USING_POINTSPRITES)
    {
#ifndef vtkVisItOpenGLPolyDataMapper
        // If we're in OpenGL and not Mesa then do this test.
        if(this->PointSpriteSupported == -1)
        {
            const char *ext = (const char *)glGetString(GL_EXTENSIONS);
            if(ext != 0 && strstr(ext, "GL_ARB_point_sprite") != 0)
                this->PointSpriteSupported = 1;
            else
            {
                if(LastWindow != 0)
                {
                    // If the window is direct then say that the extension is
                    // not supported since it should have been in the list
                    // of extensions. If the display is not direct then the
                    // list of extensions is unreliable and we should just 
                    // try and use the extension.
                    vtkRenderWindow *renWin = vtkRenderWindow::
                        SafeDownCast(LastWindow);
                    if(renWin != 0)
                        this->PointSpriteSupported = renWin->IsDirect() ? 0 : 1;
                    else
                        this->PointSpriteSupported = 0;
                }
                else
                    this->PointSpriteSupported = 0;
            }
        }

        if(this->PointSpriteSupported < 1)
        {
            // Point sprites are not supported
            return;
        }
#endif

        // Create the rextures
        if(!this->SphereTexturesDataCreated)
        {
            this->MakeTextures();
            this->SphereTexturesDataCreated = true;
        }

        // Create and bind the textures if we have not done that yet.
        if(!this->SphereTexturesLoaded)
        {
            glGenTextures(1, &this->TextureName);

            // Set up the first texture
            glBindTexture(GL_TEXTURE_2D, this->TextureName);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, 2, SPHERE_TEX_W, SPHERE_TEX_H,
                         0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, this->SphereTexture);

            this->SphereTexturesLoaded = true;
        }

        //
        // Get whether GL_BLEND is enabled.
        //
        glGetIntegerv(GL_BLEND, &atts.isBlendEnabled);
        if(atts.isBlendEnabled == 0)
            glEnable(GL_BLEND);

        glGetIntegerv(GL_BLEND_SRC, &atts.blendFunc0);
        glGetIntegerv(GL_BLEND_DST, &atts.blendFunc1);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        //
        // Get the AlphaTest mode to restore it later.
        //
        int dt = 0;
        glGetIntegerv(GL_DEPTH_TEST, &dt);
        if(dt == 1)
        {
            // Get the current alpha test function
            glGetIntegerv(GL_ALPHA_TEST, &atts.isAlphaTestEnabled);
            if(atts.isAlphaTestEnabled)
            {
                glGetIntegerv(GL_ALPHA_TEST_FUNC, &atts.alphaTestFunc);
                glGetFloatv(GL_ALPHA_TEST_REF, &atts.alphaTestRef);
            }
            else
                glEnable(GL_ALPHA_TEST);

            // Set the alpha testing mode and function.
            //glAlphaFunc(GL_EQUAL, 1.);
            glAlphaFunc(GL_GREATER, 0.7);

            atts.needAlphaTest = 1;
        }
        else
            atts.needAlphaTest = 0;

        //
        // Turn on the point sprite extension
        //
#define MY_POINT_SPRITE_ARB  0x8861
#define MY_COORD_REPLACE_ARB 0x8862
        glEnable(MY_POINT_SPRITE_ARB);

        //
        // Turn on the texture
        //
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, this->TextureName);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
        glTexEnvi(MY_POINT_SPRITE_ARB,
                  MY_COORD_REPLACE_ARB,
                  GL_TRUE);
    }
}

// ****************************************************************************
// Method: vtkVisItOpenGLPolyDataMapper::EndFancyPoints
//
// Purpose: 
//   Turns off point texturing.
//
// Arguments:
//   atts : The previous GL state to be restored.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 25 14:58:49 PST 2005
//
// Modifications:
//   Brad Whitlock, Tue Dec 6 13:39:33 PST 2005
//   I changed it to 1-pass texturing.
//   
// ****************************************************************************

void
vtkVisItOpenGLPolyDataMapper::EndFancyPoints(
    vtkVisItOpenGLPolyDataMapper::TextureState &atts)
{
    if(this->PointTextureMethod == TEXTURE_USING_POINTSPRITES)
    {
#ifndef vtkVisItOpenGLPolyDataMapper 
        // If we're in OpenGL and not Mesa then do this test.
        if(this->PointSpriteSupported < 1)
        {
            // Point sprites are not supported
            return; 
        }
#endif
        if(atts.needAlphaTest)
        {
            if(atts.isAlphaTestEnabled)
                glAlphaFunc(atts.alphaTestFunc, atts.alphaTestRef);
            else
                glDisable(GL_ALPHA_TEST);
        }

        if(atts.isBlendEnabled == 0)
            glDisable(GL_BLEND);

        // Restore the old blend function.
        glBlendFunc(atts.blendFunc0, atts.blendFunc1);

        glDisable(GL_TEXTURE_2D);

        glDisable(MY_POINT_SPRITE_ARB);
    }
}

// ****************************************************************************
// Method: vtkVisItOpenGLPolyDataMapper::MakeTextures
//
// Purpose: 
//   Calculates a simple, transparent shaded sphere texture and a mask texture.
//
// Note:       We're not using a compiled-in data array in case we later want
//             to do lighting, etc to make the texture better reflect the
//             environment.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 25 14:59:22 PST 2005
//
// Modifications:
//   Brad Whitlock, Mon Aug 29 10:04:32 PDT 2005
//   Fixed it so the first texture's alpha is based on the Z value as computed
//   for a sphere. The last equation was a kludge.
//
//   Brad Whitlock, Tue Dec 6 13:41:18 PST 2005
//   I changed it to 1-pass texturing.
//
//   Hank Childs, Fri Jun  9 13:13:20 PDT 2006
//   Remove unused variable.
//
// ****************************************************************************

void
vtkVisItOpenGLPolyDataMapper::MakeTextures()
{
   int i, j;

   /* float dx = SPHERE_TEX_H * 0.5f; */
   /* float dy = SPHERE_TEX_H * 0.5f; */
   /* float R = dx-2; */

   /* float minT = 0.; */
   /* float maxT = 0.; */

   GLubyte texture[SPHERE_TEX_H][SPHERE_TEX_H][2];

   for (j = 0; j < SPHERE_TEX_H; j++)
   {
      float y = (float(j) / float(SPHERE_TEX_H-1)) * 2. - 1.;
      for (i = 0; i < SPHERE_TEX_W; i++)
      {
         float x = (float(i) / float(SPHERE_TEX_W-1)) * 2. - 1.;
         float x2y2 = sqrt(x*x + y*y);
         if(x2y2 < 1.)
         {
             float z = sqrt(1. - x2y2);
             GLubyte rc = (GLubyte)(z * 255.);

             texture[j][i][0] = (GLubyte) 255 - rc;
             texture[j][i][1] = (GLubyte) 255;
         }
         else
         {
             texture[j][i][0] = (GLubyte) 0;
             texture[j][i][1] = (GLubyte) 0;
         }
      }
   }

   //
   // Blur the texture a little
   //
   float kernel[3][3] = {
       {0.125, 0.2, 0.125},
       {0.200, 1.0, 0.200},
       {0.125, 0.2, 0.125}};
   float kernelSum = kernel[0][0] + kernel[0][1] + kernel[0][2] + 
                     kernel[1][0] + kernel[1][1] + kernel[1][2] + 
                     kernel[2][0] + kernel[2][1] + kernel[2][2];
   for (j = 0; j < SPHERE_TEX_H; j++)
   {
       for (i = 0; i < SPHERE_TEX_W; i++)
       {
           if(i >= 1 && i < SPHERE_TEX_W-1 &&
              j >= 1 && j < SPHERE_TEX_H-1)
           {
               for(int c = 0; c < 2; ++c)
               {
                   float t = kernel[0][0] * float(texture[j-1][i-1][c]) + 
                             kernel[0][1] * float(texture[j-1][i][c]) + 
                             kernel[0][2] * float(texture[j-1][i+1][c]) + 
                             kernel[1][0] * float(texture[j][i-1][c]) + 
                             kernel[1][1] * float(texture[j][i][c]) + 
                             kernel[1][2] * float(texture[j][i+1][c]) + 
                             kernel[2][0] * float(texture[j+1][i-1][c]) + 
                             kernel[2][1] * float(texture[j+1][i][c]) + 
                             kernel[2][2] * float(texture[j+1][i+1][c]);
                   t /= kernelSum;
                   this->SphereTexture[j][i][c] = (GLubyte)t;
               }
           }
           else
           {
               this->SphereTexture[j][i][0] = (GLubyte)0;
               this->SphereTexture[j][i][1] = (GLubyte)0;
           }
       }
   }
}

// ****************************************************************************
// Method: vtkVisItOpenGLPolyDataMapper::UsesPointData
//
// Purpose: 
//   Follows the same rules as the GetScalars method except that it returns
//   true if the scalars are point data or false otherwise.
//
// Note:       We use this method to determine whether we have point data
//             so we can enable color texturing.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 24 15:54:00 PST 2006
//
// Modifications:
//   
// ****************************************************************************

bool 
vtkVisItOpenGLPolyDataMapper::UsesPointData(vtkDataSet *input, int scalarMode,
    int arrayAccessMode, int arrayId, const char *arrayName, int& offset)
{
  vtkDataArray *scalars=NULL;
  vtkPointData *pd;
  bool usesPointData = false;

  // make sure we have an input
  if ( !input )
    {
    return usesPointData;
    }
    
  // get and scalar data according to scalar mode
  if ( scalarMode == VTK_SCALAR_MODE_DEFAULT )
    {
    if (input->GetPointData()->GetScalars() != 0)
      {
      usesPointData = true;
      }
    }
  else if ( scalarMode == VTK_SCALAR_MODE_USE_POINT_DATA )
    {
    usesPointData = true;
    }
  else if ( scalarMode == VTK_SCALAR_MODE_USE_POINT_FIELD_DATA )
    {
    pd = input->GetPointData();
    if (arrayAccessMode == VTK_GET_ARRAY_BY_ID)
      {
      scalars = pd->GetArray(arrayId);
      }
    else
      {
      scalars = pd->GetArray(arrayName);
      }
    
    if ( scalars &&
         (offset < scalars->GetNumberOfComponents()) )
      {
          usesPointData = true;
      }
    }
  
  return usesPointData;
}

// ****************************************************************************
// Method: vtkVisItOpenGLPolyDataMapper::MapScalarsWithTextureSupport
//
// Purpose: 
//   This method calls MapScalars to use the lookup tables to set this->Colors.
//   Along the way, we also determine if we have point data and whether color
//   texturing should be used.
//
// Arguments:
//   opacity : The actor opacity.
//
// Returns:    True if we will use color texturing.
//
// Note:       This method calls MapScalars and performs the steps necessary
//             for setting up color texturing. Once it is determined that
//             we want to use color texturing, this method changes the colors
//             in the LUT so it will produce red values [0,255] to serve as
//             indices into the color texture. We also set up the array that
//             we use as the color texture array.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 24 15:55:26 PST 2006
//
// Modifications:
//   
// ****************************************************************************

bool
vtkVisItOpenGLPolyDataMapper::MapScalarsWithTextureSupport(double opacity)
{
    bool saveColors = this->EnableColorTexturing &&
                      this->LookupTable != NULL &&
                      this->LookupTable->IsA("vtkLookupTable");

    if(saveColors)
    {
        // Let's make sure that we have nodal data.
        if(!this->UsesPointData(
                this->GetInput(), this->ScalarMode, this->ArrayAccessMode,
                this->ArrayId, this->ArrayName, this->ArrayComponent))
        {
            saveColors = false;
        }

        // Don't allow color texturing if we're using point sprites
        // because we have to set the colors per point because
        // point sprites already use texturing.
        if(saveColors &&
           this->PointTextureMethod == TEXTURE_USING_POINTSPRITES)
        {
            saveColors = false;
        }
    }

    if(saveColors)
    {
        vtkLookupTable *LUT = (vtkLookupTable *)this->LookupTable;

        // Save the LUT's colors into a color texture.
        int NewColorTextureSize = LUT->GetNumberOfTableValues();
        int two_to_the_power = 2;
        for(int power = 1; power < 32; ++power)
        {
            if(two_to_the_power >= NewColorTextureSize)
            {
                NewColorTextureSize = two_to_the_power;
                break;
            }
            two_to_the_power <<= 1;
        }

        float *NewColorTexture = new float[NewColorTextureSize * 4];
        memset(NewColorTexture, 0, sizeof(float) * NewColorTextureSize);
        for(int i = 0; i < LUT->GetNumberOfTableValues(); ++i)
        {
            double *rgba = LUT->GetTableValue(i);
            NewColorTexture[4*i] = rgba[0];
            NewColorTexture[4*i+1] = rgba[1];
            NewColorTexture[4*i+2] = rgba[2];
            NewColorTexture[4*i+3] = rgba[3];
        }

        // Replace the current LUT's colors with black->white that we can use
        // as texture indices.
        for(int i = 0; i < LUT->GetNumberOfTableValues(); ++i)
        {
            double r,g,b,a;
            r = double(i) / double(LUT->GetNumberOfTableValues()-1);
            g = b = 0.;
            a = 1.;
            LUT->SetTableValue(i, r,g,b,a);
        }

        // Analyize the colors so we can make a guess as to whether the
        // colors in the LUT came from a continuous or discrete lookup
        // table. Just assume that the color table is discrete if the
        // colors are the same for 3 consecutive bins if we sample some
        // number of locations in the color table.
        int same_count = 0;
        for(int i = 0; i < 5; ++i)
        {
            float t = float(i) / float(LUT->GetNumberOfTableValues()-1);
            int index = int(t * (LUT->GetNumberOfTableValues() - 2));

            unsigned char c0[3], c1[3];
            c0[0] = (unsigned char)(255. * NewColorTexture[(index * 4) + 0]);
            c0[1] = (unsigned char)(255. * NewColorTexture[(index * 4) + 1]);
            c0[2] = (unsigned char)(255. * NewColorTexture[(index * 4) + 2]);

            c1[0] = (unsigned char)(255. * NewColorTexture[((index+1) * 4) + 0]);
            c1[1] = (unsigned char)(255. * NewColorTexture[((index+1) * 4) + 1]);
            c1[2] = (unsigned char)(255. * NewColorTexture[((index+1) * 4) + 2]);

            if(c0[0] == c1[0] && c0[1] == c1[1] && c0[2] == c1[2])
            {
                ++same_count;
            }
        } 
        this->ColorTextureLooksDiscrete = same_count >= 3;

        // Save the texture array.
        if(this->ColorTexture)
            delete [] this->ColorTexture;
        this->ColorTexture = NewColorTexture;
        this->ColorTextureSize = NewColorTextureSize;
    }
    
    // sets this->Colors as side effect
    this->MapScalars(opacity);
 
    if(saveColors)
    {
        // Restore the LUT's real colors.
        vtkLookupTable *LUT = (vtkLookupTable *)this->LookupTable;
        const float *rgba = this->ColorTexture;
        for(int i = 0; i < LUT->GetNumberOfTableValues(); ++i)
        {
            LUT->SetTableValue(i, rgba[4*i], rgba[4*i+1],
                                  rgba[4*i+2], rgba[4*i+3]);
        }

#ifdef GL_VERSION_1_2
        // Figure out the OpenGL version.
        const char *gl_ver = (const char *)glGetString(GL_VERSION);
        int major, minor;
        if(sscanf(gl_ver, "%d.%d", &major, &minor) == 2)
        {
            if(major == 1)
                this->OpenGLSupportsVersion1_2 = minor >= 2;
            else
                this->OpenGLSupportsVersion1_2 = major > 1;
        }
        else
        {
#endif
            this->OpenGLSupportsVersion1_2 = false;
#ifdef GL_VERSION_1_2
        }
#endif
    }

    return saveColors;
}

// ****************************************************************************
// Method: vtkVisItOpenGLPolyDataMapper::BeginColorTexturing
//
// Purpose: 
//   Begins color texturing if it is enabled.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 24 15:58:34 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
vtkVisItOpenGLPolyDataMapper::BeginColorTexturing()
{
    if(!this->ColorTexturingAllowed)
        return;

    if(!this->ColorTextureLoaded)
    {
        glGenTextures(1, &this->ColorTextureName);
        glBindTexture(GL_TEXTURE_1D, this->ColorTextureName);

#ifdef GL_VERSION_1_2
        // If we have OpenGL 1.2 then let's use clamp to edge so the colors
        // for the min/max texture values won't be blended with the border
        // color when we use GL_LINEAR.
        if(this->OpenGLSupportsVersion1_2)
        {
            glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        else
        {
#endif
            glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP);
#ifdef GL_VERSION_1_2
        }
#endif

        // Vary the filter based on what we think of the color table. Discrete
        // color tables won't have their colors blended.
        GLint m = this->ColorTextureLooksDiscrete ? GL_NEAREST : GL_LINEAR;
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, m);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, m);
        glTexImage1D(GL_TEXTURE_1D, 0, 4, this->ColorTextureSize,
                     0, GL_RGBA, GL_FLOAT, (void *)this->ColorTexture);
        this->ColorTextureLoaded = true;
    }

    if(this->ColorTextureLoaded)
    {
        // Turn on texturing.
        glEnable(GL_TEXTURE_1D);
        glBindTexture(GL_TEXTURE_1D, this->ColorTextureName);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }

    // Make the appropriate changes in the vtk1Over255 lookup table
    // so we can change the lookups so the first and last ones are not
    // used when we do GL_NEAREST. This may make the colors slightly off
    // but we won't get color values blended between the first and last
    // texture elements and the border color.
    if(!this->OpenGLSupportsVersion1_2)
    {
        if(this->ColorTextureLooksDiscrete)
        {
            vtk1Over255[0] = 0.f;
            vtk1Over255[255] = 1.f;
        }
        else
        {
            vtk1Over255[0] = 1. / 255.f;
            vtk1Over255[255] = 254. / 255.f;
        }
    }

    //
    // Enable specular color splitting so the specular highlights are done
    // after texturing. This ensures that the specular highlights look
    // right when we're in texturing mode.
    //
#ifdef VTK_IMPLEMENT_MESA_CXX
    // Mesa
    glEnable(GL_COLOR_SUM_EXT);
    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
#else
    // OpenGL
#if HAVE_LIBGLEW
    if(!this->GLEW_initialized)
    {
        this->GLEW_initialized = glewInit() == GLEW_OK;
    }
    if(this->GLEW_initialized && GLEW_EXT_secondary_color)
    {
        glEnable(GL_COLOR_SUM_EXT);
        glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
    }
#endif     
#endif
}      

// ****************************************************************************
// Method: vtkVisItOpenGLPolyDataMapper::EndColorTexturing
//
// Purpose: 
//   Ends color texturing if it is enabled.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 24 15:59:01 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
vtkVisItOpenGLPolyDataMapper::EndColorTexturing()
{
    if(!this->ColorTexturingAllowed)
        return;

    // Turn off color texturing.
    if(this->ColorTextureLoaded)
    {
        glDisable(GL_TEXTURE_1D);
    }

#ifdef VTK_IMPLEMENT_MESA_CXX
    // Mesa
    glDisable(GL_COLOR_SUM_EXT);
#else
    // OpenGL
#if HAVE_LIBGLEW
    if(this->GLEW_initialized && GLEW_EXT_secondary_color)
        glDisable(GL_COLOR_SUM_EXT);
#endif     
#endif
}
