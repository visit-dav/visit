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

// ************************************************************************* //
//                  avtOpenGLSurfaceAndWireframeRenderer.C                   //
// ************************************************************************* //

#include "avtOpenGLSurfaceAndWireframeRenderer.h"

#include <avtOpenGLExtensionManager.h>

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolygon.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkTriangle.h>

#include <DebugStream.h>


// some definitions for what the polydata has in it
#define OGL_SAWR_NORMALS            0x01
#define OGL_SAWR_COLORS             0x02
#define OGL_SAWR_TCOORDS            0x04
#define OGL_SAWR_CELL_COLORS        0x08
#define OGL_SAWR_CELL_NORMALS       0x10
#define OGL_SAWR_POINT_TYPE_FLOAT   0x20
#define OGL_SAWR_POINT_TYPE_DOUBLE  0x40
#define OGL_SAWR_NORMAL_TYPE_FLOAT  0x80
#define OGL_SAWR_NORMAL_TYPE_DOUBLE 0x100
#define OGL_SAWR_TCOORD_TYPE_FLOAT  0x200
#define OGL_SAWR_TCOORD_TYPE_DOUBLE 0x400
#define OGL_SAWR_TCOORD_1D          0x800
#define OGL_SAWR_OPAQUE_COLORS      0x1000
#define OGL_SAWR_USE_FIELD_DATA     0x2000


avtOpenGLSurfaceAndWireframeRenderer::avtOpenGLSurfaceAndWireframeRenderer()
{
  // Must be in the C file to work as a Windows DLL.
}

// ****************************************************************************
//  Desructor: avtOpenGLSurfaceAndWireframeRenderer
//
//  Modifications:
//    Kathleen Bonnell, Thu Jun 13 10:26:49 PDT 2002    
//    Delete any generated display lists.
//
//    Brad Whitlock, Fri Jun 28 13:39:21 PST 2002
//    Made it work on windows.
//
//    Hank Childs, Wed Apr 23 22:17:03 PDT 2003
//    Make sure we are dealing with the correct context or we may accidentally
//    delete the display lists for the wrong window.
//
//    Hank Childs, Mon May  5 18:17:23 PDT 2003
//    Do not assume that we have a valid VTK renderer.
//
//    Kathleen Bonnell, Tue Aug 26 13:51:22 PDT 2003 
//    Moved deletion of display lists to ReleaseGraphicsResources. 
//
// ****************************************************************************

avtOpenGLSurfaceAndWireframeRenderer::~avtOpenGLSurfaceAndWireframeRenderer()
{
    ReleaseGraphicsResources();
    surfaceListId.clear();
    edgesListId.clear();
    setupListId.clear();
}


// ****************************************************************************
//  Method: avtOpenGLSurfaceAndWireframeRenderer::ReleaseGraphicsResources
//
//  Purpose:  Releases graphics resources by deleting display lists.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    August 26, 2003 
//
//  Modifications:
//
// ****************************************************************************

void
avtOpenGLSurfaceAndWireframeRenderer::ReleaseGraphicsResources()
{
    if (VTKRen == NULL)
        return;
    ;

    VTKRen->GetRenderWindow()->MakeCurrent();

    for (size_t i = 0; i < surfaceListId.size(); i++)
    {
        if (surfaceListId[i])
        {
            glDeleteLists(surfaceListId[i], 1);
            surfaceListId[i] = 0;
            surfaceModified[i] = true;
        }
    }
    for (size_t i = 0; i < edgesListId.size(); i++)
    {
        if (edgesListId[i])
        {
            glDeleteLists(edgesListId[i], 1);
            edgesListId[i] = 0;
            edgesModified[i] = true;
        }
    }
    for (size_t i = 0; i < setupListId.size(); i++)
    {
        if (setupListId[i])
        {
            glDeleteLists(setupListId[i], 1);
            setupListId[i] = 0;
            setupModified[i] = true;
        }
    }
}

