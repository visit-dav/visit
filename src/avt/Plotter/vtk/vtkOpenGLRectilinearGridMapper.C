/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkOpenGLRectilinearGridMapper.cxx,v $
  Language:  C++
  Date:      $Date: 2003/04/28 19:13:10 $
  Version:   $Revision: 1.78 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

// Modifications:
//
//    Thomas R. Treadway, Wed Feb  7 16:18:28 PST 2007
//    The gcc-4.x compiler no longer just warns about automatic type conversion.
//
=========================================================================*/
#include <avtOpenGLExtensionManager.h>
#include "vtkOpenGLRectilinearGridMapper.h"

#include <vtkPoints.h>
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCommand.h"
#include "vtkDataArray.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkImageData.h"
#include "vtkMatrix4x4.h"
#include "vtkObjectFactory.h"
#include "vtkOpenGLRenderer.h"
#include "vtkPlane.h"
#include "vtkPlaneCollection.h"
#include "vtkPointData.h"
#include "vtkRectilinearGrid.h"
#include "vtkPolygon.h"
#include "vtkProperty.h"
#include "vtkTimerLog.h"
#include "vtkTriangle.h"
#include "vtkRenderWindow.h"

#include "vtkLookupTable.h"
#include "vtkSkewLookupTable.h"

static const int dlSize = 8192;

vtkStandardNewMacro(vtkOpenGLRectilinearGridMapper);

vtkOpenGLRectilinearGridMapper::vtkOpenGLRectilinearGridMapper()
{
  this->ListStart = 0;
  this->doingDisplayLists = false;
  this->primsInCurrentList = 0;
  this->nLists = 0;
  this->CurrentList = 0;

  this->ColorTexturingAllowed = false;
  this->ColorTextureLoaded = false;
  this->ColorTextureName = 0;
  this->ColorTexture = 0;
  this->ColorTextureSize = 0;
  this->OpenGLSupportsVersion1_2 = false;

  this->LastOpacity = -1;
}

// Destructor (don't call ReleaseGraphicsResources() since it is virtual
vtkOpenGLRectilinearGridMapper::~vtkOpenGLRectilinearGridMapper()
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
// ****************************************************************************

void vtkOpenGLRectilinearGridMapper::ReleaseGraphicsResources(vtkWindow *win)
{
  if (this->ListStart && win)
    {
    win->MakeCurrent();
    glDeleteLists(this->ListStart,nLists);
    this->ListStart = 0;
    }
  this->LastWindow = NULL; 

  if (this->ColorTextureLoaded)
    {
        win->MakeCurrent();
        glDeleteTextures(1, (GLuint*)&this->ColorTextureName);
        this->ColorTextureLoaded = false;
    }
}

// ***************************************************************************
//
// Modifications:
//
//   Tom Fogal, Fri Aug  7 12:15:21 MDT 2009
//   Initialize GLEW after the RW gets made current.
//
//   Kathleen Bonnell, Tue Feb 16 15:49:17 MST 2010
//   Remove initialization of GLEW, it is handled elsewhere.
//
//   Brad Whitlock, Wed Sep 28 17:05:03 PDT 2011
//   Push/pop lighting state so changes we make to lighting and materials
//   here do not bleed into other modules.
//
// ***************************************************************************

void vtkOpenGLRectilinearGridMapper::Render(vtkRenderer *ren, vtkActor *act)
{
  vtkIdType numPts;
  vtkRectilinearGrid *input= this->GetInput();
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
    if (!this->Static)
      this->GetInputAlgorithm()->Update();
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

  glPushAttrib(GL_LIGHTING_BIT);

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
    // Sets this->Colors as side effect.
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

  glPopAttrib();
}

// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Wed Dec 27 10:27:48 PST 2006
//    Fix indexing bug for ghost data.
//
//    Hank Childs, Thu Dec 28 10:24:53 PST 2006
//    Add support for grids in three dimensional space.
//
//    Jeremy Meredith, Thu Feb 15 11:41:19 EST 2007
//    Added support for rectilinear grids with an inherent  transform.
//    These will have a new field array in the data set.
//
//    Tom Fogal, Thu Aug  6 16:45:28 MDT 2009
//    Make sure GLEW is initialized.
//
//    Kathleen Bonnell, Tue Feb 16 15:49:17 MST 2010
//    Remove initialization of GLEW, it is handled elsewhere.
//
//    Brad Whitlock, Tue Aug  9 14:15:25 PDT 2011
//    I changed how we do 1D texturing so it is compatible with the new VTK.
//
//    Brad Whitlock, Wed Oct 26 12:29:40 PDT 2011
//    Add support for 1D rectilinear (Nx, 1, 1 case) grids as lines.
//
//    Brad Whitlock, Mon Mar 19 11:04:09 PDT 2012
//    Add double coordinate support via macro. Change loops to avoid some branching.
//
// ****************************************************************************

int vtkOpenGLRectilinearGridMapper::Draw(vtkRenderer *ren, vtkActor *act)
{
   int  i;

   vtkRectilinearGrid *input = this->GetInput();

   bool transform = false;
   if (input->GetFieldData()->GetArray("RectilinearGridTransform"))
       transform = true;


   int dims[3];
   input->GetDimensions(dims);
   bool flatI = (dims[0] <= 1);
   bool flatJ = (dims[1] <= 1);
   bool flatK = (dims[2] <= 1);

   if (!flatI && !flatJ && !flatK)
   {
       vtkErrorMacro("One of the dimensions must be flat!!\n"
                     "Rectilinear grid mapper unable to render");
       return 0;
   }

   if (this->doingDisplayLists)
   {
       glNewList(this->CurrentList,GL_COMPILE);
       this->primsInCurrentList = 0;
   }

   if (transform)
   {
       vtkDoubleArray *xform = (vtkDoubleArray*)input->GetFieldData()->
                                          GetArray("RectilinearGridTransform");
       double *matrix = xform->GetPointer(0);
       // OpenGL wants the transpose of our VTK matrix.
       float glmatrix[16] = {matrix[ 0], matrix[ 4], matrix[ 8], matrix[12],
                             matrix[ 1], matrix[ 5], matrix[ 9], matrix[13],
                             matrix[ 2], matrix[ 6], matrix[10], matrix[14],
                             matrix[ 3], matrix[ 7], matrix[11], matrix[15]};
       glPushMatrix();
       glMultMatrixf(glmatrix);
   }


   if (SceneIs3D)
       glEnable(GL_LIGHTING);
   else
       glDisable(GL_LIGHTING);
   const unsigned char *colors = NULL;
   if (this->Colors != NULL)
       colors = this->Colors->GetPointer(0);

   vtkProperty *prop = act->GetProperty();
   glDisable( GL_COLOR_MATERIAL );
   if (colors)
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

   vtkUnsignedCharArray *gz = (vtkUnsignedCharArray *) 
                               input->GetCellData()->GetArray("avtGhostZones");
   unsigned char *ghost_zones = NULL;
   if (gz != NULL)
       ghost_zones = gz->GetPointer(0);
   vtkUnsignedCharArray *gn = (vtkUnsignedCharArray *) 
                              input->GetPointData()->GetArray("avtGhostNodes");
   unsigned char *ghost_nodes = NULL;
   if (gn != NULL)
       ghost_nodes = gn->GetPointer(0);

   bool nodeData = true;
   if ( (this->ScalarMode == VTK_SCALAR_MODE_USE_CELL_DATA ||
         this->ScalarMode == VTK_SCALAR_MODE_USE_CELL_FIELD_DATA ||
         !input->GetPointData()->GetScalars() )
         && this->ScalarMode != VTK_SCALAR_MODE_USE_POINT_FIELD_DATA)
     {
     nodeData = false;
     }

#define vtkOpenGLRectilinearGridMapper_EmitGeometry(T, VERTEXFUNC) \
   T *X = new T[dims[0]]; \
   for (i = 0 ; i < dims[0] ; i++) \
       X[i] = input->GetXCoordinates()->GetTuple1(i); \
   T *Y = new T[dims[1]]; \
   for (i = 0 ; i < dims[1] ; i++) \
       Y[i] = input->GetYCoordinates()->GetTuple1(i); \
   T *Z = new T[dims[2]]; \
   for (i = 0 ; i < dims[2] ; i++) \
       Z[i] = input->GetZCoordinates()->GetTuple1(i); \
 \
   bool normalQuadOrder = !flatJ; \
   if (dims[0] > 1 && X[1] < X[0]) \
       normalQuadOrder = !normalQuadOrder; \
   if (dims[1] > 1 && Y[1] < Y[0]) \
       normalQuadOrder = !normalQuadOrder; \
   if (dims[2] > 1 && Z[1] < Z[0]) \
       normalQuadOrder = !normalQuadOrder; \
   const int normalquadorder[4] = { 0, 1, 3, 2 }; \
   const int otherquadorder[4] = { 0, 2, 3, 1 }; \
   const int *quadorder = (normalQuadOrder ? normalquadorder : otherquadorder); \
 \
   int slowDim = 0; \
   int fastDim = 0; \
   if (flatI) \
   { \
       glNormal3f(1., 0., 0.); \
       fastDim = dims[1]; \
       slowDim = dims[2]; \
   } \
   if (flatJ) \
   { \
       glNormal3f(0., 1., 0.); \
       fastDim = dims[0]; \
       slowDim = dims[2]; \
   } \
   if (flatK) \
   { \
       glNormal3f(0., 0., 1.); \
       fastDim = dims[0]; \
       slowDim = dims[1]; \
   } \
 \
   const float *texCoords = NULL; \
   if(this->ColorCoordinates != NULL) \
       texCoords = (const float *)this->ColorCoordinates->GetVoidPointer(0); \
 \
   if(!flatI && flatJ && flatK) \
   { \
       /* Draw a curve*/ \
       glBegin(GL_LINE_STRIP); \
       if(colors == NULL && texCoords == NULL) \
       { \
           for(int i = 0; i < dims[0]; ++i) \
               VERTEXFUNC(X[i], 0.f, 0.f); \
       } \
       else \
       { \
           for(int i = 0; i < dims[0]; ++i) \
           { \
               if(nodeData) \
               { \
                   if(texCoords != NULL) \
                        glTexCoord1f(texCoords[i]); \
                   else if(colors != NULL) \
                        glColor4ubv(colors + 4*i); \
               } \
               VERTEXFUNC(X[i], 0.f, 0.f); \
           } \
       } \
       glEnd(); \
   } \
   else \
   { \
       glBegin(GL_QUADS); \
       for (int j = 0 ; j < slowDim-1 ; j++) \
           for (int i = 0 ; i < fastDim-1 ; i++) \
           { \
               if (ghost_zones != NULL) \
                   if (*(ghost_zones++) != '\0') \
                       continue; \
               if (ghost_nodes != NULL) \
               { \
                   int p0 = j*fastDim+i; \
                   int p1 = j*fastDim+i+1; \
                   int p2 = (j+1)*fastDim+i; \
                   int p3 = (j+1)*fastDim+i+1; \
                   if (ghost_nodes[p0] && ghost_nodes[p1] && ghost_nodes[p2] \
                       && ghost_nodes[p3]) \
                      continue; \
               } \
\
               if (colors == NULL && texCoords == NULL) \
               { \
                   if (flatI) \
                   { \
                       for (int k = 0 ; k < 4 ; k++) \
                           VERTEXFUNC(X[0], Y[i + quadorder[k] % 2],  \
                                      Z[j + quadorder[k]/2]); \
                   } \
                   else if (flatJ) \
                   { \
                       for (int k = 0 ; k < 4 ; k++) \
                           VERTEXFUNC(X[i + quadorder[k] % 2], Y[0], \
                                      Z[j + quadorder[k]/2]); \
                   } \
                   else if (flatK) \
                   { \
                       for (int k = 0 ; k < 4 ; k++) \
                           VERTEXFUNC(X[i + quadorder[k] % 2],  \
                                      Y[j + quadorder[k]/2], Z[0]); \
                   } \
               } \
               else \
               { \
                   if (!nodeData) \
                   { \
                       int idx = j*(fastDim-1) + i; \
                       if(texCoords != NULL) \
                           glTexCoord1f(texCoords[idx]); \
                       else if(colors != NULL) \
                           glColor4ubv(colors + 4*idx); \
                       if (flatI) \
                       { \
                           for (int k = 0 ; k < 4 ; k++) \
                               VERTEXFUNC(X[0], Y[i + quadorder[k] % 2],  \
                                          Z[j + quadorder[k]/2]); \
                       } \
                       else if (flatJ) \
                       { \
                           for (int k = 0 ; k < 4 ; k++) \
                               VERTEXFUNC(X[i + quadorder[k] % 2], Y[0], \
                                          Z[j + quadorder[k]/2]); \
                       } \
                       else if (flatK) \
                       { \
                           for (int k = 0 ; k < 4 ; k++) \
                               VERTEXFUNC(X[i + quadorder[k] % 2],  \
                                          Y[j + quadorder[k]/2], Z[0]); \
                       } \
                   } \
                   else \
                   { \
                        if(texCoords != NULL) \
                        { \
                            if (flatI) \
                            { \
                                for (int k = 0 ; k < 4 ; k++) \
                                { \
                                    int idx = (j + quadorder[k]/2)*fastDim +  \
                                              (i+(quadorder[k]%2)); \
                                    glTexCoord1f(texCoords[idx]); \
                                    VERTEXFUNC(X[0], Y[i + quadorder[k] % 2],  \
                                               Z[j + quadorder[k]/2]); \
                                } \
                            } \
                            else if (flatJ) \
                            { \
                                for (int k = 0 ; k < 4 ; k++) \
                                { \
                                    int idx = (j + quadorder[k]/2)*fastDim +  \
                                              (i+(quadorder[k]%2)); \
                                    glTexCoord1f(texCoords[idx]); \
                                    VERTEXFUNC(X[i + quadorder[k] % 2], Y[0], \
                                               Z[j + quadorder[k]/2]); \
                                } \
                            } \
                            else if (flatK) \
                            { \
                                for (int k = 0 ; k < 4 ; k++) \
                                { \
                                    int idx = (j + quadorder[k]/2)*fastDim +  \
                                              (i+(quadorder[k]%2)); \
                                    glTexCoord1f(texCoords[idx]); \
                                    VERTEXFUNC(X[i + quadorder[k] % 2],  \
                                               Y[j + quadorder[k]/2], Z[0]); \
                                } \
                           } \
                       } \
                       else if(colors != NULL) \
                       { \
                            if (flatI) \
                            { \
                                for (int k = 0 ; k < 4 ; k++) \
                                { \
                                    int idx = (j + quadorder[k]/2)*fastDim +  \
                                              (i+(quadorder[k]%2)); \
                                    glColor4ubv(colors + 4*idx); \
                                    VERTEXFUNC(X[0], Y[i + quadorder[k] % 2],  \
                                               Z[j + quadorder[k]/2]); \
                                } \
                            } \
                            else if (flatJ) \
                            { \
                                for (int k = 0 ; k < 4 ; k++) \
                                { \
                                    int idx = (j + quadorder[k]/2)*fastDim +  \
                                              (i+(quadorder[k]%2)); \
                                    glColor4ubv(colors + 4*idx); \
                                    VERTEXFUNC(X[i + quadorder[k] % 2], Y[0], \
                                               Z[j + quadorder[k]/2]); \
                                } \
                            } \
                            else if (flatK) \
                            { \
                                for (int k = 0 ; k < 4 ; k++) \
                                { \
                                    int idx = (j + quadorder[k]/2)*fastDim +  \
                                              (i+(quadorder[k]%2)); \
                                    glColor4ubv(colors + 4*idx); \
                                    VERTEXFUNC(X[i + quadorder[k] % 2],  \
                                               Y[j + quadorder[k]/2], Z[0]); \
                                } \
                           } \
                       } \
                   } \
               } \
\
               if (this->doingDisplayLists) \
               { \
                   this->primsInCurrentList++; \
                   if (this->primsInCurrentList >= dlSize) \
                   { \
                       glEnd(); \
                       glEndList(); \
                       this->CurrentList++; \
                       glNewList(this->CurrentList,GL_COMPILE); \
                       glBegin(GL_QUADS); \
                       this->primsInCurrentList = 0; \
                   } \
               } \
           } \
       glEnd(); \
   } \
   delete [] X; \
   delete [] Y; \
   delete [] Z;

   if(input->GetXCoordinates()->GetDataType() == VTK_DOUBLE ||
      input->GetYCoordinates()->GetDataType() == VTK_DOUBLE ||
      input->GetZCoordinates()->GetDataType() == VTK_DOUBLE)
   {
       vtkOpenGLRectilinearGridMapper_EmitGeometry(double, glVertex3d);
   }
   else
   {
       vtkOpenGLRectilinearGridMapper_EmitGeometry(float, glVertex3f);
   }

   if (transform)
       glPopMatrix();

   glEnable(GL_LIGHTING);
   if (this->doingDisplayLists)
       glEndList();

   return 1;
}


void vtkOpenGLRectilinearGridMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

// ****************************************************************************
// Method: vtkOpenGLRectilinearGridMapper::MapScalarsWithTextureSupport
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
// Note:       Taken wholesale from routine written by Brad Whitlock
//             for vtkVisItOpenGLPolyDataMapper.
//
// Programmer: Hank Childs
// Creation:   December 19, 2006
//
// Modifications:
//   Jeremy Meredith, Tue Jun 24 17:40:19 EDT 2008
//   Check for NULL version string before scanf.
//
//   Brad Whitlock, Tue Aug  9 14:24:49 PDT 2011
//   Remove code to fill up texture. I changed how it gets done and I moved it
//   to BeginColorTexturing.
//
// ****************************************************************************

bool
vtkOpenGLRectilinearGridMapper::MapScalarsWithTextureSupport(double opacity)
{
    bool saveColors = this->InterpolateScalarsBeforeMapping &&
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
    }

    if (opacity != this->LastOpacity)
    {
       // VTK can get confused about whether or not the opacities are
       // correct (... and incorrectly re-use cached objects when they
       // have the wrong transparency).  So remove the cached Colors here.
       if (this->Colors != NULL)
           this->Colors->Delete();
       this->Colors = NULL;
    }

    // sets this->Colors and this->ColorCoordinates as side effect
    this->MapScalars(opacity);
    this->LastOpacity = opacity;

    return saveColors;
}