// ****************************************************************************
//  Method:  BeginPolyTriangleOrQuad
//
//  Purpose:
//    Helper routine which starts a poly, triangle or quad based upon
//    the number of points in the polygon and whether triangles or quads
//    were the last thing being drawn (we can get better performance if we
//    can draw several triangles within a single glBegin(GL_TRIANGLES) or
//    several quads within a single glBegin(GL_QUADS). 
//
//  Notes:  This routine was taken directly from vtkOpenGLPolyDataMapper.
//          with a few minor modifications.
//
//  Arguments:
//    aGlFunction         The current type of glfunction we wish to begin.
//    previousGlFunction  The previous type of glfunction we had begun.
//    npts                The number of points.
//
//  Programmer:  Kathleen Bonnell (Thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
BeginPolyTriangleOrQuad(GLenum aGlFunction, GLenum &previousGlFunction, 
                        int npts)
{
    if (aGlFunction == GL_POLYGON)
    {
        switch (npts)
        {
          case 3: // Need to draw a triangle.
                  if (previousGlFunction != GL_TRIANGLES)
                  {
                      // we were not already drawing triangles, 
                      // were we drawing quads?
                      if (previousGlFunction == GL_QUADS)
                      {
                          // we were previously drawing quads, 
                          // close down the quads.
                          glEnd();
                      }
                      // start drawing triangles
                      previousGlFunction = GL_TRIANGLES;
                      glBegin(GL_TRIANGLES);
                  }
                  break;
          case 4: // Need to draw a quad
                  if (previousGlFunction != GL_QUADS)
                  {
                      // we were not already drawing quads, 
                      // were we drawing triangles?
                      if (previousGlFunction == GL_TRIANGLES)
                      {
                          // we were previously drawing triangles, 
                          // close down the triangles.
                          glEnd();
                      }
                      // start drawing quads
                      previousGlFunction = GL_QUADS;
                      glBegin(GL_QUADS);
                  }
                  break;
          default:
                  // if we were supposed to be drawing polygons but 
                  // were really drawing triangles or quads, 
                  // then we need to close down the triangles or quads 
                  // and begin a polygon
                  if (previousGlFunction != GL_INVALID_VALUE &&
                      previousGlFunction != GL_POLYGON)
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
            // since we draw all points before drawing any polygons (i.e in 
            // the polys case switch to triangles and quads as an optimization,
            // there is nothing to switch to that is below points).
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


//-----------------------------------------

#define vtkDrawPointsMacro(ptype,ntype,glVertFuncs,glInitFuncs) \
{ \
  vtkIdType nPts; unsigned short count = 0; \
  ptype *points = static_cast<ptype *>(voidPoints);     \
  glInitFuncs \
  glBegin(GL_POINTS); \
  while (ptIds < endPtIds) \
    { \
    nPts = *ptIds; \
    ++ptIds; \
    while (nPts > 0) \
      { \
      glVertFuncs \
      ++ptIds; \
      --nPts; \
      } \
    if (++count == 10000) \
      { \
      cellNum += 10000; \
      count = 0; \
      } \
    } \
  cellNum += count; \
  glEnd(); \
}



#define vtkDrawPrimsMacro(ptype,ntype,prim,glVertFuncs,glInitFuncs) \
{ \
  vtkIdType nPts; unsigned short count = 0; \
  ptype *points = static_cast<ptype *>(voidPoints);    \
  glInitFuncs \
  while (ptIds < endPtIds) \
    { \
    nPts = *ptIds; \
    ++ptIds; \
    glBegin(prim); \
    while (nPts > 0) \
      { \
      glVertFuncs \
      ++ptIds; \
      --nPts; \
      } \
    glEnd(); \
    if (++count == 10000) \
      { \
      cellNum += 10000; \
      count = 0; \
      } \
    } \
  cellNum += count; \
}


#define vtkDrawPolysMacro(ptype,ntype,ttype,prim,glVertFuncs,glCellFuncs,glInitFuncs) \
{ \
  vtkIdType nPts; unsigned short count = 0; \
  ptype *points = static_cast<ptype *>(voidPoints);    \
  GLenum previousGlFunction=GL_INVALID_VALUE; \
  glInitFuncs \
while (ptIds < endPtIds) \
    { \
    nPts = *ptIds; \
    ++ptIds; \
    BeginPolyTriangleOrQuad( prim, previousGlFunction, nPts ); \
    glCellFuncs \
    while (nPts > 0) \
      { \
      glVertFuncs \
      ++ptIds; \
      --nPts; \
      } \
    if (++count == 10000) \
      { \
      cellNum += 10000; \
      count = 0; \
      } \
    if ((previousGlFunction != GL_TRIANGLES)  \
        && (previousGlFunction != GL_QUADS)   \
        && (previousGlFunction != GL_POINTS)) \
      {  \
      glEnd(); \
      } \
    } \
  cellNum += count; \
  if ((previousGlFunction == GL_TRIANGLES)  \
      || (previousGlFunction == GL_QUADS)   \
      || (previousGlFunction == GL_POINTS)) \
    { \
    glEnd(); \
    } \
}


#define vtkDrawPolysMacro4Tri(ptype,ntype,ttype,prim,glVertFuncs,glCellFuncs,glInitFuncs) \
{ \
  vtkIdType nPts; unsigned short count = 0; \
  ptype *points = static_cast<ptype *>(voidPoints);     \
  GLenum previousGlFunction=GL_INVALID_VALUE; \
  glInitFuncs \
  \
  double quad_center[3] = {0, 0, 0}; \
  double quad_center_col[4] = {0, 0, 0, 0}; \
  double quad_points[4][3]; \
  double quad_points_col[4][4]; \
  double dist_center[4] = {0, 0, 0, 0}; \
  \
  while (ptIds < endPtIds) \
    { \
    nPts = *ptIds; \
  ++ptIds; \
  /* If we don't want to draw a QUAD (ex : a triangle nPts = 3) */ \
  if (nPts != 4) { \
  /* Classic method */ \
    BeginPolyTriangleOrQuad( prim, previousGlFunction, nPts ); \
    glCellFuncs \
    while (nPts > 0) \
    { \
    glVertFuncs \
    ++ptIds; \
    --nPts; \
    } \
  } \
  /* If we want to draw a QUAD */ \
  else { \
  /* We launch glBegin(GL_TRIANGLES) mode in order to draw 4 triangles */ \
    BeginPolyTriangleOrQuad( prim, previousGlFunction, 3 ); \
    glCellFuncs \
  /* We keep pointer on the first point of the first triangle */ \
  /* ptIdsFirstPtQuad will be used for center calculation and for 2nd point of 4th triangle */ \
    vtkIdType *ptIdsFirstPtQuad; \
    ptIdsFirstPtQuad = ptIds; \
  /* QUAD Center calculation */ \
  /* We save the 4 QUAD points and their color */ \
    GLfloat *vpt; \
    GLubyte *vcol; \
    for (int i=0; i<4; i++) { \
      /* Position : */ \
      vpt = points + 3**ptIds; \
      quad_points[i][0] = vpt[0]; \
      quad_points[i][1] = vpt[1]; \
      quad_points[i][2] = vpt[2]; \
      /* Color : */ \
      vcol = colors + 4**ptIds; \
      quad_points_col[i][0] = vcol[0]; \
      quad_points_col[i][1] = vcol[1]; \
      quad_points_col[i][2] = vcol[2]; \
      quad_points_col[i][3] = vcol[3]; \
      ++ptIds; \
    } \
  /* Actual calculation of QUAD center with the 4 summits */ \
    quad_center[0] = (quad_points[0][0] + quad_points[1][0] + quad_points[2][0] + quad_points[3][0])/4; \
    quad_center[1] = (quad_points[0][1] + quad_points[1][1] + quad_points[2][1] + quad_points[3][1])/4; \
    quad_center[2] = (quad_points[0][2] + quad_points[1][2] + quad_points[2][2] + quad_points[3][2])/4; \
  /* Color center calculation  (Interpolation on each component of RGB vector) */ \
  /* Calculation of distances between center and summits */ \
    for (int i=0; i<4; i++) { \
      dist_center[i] = sqrt((quad_points[i][0] - quad_center[0])*(quad_points[i][0] - quad_center[0]) + \
                (quad_points[i][1] - quad_center[1])*(quad_points[i][1] - quad_center[1]) + \
                (quad_points[i][2] - quad_center[2])*(quad_points[i][2] - quad_center[2])); \
    } \
  /* Color interpolation (3 for RGB and 1 for Alpha transparency) */ \
    for (int i=0; i<4; i++) { \
      quad_center_col[i] = ((dist_center[3]*quad_points_col[1][i] + dist_center[1]*quad_points_col[3][i])/(dist_center[1] + dist_center[3]) + \
                (dist_center[2]*quad_points_col[0][i] + dist_center[0]*quad_points_col[2][i])/(dist_center[2] + dist_center[0]) \
                )/2; \
    } \
  /* We take pointer on the first QUAD point */ \
    ptIds = ptIdsFirstPtQuad; \
  /* Actual drawing of 4 triangles */ \
    for (int i=0; i<4; i++) { \
      /* 1st point */ \
      glVertFuncs \
      ++ptIds; \
      /* 2nd point */ \
      if (i >= 3) { /* If it is the last triangle */ \
        /* this 2nd point = the 1st point of 1st triangle */ \
        glColor3ubv(colors + 4**ptIdsFirstPtQuad); \
        glVertex3fv(static_cast<float*>(points) + 3**ptIdsFirstPtQuad); \
      } \
      else { \
        /* Else 2nd point = next point */ \
        glVertFuncs \
      } \
      /* 3rd point */ \
      glColor4f(quad_center_col[0],quad_center_col[1],quad_center_col[2],quad_center_col[3]); \
      glVertex3f(quad_center[0],quad_center[1],quad_center[2]); \
    } \
  } /* End of if (nPts == 4) */ \
    if (++count == 10000) \
      { \
      cellNum += 10000; \
      count = 0; \
      } \
    if ((previousGlFunction != GL_TRIANGLES)  \
        && (previousGlFunction != GL_QUADS)   \
        && (previousGlFunction != GL_POINTS)) \
      {  \
      glEnd(); \
      } \
    } \
  cellNum += count; \
  if ((previousGlFunction == GL_TRIANGLES)  \
      || (previousGlFunction == GL_QUADS)   \
      || (previousGlFunction == GL_POINTS)) \
    { \
    glEnd(); \
    } \
}


#define vtkDrawPolysMacro4TriTex(ptype,ntype,ttype,prim,glVertFuncs,glCellFuncs,glInitFuncs) \
{ \
  vtkIdType nPts; unsigned short count = 0; \
  ptype *points = static_cast<ptype *>(voidPoints);     \
  GLenum previousGlFunction=GL_INVALID_VALUE; \
  glInitFuncs \
  \
double quad_center[3] = {0, 0, 0}; \
double quad_center_tex = 0; \
double quad_points[4][3]; \
double quad_points_tex[4]; \
double dist_center[4] = {0, 0, 0, 0}; \
  \
while (ptIds < endPtIds) \
    { \
    nPts = *ptIds; \
  ++ptIds; \
  /* If we don't want to draw a QUAD (ex : a triangle nPts = 3) */ \
  if (nPts != 4) { \
  /* Classic method */ \
    BeginPolyTriangleOrQuad( prim, previousGlFunction, nPts ); \
    glCellFuncs \
    while (nPts > 0) \
    { \
    glVertFuncs \
    ++ptIds; \
    --nPts; \
    } \
  } \
  /* If we want to draw a QUAD */ \
  else { \
    /* We launch glBegin(GL_TRIANGLES) mode in order to draw 4 triangles */ \
    BeginPolyTriangleOrQuad( prim, previousGlFunction, 3 ); \
    glCellFuncs \
    /* We keep pointer on the first point of the first triangle */ \
    /* ptIdsFirstPtQuad will be used for center calculation and for 2nd point of 4th triangle */ \
    vtkIdType *ptIdsFirstPtQuad; \
    ptIdsFirstPtQuad = ptIds; \
  /* QUAD Center calculation */ \
  /* We save the 4 QUAD points and their texture value */ \
    GLfloat *vpt; \
    GLfloat *vtex; \
    for (int i=0; i<4; i++) { \
      /* Position : */ \
      vpt = points + 3**ptIds; \
      quad_points[i][0] = vpt[0]; \
      quad_points[i][1] = vpt[1]; \
      quad_points[i][2] = vpt[2]; \
      /* Texture : */ \
      vtex = tcoords + *ptIds; \
      quad_points_tex[i] = vtex[0]; \
      ++ptIds; \
    } \
  /* Actual calculation of QUAD center with the 4 summits */ \
    quad_center[0] = (quad_points[0][0] + quad_points[1][0] + quad_points[2][0] + quad_points[3][0])/4; \
    quad_center[1] = (quad_points[0][1] + quad_points[1][1] + quad_points[2][1] + quad_points[3][1])/4; \
    quad_center[2] = (quad_points[0][2] + quad_points[1][2] + quad_points[2][2] + quad_points[3][2])/4; \
  /* Texture center calculation  (Interpolation on each component of RGB vector) */ \
  /* Calculation of distances between center and summits */ \
    for (int i=0; i<4; i++) { \
      dist_center[i] = sqrt((quad_points[i][0] - quad_center[0])*(quad_points[i][0] - quad_center[0]) + \
                (quad_points[i][1] - quad_center[1])*(quad_points[i][1] - quad_center[1]) + \
                (quad_points[i][2] - quad_center[2])*(quad_points[i][2] - quad_center[2])); \
    } \
  /* Texture interpolation */ \
    quad_center_tex = ((dist_center[3]*quad_points_tex[1] + dist_center[1]*quad_points_tex[3])/(dist_center[1] + dist_center[3]) + \
              (dist_center[2]*quad_points_tex[0] + dist_center[0]*quad_points_tex[2])/(dist_center[2] + dist_center[0]) \
              )/2; \
  /* We take pointer on the first QUAD point */ \
    ptIds = ptIdsFirstPtQuad; \
  /* Actual drawing of 4 triangles */ \
    for (int i=0; i<4; i++) { \
      /* 1st point */ \
      glVertFuncs \
      ++ptIds; \
      /* 2nd point */ \
      if (i >= 3) { /* If it is the last triangle */ \
        /* this 2nd point = the 1st point of 1st triangle */ \
        glTexCoord1fv(tcoords + *ptIdsFirstPtQuad); \
        glVertex3fv(points + 3**ptIdsFirstPtQuad); \
      } \
      else { \
        /* Else 2nd point = next point */ \
        glVertFuncs \
      } \
      /* 3rd point */ \
      glTexCoord1f(quad_center_tex); \
      glVertex3f(quad_center[0],quad_center[1],quad_center[2]); \
    } \
  } /* End of if (nPts == 4) */ \
    if (++count == 10000) \
      { \
      cellNum += 10000; \
      count = 0; \
      } \
    if ((previousGlFunction != GL_TRIANGLES)  \
        && (previousGlFunction != GL_QUADS)   \
        && (previousGlFunction != GL_POINTS)) \
      {  \
      glEnd(); \
      } \
    } \
  cellNum += count; \
  if ((previousGlFunction == GL_TRIANGLES)  \
      || (previousGlFunction == GL_QUADS)   \
      || (previousGlFunction == GL_POINTS)) \
    { \
    glEnd(); \
    } \
}


#define vtkDrawStripLinesMacro(ptype,ntype,ttype,prim,glVertFuncs,glCellFuncs,glInitFuncs) \
{ \
  vtkIdType nPts; \
  ptype *points = static_cast<ptype *>(voidPoints);     \
  vtkIdType *savedPtIds = ptIds; \
  glInitFuncs \
  while (ptIds < endPtIds) \
    { \
    glBegin(prim); \
    nPts = *ptIds; \
    ++ptIds; \
    glCellFuncs \
    while (nPts > 0) \
      { \
      glVertFuncs \
      ptIds += 2; \
      nPts -= 2; \
      } \
    glEnd(); \
    ptIds += nPts; /* nPts could be 0 or -1 here */ \
    } \
  ptIds = savedPtIds; \
  while (ptIds < endPtIds) \
    { \
    glBegin(prim); \
    nPts = *ptIds; \
    ++ptIds; \
    glCellFuncs \
    ++ptIds; \
    --nPts; \
    while (nPts > 0) \
      { \
      glVertFuncs \
      ptIds += 2; \
      nPts -= 2; \
      } \
    glEnd(); \
    ptIds += nPts; /* nPts could be 0 or -1 here */ \
    } \
}


void
avtOpenGLSurfaceAndWireframeRenderer::GenericDrawPoints(
    int idx,
    vtkPoints *p, 
    vtkDataArray *n,
    vtkUnsignedCharArray *c,
    vtkDataArray *t,
    vtkIdType &cellNum,
    vtkCellArray *ca)
{
    vtkIdType *pts = 0;
    vtkIdType npts = 0;
    glBegin(GL_POINTS);
    for (ca->InitTraversal(); ca->GetNextCell(npts,pts); )
    { 
        for (int j = 0; j < npts; j++) 
        {
            if (c)
            {
                if (idx & OGL_SAWR_CELL_COLORS)
                {
                    glColor4ubv(c->GetPointer(cellNum << 2));
                }
                else
                {
                    glColor4ubv(c->GetPointer(pts[j]<< 2));
                }
            }
            if (t)
            {
                if (idx & OGL_SAWR_TCOORD_1D)
                {
                    glTexCoord1dv(t->GetTuple(pts[j]));
                }
                else
                {
                    glTexCoord2dv(t->GetTuple(pts[j]));
                }
            }
            if (n)
            {
                if (idx & OGL_SAWR_CELL_NORMALS)
                {
                    glNormal3dv(n->GetTuple(cellNum));
                }
                else
                {
                    glNormal3dv(n->GetTuple(pts[j]));
                }
            }
            glVertex3dv(p->GetPoint(pts[j]));
        }
        ++cellNum;
    }
    glEnd();
}

void
avtOpenGLSurfaceAndWireframeRenderer::DrawPoints(
    int idx,
    vtkPoints *p, 
    vtkDataArray *n,
    vtkUnsignedCharArray *c,
    vtkDataArray *t,
    vtkIdType &cellNum,
    vtkCellArray *ca)
{
    void *voidPoints = p->GetVoidPointer(0);
    void *voidNormals = 0;
    unsigned char *colors = 0;
    if (ca->GetNumberOfCells() == 0)
    {
        return;
    }
    if (n)
    {
        voidNormals = n->GetVoidPointer(0);
    }
    if (c)
    {
        colors = c->GetPointer(0);
    }
  
    vtkIdType *ptIds = ca->GetPointer();
    vtkIdType *endPtIds = ptIds + ca->GetNumberOfConnectivityEntries();
    
    // draw all the elements, use fast path if available
    switch (idx)
    {
        case OGL_SAWR_POINT_TYPE_FLOAT:
            vtkDrawPointsMacro(float, float, glVertex3fv(points + 3**ptIds);,;);
            break;
        case OGL_SAWR_POINT_TYPE_DOUBLE:
            vtkDrawPointsMacro(double,float, glVertex3dv(points + 3**ptIds);,;);
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT|OGL_SAWR_NORMAL_TYPE_FLOAT|OGL_SAWR_NORMALS:
            vtkDrawPointsMacro(float, float, 
                    glNormal3fv(normals + 3**ptIds);
                    glVertex3fv(points + 3**ptIds);,
                    float *normals = static_cast<float *>(voidNormals););
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT | OGL_SAWR_COLORS:
            vtkDrawPointsMacro(float, float, 
                               glColor4ubv(colors + 4**ptIds);
                               glVertex3fv(points + 3**ptIds);,;);
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT | OGL_SAWR_COLORS | OGL_SAWR_OPAQUE_COLORS:
            vtkDrawPointsMacro(float, float, 
                               glColor3ubv(colors + 4**ptIds);
                               glVertex3fv(points + 3**ptIds);,;);      
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT | OGL_SAWR_NORMAL_TYPE_FLOAT |
             OGL_SAWR_NORMALS | OGL_SAWR_COLORS:
            vtkDrawPointsMacro(float, float, 
                    glNormal3fv(normals + 3**ptIds);
                    glColor4ubv(colors + 4**ptIds);
                    glVertex3fv(points + 3**ptIds);,
                    float *normals = static_cast<float *>(voidNormals););
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT | OGL_SAWR_NORMAL_TYPE_FLOAT |
             OGL_SAWR_NORMALS | OGL_SAWR_COLORS | OGL_SAWR_OPAQUE_COLORS:
            vtkDrawPointsMacro(float, float, 
                    glNormal3fv(normals + 3**ptIds);
                    glColor3ubv(colors + 4**ptIds);
                    glVertex3fv(points + 3**ptIds);,
                    float *normals = static_cast<float *>(voidNormals););
            break;
        default:
            GenericDrawPoints(idx, p, n, c, t, cellNum, ca);
            break;
    }
}


void
avtOpenGLSurfaceAndWireframeRenderer::GenericDrawLines(
    int idx,
    vtkPoints *p, 
    vtkDataArray *n,
    vtkUnsignedCharArray *c,
    vtkDataArray *t,
    vtkIdType &cellNum,
    vtkCellArray *ca)
{
    vtkIdType *pts = 0;
    vtkIdType npts = 0;
    for (ca->InitTraversal(); ca->GetNextCell(npts,pts); )
    { 
        glBegin(GL_LINE_STRIP);
        for (int j = 0; j < npts; j++) 
        {
            if (c)
            {
                if (idx & OGL_SAWR_CELL_COLORS)
                {
                    glColor4ubv(c->GetPointer(cellNum << 2));
                }
                else
                {
                    glColor4ubv(c->GetPointer(pts[j] << 2));
                }
            }
            if (t)
            {
                if (idx & OGL_SAWR_TCOORD_1D)
                {
                    glTexCoord1dv(t->GetTuple(pts[j]));
                }
                else
                {
                    glTexCoord2dv(t->GetTuple(pts[j]));
                }
            }
            if (n)
            {
                if (idx & OGL_SAWR_CELL_NORMALS)
                {
                    glNormal3dv(n->GetTuple(cellNum));
                }
                else
                {
                    glNormal3dv(n->GetTuple(pts[j]));
                }
            }
            glVertex3dv(p->GetPoint(pts[j]));
        }
        glEnd();
      
        ++cellNum;
    }
}  

void
avtOpenGLSurfaceAndWireframeRenderer::DrawLines(
    int idx,
    vtkPoints *p, 
    vtkDataArray *n,
    vtkUnsignedCharArray *c,
    vtkDataArray *t,
    vtkIdType &cellNum,
    vtkCellArray *ca)
{
    void *voidPoints = p->GetVoidPointer(0);
    void *voidNormals = 0;
    void *voidTCoords = 0;
    unsigned char *colors = 0;
    if (ca->GetNumberOfCells() == 0)
    {
        return;
    }
    if (n)
    {
        voidNormals = n->GetVoidPointer(0);
    }
    if (t)
    {
        voidTCoords = t->GetVoidPointer(0);
    }
    if (c)
    {
        colors = c->GetPointer(0);
    }
    vtkIdType *ptIds = ca->GetPointer();
    vtkIdType *endPtIds = ptIds + ca->GetNumberOfConnectivityEntries();
  
    // draw all the elements, use fast path if available
    switch (idx)
    {
        case OGL_SAWR_POINT_TYPE_FLOAT:
            vtkDrawPrimsMacro(float, float, GL_LINE_STRIP, 
                              glVertex3fv(points + 3**ptIds);,;);
            break;
        case OGL_SAWR_POINT_TYPE_DOUBLE:
            vtkDrawPrimsMacro(double, float, GL_LINE_STRIP,
                              glVertex3dv(points + 3**ptIds);,;);
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT|OGL_SAWR_NORMAL_TYPE_FLOAT|OGL_SAWR_NORMALS:
            vtkDrawPrimsMacro(float, float, GL_LINE_STRIP,
                    glNormal3fv(normals + 3**ptIds);
                    glVertex3fv(points + 3**ptIds);,
                    float *normals = static_cast<float *>(voidNormals););
      
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT | OGL_SAWR_COLORS:
            vtkDrawPrimsMacro(float, float, GL_LINE_STRIP,
                              glColor4ubv(colors + 4**ptIds);
                              glVertex3fv(points + 3**ptIds);,;);
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT | OGL_SAWR_COLORS | OGL_SAWR_OPAQUE_COLORS:
            vtkDrawPrimsMacro(float, float, GL_LINE_STRIP,
                              glColor3ubv(colors + 4**ptIds);
                              glVertex3fv(points + 3**ptIds);,;);
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT | OGL_SAWR_NORMAL_TYPE_FLOAT | 
             OGL_SAWR_NORMALS | OGL_SAWR_COLORS:
            vtkDrawPrimsMacro(float, float, GL_LINE_STRIP,
                    glNormal3fv(normals + 3**ptIds);
                    glColor4ubv(colors + 4**ptIds);
                    glVertex3fv(points + 3**ptIds);,
                    float *normals = static_cast<float *>(voidNormals););
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT | OGL_SAWR_NORMAL_TYPE_FLOAT | 
             OGL_SAWR_NORMALS | OGL_SAWR_COLORS  | OGL_SAWR_OPAQUE_COLORS:
            vtkDrawPrimsMacro(float, float, GL_LINE_STRIP,
                    glNormal3fv(normals + 3**ptIds);
                    glColor3ubv(colors + 4**ptIds);
                    glVertex3fv(points + 3**ptIds);,
                    float *normals = static_cast<float *>(voidNormals););
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT | 
             OGL_SAWR_TCOORD_TYPE_FLOAT | OGL_SAWR_TCOORD_1D | OGL_SAWR_TCOORDS:
            vtkDrawPrimsMacro(float, float, GL_LINE_STRIP, 
                    glTexCoord1fv(tcoords + *ptIds);
                    glVertex3fv(points + 3**ptIds);,
                    float *tcoords = static_cast<float *>(voidTCoords););
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT | 
             OGL_SAWR_NORMAL_TYPE_FLOAT | OGL_SAWR_NORMALS |
             OGL_SAWR_TCOORD_TYPE_FLOAT | OGL_SAWR_TCOORD_1D | OGL_SAWR_TCOORDS:
            vtkDrawPrimsMacro(float, float, GL_LINE_STRIP, 
                    glNormal3fv(normals + 3**ptIds);
                    glTexCoord1fv(tcoords + *ptIds);
                    glVertex3fv(points + 3**ptIds);,
                    float *tcoords = static_cast<float *>(voidTCoords);
                    float *normals = static_cast<float *>(voidNormals););
            break;
        default:
            GenericDrawLines(idx,p, n, c, t, cellNum, ca);
            break;
    }
}



#define PolyNormal \
{ double polyNorm[3]; vtkPolygon::ComputeNormal(p,nPts,ptIds,polyNorm); glNormal3dv(polyNorm); }


void 
avtOpenGLSurfaceAndWireframeRenderer::GenericDrawPolygons(
    int idx,
    vtkPoints *p, 
    vtkDataArray *n,
    vtkUnsignedCharArray *c,
    vtkDataArray *t,
    vtkIdType &cellNum,
    GLenum rep,
    vtkCellArray *ca)
{
    vtkIdType *pts = 0;
    vtkIdType npts = 0;
    for (ca->InitTraversal(); ca->GetNextCell(npts,pts); )
    { 
        glBegin(rep);
        if (!n)
        { 
            double polyNorm[3]; 
            vtkPolygon::ComputeNormal(p,npts,pts,polyNorm); 
            glNormal3dv(polyNorm);
        }
        for (int j = 0; j < npts; j++) 
        {
            if (c)
            {
                if (idx & OGL_SAWR_CELL_COLORS)
                {
                    glColor4ubv(c->GetPointer(cellNum << 2));
                }
                else
                {
                    glColor4ubv(c->GetPointer(pts[j] << 2));
                }
            }
            if (t)
            {
                if (idx & OGL_SAWR_TCOORD_1D)
                {
                    glTexCoord1dv(t->GetTuple(pts[j]));
                }
                else
                {
                    glTexCoord2dv(t->GetTuple(pts[j]));
                }
            }
            if (n)
            {
                if (idx & OGL_SAWR_CELL_NORMALS)
                {
                    glNormal3dv(n->GetTuple(cellNum));
                }
                else
                {
                    glNormal3dv(n->GetTuple(pts[j]));
                }
            }
            glVertex3dv(p->GetPoint(pts[j]));
        }
        glEnd();
        ++cellNum;
    }
}

void 
avtOpenGLSurfaceAndWireframeRenderer::DrawPolygons(
    int idx,
    vtkPoints *p, 
    vtkDataArray *n,
    vtkUnsignedCharArray *c,
    vtkDataArray *t,
    vtkIdType &cellNum,
    GLenum rep,
    vtkCellArray *ca)
{
    void *voidPoints = p->GetVoidPointer(0);
    void *voidNormals = 0;
    void *voidTCoords = 0;
    unsigned char *colors = 0;
    if (ca->GetNumberOfCells() == 0)
    {
        return;
    }
    if (n)
    {
        voidNormals = n->GetVoidPointer(0);
    }
    if (c)
    {
        colors = c->GetPointer(0);
        // if these are cell colors then advance to the first cell
        if (idx & OGL_SAWR_CELL_COLORS)
        {
            colors = colors + cellNum*4;
        }
    }
    if (t)
    {
        voidTCoords = t->GetVoidPointer(0);
    }
    vtkIdType *ptIds = ca->GetPointer();
    vtkIdType *endPtIds = ptIds + ca->GetNumberOfConnectivityEntries();

    // draw all the elements, use fast path if available
    switch (idx)
    {
        case OGL_SAWR_POINT_TYPE_FLOAT:
            vtkDrawPolysMacro(float, float, float, rep, 
                    glVertex3fv(points + 3**ptIds);, 
                    PolyNormal,;);
            break;
        case OGL_SAWR_POINT_TYPE_DOUBLE:
            vtkDrawPolysMacro(double, float, float, rep, 
                    glVertex3dv(points + 3**ptIds);,
                    PolyNormal,;);
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT|OGL_SAWR_NORMAL_TYPE_FLOAT|OGL_SAWR_NORMALS:
            vtkDrawPolysMacro(float, float, float, rep,
                    glNormal3fv(normals + 3**ptIds);
                    glVertex3fv(points + 3**ptIds);,;,
                    float *normals = static_cast<float *>(voidNormals););
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT | OGL_SAWR_COLORS:
            vtkDrawPolysMacro(float, float, float, rep,
                    glColor4ubv(colors + 4**ptIds);
                    glVertex3fv(points + 3**ptIds);, 
                    PolyNormal,;);
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT | OGL_SAWR_COLORS  | OGL_SAWR_OPAQUE_COLORS:
            vtkDrawPolysMacro4Tri(float, float, float, rep, 
                    glColor3ubv(colors + 4**ptIds);
                    glVertex3fv(points + 3**ptIds);, 
                    PolyNormal,;);
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT | OGL_SAWR_NORMAL_TYPE_FLOAT | 
             OGL_SAWR_NORMALS | OGL_SAWR_COLORS:
            vtkDrawPolysMacro(float, float, float, rep, 
                    glNormal3fv(normals + 3**ptIds);
                    glColor4ubv(colors + 4**ptIds);
                    glVertex3fv(points + 3**ptIds);,;,
                    float *normals = static_cast<float *>(voidNormals););
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT | OGL_SAWR_NORMAL_TYPE_FLOAT | 
             OGL_SAWR_NORMALS | OGL_SAWR_COLORS  | OGL_SAWR_OPAQUE_COLORS:
            vtkDrawPolysMacro(float, float, float, rep, 
                    glNormal3fv(normals + 3**ptIds); 
                    glColor3ubv(colors + 4**ptIds);
                    glVertex3fv(points + 3**ptIds);,;, 
                    float *normals = static_cast<float *>(voidNormals););
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT | OGL_SAWR_NORMAL_TYPE_FLOAT | 
             OGL_SAWR_NORMALS | OGL_SAWR_TCOORD_TYPE_FLOAT | OGL_SAWR_TCOORD_1D | 
             OGL_SAWR_TCOORDS:
            vtkDrawPolysMacro(float, float, float, rep, 
                    glNormal3fv(normals + 3**ptIds);
                    glTexCoord1fv(tcoords + *ptIds);
                    glVertex3fv(points + 3**ptIds);,;,
                    float *normals = static_cast<float *>(voidNormals);
                    float *tcoords = static_cast<float *>(voidTCoords););
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT | OGL_SAWR_NORMAL_TYPE_FLOAT | 
             OGL_SAWR_CELL_NORMALS | OGL_SAWR_TCOORD_TYPE_FLOAT | 
             OGL_SAWR_TCOORD_1D | OGL_SAWR_TCOORDS:
            vtkDrawPolysMacro(float, float, float, rep, 
                    glTexCoord1fv(tcoords + *ptIds);
                    glVertex3fv(points + 3**ptIds);,
                    glNormal3fv(normals); normals += 3;,
                    float *tcoords = static_cast<float *>(voidTCoords);
                    float *normals = static_cast<float *>(voidNormals);
                    normals += cellNum*3;);
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT | 
             OGL_SAWR_TCOORD_TYPE_FLOAT | OGL_SAWR_TCOORD_1D | OGL_SAWR_TCOORDS:
            vtkDrawPolysMacro4TriTex(float, float, float, rep, 
                    glTexCoord1fv(tcoords + *ptIds);
                    glVertex3fv(points + 3**ptIds);,
                    PolyNormal;,
                    float *tcoords = static_cast<float *>(voidTCoords););
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT | OGL_SAWR_NORMAL_TYPE_FLOAT | 
             OGL_SAWR_NORMALS | OGL_SAWR_TCOORD_TYPE_FLOAT | OGL_SAWR_TCOORDS:
            vtkDrawPolysMacro(float, float, float, rep, 
                    glNormal3fv(normals + 3**ptIds);
                    glTexCoord2fv(tcoords + 2**ptIds);
                    glVertex3fv(points + 3**ptIds);,;,
                    float *normals = static_cast<float *>(voidNormals);
                    float *tcoords = static_cast<float *>(voidTCoords););
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT | OGL_SAWR_NORMAL_TYPE_FLOAT |
             OGL_SAWR_CELL_NORMALS:
            vtkDrawPolysMacro(float, float, float, rep, 
                    glVertex3fv(points + 3**ptIds);, 
                    glNormal3fv(normals); normals += 3;,
                    float *normals = static_cast<float *>(voidNormals);
                    normals += cellNum*3;);
              break;
        case OGL_SAWR_POINT_TYPE_FLOAT | OGL_SAWR_NORMAL_TYPE_FLOAT |
             OGL_SAWR_CELL_NORMALS | OGL_SAWR_COLORS:
            vtkDrawPolysMacro(float, float, float, rep, 
                    glColor4ubv(colors + 4**ptIds);
                    glVertex3fv(points + 3**ptIds);,
                    glNormal3fv(normals); normals += 3;,
                    float *normals = static_cast<float *>(voidNormals);
                    normals += cellNum*3;);
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT | OGL_SAWR_NORMAL_TYPE_FLOAT |
             OGL_SAWR_CELL_NORMALS | OGL_SAWR_COLORS | OGL_SAWR_OPAQUE_COLORS:
            vtkDrawPolysMacro(float, float, float, rep,
                    glColor3ubv(colors + 4**ptIds);
                    glVertex3fv(points + 3**ptIds);,
                    glNormal3fv(normals); normals += 3;,
                    float *normals = static_cast<float *>(voidNormals);
                    normals += cellNum*3;);
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT | OGL_SAWR_NORMAL_TYPE_FLOAT | 
             OGL_SAWR_NORMALS | OGL_SAWR_COLORS | OGL_SAWR_CELL_COLORS:
            vtkDrawPolysMacro(float, float, float, rep, 
                    glNormal3fv(normals + 3**ptIds);
                    glVertex3fv(points + 3**ptIds);,
                    glColor4ubv(colors); colors += 4;,
                    float *normals = static_cast<float *>(voidNormals););
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT | OGL_SAWR_NORMAL_TYPE_FLOAT | 
             OGL_SAWR_NORMALS | OGL_SAWR_COLORS  | OGL_SAWR_OPAQUE_COLORS | 
             OGL_SAWR_CELL_COLORS:
            vtkDrawPolysMacro(float, float, float, rep,
                    glNormal3fv(normals + 3**ptIds);
                    glVertex3fv(points + 3**ptIds);,
                    glColor3ubv(colors); colors += 4;,
                    float *normals = static_cast<float *>(voidNormals););
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT | OGL_SAWR_NORMAL_TYPE_FLOAT | 
             OGL_SAWR_CELL_NORMALS | OGL_SAWR_COLORS | OGL_SAWR_CELL_COLORS:
            vtkDrawPolysMacro(float, float, float, rep, 
                    glVertex3fv(points + 3**ptIds);,
                    glNormal3fv(normals); normals += 3;
                    glColor4ubv(colors); colors += 4;,
                    float *normals = static_cast<float *>(voidNormals);
                    normals += cellNum*3;);
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT | OGL_SAWR_NORMAL_TYPE_FLOAT | 
             OGL_SAWR_CELL_NORMALS | OGL_SAWR_COLORS  | OGL_SAWR_OPAQUE_COLORS | 
             OGL_SAWR_CELL_COLORS:
            vtkDrawPolysMacro(float, float, float, rep,
                    glVertex3fv(points + 3**ptIds);,
                    glNormal3fv(normals); normals += 3;
                    glColor3ubv(colors); colors += 4;,
                    float *normals = static_cast<float *>(voidNormals);
                    normals += cellNum*3;);
            break;
        default:
            GenericDrawPolygons(idx, p, n, c, t, cellNum, rep, ca);
            break;
    }
}

// fix refs here
#define TStripNormal \
if ( vcount > 2) \
{ \
  if (vcount % 2) \
    { \
    normIdx[0] = ptIds[-2]; normIdx[1] = ptIds[0]; normIdx[2] = ptIds[-1]; \
    vtkTriangle::ComputeNormal(p, 3, normIdx, polyNorm); \
    } \
  else \
    { \
    normIdx[0] = ptIds[-2]; normIdx[1] = ptIds[-1]; normIdx[2] = ptIds[0]; \
    vtkTriangle::ComputeNormal(p, 3, normIdx, polyNorm); \
    } \
  glNormal3dv(polyNorm); \
} \
vcount++; 

#define TStripNormalStart \
  vtkTriangle::ComputeNormal(p, 3, ptIds, polyNorm); \
  glNormal3dv(polyNorm); int vcount = 0;


void 
avtOpenGLSurfaceAndWireframeRenderer::GenericDrawTStrips(
    int idx,
    vtkPoints *p, 
    vtkDataArray *n,
    vtkUnsignedCharArray *c,
    vtkDataArray *t,
    vtkIdType &cellNum,
    GLenum rep,
    vtkCellArray *ca)
{
    vtkIdType *ptIds = 0;
    vtkIdType nPts = 0;
    double polyNorm[3];
    vtkIdType normIdx[3];
    unsigned long coloroffset = cellNum;
    for (ca->InitTraversal(); ca->GetNextCell(nPts,ptIds); )
    { 
        glBegin(rep);
        vtkTriangle::ComputeNormal(p, 3, ptIds, polyNorm);
        glNormal3dv(polyNorm);
        for (int j = 0; j < nPts; j++) 
        {
            if (c)
            {
                if ( (idx & OGL_SAWR_USE_FIELD_DATA) && j>=2 )
                {
                    glColor4ubv(c->GetPointer(coloroffset << 2));
                    coloroffset++;
                }
                else if (idx & OGL_SAWR_CELL_COLORS)
                {
                    glColor4ubv(c->GetPointer(cellNum << 2));
                }
                else
                {
                    glColor4ubv(c->GetPointer(ptIds[j] << 2));
                }
            }
            if (t)
            {
                if (idx & OGL_SAWR_TCOORD_1D)
                {
                    glTexCoord1dv(t->GetTuple(ptIds[j]));
                }
                else
                {
                    glTexCoord2dv(t->GetTuple(ptIds[j]));
                }
            }
            if (n)
            {
                if (idx & OGL_SAWR_CELL_NORMALS)
                {
                    glNormal3dv(n->GetTuple(cellNum));
                }
                else
                {
                    glNormal3dv(n->GetTuple(ptIds[j]));
                }
            }
            else
            {
                if (j >= 2) 
                { 
                    if (j % 2) 
                    { 
                        normIdx[0] = ptIds[j-2]; normIdx[1] = ptIds[j]; 
                        normIdx[2] = ptIds[j-1]; 
                        vtkTriangle::ComputeNormal(p, 3, normIdx, polyNorm); 
                    } 
                    else 
                    { 
                        normIdx[0] = ptIds[j-2]; normIdx[1] = ptIds[j-1]; 
                        normIdx[2] = ptIds[j]; 
                        vtkTriangle::ComputeNormal(p, 3, normIdx, polyNorm); 
                    } 
                } 
                glNormal3dv(polyNorm);
            }
            glVertex3dv(p->GetPoint(ptIds[j]));
        }
        glEnd();

        ++cellNum;
    }
}


void
avtOpenGLSurfaceAndWireframeRenderer::DrawTStrips(
    int idx,
    vtkPoints *p, 
    vtkDataArray *n,
    vtkUnsignedCharArray *c,
    vtkDataArray *t,
    vtkIdType &cellNum,
    GLenum rep,
    vtkCellArray *ca)
{
    void *voidPoints = p->GetVoidPointer(0);
    void *voidNormals = 0;
    void *voidTCoords = 0;
    unsigned char *colors = 0;
    double polyNorm[3];
    vtkIdType normIdx[3];
  
    if (ca->GetNumberOfCells() == 0)
    {
        return;
    }
    if (n)
    {
        voidNormals = n->GetVoidPointer(0);
    }
    if (c)
    {
        colors = c->GetPointer(0);
    }
    if (t)
    {
        voidTCoords = t->GetVoidPointer(0);
    }
    vtkIdType *ptIds = ca->GetPointer();
    vtkIdType *endPtIds = ptIds + ca->GetNumberOfConnectivityEntries();

    // draw all the elements, use fast path if available
    switch (idx)
    {
        case OGL_SAWR_POINT_TYPE_FLOAT:
            vtkDrawPolysMacro(float, float, float, rep, 
                        TStripNormal glVertex3fv(points + 3**ptIds);, 
                        TStripNormalStart,;);
            break;
        case OGL_SAWR_POINT_TYPE_DOUBLE:
            vtkDrawPolysMacro(double, float, float, rep, 
                        TStripNormal glVertex3dv(points + 3**ptIds);,
                        TStripNormalStart,;);
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT|OGL_SAWR_NORMAL_TYPE_FLOAT|OGL_SAWR_NORMALS:
            vtkDrawPolysMacro(float, float, float, rep,
                        glNormal3fv(normals + 3**ptIds);
                        glVertex3fv(points + 3**ptIds);,;,
                        float *normals = static_cast<float *>(voidNormals););
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT | OGL_SAWR_COLORS:
            vtkDrawPolysMacro(float, float, float, rep, 
                        TStripNormal 
                        glColor4ubv(colors + (*ptIds << 2));
                        glVertex3fv(points + 3**ptIds);,
                        TStripNormalStart,;);
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT | OGL_SAWR_COLORS  | OGL_SAWR_OPAQUE_COLORS:
            vtkDrawPolysMacro(float, float, float, rep, 
                        TStripNormal 
                        glColor3ubv(colors + (*ptIds << 2));
                        glVertex3fv(points + 3**ptIds);,
                        TStripNormalStart,;);
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT | OGL_SAWR_NORMAL_TYPE_FLOAT | 
             OGL_SAWR_NORMALS | OGL_SAWR_COLORS:
            vtkDrawPolysMacro(float, float, float, rep, 
                        glNormal3fv(normals + 3**ptIds);
                        glColor4ubv(colors + (*ptIds << 2));
                        glVertex3fv(points + 3**ptIds);,;, 
                        float *normals = static_cast<float *>(voidNormals););
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT | OGL_SAWR_NORMAL_TYPE_FLOAT | 
             OGL_SAWR_NORMALS | OGL_SAWR_COLORS  | OGL_SAWR_OPAQUE_COLORS:
            vtkDrawPolysMacro(float, float, float, rep, 
                        glNormal3fv(normals + 3**ptIds);
                        glColor3ubv(colors + (*ptIds << 2));
                        glVertex3fv(points + 3**ptIds);,;, 
                        float *normals = static_cast<float *>(voidNormals););
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT | OGL_SAWR_NORMAL_TYPE_FLOAT | 
             OGL_SAWR_NORMALS | OGL_SAWR_TCOORD_1D | OGL_SAWR_TCOORD_TYPE_FLOAT | 
             OGL_SAWR_TCOORDS:
            vtkDrawPolysMacro(float, float, float, rep, 
                        glNormal3fv(normals + 3**ptIds);
                        glTexCoord1fv(tcoords + *ptIds);
                        glVertex3fv(points + 3**ptIds);,;, 
                        float *normals = static_cast<float *>(voidNormals);
                        float *tcoords = static_cast<float *>(voidTCoords););
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT | OGL_SAWR_NORMAL_TYPE_FLOAT | 
             OGL_SAWR_NORMALS | OGL_SAWR_TCOORD_TYPE_FLOAT | OGL_SAWR_TCOORDS:
            vtkDrawPolysMacro(float, float, float, rep, 
                        glNormal3fv(normals + 3**ptIds);
                        glTexCoord2fv(tcoords + 2**ptIds);
                        glVertex3fv(points + 3**ptIds);,;, 
                        float *normals = static_cast<float *>(voidNormals);
                        float *tcoords = static_cast<float *>(voidTCoords););
            break;
        default:
            GenericDrawTStrips(idx, p, n, c, t, cellNum, rep, ca);
            break;
    }
}


void
avtOpenGLSurfaceAndWireframeRenderer::GenericDrawTStripLines(
    int idx,
    vtkPoints *p, 
    vtkDataArray *n,
    vtkUnsignedCharArray *c,
    vtkDataArray *t,
    vtkIdType &cellNum,
    GLenum rep,
    vtkCellArray *ca)
{
    vtkIdType *ptIds = 0;
    vtkIdType nPts = 0;
    unsigned long coloroffset = cellNum;
    double polyNorm[3];
    vtkIdType normIdx[3];
    for (ca->InitTraversal(); ca->GetNextCell(nPts,ptIds); )
    { 
        glBegin(rep);
        for (int j = 0; j < nPts; j += 2) 
        {
            if (c)
            {
                if ( (idx & OGL_SAWR_USE_FIELD_DATA) && j >= 2)
                {
                    glColor4ubv(c->GetPointer((coloroffset+j) << 2));
                }
                else if (idx & OGL_SAWR_CELL_COLORS)
                {
                    glColor4ubv(c->GetPointer(cellNum << 2));
                }
                else
                {
                    glColor4ubv(c->GetPointer(ptIds[j] << 2));
                }
            }
            if (t)
            {
                if (idx & OGL_SAWR_TCOORD_1D)
                {
                    glTexCoord1dv(t->GetTuple(ptIds[j]));
                }
                else
                {
                    glTexCoord2dv(t->GetTuple(ptIds[j]));
                }
            }
            if (n)
            {
                if (idx & OGL_SAWR_CELL_NORMALS)
                {
                    glNormal3dv(n->GetTuple(cellNum));
                }
                else
                {
                    glNormal3dv(n->GetTuple(ptIds[j]));
                }
            }
            else
            {
                if ( j == 0 )
                {
                    vtkTriangle::ComputeNormal(p, 3, ptIds, polyNorm);
                }
                else
                {
                    normIdx[0] = ptIds[j-2]; normIdx[1] = ptIds[j-1]; 
                    normIdx[2] = ptIds[j]; 
                    vtkTriangle::ComputeNormal(p, 3, normIdx, polyNorm);
                }
                glNormal3dv(polyNorm);
            }
            glVertex3dv(p->GetPoint(ptIds[j]));
        }
        glEnd();
      
        glBegin(rep);
        for (int j = 1; j < nPts; j += 2) 
        {
            if (c)
            {
                if ( (idx & OGL_SAWR_USE_FIELD_DATA) && j >= 2)
                {
                    glColor4ubv(c->GetPointer((coloroffset+j) << 2));
                }
                else if (idx & OGL_SAWR_CELL_COLORS)
                {
                    glColor4ubv(c->GetPointer(cellNum << 2));
                }
                else
                {
                    glColor4ubv(c->GetPointer(ptIds[j] << 2));
                }
            }
            if (t)
            {
                if (idx & OGL_SAWR_TCOORD_1D)
                {
                    glTexCoord1dv(t->GetTuple(ptIds[j]));
                }
                else
                {
                    glTexCoord2dv(t->GetTuple(ptIds[j]));
                }
            }
            if (n)
            {
                if (idx & OGL_SAWR_CELL_NORMALS)
                {
                    glNormal3dv(n->GetTuple(cellNum));
                }
                else
                {
                    glNormal3dv(n->GetTuple(ptIds[j]));
                }
            }
            else
            {
                if (j == 1)
                {
                    vtkTriangle::ComputeNormal(p, 3, ptIds, polyNorm);
                }
                else
                {
                    normIdx[0] = ptIds[j-2]; normIdx[1] = ptIds[j]; 
                    normIdx[2] = ptIds[j-1]; 
                    vtkTriangle::ComputeNormal(p, 3, normIdx, polyNorm);
                }
                glNormal3dv(polyNorm);
            }
            glVertex3dv(p->GetPoint(ptIds[j]));
        }
        glEnd();

        ++cellNum;
        coloroffset += (nPts >= 2)? (nPts - 2) : 0;
    }
}


void 
avtOpenGLSurfaceAndWireframeRenderer::DrawTStripLines(
    int idx,
    vtkPoints *p, 
    vtkDataArray *n,
    vtkUnsignedCharArray *c,
    vtkDataArray *t,
    vtkIdType &cellNum,
    GLenum rep,
    vtkCellArray *ca)
{
    void *voidPoints = p->GetVoidPointer(0);
    void *voidNormals = 0;
    void *voidTCoords = 0;
    unsigned char *colors = 0;
    double polyNorm[3];
    vtkIdType normIdx[3];
  
    if (n)
    {
        voidNormals = n->GetVoidPointer(0);
    }
    if (c)
    {
        colors = c->GetPointer(0);
    }
    if (t)
    {
        voidTCoords = t->GetVoidPointer(0);
    }
    vtkIdType *ptIds = ca->GetPointer();
    vtkIdType *endPtIds = ptIds + ca->GetNumberOfConnectivityEntries();

    // draw all the elements, use fast path if available
    switch (idx)
    {
        case OGL_SAWR_POINT_TYPE_FLOAT:
            vtkDrawStripLinesMacro(float, float, float, rep, 
                    TStripNormal; glVertex3fv(points + 3**ptIds);, 
                    TStripNormalStart,;);
            break;
        case OGL_SAWR_POINT_TYPE_DOUBLE:
            vtkDrawStripLinesMacro(double, float, float, rep, 
                    TStripNormal glVertex3dv(points + 3**ptIds);, 
                    TStripNormalStart,;);
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT|OGL_SAWR_NORMAL_TYPE_FLOAT|OGL_SAWR_NORMALS:
            vtkDrawStripLinesMacro(float, float, float, rep, 
                    glNormal3fv(normals + 3**ptIds);
                    glVertex3fv(points + 3**ptIds);,;,
                    float *normals = static_cast<float *>(voidNormals););
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT | OGL_SAWR_COLORS:
            vtkDrawStripLinesMacro(float, float, float, rep, 
                    TStripNormal;
                    glColor4ubv(colors + 4**ptIds);
                    glVertex3fv(points + 3**ptIds);,
                    TStripNormalStart,;);
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT | OGL_SAWR_COLORS | OGL_SAWR_OPAQUE_COLORS:
            vtkDrawStripLinesMacro(float, float, float, rep, 
                    TStripNormal;
                    glColor3ubv(colors + 4**ptIds);
                    glVertex3fv(points + 3**ptIds);, 
                    TStripNormalStart,;);
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT | OGL_SAWR_NORMAL_TYPE_FLOAT | 
             OGL_SAWR_NORMALS | OGL_SAWR_COLORS:
            vtkDrawStripLinesMacro(float, float, float, rep, 
                    glNormal3fv(normals + 3**ptIds);
                    glColor4ubv(colors + 4**ptIds);
                    glVertex3fv(points + 3**ptIds);,;,
                    float *normals = static_cast<float *>(voidNormals););
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT | OGL_SAWR_NORMAL_TYPE_FLOAT | 
             OGL_SAWR_NORMALS | OGL_SAWR_COLORS | OGL_SAWR_OPAQUE_COLORS:
            vtkDrawStripLinesMacro(float, float, float, rep, 
                    glNormal3fv(normals + 3**ptIds); 
                    glColor3ubv(colors + 4**ptIds);
                    glVertex3fv(points + 3**ptIds);,;,
                    float *normals = static_cast<float *>(voidNormals););
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT | OGL_SAWR_NORMAL_TYPE_FLOAT | 
             OGL_SAWR_NORMALS | OGL_SAWR_TCOORD_TYPE_FLOAT | OGL_SAWR_TCOORD_1D | 
             OGL_SAWR_TCOORDS:
            vtkDrawStripLinesMacro(float, float, float, rep, 
                    glNormal3fv(normals + 3**ptIds);
                    glTexCoord1fv(tcoords + *ptIds);
                    glVertex3fv(points + 3**ptIds);,;,
                    float *normals = static_cast<float *>(voidNormals);
                    float *tcoords = static_cast<float *>(voidTCoords););
            break;
        case OGL_SAWR_POINT_TYPE_FLOAT | OGL_SAWR_NORMAL_TYPE_FLOAT | 
             OGL_SAWR_NORMALS | OGL_SAWR_TCOORD_TYPE_FLOAT | OGL_SAWR_TCOORDS:
            vtkDrawStripLinesMacro(float, float, float, rep, 
                    glNormal3fv(normals + 3**ptIds);
                    glTexCoord2fv(tcoords + 2**ptIds);
                    glVertex3fv(points + 3**ptIds);,;,
                    float *normals = static_cast<float *>(voidNormals);
                    float *tcoords = static_cast<float *>(voidTCoords););
            break;
        default:
            GenericDrawTStripLines(idx, p, n, c, t, cellNum, rep, ca);
            break;
    }
}


// ****************************************************************************
// Method: avtOpenGLSurfaceAndWireframeRenderer::SetUpGraphicsLibrary
//
// Purpose:
//   Generates display lists as necessary. 
//
// Programmer: Kathleen Bonnell 
// Creation:   June 13, 2002 
//
// Modifications:
//
//   Hank Childs, Wed Apr 23 22:17:03 PDT 2003
//   Make sure we are dealing with the correct context or we may accidentally
//   delete the display lists for the wrong window.
//
//   Hank Childs, Mon May  5 18:17:23 PDT 2003
//   Do not assume that we have a valid VTK renderer.
//
//   Hank Childs, Tue Jun  1 09:35:46 PDT 2004
//   Make sure that a display list has been created before it is called.
//   Also re-organize code to be more intuitive for display lists.
//
//   Kathleen Biagas, Fri Mar 09 15:36:32 PST 2012
//   Delete the list if it's ID is NOT zero.
//
// ****************************************************************************

void 
avtOpenGLSurfaceAndWireframeRenderer::SetupGraphicsLibrary()
{
    if (VTKRen == NULL)
    {
        debug1 << "Asked to set up graphics lib when we don't have a renderer."
               << endl;
        return;
    }

    //
    //  Make sure we have a display list index for this input. 
    //
    if ((size_t)inputNum >= setupListId.size())
        setupListId.push_back(0);

    VTKRen->GetRenderWindow()->MakeCurrent();

    if (immediateModeRendering) 
    {
        SetupGraphicsLibrary2();
    }
    else
    {
        //
        // If the plot has changed or we haven't created a display list, then
        // do that now.
        //
        if (setupModified[inputNum] || (propMTime[inputNum] < prop->GetMTime())
            || (setupListId[inputNum] == 0))
        {
            if (setupListId[inputNum] != 0)
            {
                glDeleteLists(setupListId[inputNum], 1);
            }

            // 
            // Get a unique display list id  
            // 
            setupListId[inputNum] = glGenLists(1);
            glNewList(setupListId[inputNum], GL_COMPILE);
            SetupGraphicsLibrary2();
            glEndList();
        }

#ifdef DEBUG_GL_LIST_IDS
        if (!glIsList(setupListId[inputNum]))
           cerr << "calling invalid setup list" << endl;
#endif
        glCallList(setupListId[inputNum]);
    }
}


// ****************************************************************************
// Method: avtOpenGLSurfaceAndWireframeRenderer::SetupGraphicsLibrary
//
// Purpose:
//   Makes various gl calls in preparation for the Draw method. 
//
// Programmer: Kathleen Bonnell 
// Creation:   Aug 21, 2001 
//
// Modifications:
//   Kathleen Bonnell, Wed Aug 22 15:06:29 PDT 2001
//   Initialize Info[3] -- the alpha value for gl colors.  Having
//   this unitialized caused havoc when user turned off lighting.
//
//   Kathleen Bonnell, Thu Jun 13 10:26:49 PDT 2002    
//   Renamed SetupGraphicsLibrary2. 
//   
//   Kathleen Bonnell, Tue Aug 13 15:15:37 PDT 2002 
//   Modified so that glColorMaterial is enabled only if there are colors
//   and this renderer scalarVisibility is on.  
//   
// ****************************************************************************

void 
avtOpenGLSurfaceAndWireframeRenderer::SetupGraphicsLibrary2()
{
    int i;
    GLenum method;
    float Info[4];
    GLenum Face;
    double  color[4];

    // unbind any textures for starters
    glDisable(GL_TEXTURE_2D);

    // disable alpha testing (this may have been enabled
    // by another actor in OpenGLTexture)
    glDisable(GL_ALPHA_TEST);

    Face = GL_FRONT_AND_BACK;
    // turn on/off backface culling
    if (!prop->GetBackfaceCulling() && !prop->GetFrontfaceCulling())
    {
        glDisable(GL_CULL_FACE);
    }
    else if (prop->GetBackfaceCulling())
    {
        glCullFace(GL_BACK);
        glEnable(GL_CULL_FACE);
    }
    else //if both front & back culling on, will fall into backface culling
    { //if you really want both front and back, use the Actor's visibility flag
        glCullFace(GL_FRONT);
        glEnable(GL_CULL_FACE);
    }

    Info[3] = prop->GetOpacity();
    // set ambient color
    double coeff = prop->GetAmbient();
    prop->GetAmbientColor(color);
    for (i=0; i < 3; i++) 
    {
        Info[i] = coeff*color[i];
    }
    glMaterialfv( Face, GL_AMBIENT, Info );

    // set diffuse color
    coeff = prop->GetDiffuse();
    prop->GetDiffuseColor(color);
    for (i=0; i < 3; i++) 
    {
        Info[i] = coeff*color[i];
    }
    glMaterialfv( Face, GL_DIFFUSE, Info );

    // set specular color
    coeff = prop->GetSpecular();
    prop->GetSpecularColor(color);
    for (i=0; i < 3; i++) 
    {
        Info[i] = coeff*color[i];
    }
    glMaterialfv( Face, GL_SPECULAR, Info );

    Info[0] = prop->GetSpecularPower();
    glMaterialfv(Face, GL_SHININESS, Info);

    // set interpolation 
    switch (prop->GetInterpolation()) 
    {
        case VTK_FLAT:     method = GL_FLAT; break;
        case VTK_GOURAUD:
        case VTK_PHONG:    method = GL_SMOOTH; break;
        default:           method = GL_SMOOTH; break;
    }
  
    glShadeModel(method);
    glDisable(GL_COLOR_MATERIAL);
    if (this->Colors && scalarVisibility)
    { 
        if (prop->GetAmbient() > prop->GetDiffuse())
        {
            glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT);
        }
        else
        {
            glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
        }
        glEnable(GL_COLOR_MATERIAL);
    }

    // The material properties set above are used if shading is
    // enabled. This color set here is used if shading is 
    // disabled. Shading is disabled in the Draw() methods if 
    // points or lines are encountered without normals. 
    prop->GetColor(color);
    color[3] = 1.0;
    glColor4dv(color);

    // Set the PointSize
    glPointSize(prop->GetPointSize());

    // Set the LineWidth
    glLineWidth(prop->GetLineWidth());

    // Set the LineStipple
    if (prop->GetLineStipplePattern() != 0xFFFF)
    {
        glEnable(GL_LINE_STIPPLE);
        glLineStipple(prop->GetLineStippleRepeatFactor(), 
                      prop->GetLineStipplePattern());
    }
    else
    {
        // In case it was set elsewhere
        glDisable(GL_LINE_STIPPLE);
    }
}


// ****************************************************************************
// Method: avtOpenGLSurfaceAndWireframeRenderer::DrawSurface
//
// Purpose:
//   Generates display lists as necessary. 
//
// Programmer: Kathleen Bonnell 
// Creation:   June 13, 2002 
//
// Modifications:
//   Kathleen Bonnell, Tue Nov 26 15:39:16 PST 2002
//   The display list needs to be regenerated if the lut colors have changed,
//   so added a test for this condition. 
//
//   Hank Childs, Wed Apr 23 22:17:03 PDT 2003
//   Make sure we are dealing with the correct context or we may accidentally
//   delete the display lists for the wrong window.
//
//   Hank Childs, Mon May  5 18:17:23 PDT 2003
//   Do not assume that we have a valid VTK renderer.
//
//   Hank Childs, Tue Jun  1 09:35:46 PDT 2004
//   Make sure that a display list has been created before it is called.
//   Also re-organize code to be more intuitive for display lists.
//
//   Kathleen Biagas, Fri Mar 09 15:36:32 PST 2012
//   Delete the list if it's ID is NOT zero.
//
// ****************************************************************************

void 
avtOpenGLSurfaceAndWireframeRenderer::DrawSurface()
{
    if (VTKRen == NULL)
    {
        debug1 << "Asked to draw surface when we don't have a renderer."
               << endl;
        return;
    }

    //
    //  Make sure we have a display list index for this input.
    //
    if ((size_t)inputNum >= surfaceListId.size())
        surfaceListId.push_back(0);

    VTKRen->GetRenderWindow()->MakeCurrent();

    if (immediateModeRendering)
    {
        DrawSurface2();
    }
    else
    {
        //
        // See if display list needs to be regenerated. 
        //
        if (  lutColorsChanged ||
            ((propMTime[inputNum] < prop->GetMTime()) &&
            ((lastRep[inputNum]   != prop->GetRepresentation()) ||
             (lastInterp[inputNum] != prop->GetInterpolation()) )))
               surfaceModified[inputNum] = true;

        if (surfaceModified[inputNum] || (surfaceListId[inputNum] == 0))
        {
            if (surfaceListId[inputNum])
            {
                glDeleteLists(surfaceListId[inputNum], 1);
            }

            // 
            // Get a unique display list id and create the display list.
            // 
            surfaceListId[inputNum] = glGenLists(1);
            glNewList(surfaceListId[inputNum], GL_COMPILE);
            DrawSurface2();
            glEndList();
        }

#ifdef DEBUG_GL_LIST_IDS
        if (!glIsList(surfaceListId[inputNum]))
           cerr << "calling invalid surface list" << endl;
#endif
        glCallList(surfaceListId[inputNum]);
    }
}


// ****************************************************************************
// Method: avtOpenGLSurfaceAndWireframeRenderer::DrawSurface
//
// Purpose:
//   Sets up for and calls the various helper 'Draw' methods that
//   will draw the surface appropriately. 
//
// Programmer: Kathleen Bonnell 
// Creation:   Aug 21, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Wed Aug 22 15:06:29 PDT 2001
//    Draw only cell types that are 'on'. 
//
//    Kathleen Bonnell, Mon Oct 29 15:27:41 PST 2001 
//    Use vtkUnsignedCharArray to represent colors, instead of vtkScalars, to
//    match VTK 4.0 API. Use proper access methods for this array.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Kathleen Bonnell, Thu Jun 13 10:26:49 PDT 2002    
//    Renamed to DrawSurface2.
//
//    Tom Fogal, Fri Jan 28 14:42:57 MST 2011
//    Typing fixes (vtkIdType).
//
//    Kathleen Biagas, Thu Mar 15 16:30:48 PDT 2012
//    Follow vtk 5.8's vtkOpenGLPolyDataMapper, Draw methods have been
//    consolidatd and make use of macro's that support multiple data types.
//
// ****************************************************************************

void 
avtOpenGLSurfaceAndWireframeRenderer::DrawSurface2()
{
    int rep;
    vtkPoints *p;
    vtkUnsignedCharArray *c=NULL;
    vtkDataArray *n;
    vtkDataArray *t;
    int tDim;
    int cellScalars = 0;
    vtkIdType cellNum = 0;
    int cellNormals = 0;
  
    // get the representation (e.g., surface / wireframe / points)
    rep = prop->GetRepresentation();

    // and draw the display list
    p = input->GetPoints();
  
    // are they cell or point scalars
    if (this->Colors && scalarVisibility)
    {
        c = this->Colors;
#if 1
        if (!input->GetPointData()->GetScalars() &&
             input->GetCellData()->GetScalars())
        {
            cellScalars = 1;
        }
#else
        // VTK 5.8 vtkOpenGLPolyDataMapper new logc, not sure if we should
        // use it.
        if ( (this->ScalarMode == VTK_SCALAR_MODE_USE_CELL_DATA ||
              this->ScalarMode == VTK_SCALAR_MODE_USE_CELL_FIELD_DATA ||
              this->ScalarMode == VTK_SCALAR_MODE_USE_FIELD_DATA ||
              !input->GetPointData()->GetScalars() )
             && this->ScalarMode != VTK_SCALAR_MODE_USE_POINT_FIELD_DATA)
        {
            cellScalars = 1;
        }
#endif
    }

    n = input->GetPointData()->GetNormals();
    if (prop->GetInterpolation() == VTK_FLAT)
    {
        n = 0;
    }
  
    cellNormals = 0;
    if (n == 0 && input->GetCellData()->GetNormals())
    {
        cellNormals = 1;
        n = input->GetCellData()->GetNormals();
    }
    
    t = input->GetPointData()->GetTCoords();
    if (t) 
    {
        tDim = t->GetNumberOfComponents();
        if (tDim > 2)
        {
            debug5 << "Currently only 1d and 2d textures are supported.\n";
            t = NULL;
        }
    }
 
    //
    //  Create an index that helps determine which drawing function to use.
    //
    unsigned long idx = 0;
    if (n && !cellNormals)
    {
        idx |= OGL_SAWR_NORMALS;
    }
    if (c)
    {
        idx |= OGL_SAWR_COLORS;
    }
    if (cellScalars)
    {
        idx |= OGL_SAWR_CELL_COLORS;
    }
    if (cellNormals)
    {
        idx |= OGL_SAWR_CELL_NORMALS;
    }

    // store the types in the index
    if (p->GetDataType() == VTK_FLOAT)
    {
        idx |= OGL_SAWR_POINT_TYPE_FLOAT;
    }
    else if (p->GetDataType() == VTK_DOUBLE)
    {
        idx |= OGL_SAWR_POINT_TYPE_DOUBLE;
    }

    if (n)
    {
        if (n->GetDataType() == VTK_FLOAT)
        {
            idx |= OGL_SAWR_NORMAL_TYPE_FLOAT;
        }
        else if (n->GetDataType() == VTK_DOUBLE)
        {
            idx |= OGL_SAWR_NORMAL_TYPE_DOUBLE;
        }
    }


#if 0
    // VTK 5.8 new logic, may want to incoorporate.

    // Set the texture if we are going to use texture
    // for coloring with a point attribute.
    // fixme ... make the existance of the coordinate array the signal.
    if (this->InterpolateScalarsBeforeMapping && this->ColorCoordinates &&
        ! (idx & OGL_SAWR_CELL_COLORS))
    {
        t = this->ColorCoordinates;
    }
#endif

    if (t)
    {
        idx |= OGL_SAWR_TCOORDS;
        if (t->GetDataType() == VTK_FLOAT)
        {
            idx |= OGL_SAWR_TCOORD_TYPE_FLOAT;
        }
        else if (t->GetDataType() == VTK_DOUBLE)
        {
            idx |= OGL_SAWR_TCOORD_TYPE_DOUBLE;
        }
        if (t->GetNumberOfComponents() == 1)
        {
            idx |= OGL_SAWR_TCOORD_1D;
        }
        // Not 1D assumes 2D texture coordinates.
    }
 
    if (resolveTopology) 
    {
#ifdef GL_VERSION_1_1
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.,1.);
#if 0
        // new vtk feature?? should we incorporate?
        double f, u;
        this->GetResolveCoincidentTopologyPolygonOffsetParameters(f,u);
        glPolygonOffset(f,u);
#endif      
#endif      
    }

    // For verts or lines that have no normals, disable shading.
    // This will fall back on the color set in the glColor4fv() 
    // call in vtkOpenGLProperty::Render() - the color returned
    // by vtkProperty::GetColor() with alpha set to 1.0.
    if (!n)
    {
        glDisable(GL_LIGHTING);
    }
  
    // do verts
    if (drawSurfaceVerts)
    {
        DrawPoints(idx,p,n,c,t,cellNum,input->GetVerts());
    }
  
    // do lines
  
    // draw all the elements
    if (drawSurfaceLines)
    {
        if (rep == VTK_POINTS)
            DrawPoints(idx,p,n,c,t,cellNum,input->GetLines());
        else 
            DrawLines(idx,p,n,c,t,cellNum,input->GetLines());
    }
  
    // reset the lighting if we turned it off
    if (!n)
    {
        glEnable(GL_LIGHTING);
    }

    // disable shading if we are rendering points, but have no normals
    if (!n && rep == VTK_POINTS)
    {
        glDisable(GL_LIGHTING);
    }
 
    // VTK does polys first??? should we switch to same? 
    // do tstrips
    if (drawSurfaceStrips)
    {
        if (rep == VTK_POINTS)
        {
            DrawPoints(idx,p,n,c,t,cellNum,input->GetStrips());
        }
        else if (rep == VTK_WIREFRAME)
        {
            vtkIdType oldCellNum = cellNum;
            DrawTStrips(idx,p,n,c,t,cellNum, 
                        GL_LINE_STRIP, input->GetStrips());
            DrawTStripLines(idx,p,n,c,t,oldCellNum, 
                        GL_LINE_STRIP, input->GetStrips());
        }
        else 
        {
            DrawTStrips(idx,p,n,c,t,cellNum,
                        GL_TRIANGLE_STRIP, input->GetStrips());
        }
    }

    // do polys
    if (drawSurfacePolys)
    {
        if (rep == VTK_POINTS)
            DrawPoints(idx,p,n,c,t,cellNum,input->GetPolys());
        else if (rep == VTK_WIREFRAME)
            DrawPolygons(idx,p,n,c,t,cellNum, GL_LINE_LOOP, input->GetPolys());
        else 
            DrawPolygons(idx,p,n,c,t,cellNum, GL_POLYGON, input->GetPolys());
    }

    // enable lighting again if necessary
    if (!n && rep == VTK_POINTS)
    {
        glEnable(GL_LIGHTING);
    }

    if (resolveTopology)
    {
#ifdef GL_VERSION_1_1
        glDisable(GL_POLYGON_OFFSET_FILL);
#endif
    }
}


// ****************************************************************************
// Method: avtOpenGLSurfaceAndWireframeRenderer::DrawEdges
//
// Purpose:
//   Generates display lists as necessary. 
//
// Programmer: Kathleen Bonnell 
// Creation:   June 13, 2002 
//
// Modifications:
//
//   Hank Childs, Wed Apr 23 22:17:03 PDT 2003
//   Make sure we are dealing with the correct context or we may accidentally
//   delete the display lists for the wrong window.
//
//   Hank Childs, Mon May  5 18:17:23 PDT 2003
//   Do not assume that we have a valid VTK renderer.
//
//   Hank Childs, Tue Jun  1 09:35:46 PDT 2004
//   Make sure that a display list has been created before it is called.
//   Also re-organize code to be more intuitive for display lists.
//
//   Eric Brugger, Wed Jun  2 13:59:24 PDT 2010
//   I moved the code that shifts edges toward the viewer in DrawEdges2
//   up in the call sequence to the routine DrawEdges so that it would
//   always be executed when DrawEdges is executed.
//
//   Hank Childs, Thu Mar  1 13:34:09 PST 2012
//   Change test for near/far planes to use the exact near & far values, rather
//   than reverse engineering them.  This change came about because the test
//   only worked for perspective transformations.
//
//   Kathleen Biagas, Fri Mar 09 15:36:32 PST 2012
//   Delete the list if it's ID is NOT zero.
//
// ****************************************************************************

void
avtOpenGLSurfaceAndWireframeRenderer::DrawEdges()
{
    if (VTKRen == NULL)
    {
        debug1 << "Asked to draw edges when we don't have a renderer." << endl;
        return;
    }

    //
    //  Make sure we have a display list index for this input. 
    //
    if ((size_t)inputNum >= edgesListId.size())
        edgesListId.push_back(0);

    VTKRen->GetRenderWindow()->MakeCurrent();

    //
    // If any edges in the list of edges are supposed to appear just
    // on top of polygons on the surface, We need to bring edges just
    // a little bit closer to the viewer. GL has some support for this
    // kind of thing through the glPolygonOffset and GL_POLYGON_OFFSET_FILL
    // settings. However, it is my understanding that that functionality
    // is specific to the case in which the same graphics primitves are
    // being drawn in two passes, once to produce polygon interiors and
    // a second time to produce polygon edges. That isn't really what
    // we're doing here. We are drawing an entirely different set of
    // primitives, lines. Granted, those lines may have been computed
    // as the edges of the polygons we've drawn previously. However, 
    // from the point of view GL and its display lists, they are totally
    // different lists of primitives. So, we can't rely on GL's
    // glPolygonOffset stuff to help us here.
    //
    bool didZShift = false;
    if (ShouldDrawSurface() && surfaceListId.size() > 0)
    {
        double farPlane = view.farPlane;
        double nearPlane = view.nearPlane;

        // compute a shift based upon total range in Z
        double zShift1 = (farPlane - nearPlane) / 1.0e+4;

        // compute a shift based upon distance between eye and near clip
        double zShift2 = nearPlane / 2.0;

        // use whatever shift is smaller
        double zShift = zShift1 < zShift2 ? zShift1 : zShift2;

        //
        // Modify the viewing transformation to shift things forward in Z
        //
        float current_matrix[16];
        glPushMatrix();
        glGetFloatv(GL_MODELVIEW_MATRIX, current_matrix);
        glLoadIdentity();
        glTranslatef(0.0, 0.0, zShift);
        glMultMatrixf(current_matrix);

        didZShift = true;
    }

    if (immediateModeRendering) 
    {
        DrawEdges2();
    }
    else
    {
        //
        // See if display list needs to be regenerated. 
        //
        if (propMTime[inputNum] < prop->GetMTime())
        {
            bool colorChanged = 
                 ((lastEdgeColor[inputNum].r != prop->GetEdgeColor()[0]) ||
                  (lastEdgeColor[inputNum].g != prop->GetEdgeColor()[1]) ||
                  (lastEdgeColor[inputNum].b != prop->GetEdgeColor()[2]) );
            if ( colorChanged || 
                (lastRep[inputNum] != prop->GetRepresentation()) ||
                (lastInterp[inputNum] != prop->GetInterpolation()))
               edgesModified[inputNum] = true;
        }
    
        if (edgesModified[inputNum] || (edgesListId[inputNum] == 0))
        {
            if (edgesListId[inputNum])
            {
                glDeleteLists(edgesListId[inputNum], 1);
            }

            // 
            // Get a unique display list id  
            // 
            edgesListId[inputNum] = glGenLists(1);
            glNewList(edgesListId[inputNum], GL_COMPILE);
            DrawEdges2();
            glEndList();
        }

#ifdef DEBUG_GL_LIST_IDS
        if (!glIsList(edgesListId[inputNum]))
           cerr << "calling invalid edges list" << endl;
#endif
        glCallList(edgesListId[inputNum]);
    }

    //
    // Undue changes we made to the view transform 
    //
    if (didZShift)
        glPopMatrix();
}

// ****************************************************************************
// Method: avtOpenGLSurfaceAndWireframeRenderer::DrawEdges
//
// Purpose:
//   Sets up for and calls the various helper 'Draw' methods that
//   will draw the edges appropriately. 
//
// Programmer: Kathleen Bonnell 
// Creation:   Aug 21, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Wed Aug 22 15:06:29 PDT 2001
//    Draw only cell types that are 'on'.  Removed extra calls
//    to enable/disable lighting, and unnecessary calls to
//    enable/disable polygon offset.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Kathleen Bonnell, Thu Jun 13 10:26:49 PDT 2002    
//    Renamed DrawEdges2. 
//    
//    Jeremy Meredith, Tue May  4 12:24:07 PDT 2004
//    Added support for un-glyphed point meshes (Verts in a vtkCellArray).
//
//    Mark C. Miller, Wed Aug 11 13:38:14 PDT 2004
//    Added a code to adjust glDepthRange and reset it upon exit
//
//    Kathleen Bonnell, Thu Sep  2 16:15:35 PDT 2004 
//    Added code to select the glFunction depending upon Representation.
//
//    Mark C. Miller, Mon Nov 22 17:42:22 PST 2004
//    Changed how line geometry is shifted relative to surface geometry.
//    Now, we use direct manipulation of the viewing transformation instead
//    of adjusting depth range.
//
//    Brad Whitlock, Tue Jan 4 15:02:09 PST 2005
//    Renamed near, far to nearPlane and farPlane since near,far are
//    reserve words in the Windows compiler.
//
//    Jeremy Meredith, Fri Feb 20 17:26:49 EST 2009
//    Use the property's opacity for the edge color (mesh lines) as well.
//
//    Tom Fogal, Tue Apr 27 17:26:03 MDT 2010
//    Remove special case Mesa code; always follow path.  This fixes legends.py
//    when using HW-accel'd parallel rendering.
//
//    Eric Brugger, Wed Jun  2 13:59:24 PDT 2010
//    I moved the code that shifts edges toward the viewer in DrawEdges2
//    up in the call sequence to the routine DrawEdges so that it would
//    always be executed when DrawEdges is executed.
//
//    Tom Fogal, Fri Jan 28 14:42:57 MST 2011
//    Typing fixes (vtkIdType).
//
//    Kathleen Biagas, Thu Mar 15 16:30:48 PDT 2012
//    Follow vtk 5.8's vtkOpenGLPolyDataMapper, Draw methods have been
//    consolidatd and make use of macro's that support multiple data types.
//
// ****************************************************************************

void
avtOpenGLSurfaceAndWireframeRenderer::DrawEdges2()
{
    int rep;
    vtkPoints *p;
    vtkDataArray *n;
    vtkUnsignedCharArray *c = NULL;
    vtkDataArray *t;
    int tDim;
    vtkIdType cellNum = 0;
    int cellNormals = 0;

    // get the representation (e.g., surface / wireframe / points)
    rep = prop->GetRepresentation();

    p = input->GetPoints();
  
    double edgeColor[4];
    prop->GetEdgeColor(edgeColor); 
    edgeColor[3] = prop->GetOpacity(); 
    glColor4dv(edgeColor);

    t = input->GetPointData()->GetTCoords();
    if (t) 
    {
        tDim = t->GetNumberOfComponents();
        if (tDim  > 2)
        {
            debug5 << "Currently only 1d and 2d textures are supported.\n";
            t = NULL;
        }
    }

    n = input->GetPointData()->GetNormals();
    if (prop->GetInterpolation() == VTK_FLAT)
    {
        n = 0;
    }
  
    cellNormals = 0;
    if (input->GetCellData()->GetNormals())
    {
        cellNormals = 1;
        n = input->GetCellData()->GetNormals();
    }
  
    // 
    // although we have eliminated some of the tests here,
    // for consistency, we are using the same increments 
    // for idx as DrawSurface(),
    // 
    unsigned long idx = 0;
    if (n && !cellNormals)
    {
        idx |= OGL_SAWR_NORMALS;
    }
    if (t)
    {
        idx |= OGL_SAWR_TCOORDS;
        if (t->GetDataType() == VTK_FLOAT)
        {
            idx |= OGL_SAWR_TCOORD_TYPE_FLOAT;
        }
        else if (t->GetDataType() == VTK_DOUBLE)
        {
            idx |= OGL_SAWR_TCOORD_TYPE_DOUBLE;
        }
        if (tDim == 1)
        {
            idx |= OGL_SAWR_TCOORD_1D;
        }
        // Not 1D assumes 2D texture coordinates.
    }
    if (cellNormals)
    {
        idx |= OGL_SAWR_CELL_NORMALS;
    }

    glDisable(GL_LIGHTING);

    // draw all the points
    if (drawEdgeVerts)
    {
        DrawPoints(idx,p,n,c,t,cellNum, input->GetVerts());
    }
  
    // draw all the elements
    if (drawEdgeLines)
    {
        if (rep == VTK_POINTS)
        {
            DrawPoints(idx,p,n,c,t,cellNum, input->GetLines());
        }
        else
        {
            DrawLines(idx,p,n,c,t,cellNum, input->GetLines());
        }
    }
  
        
    // do tstrips
    if (drawEdgeStrips)
    {
        if (rep == VTK_POINTS)
        {
            DrawPoints(idx,p,n,c,t,cellNum,input->GetStrips());
        }
        else if (rep == VTK_WIREFRAME)
        {
            vtkIdType oldCellNum = cellNum;
            DrawTStrips(idx,p,n,c,t,cellNum, 
                        GL_LINE_STRIP, input->GetStrips());
            DrawTStripLines(idx,p,n,c,t,oldCellNum, 
                        GL_LINE_STRIP, input->GetStrips());
        }
        else 
        {
            DrawTStrips(idx,p,n,c,t,cellNum,
                        GL_TRIANGLE_STRIP, input->GetStrips());
        }
    }

    // do polys
    if (drawEdgePolys)
    {
        if (rep == VTK_POINTS)
            DrawPoints(idx,p,n,c,t,cellNum,input->GetPolys());
        else if (rep == VTK_WIREFRAME)
            DrawPolygons(idx,p,n,c,t,cellNum, GL_LINE_LOOP, input->GetPolys());
        else 
            DrawPolygons(idx,p,n,c,t,cellNum, GL_POLYGON, input->GetPolys());
    }

    glEnable(GL_LIGHTING);

} // DrawEdges