// ****************************************************************************
// Method: vtkOpenGLRectilinearGridMapper::LooksDiscrete
//
// Purpose: 
//   Look at the color table to see if it looks discrete.
//
// Arguments:
//
// Returns:    True if the color table looks discrete.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 10 10:50:45 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

bool
vtkOpenGLRectilinearGridMapper::LooksDiscrete() const
{
    bool looksDiscrete = false;

    if(this->ColorTexture != NULL)
    {
        // Analyize the colors so we can make a guess as to whether the
        // colors in the LUT came from a continuous or discrete lookup
        // table. Just assume that the color table is discrete if the
        // colors are the same for 3 consecutive bins if we sample some
        // number of locations in the color table.
        int same_count = 0;
        for(int i = 0; i < 5; ++i)
        {
            float t = float(i) / float(this->ColorTextureSize-1);
            int index = int(t * (this->ColorTextureSize - 2));

            unsigned char c0[3], c1[3];
            c0[0] = (unsigned char)(255. * this->ColorTexture[(index * 4) + 0]);
            c0[1] = (unsigned char)(255. * this->ColorTexture[(index * 4) + 1]);
            c0[2] = (unsigned char)(255. * this->ColorTexture[(index * 4) + 2]);

            c1[0] = (unsigned char)(255. * this->ColorTexture[((index+1) * 4) + 0]);
            c1[1] = (unsigned char)(255. * this->ColorTexture[((index+1) * 4) + 1]);
            c1[2] = (unsigned char)(255. * this->ColorTexture[((index+1) * 4) + 2]);

            if(c0[0] == c1[0] && c0[1] == c1[1] && c0[2] == c1[2])
            {
                ++same_count;
            }
        }

        looksDiscrete = same_count >= 3;
    }

    return looksDiscrete;
}

// ****************************************************************************
// Method: vtkOpenGLRectilinearGridMapper::BeginColorTexturing
//
// Purpose:
//   Begins color texturing if it is enabled.
//
// Note:       Taken wholesale from routine written by Brad Whitlock
//             for vtkVisItOpenGLPolyDataMapper.
//
// Programmer: Hank Childs
// Creation:   December 19, 2006
//
// Modifications:
//
//    Thomas R. Treadway, Wed Feb  7 16:18:28 PST 2007
//    The gcc-4.x compiler no longer just warns about automatic type conversion.
//
//    Tom Fogal, Thu Aug  6 16:46:55 MDT 2009
//    Simplify; GLEW is always present.
//
//    Brad Whitlock, Tue Aug  9 14:24:03 PDT 2011
//    I moved code from other methods to this one. I also changed how we get
//    the colors so we get them from the ColorTextureMap of the mapper.
//
// ****************************************************************************

void
vtkOpenGLRectilinearGridMapper::BeginColorTexturing()
{
    if(!this->ColorTexturingAllowed)
        return;

    if(!this->ColorTextureLoaded)
    {
        // Copy the image data texture that VTK created but convert to GL_FLOAT
        vtkDataArray *textColors = this->ColorTextureMap->GetPointData()->
            GetScalars();
        const unsigned char *rgba = (const unsigned char *)textColors->
            GetVoidPointer(0);
        if(this->ColorTexture != NULL)
            delete [] this->ColorTexture;
        this->ColorTextureSize = textColors->GetNumberOfTuples();
        this->ColorTexture = new float[this->ColorTextureSize * 4];
        for(int i = 0; i < this->ColorTextureSize * 4; ++i)
            this->ColorTexture[i] = float(rgba[i]) / 256.f;

        // Create a GL texture name for texture.
        glGenTextures(1, (GLuint*)&this->ColorTextureName);
        glBindTexture(GL_TEXTURE_1D, this->ColorTextureName);

#ifdef GL_VERSION_1_2
        // Figure out the OpenGL version.
        const char *gl_ver = (const char *)glGetString(GL_VERSION);
        int major, minor;
        if(gl_ver && sscanf(gl_ver, "%d.%d", &major, &minor) == 2)
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
        GLint m = this->LooksDiscrete() ? GL_NEAREST : GL_LINEAR;
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

    //
    // Enable specular color splitting so the specular highlights are done
    // after texturing. This ensures that the specular highlights look
    // right when we're in texturing mode.
    //
#ifdef HAVE_LIBGLEW
    if(GLEW_EXT_secondary_color)
    {
        glEnable(GL_COLOR_SUM_EXT);
        glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
    }
#endif
}


// ****************************************************************************
// Method: vtkOpenGLRectilinearGridMapper::EndColorTexturing
//
// Purpose:
//   Ends color texturing if it is enabled.
//
// Note:       Taken wholesale from routine written by Brad Whitlock
//             for vtkVisItOpenGLPolyDataMapper.
//
// Programmer: Hank Childs
// Creation:   December 19, 2006
//
// Modifications:
//
//   Tom Fogal, Thu Aug  6 16:46:34 MDT 2009
//   Simplify; GLEW is always present.
//
// ****************************************************************************

void
vtkOpenGLRectilinearGridMapper::EndColorTexturing()
{
    if(!this->ColorTexturingAllowed)
        return;

    // Turn off color texturing.
    if(this->ColorTextureLoaded)
    {
        glDisable(GL_TEXTURE_1D);
    }
#ifdef HAVE_LIBGLEW
    if(GLEW_EXT_secondary_color)
    {
        glDisable(GL_COLOR_SUM_EXT);
    }
#endif
}


// ****************************************************************************
// Method: vtkOpenGLRectilinearGridMapper::UsesPointData
//
// Purpose:
//   Follows the same rules as the GetScalars method except that it returns
//   true if the scalars are point data or false otherwise.
//
// Note:       We use this method to determine whether we have point data
//             so we can enable color texturing.
//
// Note:       Taken wholesale from routine written by Brad Whitlock
//             for vtkVisItOpenGLPolyDataMapper.
//
// Programmer: Hank Childs
// Creation:   December 19, 2006
//
// Modifications:
//
// ****************************************************************************

bool
vtkOpenGLRectilinearGridMapper::UsesPointData(vtkDataSet *input, int scalarMode,
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

    if ( scalars && (offset < scalars->GetNumberOfComponents()) )
      {
          usesPointData = true;
      }
    }

  return usesPointData;
}


