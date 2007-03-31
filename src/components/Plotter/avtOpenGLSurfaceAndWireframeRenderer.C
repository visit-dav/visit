// ************************************************************************* //
//                  avtOpenGLSurfaceAndWireframeRenderer.C                   //
// ************************************************************************* //

#include "avtOpenGLSurfaceAndWireframeRenderer.h"

#if defined(_WIN32)
#include <windows.h>
#endif
#include <GL/gl.h>

#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkPolygon.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkTriangle.h>

#include <DebugStream.h>


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

    int i;

    VTKRen->GetRenderWindow()->MakeCurrent();

    for (i = 0; i < surfaceListId.size(); i++)
    {
        if (surfaceListId[i])
        {
            glDeleteLists(surfaceListId[i], 1);
            surfaceListId[i] = 0;
            surfaceModified[i] = true;
        }
    }
    for (i = 0; i < edgesListId.size(); i++)
    {
        if (edgesListId[i])
        {
            glDeleteLists(edgesListId[i], 1);
            edgesListId[i] = 0;
            edgesModified[i] = true;
        }
    }
    for (i = 0; i < setupListId.size(); i++)
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


// ****************************************************************************
//  Method:  Draw01 
//
//  Purpose:
//    Draw with no normals, colors or textures. 
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework. 
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    aGlFunction   Which GL function is to be used for drawing. 
//    <not_named>  
//    p             The vertices of the primitives.
//    <not_named>
//    <not_named>
//    <not_named>
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
Draw01(vtkCellArray *aPrim, GLenum aGlFunction, int &, vtkPoints *p, 
       vtkDataArray *, vtkUnsignedCharArray *, vtkDataArray *) 
{
    int j; 
    int count = 0;
    vtkIdType npts, *pts; 
    GLenum previousGlFunction=GL_INVALID_VALUE;

    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); count++)
    { 
        BeginPolyTriangleOrQuad(aGlFunction, previousGlFunction, npts);

        for (j = 0; j < npts; j++) 
        {
            glVertex3fv(p->GetPoint(pts[j]));
        }

        if ((previousGlFunction != GL_TRIANGLES)  
            && (previousGlFunction != GL_QUADS)  
            && (previousGlFunction != GL_POINTS))
        {
            glEnd();
        }
    
    }
    if ((previousGlFunction == GL_TRIANGLES)
        || (previousGlFunction == GL_QUADS)
        || (previousGlFunction == GL_POINTS))
    {
        glEnd();
    }
}


// ****************************************************************************
//  Method:  DrawN013
//
//  Purpose:
//    Draw with vertex normals.
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    aGlFunction   Which GL function is to be used for drawing. 
//    <not_named>  
//    p             The vertices of the primitives.
//    n             The vertex normals.
//    <not_named>
//    <not_named>
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
DrawN013(vtkCellArray *aPrim, GLenum aGlFunction, int &, vtkPoints *p, 
         vtkDataArray *n, vtkUnsignedCharArray *, vtkDataArray *) 
{
    int j;
    vtkIdType npts, *pts;
    int count = 0;

    GLenum previousGlFunction=GL_INVALID_VALUE;
  
    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); count++)
    {
        BeginPolyTriangleOrQuad(aGlFunction, previousGlFunction, npts);
    
        for (j = 0; j < npts; j++) 
        {
            glNormal3fv(n->GetTuple3(pts[j]));
            glVertex3fv(p->GetPoint(pts[j]));
        }

        if ((previousGlFunction != GL_TRIANGLES) 
            && (previousGlFunction != GL_QUADS)
            && (previousGlFunction != GL_POINTS))
        {
            glEnd();
        }
    }
    if ((previousGlFunction == GL_TRIANGLES)
        || (previousGlFunction == GL_QUADS)
        || (previousGlFunction == GL_POINTS))
    {
        glEnd();
    }
}


// ****************************************************************************
//  Method:  DrawCN013
//
//  Purpose:
//    Draw with cell normals. 
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    aGlFunction   Which GL function is to be used for drawing. 
//    cellNum       Keeps track of current cell number.
//    p             The vertices of the primitives.
//    n             The cell normals.
//    <not_named>
//    <not_named>
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
DrawCN013(vtkCellArray *aPrim, GLenum aGlFunction, int &cellNum, vtkPoints *p, 
          vtkDataArray *n, vtkUnsignedCharArray *, vtkDataArray *) 
{
    int j;
    vtkIdType  npts, *pts;
    int count = 0;
  
    GLenum previousGlFunction=GL_INVALID_VALUE;

    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); 
         count++, cellNum++)
    { 
        BeginPolyTriangleOrQuad(aGlFunction, previousGlFunction, npts);
    
        glNormal3fv(n->GetTuple3(cellNum));
    
        for (j = 0; j < npts; j++) 
        {
            glVertex3fv(p->GetPoint(pts[j]));
        }

        if ((previousGlFunction != GL_TRIANGLES) 
            && (previousGlFunction != GL_QUADS)
            && (previousGlFunction != GL_POINTS))
        {
            glEnd();
        }
    }
    if ((previousGlFunction == GL_TRIANGLES)
        || (previousGlFunction == GL_QUADS)
        || (previousGlFunction == GL_POINTS))
    {
        glEnd();
    }
}


// ****************************************************************************
//  Method:  DrawS01
//
//  Purpose:
//    Draw with vertex colors. 
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    aGlFunction   Which GL function is to be used for drawing. 
//    <not_named>   
//    p             The vertices of the primitives.
//    <not_named>   
//    c             The colors for the vertices.
//    <not_named>
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
DrawS01(vtkCellArray *aPrim, GLenum aGlFunction, int &, vtkPoints *p, 
        vtkDataArray *, vtkUnsignedCharArray *c, vtkDataArray *) 
{
    int j;
    vtkIdType npts, *pts;
    int count = 0;
  
    GLenum previousGlFunction=GL_INVALID_VALUE;

    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); count++)
    { 
        BeginPolyTriangleOrQuad(aGlFunction, previousGlFunction, npts);
    
        for (j = 0; j < npts; j++) 
        {
            glColor4ubv(c->GetPointer(4*pts[j]));
            glVertex3fv(p->GetPoint(pts[j]));
        }

        if ((previousGlFunction != GL_TRIANGLES) 
            && (previousGlFunction != GL_QUADS)
            && (previousGlFunction != GL_POINTS))
        {
            glEnd();
        }
    }
    if ((previousGlFunction == GL_TRIANGLES)
        || (previousGlFunction == GL_QUADS)
        || (previousGlFunction == GL_POINTS))
    {
        glEnd();
    }
}


// ****************************************************************************
//  Method:  DrawNS013
//
//  Purpose:
//    Draw with vertex normals and vertex colors.
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    aGlFunction   Which GL function is to be used for drawing. 
//    <not_named>   Keeps track of the current cell number.
//    p             The vertices of the primitives.
//    n             The normals for the vertices.
//    c             The colors for the  vertices.
//    <not_named>             
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
DrawNS013(vtkCellArray *aPrim, GLenum aGlFunction, int &, vtkPoints *p, 
          vtkDataArray *n, vtkUnsignedCharArray *c, vtkDataArray *) 
{
    int j;
    vtkIdType npts, *pts;
    int count = 0;
  
    GLenum previousGlFunction=GL_INVALID_VALUE;

    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); count++)
    { 
        BeginPolyTriangleOrQuad(aGlFunction, previousGlFunction, npts);
    
        for (j = 0; j < npts; j++) 
        {
            glColor4ubv(c->GetPointer(4*pts[j]));
            glNormal3fv(n->GetTuple3(pts[j]));
            glVertex3fv(p->GetPoint(pts[j]));
        }

        if ((previousGlFunction != GL_TRIANGLES) 
            && (previousGlFunction != GL_QUADS)
            && (previousGlFunction != GL_POINTS))
        {
            glEnd();
        }
    }
    if ((previousGlFunction == GL_TRIANGLES)
        || (previousGlFunction == GL_QUADS)
        || (previousGlFunction == GL_POINTS))
    {
        glEnd();
    }
}


// ****************************************************************************
//  Method:  DrawCNS013
//
//  Purpose:
//    Draw with cell normals and vertex colors.
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    aGlFunction   Which GL function is to be used for drawing. 
//    cellNum       Keeps track of the current cell number.
//    p             The vertices of the primitives.
//    n             The normals for the cells.
//    c             The colors for the  vertices.
//    <not_named>             
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
DrawCNS013(vtkCellArray *aPrim, GLenum aGlFunction, int &cellNum, vtkPoints *p,
           vtkDataArray *n, vtkUnsignedCharArray *c, vtkDataArray *) 
{
    int j;
    vtkIdType npts, *pts;
    int count = 0;
  
    GLenum previousGlFunction=GL_INVALID_VALUE;

    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); 
         count++, cellNum++)
    { 
        BeginPolyTriangleOrQuad(aGlFunction, previousGlFunction, npts);
    
        glNormal3fv(n->GetTuple3(cellNum));
    
        for (j = 0; j < npts; j++) 
        {
            glColor4ubv(c->GetPointer(4*pts[j]));
            glVertex3fv(p->GetPoint(pts[j]));
        }

        if ((previousGlFunction != GL_TRIANGLES) 
            && (previousGlFunction != GL_QUADS)
            && (previousGlFunction != GL_POINTS))
        {
            glEnd();
        }
    }
    if ((previousGlFunction == GL_TRIANGLES)
        || (previousGlFunction == GL_QUADS)
        || (previousGlFunction == GL_POINTS))
    {
        glEnd();
    }
}


// ****************************************************************************
//  Method:  DrawT01
//
//  Purpose:
//    Draw with textures.
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    aGlFunction   Which GL function is to be used for drawing. 
//    <not_named> 
//    p             The vertices of the primitives.
//    <not_named> 
//    <not_named> 
//    t             The texture coordinates.
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
DrawT01(vtkCellArray *aPrim, GLenum aGlFunction, int &, vtkPoints *p, 
        vtkDataArray *, vtkUnsignedCharArray *, vtkDataArray *t) 
{
    int j;
    vtkIdType npts, *pts;
    int count = 0;
  
    GLenum previousGlFunction=GL_INVALID_VALUE;

    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); count++)
    { 
        BeginPolyTriangleOrQuad(aGlFunction, previousGlFunction, npts);
    
        for (j = 0; j < npts; j++) 
        {
            glTexCoord2fv(t->GetTuple2(pts[j]));
            glVertex3fv(p->GetPoint(pts[j]));
        }

        if ((previousGlFunction != GL_TRIANGLES) 
            && (previousGlFunction != GL_QUADS)
            && (previousGlFunction != GL_POINTS))
        {
            glEnd();
        }
    }
    if ((previousGlFunction == GL_TRIANGLES)
        || (previousGlFunction == GL_QUADS)
        || (previousGlFunction == GL_POINTS))
    {
        glEnd();
    }
}


// ****************************************************************************
//  Method:  DrawNT013
//
//  Purpose:
//    Draw with vertex normals and textures.
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    aGlFunction   Which GL function is to be used for drawing. 
//    <not_named> 
//    p             The vertices of the primitives.
//    n             The normals for the vertices.
//    <not_named> 
//    t             The texture coordinates.
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
DrawNT013(vtkCellArray *aPrim, GLenum aGlFunction, int &, vtkPoints *p, 
          vtkDataArray *n, vtkUnsignedCharArray *, vtkDataArray *t) 
{
    int j;
    vtkIdType  npts, *pts;
    int count = 0;
  
    GLenum previousGlFunction=GL_INVALID_VALUE;
  
    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); count++)
    { 
        BeginPolyTriangleOrQuad(aGlFunction, previousGlFunction, npts);
    
        for (j = 0; j < npts; j++) 
        {
            glTexCoord2fv(t->GetTuple2(pts[j]));
            glNormal3fv(n->GetTuple3(pts[j]));
            glVertex3fv(p->GetPoint(pts[j]));
        }

        if ((previousGlFunction != GL_TRIANGLES) 
            && (previousGlFunction != GL_QUADS)
            && (previousGlFunction != GL_POINTS))
        {
            glEnd();
        }
    }
    if ((previousGlFunction == GL_TRIANGLES)
        || (previousGlFunction == GL_QUADS)
        || (previousGlFunction == GL_POINTS))
    {
        glEnd();
    }
}


// ****************************************************************************
//  Method:  DrawCNT013
//
//  Purpose:
//    Draw tih cell normals and textures.  
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    aGlFunction   Which GL function is to be used for drawing. 
//    cellNum       Keeps track of the current cell number.
//    p             The vertices of the primitives.
//    n             The normals for the cells.
//    <not_named> 
//    t             The texture coordinates.
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
DrawCNT013(vtkCellArray *aPrim, GLenum aGlFunction, int &cellNum, vtkPoints *p,
           vtkDataArray *n, vtkUnsignedCharArray *, vtkDataArray *t) 
{
    int j; 
    vtkIdType npts, *pts;
    int count = 0;
  
    GLenum previousGlFunction=GL_INVALID_VALUE;

    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); 
         count++, cellNum++)
    { 
        BeginPolyTriangleOrQuad(aGlFunction, previousGlFunction, npts);
    
        glNormal3fv(n->GetTuple3(cellNum));
    
        for (j = 0; j < npts; j++) 
        {
            glTexCoord2fv(t->GetTuple2(pts[j]));
            glVertex3fv(p->GetPoint(pts[j]));
        }

        if ((previousGlFunction != GL_TRIANGLES) 
            && (previousGlFunction != GL_QUADS)
            && (previousGlFunction != GL_POINTS))
        {
            glEnd();
        }
    }
    if ((previousGlFunction == GL_TRIANGLES)
        || (previousGlFunction == GL_QUADS)
        || (previousGlFunction == GL_POINTS))
    {
        glEnd();
    }
}


// ****************************************************************************
//  Method:  DrawST01
//
//  Purpose:
//    Draw with vertex colors and textures.
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    aGlFunction   Which GL function is to be used for drawing. 
//    <not_named> 
//    p             The vertices of the primitives.
//    <not_named> 
//    c             The colors for the vertices.
//    t             The texture coordinates.
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
DrawST01(vtkCellArray *aPrim, GLenum aGlFunction, int &, vtkPoints *p, 
         vtkDataArray *, vtkUnsignedCharArray *c, vtkDataArray *t) 
{
    int j;
    vtkIdType npts, *pts;
    int count = 0;
  
    GLenum previousGlFunction=GL_INVALID_VALUE;

    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); count++)
    { 
        BeginPolyTriangleOrQuad(aGlFunction, previousGlFunction, npts);
    
        for (j = 0; j < npts; j++) 
        {
            glColor4ubv(c->GetPointer(4*pts[j]));
            glTexCoord2fv(t->GetTuple2(pts[j]));
            glVertex3fv(p->GetPoint(pts[j]));
        }

        if ((previousGlFunction != GL_TRIANGLES) 
            && (previousGlFunction != GL_QUADS)
            && (previousGlFunction != GL_POINTS))
        {
            glEnd();
        }
    }
    if ((previousGlFunction == GL_TRIANGLES)
        || (previousGlFunction == GL_QUADS)
        || (previousGlFunction == GL_POINTS))
    {
        glEnd();
    }
}


// ****************************************************************************
//  Method:  DrawNST013 
//
//  Purpose:
//    Draw with vertex normals, vertex colors and textures.
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    aGlFunction   Which GL function is to be used for drawing. 
//    <not_named> 
//    p             The vertices of the primitives.
//    n             The normals for the vertices.
//    c             The colors for the vertices.
//    t             The texture coordinates.
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
DrawNST013(vtkCellArray *aPrim, GLenum aGlFunction, int &, vtkPoints *p, 
           vtkDataArray *n, vtkUnsignedCharArray *c, vtkDataArray *t) 
{
    int j;
    vtkIdType npts, *pts;
    int count = 0;
  
    GLenum previousGlFunction=GL_INVALID_VALUE;

    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); count++)
    { 
        BeginPolyTriangleOrQuad(aGlFunction, previousGlFunction, npts);
    
        for (j = 0; j < npts; j++) 
        {
            glColor4ubv(c->GetPointer(4*pts[j]));
            glTexCoord2fv(t->GetTuple2(pts[j]));
            glNormal3fv(n->GetTuple3(pts[j]));
            glVertex3fv(p->GetPoint(pts[j]));
        }

        if ((previousGlFunction != GL_TRIANGLES) 
            && (previousGlFunction != GL_QUADS)
            && (previousGlFunction != GL_POINTS))
        {
            glEnd();
        }
    }
    if ((previousGlFunction == GL_TRIANGLES)
        || (previousGlFunction == GL_QUADS)
        || (previousGlFunction == GL_POINTS))
    {
        glEnd();
    }
}

// ****************************************************************************
//  Method:  DrawCNST013 
//
//  Purpose:
//    Draw with cell normals, vertex colors and textures.
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    aGlFunction   Which GL function is to be used for drawing. 
//    cellNum       Keeps track of the current cell number.
//    p             The vertices of the primitives.
//    n             The normals for the cells.
//    c             The colors for the vertices.
//    t             The texture coordinates.
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
DrawCNST013(vtkCellArray *aPrim, GLenum aGlFunction, int &cellNum, 
            vtkPoints *p, vtkDataArray *n, vtkUnsignedCharArray *c, vtkDataArray *t) 
{
    int j;
    vtkIdType npts, *pts;
    int count = 0;
  
    GLenum previousGlFunction=GL_INVALID_VALUE;

    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); 
         count++, cellNum++)
    {
        BeginPolyTriangleOrQuad(aGlFunction, previousGlFunction, npts);
    
        glNormal3fv(n->GetTuple3(cellNum));
    
        for (j = 0; j < npts; j++) 
        {
            glColor4ubv(c->GetPointer(4*pts[j]));
            glTexCoord2fv(t->GetTuple2(pts[j]));
            glVertex3fv(p->GetPoint(pts[j]));
        }

        if ((previousGlFunction != GL_TRIANGLES) 
            && (previousGlFunction != GL_QUADS)
            && (previousGlFunction != GL_POINTS))
        {
            glEnd();
        }
    }
    if ((previousGlFunction == GL_TRIANGLES)
        || (previousGlFunction == GL_QUADS)
        || (previousGlFunction == GL_POINTS))
    {
        glEnd();
    }
}


// ****************************************************************************
//  Method:  DrawCS01 
//
//  Purpose:
//    Draw with cell colors.
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    aGlFunction   Which GL function is to be used for drawing. 
//    cellNum       Keeps track of the current cell number.
//    p             The vertices of the primitives.
//    <not_named> 
//    c             The colors for the cells.
//    <not_named> 
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
DrawCS01(vtkCellArray *aPrim, GLenum aGlFunction, int &cellNum, vtkPoints *p, 
         vtkDataArray *, vtkUnsignedCharArray *c, vtkDataArray *) 
{
    int j;
    vtkIdType npts, *pts;
    int count = 0;
  
    GLenum previousGlFunction=GL_INVALID_VALUE;

    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); 
         count++, cellNum++)
    { 
        BeginPolyTriangleOrQuad(aGlFunction, previousGlFunction, npts);
        glColor4ubv(c->GetPointer(4*cellNum));
    
        for (j = 0; j < npts; j++) 
        {
            glVertex3fv(p->GetPoint(pts[j]));
        }

        if ((previousGlFunction != GL_TRIANGLES) 
            && (previousGlFunction != GL_QUADS)
            && (previousGlFunction != GL_POINTS))
        {
            glEnd();
        }
    }
    if ((previousGlFunction == GL_TRIANGLES)
        || (previousGlFunction == GL_QUADS)
        || (previousGlFunction == GL_POINTS))
    {
        glEnd();
    }
}


// ****************************************************************************
//  Method:  DrawNCS013 
//
//  Purpose:
//    Draw with vertex normals and cell colors.
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    aGlFunction   Which GL function is to be used for drawing. 
//    cellNum       Keeps track of the current cell number.
//    p             The vertices of the primitives.
//    n             The normals for the vertices.
//    c             The colors for the cells.
//    <not_named> 
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
DrawNCS013(vtkCellArray *aPrim, GLenum aGlFunction, int &cellNum, vtkPoints *p,
           vtkDataArray *n, vtkUnsignedCharArray *c, vtkDataArray *) 
{
    int j;
    vtkIdType npts, *pts;
    int count = 0;
  
    GLenum previousGlFunction=GL_INVALID_VALUE;

    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); 
         count++, cellNum++)
    { 
        BeginPolyTriangleOrQuad(aGlFunction, previousGlFunction, npts);
    
        glColor4ubv(c->GetPointer(4*cellNum));
    
        for (j = 0; j < npts; j++) 
        {
            glNormal3fv(n->GetTuple3(pts[j]));
            glVertex3fv(p->GetPoint(pts[j]));
        }

        if ((previousGlFunction != GL_TRIANGLES) 
            && (previousGlFunction != GL_QUADS)
            && (previousGlFunction != GL_POINTS))
        {
            glEnd();
        }
    }
    if ((previousGlFunction == GL_TRIANGLES)
        || (previousGlFunction == GL_QUADS)
        || (previousGlFunction == GL_POINTS))
    {
        glEnd();
    }
}


// ****************************************************************************
//  Method:  DrawCNCS013 
//
//  Purpose:
//    Draw with cell normals and cell scalars.
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    aGlFunction   Which GL function is to be used for drawing. 
//    cellNum       Keeps track of the current cell number.
//    p             The vertices of the primitives.
//    n             The normals for the cells.
//    c             The colors for the cells.
//    <not_named> 
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
DrawCNCS013(vtkCellArray *aPrim, GLenum aGlFunction, int &cellNum, 
            vtkPoints *p, vtkDataArray *n, vtkUnsignedCharArray *c, vtkDataArray *) 
{
    int j;
    vtkIdType npts, *pts;
    int count = 0;
  
    GLenum previousGlFunction=GL_INVALID_VALUE;

    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); 
         count++, cellNum++)
    { 
        BeginPolyTriangleOrQuad(aGlFunction, previousGlFunction, npts);
    
        glColor4ubv(c->GetPointer(4*cellNum));
        glNormal3fv(n->GetTuple3(cellNum));
    
        for (j = 0; j < npts; j++) 
        {
            glVertex3fv(p->GetPoint(pts[j]));
        }

        if ((previousGlFunction != GL_TRIANGLES) 
            && (previousGlFunction != GL_QUADS)
            && (previousGlFunction != GL_POINTS))
        {
            glEnd();
        }
    }
    if ((previousGlFunction == GL_TRIANGLES)
        || (previousGlFunction == GL_QUADS)
        || (previousGlFunction == GL_POINTS))
    {
        glEnd();
    }
}


// ****************************************************************************
//  Method:  DrawCST01 
//
//  Purpose:
//    Draw with cell colors and textures. 
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    aGlFunction   Which GL function is to be used for drawing. 
//    cellNum       Keeps track of the current cell number.
//    p             The vertices of the primitives.
//    <not_named> 
//    c             The colors for the cells.
//    t             The texture coordinates.
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
DrawCST01(vtkCellArray *aPrim, GLenum aGlFunction, int &cellNum, vtkPoints *p, 
          vtkDataArray *, vtkUnsignedCharArray *c, vtkDataArray *t) 
{
    int j;
    vtkIdType npts, *pts;
    int count = 0;
  
    GLenum previousGlFunction=GL_INVALID_VALUE;

    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); 
         count++, cellNum++)
    { 
        BeginPolyTriangleOrQuad(aGlFunction, previousGlFunction, npts);
        glColor4ubv(c->GetPointer(4*cellNum));
    
        for (j = 0; j < npts; j++) 
        {
            glTexCoord2fv(t->GetTuple2(pts[j]));
            glVertex3fv(p->GetPoint(pts[j]));
        }

        if ((previousGlFunction != GL_TRIANGLES) 
            && (previousGlFunction != GL_QUADS)
            && (previousGlFunction != GL_POINTS))
        {
            glEnd();
        }
    }
    if ((previousGlFunction == GL_TRIANGLES)
        || (previousGlFunction == GL_QUADS)
        || (previousGlFunction == GL_POINTS))
    {
        glEnd();
    }
}


// ****************************************************************************
//  Method:  DrawNCST013
//
//  Purpose:
//    Draw with vertex normals, cell colors and textures.
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    aGlFunction   Which GL function is to be used for drawing. 
//    cellNum       Keeps track of the current cell number.
//    p             The vertices of the primitives.
//    n             The normals for the vertices.
//    c             The colors for the cells.
//    t             The texture coordinates.
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
DrawNCST013(vtkCellArray *aPrim, GLenum aGlFunction, int &cellNum, 
            vtkPoints *p, vtkDataArray *n, vtkUnsignedCharArray *c, vtkDataArray *t) 
{
    int j;
    vtkIdType npts, *pts;
    int count = 0;
  
    GLenum previousGlFunction=GL_INVALID_VALUE;

    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); 
         count++, cellNum++)
    { 
        BeginPolyTriangleOrQuad(aGlFunction, previousGlFunction, npts);
        glColor4ubv(c->GetPointer(4*cellNum));
    
        for (j = 0; j < npts; j++) 
        {
            glTexCoord2fv(t->GetTuple2(pts[j]));
            glNormal3fv(n->GetTuple3(pts[j]));
            glVertex3fv(p->GetPoint(pts[j]));
        }

        if ((previousGlFunction != GL_TRIANGLES) 
            && (previousGlFunction != GL_QUADS)
            && (previousGlFunction != GL_POINTS))
        {
            glEnd();
        }
    }
    if ((previousGlFunction == GL_TRIANGLES)
        || (previousGlFunction == GL_QUADS)
        || (previousGlFunction == GL_POINTS))
    {
        glEnd();
    }
}


// ****************************************************************************
//  Method:  DrawCNCST013
//
//  Purpose:
//    Draw with cell normals, cell scalars and textures.
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    aGlFunction   Which GL function is to be used for drawing. 
//    cellNum       Keeps track of the current cell number.
//    p             The vertices of the primitives.
//    n             The normals for the cells.
//    c             The colors for the cells.
//    t             The texture coordinates.
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
DrawCNCST013(vtkCellArray *aPrim, GLenum aGlFunction, int &cellNum, 
             vtkPoints *p, vtkDataArray *n, vtkUnsignedCharArray *c, vtkDataArray *t) 
{
    int j;
    vtkIdType npts, *pts;
    int count = 0;
  
    GLenum previousGlFunction=GL_INVALID_VALUE;

    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); 
         count++, cellNum++)
    { 
        BeginPolyTriangleOrQuad(aGlFunction, previousGlFunction, npts);
    
        glColor4ubv(c->GetPointer(4*cellNum));
        glNormal3fv(n->GetTuple3(cellNum));
      
        for (j = 0; j < npts; j++) 
        {
            glTexCoord2fv(t->GetTuple2(pts[j]));
            glVertex3fv(p->GetPoint(pts[j]));
        }

        if ((previousGlFunction != GL_TRIANGLES) 
            && (previousGlFunction != GL_QUADS)
            && (previousGlFunction != GL_POINTS))
        {
            glEnd();
        }
    }
    if ((previousGlFunction == GL_TRIANGLES)
        || (previousGlFunction == GL_QUADS)
        || (previousGlFunction == GL_POINTS))
    {
        glEnd();
    }
}


// ****************************************************************************
//  Method:  Draw3
//
//  Purpose:
//    Draw with no colors or textures.  Normals are computed.
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    aGlFunction   Which GL function is to be used for drawing. 
//    <not_named> 
//    p             The vertices of the primitives.
//    <not_named> 
//    <not_named> 
//    <not_named> 
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
Draw3(vtkCellArray *aPrim, GLenum aGlFunction, int &, vtkPoints *p, 
      vtkDataArray *, vtkUnsignedCharArray *, vtkDataArray *) 
{
    int j;
    vtkIdType npts, *pts;
    int count = 0;
    float polyNorm[3];
  
    GLenum previousGlFunction=GL_INVALID_VALUE;

    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); count++)
    {
        BeginPolyTriangleOrQuad(aGlFunction, previousGlFunction, npts);
    
        vtkPolygon::ComputeNormal(p,npts,pts,polyNorm);
        glNormal3fv(polyNorm);
    
        for (j = 0; j < npts; j++) 
        {
            glVertex3fv(p->GetPoint(pts[j]));
        }

        if ((previousGlFunction != GL_TRIANGLES) 
            && (previousGlFunction != GL_QUADS)
            && (previousGlFunction != GL_POINTS))
        {
          glEnd();
        }
    }
    if ((previousGlFunction == GL_TRIANGLES)
        || (previousGlFunction == GL_QUADS)
        || (previousGlFunction == GL_POINTS))
    {
        glEnd();
    }
}


// ****************************************************************************
//  Method:  DrawS3
//
//  Purpose:
//    Draw with vertex colors.  Normals are computed.
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    aGlFunction   Which GL function is to be used for drawing. 
//    <not_named> 
//    p             The vertices of the primitives.
//    <not_named> 
//    c             The colors for the vertices.
//    <not_named> 
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
DrawS3(vtkCellArray *aPrim, GLenum aGlFunction, int &, vtkPoints *p, 
       vtkDataArray *, vtkUnsignedCharArray *c, vtkDataArray *) 
{
    int j;
    vtkIdType npts, *pts;
    int count = 0;
    float polyNorm[3];
  
    GLenum previousGlFunction=GL_INVALID_VALUE;

    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); count++)
    { 
        BeginPolyTriangleOrQuad(aGlFunction, previousGlFunction, npts);
    
        vtkPolygon::ComputeNormal(p,npts,pts,polyNorm);

        glNormal3fv(polyNorm);
        for (j = 0; j < npts; j++) 
        {
            glColor4ubv(c->GetPointer(4*pts[j]));
            glVertex3fv(p->GetPoint(pts[j]));
        }

        if ((previousGlFunction != GL_TRIANGLES) 
            && (previousGlFunction != GL_QUADS)
            && (previousGlFunction != GL_POINTS))
        {
            glEnd();
        }
    }
    if ((previousGlFunction == GL_TRIANGLES)
        || (previousGlFunction == GL_QUADS)
        || (previousGlFunction == GL_POINTS))
    {
        glEnd();
    }
}


// ****************************************************************************
//  Method:  Draw
//
//  Purpose:
//    Draw with textures.  Normals are computed. 
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    aGlFunction   Which GL function is to be used for drawing. 
//    <not_named> 
//    p             The vertices of the primitives.
//    <not_named> 
//    <not_named> 
//    t             The texture coordinates.
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
DrawT3(vtkCellArray *aPrim, GLenum aGlFunction, int &, vtkPoints *p, 
       vtkDataArray *, vtkUnsignedCharArray *, vtkDataArray *t) 
{
    int j;
    vtkIdType npts, *pts;
    int count = 0;
    float polyNorm[3];
  
    GLenum previousGlFunction=GL_INVALID_VALUE;

    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); count++)
    { 
        BeginPolyTriangleOrQuad(aGlFunction, previousGlFunction, npts);
    
        vtkPolygon::ComputeNormal(p,npts,pts,polyNorm);
        glNormal3fv(polyNorm);
    
        for (j = 0; j < npts; j++) 
        {
            glTexCoord2fv(t->GetTuple2(pts[j]));
            glVertex3fv(p->GetPoint(pts[j]));
        }

        if ((previousGlFunction != GL_TRIANGLES) 
            && (previousGlFunction != GL_QUADS)
            && (previousGlFunction != GL_POINTS))
        {
            glEnd();
        }
    }
    if ((previousGlFunction == GL_TRIANGLES)
        || (previousGlFunction == GL_QUADS)
        || (previousGlFunction == GL_POINTS))
    {
        glEnd();
    }
}


// ****************************************************************************
//  Method:  DrawST3
//
//  Purpose:
//    Draw with vertex colors and textures.  Normals are computed. 
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    aGlFunction   Which GL function is to be used for drawing. 
//    <not_named> 
//    p             The vertices of the primitives.
//    <not_named> 
//    c             The colors for the vertices.
//    t             The texture coordinates.
//    <not_named> 
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
DrawST3(vtkCellArray *aPrim, GLenum aGlFunction, int &, vtkPoints *p, 
        vtkDataArray *, vtkUnsignedCharArray *c, vtkDataArray *t) 
{
    int j; 
    vtkIdType npts, *pts;
    int count = 0;
    float polyNorm[3];
  
    GLenum previousGlFunction=GL_INVALID_VALUE;

    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); count++)
    { 
        BeginPolyTriangleOrQuad(aGlFunction, previousGlFunction, npts);
    
        vtkPolygon::ComputeNormal(p,npts,pts,polyNorm);
        glNormal3fv(polyNorm);
    
        for (j = 0; j < npts; j++) 
        {
            glColor4ubv(c->GetPointer(4*pts[j]));
            glTexCoord2fv(t->GetTuple2(pts[j]));
            glVertex3fv(p->GetPoint(pts[j]));
        }

        if ((previousGlFunction != GL_TRIANGLES)
            && (previousGlFunction != GL_QUADS)
            && (previousGlFunction != GL_POINTS))
        {
            glEnd();
        }
    }
    if ((previousGlFunction == GL_TRIANGLES)
        || (previousGlFunction == GL_QUADS)
        || (previousGlFunction == GL_POINTS))
    {
        glEnd();
    }
}


// ****************************************************************************
//  Method:  DrawCS3
//
//  Purpose:
//    Draw with cell colors.  Normals are computed.
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    aGlFunction   Which GL function is to be used for drawing. 
//    cellNum       Keeps track of the current cell number.
//    p             The vertices of the primitives.
//    <not_named> 
//    c             The colors for the cells.
//    <not_named> 
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
DrawCS3(vtkCellArray *aPrim, GLenum aGlFunction, int &cellNum, vtkPoints *p, 
        vtkDataArray *, vtkUnsignedCharArray *c, vtkDataArray *) 
{
    int j; 
    vtkIdType npts, *pts;
    int count = 0;
    float polyNorm[3];
  
    GLenum previousGlFunction=GL_INVALID_VALUE;

    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); 
         count++, cellNum++)
    { 
        BeginPolyTriangleOrQuad(aGlFunction, previousGlFunction, npts);
    
        vtkPolygon::ComputeNormal(p,npts,pts,polyNorm);
        glColor4ubv(c->GetPointer(4*cellNum));
        glNormal3fv(polyNorm);

        for (j = 0; j < npts; j++) 
        {
            glVertex3fv(p->GetPoint(pts[j]));
        }

        if ((previousGlFunction != GL_TRIANGLES)
            && (previousGlFunction != GL_QUADS)
            && (previousGlFunction != GL_POINTS))
        {
            glEnd();
        }
    }
    if ((previousGlFunction == GL_TRIANGLES)
        || (previousGlFunction == GL_QUADS)
        || (previousGlFunction == GL_POINTS))
    {
        glEnd();
    }
}


// ****************************************************************************
//  Method:  DrawCST3
//
//  Purpose:
//    Draw with cell colors and textures.  Normals are computed. 
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    aGlFunction   Which GL function is to be used for drawing. 
//    cellNum       Keeps track of the current cell number.
//    p             The vertices of the primitives.
//    <not_named> 
//    c             The colors for the cells.
//    t             The texture coordinates.
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
DrawCST3(vtkCellArray *aPrim, GLenum aGlFunction, int &cellNum, vtkPoints *p, 
         vtkDataArray *, vtkUnsignedCharArray *c, vtkDataArray *t) 
{
    int j; 
    vtkIdType npts, *pts;
    int count = 0;
    float polyNorm[3];
  
    GLenum previousGlFunction=GL_INVALID_VALUE;

    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); 
         count++, cellNum++)
    { 
        BeginPolyTriangleOrQuad(aGlFunction, previousGlFunction, npts);
    
        vtkPolygon::ComputeNormal(p,npts,pts,polyNorm);
        glColor4ubv(c->GetPointer(4*cellNum));
        glNormal3fv(polyNorm);
    
        for (j = 0; j < npts; j++) 
        {
            glTexCoord2fv(t->GetTuple2(pts[j]));
            glVertex3fv(p->GetPoint(pts[j]));
        }

        if ((previousGlFunction != GL_TRIANGLES) 
            && (previousGlFunction != GL_QUADS)
            && (previousGlFunction != GL_POINTS))
        {
            glEnd();
        }
    }
    if ((previousGlFunction == GL_TRIANGLES)
        || (previousGlFunction == GL_QUADS)
        || (previousGlFunction == GL_POINTS))
    {
        glEnd();
    }
}


  
// ****************************************************************************
//  Method:  Draw2
//
//  Purpose:
//    Draw triStrips with no colors or textures. Normals are computed.
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    aGlFunction   Which GL function is to be used for drawing. 
//    <not_named> 
//    p             The vertices of the primitives.
//    <not_named> 
//    <not_named> 
//    <not_named> 
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
Draw2(vtkCellArray *aPrim, GLenum aGlFunction, int &, vtkPoints *p, 
      vtkDataArray *, vtkUnsignedCharArray *, vtkDataArray *) 
{
    int j;
    vtkIdType npts, *pts;
    vtkIdType idx[3];
    int count = 0;
    float polyNorm[3];
  
    GLenum previousGlFunction=GL_INVALID_VALUE;

    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); count++)
    { 
        BeginPolyTriangleOrQuad(aGlFunction, previousGlFunction, npts);
    
        vtkTriangle::ComputeNormal(p,3,pts,polyNorm);
    
        for (j = 0; j < npts; j++) 
        {
            if ( j > 2)
            {
                if (j % 2)
                {
                    idx[0] = pts[j-2]; idx[1] = pts[j]; idx[2] = pts[j-1]; 
                    vtkTriangle::ComputeNormal(p,3, idx, polyNorm);
                }
                else
                {
                    idx[0] = pts[j-2]; idx[1] = pts[j-1]; idx[2] = pts[j]; 
                    vtkTriangle::ComputeNormal(p,3, idx, polyNorm);
                }
            }
            else if ( j == 0 )
            {
                vtkTriangle::ComputeNormal(p,3, pts, polyNorm);
            }
            glNormal3fv(polyNorm);
            glVertex3fv(p->GetPoint(pts[j]));
        }

        if ((previousGlFunction != GL_TRIANGLES) 
            && (previousGlFunction != GL_QUADS)
            && (previousGlFunction != GL_POINTS))
        {
            glEnd();
        }
    }
    if ((previousGlFunction == GL_TRIANGLES)
        || (previousGlFunction == GL_QUADS)
        || (previousGlFunction == GL_POINTS))
    {
        glEnd();
    }
}


// ****************************************************************************
//  Method:  DrawS2
//
//  Purpose:
//    Draw triStrips with vertex colors.  Normals are computed.
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    aGlFunction   Which GL function is to be used for drawing. 
//    <not_named> 
//    p             The vertices of the primitives.
//    <not_named> 
//    c             The colors for the vertices.
//    <not_named> 
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
DrawS2(vtkCellArray *aPrim, GLenum aGlFunction, int &, vtkPoints *p, 
       vtkDataArray *, vtkUnsignedCharArray *c, vtkDataArray *)
{
    int j;
    vtkIdType npts, *pts;
    vtkIdType idx[3];
    int count = 0;
    float polyNorm[3];
  
    GLenum previousGlFunction=GL_INVALID_VALUE;

    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); count++)
    { 
        BeginPolyTriangleOrQuad(aGlFunction, previousGlFunction, npts);
    
        vtkTriangle::ComputeNormal(p,3,pts,polyNorm);
    
        for (j = 0; j < npts; j++) 
        {
            glColor4ubv(c->GetPointer(4*pts[j]));
            if ( j > 2)
            {
                if (j % 2)
                {
                    idx[0] = pts[j-2]; idx[1] = pts[j]; idx[2] = pts[j-1]; 
                    vtkTriangle::ComputeNormal(p,3, idx, polyNorm);
                }
                else
                {
                    idx[0] = pts[j-2]; idx[1] = pts[j-1]; idx[2] = pts[j]; 
                    vtkTriangle::ComputeNormal(p,3, idx, polyNorm);
                }
            }
            else if ( j == 0 )
            {
                vtkTriangle::ComputeNormal(p,3, pts, polyNorm);
            }
            glNormal3fv(polyNorm);
            glVertex3fv(p->GetPoint(pts[j]));
        }

        if ((previousGlFunction != GL_TRIANGLES) 
            && (previousGlFunction != GL_QUADS)
            && (previousGlFunction != GL_POINTS))
        {
            glEnd();
        }
    }
    if ((previousGlFunction == GL_TRIANGLES)
        || (previousGlFunction == GL_QUADS)
        || (previousGlFunction == GL_POINTS))
    {
        glEnd();
    }
}


// ****************************************************************************
//  Method:  DrawT2
//
//  Purpose:
//    Draw triStrips with textures.  Normals are computed. 
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    aGlFunction   Which GL function is to be used for drawing. 
//    <not_named> 
//    p             The vertices of the primitives.
//    <not_named> 
//    <not_named> 
//    t             The texture coordinates.
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
DrawT2(vtkCellArray *aPrim, GLenum aGlFunction, int &, vtkPoints *p, 
       vtkDataArray *, vtkUnsignedCharArray *, vtkDataArray *t) 
{
    int j;
    vtkIdType npts, *pts;
    vtkIdType idx[3];
    int count = 0;
    float polyNorm[3];
  
    GLenum previousGlFunction=GL_INVALID_VALUE;

    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); count++)
    { 
        BeginPolyTriangleOrQuad(aGlFunction, previousGlFunction, npts);
    
        vtkTriangle::ComputeNormal(p,3,pts,polyNorm);
    
        for (j = 0; j < npts; j++) 
        {
            glTexCoord2fv(t->GetTuple2(pts[j]));
            if ( j > 2)
            {
                if (j % 2)
                {
                    idx[0] = pts[j-2]; idx[1] = pts[j]; idx[2] = pts[j-1]; 
                    vtkTriangle::ComputeNormal(p,3, idx, polyNorm);
                }
                else
                {
                    idx[0] = pts[j-2]; idx[1] = pts[j-1]; idx[2] = pts[j]; 
                    vtkTriangle::ComputeNormal(p,3, idx, polyNorm);
                }
            }
            else if ( j == 0 )
            {
                vtkTriangle::ComputeNormal(p,3, pts, polyNorm);
            }
            glNormal3fv(polyNorm);
            glVertex3fv(p->GetPoint(pts[j]));
        }

        if ((previousGlFunction != GL_TRIANGLES) 
            && (previousGlFunction != GL_QUADS)
            && (previousGlFunction != GL_POINTS))
        {
            glEnd();
        }
    }
    if ((previousGlFunction == GL_TRIANGLES)
        || (previousGlFunction == GL_QUADS)
        || (previousGlFunction == GL_POINTS))
    {
        glEnd();
    }
}


// ****************************************************************************
//  Method:  DrawST2
//
//  Purpose:
//    Draw triStrips with vertex colors and textures.  Normals are computed. 
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    aGlFunction   Which GL function is to be used for drawing. 
//    <not_named> 
//    p             The vertices of the primitives.
//    <not_named> 
//    c             The colors for the vertices.
//    t             The texture coordinates.
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
DrawST2(vtkCellArray *aPrim, GLenum aGlFunction, int &, vtkPoints *p, 
        vtkDataArray *, vtkUnsignedCharArray *c, vtkDataArray *t)
{
    int j;
    vtkIdType npts, *pts;
    vtkIdType idx[3];
    int count = 0;
    float polyNorm[3];
  
    GLenum previousGlFunction=GL_INVALID_VALUE;

    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); count++)
    { 
        BeginPolyTriangleOrQuad(aGlFunction, previousGlFunction, npts);
    
        vtkTriangle::ComputeNormal(p,3,pts,polyNorm);
    
        for (j = 0; j < npts; j++) 
        {
            glColor4ubv(c->GetPointer(4*pts[j]));
            glTexCoord2fv(t->GetTuple2(pts[j]));
            if ( j > 2)
            {
                if (j % 2)
                {
                    idx[0] = pts[j-2]; idx[1] = pts[j]; idx[2] = pts[j-1]; 
                    vtkTriangle::ComputeNormal(p,3, idx, polyNorm);
                }
                else
                {
                    idx[0] = pts[j-2]; idx[1] = pts[j-1]; idx[2] = pts[j]; 
                    vtkTriangle::ComputeNormal(p,3, idx, polyNorm);
                }
            }
            else if ( j == 0 )
            {
                vtkTriangle::ComputeNormal(p,3, pts, polyNorm);
            }
            glNormal3fv(polyNorm);
            glVertex3fv(p->GetPoint(pts[j]));
        }

        if ((previousGlFunction != GL_TRIANGLES) 
            && (previousGlFunction != GL_QUADS)
            && (previousGlFunction != GL_POINTS))
        {
            glEnd();
        }
    }
    if ((previousGlFunction == GL_TRIANGLES)
        || (previousGlFunction == GL_QUADS)
        || (previousGlFunction == GL_POINTS))
    {
        glEnd();
    }
}


// ****************************************************************************
//  Method:  DrawCS2
//
//  Purpose:
//    Draw triStrips with cell colors.  Normals are computed. 
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    aGlFunction   Which GL function is to be used for drawing. 
//    cellNum       Keeps track of the current cell number.
//    p             The vertices of the primitives.
//    <not_named> 
//    c             The colors for the 
//    <not_named> 
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, idx, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
DrawCS2(vtkCellArray *aPrim, GLenum aGlFunction, int &cellNum, vtkPoints *p, 
        vtkDataArray *, vtkUnsignedCharArray *c, vtkDataArray *) 
{
    int j; 
    vtkIdType npts, *pts;
    vtkIdType idx[3];
    int count = 0;
    float polyNorm[3];
  
    GLenum previousGlFunction=GL_INVALID_VALUE;

    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); 
         count++, cellNum++)
    { 
        BeginPolyTriangleOrQuad(aGlFunction, previousGlFunction, npts);
    
        vtkTriangle::ComputeNormal(p,3,pts,polyNorm);
        glColor4ubv(c->GetPointer(4*cellNum));
    
        for (j = 0; j < npts; j++) 
        {
            if ( j > 2)
            {
                if (j % 2)
                {
                    idx[0] = pts[j-2]; idx[1] = pts[j]; idx[2] = pts[j-1]; 
                    vtkTriangle::ComputeNormal(p,3, idx, polyNorm);
                }
                else
                {
                    idx[0] = pts[j-2]; idx[1] = pts[j-1]; idx[2] = pts[j]; 
                    vtkTriangle::ComputeNormal(p,3, idx, polyNorm);
                }
            }
            else if ( j == 0 )
            {
                vtkTriangle::ComputeNormal(p,3, pts, polyNorm);
            }
            glNormal3fv(polyNorm);
            glVertex3fv(p->GetPoint(pts[j]));
        }

        if ((previousGlFunction != GL_TRIANGLES) 
            && (previousGlFunction != GL_QUADS)
            && (previousGlFunction != GL_POINTS))
        {
            glEnd();
        }
    }
    if ((previousGlFunction == GL_TRIANGLES)
        || (previousGlFunction == GL_QUADS)
        || (previousGlFunction == GL_POINTS))
    {
        glEnd();
    }
}


// ****************************************************************************
//  Method:  DrawCST2
//
//  Purpose:
//    Draw triStrips with cell colors and textures.  Normals are computed. 
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    aGlFunction   Which GL function is to be used for drawing. 
//    cellNum       Keeps track of the current cell number.
//    p             The vertices of the primitives.
//    <not_named> 
//    c             The colors for the 
//    t             The texture coordinates.
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, idx, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
DrawCST2(vtkCellArray *aPrim, GLenum aGlFunction, int &cellNum, vtkPoints *p, 
         vtkDataArray *, vtkUnsignedCharArray *c, vtkDataArray *t) 
{
    int j;
    vtkIdType npts, *pts;
    vtkIdType idx[3];
    int count = 0;
    float polyNorm[3];
  
    GLenum previousGlFunction=GL_INVALID_VALUE;

    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); 
         count++, cellNum++)
    { 
        BeginPolyTriangleOrQuad(aGlFunction, previousGlFunction, npts);
    
        vtkTriangle::ComputeNormal(p,3,pts,polyNorm);
        glColor4ubv(c->GetPointer(4*cellNum));

        for (j = 0; j < npts; j++) 
        {
            glTexCoord2fv(t->GetTuple2(pts[j]));
            if ( j > 2)
            {
                if (j % 2)
                {
                    idx[0] = pts[j-2]; idx[1] = pts[j]; idx[2] = pts[j-1]; 
                    vtkTriangle::ComputeNormal(p,3, idx, polyNorm);
                }
                else
                {
                    idx[0] = pts[j-2]; idx[1] = pts[j-1]; idx[2] = pts[j]; 
                    vtkTriangle::ComputeNormal(p,3, idx, polyNorm);
                }
            }
            else if ( j == 0 )
            {
                vtkTriangle::ComputeNormal(p,3, pts, polyNorm);
            }
            glNormal3fv(polyNorm);
            glVertex3fv(p->GetPoint(pts[j]));
        }

        if ((previousGlFunction != GL_TRIANGLES) 
            && (previousGlFunction != GL_QUADS)
            && (previousGlFunction != GL_POINTS))
        {
            glEnd();
        }
    }
    if ((previousGlFunction == GL_TRIANGLES)
        || (previousGlFunction == GL_QUADS)
        || (previousGlFunction == GL_POINTS))
    {
        glEnd();
    }
}


// ****************************************************************************
//  Method:  DrawW
//
//  Purpose:
//    Draw wireframe with no colors or textures. Normals are computed.
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    <not_named> 
//    <not_named> 
//    p             The vertices of the primitives.
//    <not_named> 
//    <not_named> 
//    <not_named> 
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, idx, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
DrawW(vtkCellArray *aPrim, GLenum, int &, vtkPoints *p, 
      vtkDataArray *, vtkUnsignedCharArray *, vtkDataArray *) 
{
    int j;
    vtkIdType npts, *pts;
    vtkIdType idx[3];
    int count = 0;
    float polyNorm[3];

    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); count++)
    { 
        // draw first line
        glBegin(GL_LINE_STRIP);
        for (j = 0; j < npts; j += 2) 
        {
            if ( j == 0 )
            {
                vtkTriangle::ComputeNormal(p,3, pts, polyNorm);
            }
            else
            {
                idx[0] = pts[j-2]; idx[1] = pts[j-1]; idx[2] = pts[j]; 
                vtkTriangle::ComputeNormal(p,3, idx, polyNorm);
            }
            glNormal3fv(polyNorm);
            glVertex3fv(p->GetPoint(pts[j]));
        }
        glEnd();
      
        // draw second line
        glBegin(GL_LINE_STRIP);
        for (j = 1; j < npts; j += 2) 
        {
            if (j == 1)
            {
                vtkTriangle::ComputeNormal(p,3, pts, polyNorm);
            }
            else
            {
                idx[0] = pts[j-2]; idx[1] = pts[j]; idx[2] = pts[j-1]; 
                vtkTriangle::ComputeNormal(p,3, idx, polyNorm);
            }
            glNormal3fv(polyNorm);
            glVertex3fv(p->GetPoint(pts[j]));
        }
        glEnd();
    }
}


// ****************************************************************************
//  Method:  DrawNW
//
//  Purpose:
//    Draw wireframe triStrips with vertex normals. 
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    <not_named> 
//    <not_named> 
//    p             The vertices of the primitives.
//    n             The normals for the vertices.
//    <not_named> 
//    <not_named> 
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
DrawNW(vtkCellArray *aPrim, GLenum, int &, vtkPoints *p, 
       vtkDataArray *n, vtkUnsignedCharArray *, vtkDataArray *) 
{
    int j;
    vtkIdType npts, *pts;
    int count = 0;
  
    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); count++)
    { 
        // draw first line
        glBegin(GL_LINE_STRIP);
        for (j = 0; j < npts; j += 2) 
        {
            glNormal3fv(n->GetTuple3(pts[j]));
            glVertex3fv(p->GetPoint(pts[j]));
        }
        glEnd();
    
        // draw second line
        glBegin(GL_LINE_STRIP);
        for (j = 1; j < npts; j += 2) 
        {
            glNormal3fv(n->GetTuple3(pts[j]));
            glVertex3fv(p->GetPoint(pts[j]));
        }
        glEnd();
    }
}


// ****************************************************************************
//  Method:  DrawSW
//
//  Purpose:
//    Draw wireframe triStrips with vertex colors.  Normals are computed. 
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    <not_named> 
//    <not_named> 
//    p             The vertices of the primitives.
//    <not_named> 
//    c             The colors for the vertices.
//    <not_named> 
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, idx, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
DrawSW(vtkCellArray *aPrim, GLenum, int &, vtkPoints *p, 
       vtkDataArray *, vtkUnsignedCharArray *c, vtkDataArray *) 
{
    int j;
    vtkIdType npts, *pts;
    vtkIdType idx[3];
    int count = 0;
    float polyNorm[3];
  
    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); count++)
    { 
        // draw first line
        glBegin(GL_LINE_STRIP);
        for (j = 0; j < npts; j += 2) 
        {
            glColor4ubv(c->GetPointer(4*pts[j]));
            if ( j == 0 )
            {
                vtkTriangle::ComputeNormal(p, 3, pts, polyNorm);
            }
            else
            {
                idx[0] = pts[j-2]; idx[1] = pts[j-1]; idx[2] = pts[j]; 
                vtkTriangle::ComputeNormal(p,3, idx, polyNorm);
            }
            glNormal3fv(polyNorm);
            glVertex3fv(p->GetPoint(pts[j]));
        }
        glEnd();
    
        // draw second line
        glBegin(GL_LINE_STRIP);
        for (j = 1; j < npts; j += 2) 
        {
            glColor4ubv(c->GetPointer(4*pts[j]));
            if (j == 1)
            {
                vtkTriangle::ComputeNormal(p,3, pts, polyNorm);
            }
            else
            {
                idx[0] = pts[j-2]; idx[1] = pts[j]; idx[2] = pts[j-1]; 
                vtkTriangle::ComputeNormal(p,3, idx, polyNorm);
            }
            glNormal3fv(polyNorm);
            glVertex3fv(p->GetPoint(pts[j]));
        }
        glEnd();
    }
}


// ****************************************************************************
//  Method:  DrawNSW
//
//  Purpose:
//    Draw wireframe trisStrips with vertex normals and vertex colors. 
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    <not_named> 
//    <not_named> 
//    p             The vertices of the primitives.
//    n             The normals for the vertices.
//    c             The colors for the vertices.
//    <not_named> 
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
DrawNSW(vtkCellArray *aPrim, GLenum, int &, vtkPoints *p, vtkDataArray *n, 
        vtkUnsignedCharArray *c, vtkDataArray *)
{
    int j; 
    vtkIdType npts, *pts;
    int count = 0;
  
    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); count++)
    { 
        // draw first line
        glBegin(GL_LINE_STRIP);
        for (j = 0; j < npts; j += 2) 
        {
            glColor4ubv(c->GetPointer(4*pts[j]));
            glNormal3fv(n->GetTuple3(pts[j]));
            glVertex3fv(p->GetPoint(pts[j]));
        }
        glEnd();
    
        // draw second line
        glBegin(GL_LINE_STRIP);
        for (j = 1; j < npts; j += 2) 
        {
            glColor4ubv(c->GetPointer(4*pts[j]));
            glNormal3fv(n->GetTuple3(pts[j]));
            glVertex3fv(p->GetPoint(pts[j]));
        }
        glEnd();
    }
}


// ****************************************************************************
//  Method:  DrawTW
//
//  Purpose:
//    Draw wireframe triStrips with textures.  Normals are computed. 
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    <not_named> 
//    <not_named> 
//    p             The vertices of the primitives.
//    <not_named> 
//    <not_named> 
//    t             The texture coordinates.
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, idx, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
DrawTW(vtkCellArray *aPrim, GLenum, int &, vtkPoints *p, vtkDataArray *, 
       vtkUnsignedCharArray *, vtkDataArray *t) 
{
    int j; 
    vtkIdType npts, *pts;
    vtkIdType idx[3];
    int count = 0;
    float polyNorm[3];
  
    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); count++)
    { 

        // draw first line
        glBegin(GL_LINE_STRIP);

        for (j = 0; j < npts; j += 2) 
        {
            if ( j == 0 )
            {
                vtkTriangle::ComputeNormal(p,3, pts, polyNorm);
            }
            else
            {
                idx[0] = pts[j-2]; idx[1] = pts[j-1]; idx[2] = pts[j]; 
                vtkTriangle::ComputeNormal(p,3, idx, polyNorm);
            }
            glNormal3fv(polyNorm);
            glTexCoord2fv(t->GetTuple2(pts[j]));
            glVertex3fv(p->GetPoint(pts[j]));
        }
        glEnd();
    
        // draw second line
        glBegin(GL_LINE_STRIP);
        for (j = 1; j < npts; j += 2) 
        {
            if (j == 1)
            {
                vtkTriangle::ComputeNormal(p,3, pts, polyNorm);
            }
            else
            {
                idx[0] = pts[j-2]; idx[1] = pts[j]; idx[2] = pts[j-1]; 
                vtkTriangle::ComputeNormal(p,3, idx, polyNorm);
            }
            glNormal3fv(polyNorm);
            glTexCoord2fv(t->GetTuple2(pts[j]));
            glVertex3fv(p->GetPoint(pts[j]));
        }
        glEnd();
    }
}


// ****************************************************************************
//  Method:  DrawNTW
//
//  Purpose:
//    Draw wireframe triStrips with vertex normals and textures. 
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    <not_named> 
//    <not_named> 
//    p             The vertices of the primitives.
//    n             The normals for the vertices.
//    <not_named> 
//    t             The texture coordinates.
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
DrawNTW(vtkCellArray *aPrim, GLenum, int &, vtkPoints *p, vtkDataArray *n, 
        vtkUnsignedCharArray *, vtkDataArray *t) 
{
    int j; 
    vtkIdType npts, *pts;
    int count = 0;
  
    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); count++)
    { 
        glBegin(GL_LINE_STRIP);
        for (j = 0; j < npts; j += 2) 
        {
            glNormal3fv(n->GetTuple3(pts[j]));
            glTexCoord2fv(t->GetTuple2(pts[j]));
            glVertex3fv(p->GetPoint(pts[j]));
        }
        glEnd();
    
        // draw second line
        glBegin(GL_LINE_STRIP);
        for (j = 1; j < npts; j += 2) 
        {
            glNormal3fv(n->GetTuple3(pts[j]));
            glTexCoord2fv(t->GetTuple2(pts[j]));
            glVertex3fv(p->GetPoint(pts[j]));
        }
        glEnd();
    }
}


// ****************************************************************************
//  Method:  DrawSTW
//
//  Purpose:
//    Draw wireframe triStrips with vertex colors and textures. 
//    Normals are calculated.
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    <not_named> 
//    <not_named> 
//    p             The vertices of the primitives.
//    <not_named> 
//    c             The colors for the  vertices.
//    t             The texture coordinates.
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, idx, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
DrawSTW(vtkCellArray *aPrim, GLenum, int &, vtkPoints *p, vtkDataArray *, 
        vtkUnsignedCharArray *c, vtkDataArray *t)
{
    int j; 
    vtkIdType npts, *pts;
    vtkIdType idx[3];
    int count = 0;
    float polyNorm[3];
  
    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); count++)
    { 
        // draw first line
        glBegin(GL_LINE_STRIP);
        for (j = 0; j < npts; j += 2) 
        {
            glColor4ubv(c->GetPointer(4*pts[j]));
            if ( j == 0 )
            {
                vtkTriangle::ComputeNormal(p,3, pts, polyNorm);
            }
            else
            {
                idx[0] = pts[j-2]; idx[1] = pts[j-1]; idx[2] = pts[j]; 
                vtkTriangle::ComputeNormal(p,3, idx, polyNorm);
            }
            glNormal3fv(polyNorm);
            glTexCoord2fv(t->GetTuple2(pts[j]));
            glVertex3fv(p->GetPoint(pts[j]));
        }
        glEnd();
      
        // draw second line
        glBegin(GL_LINE_STRIP);
        for (j = 1; j < npts; j += 2) 
        {
            glColor4ubv(c->GetPointer(4*pts[j]));
            if (j == 1)
            {
                vtkTriangle::ComputeNormal(p,3, pts, polyNorm);
            }
            else
            {
                idx[0] = pts[j-2]; idx[1] = pts[j]; idx[2] = pts[j-1]; 
                vtkTriangle::ComputeNormal(p,3, idx, polyNorm);
            }
            glNormal3fv(polyNorm);
            glTexCoord2fv(t->GetTuple2(pts[j]));
            glVertex3fv(p->GetPoint(pts[j]));
        }
        glEnd();
    }
}


// ****************************************************************************
//  Method:  DrawNSTW
//
//  Purpose:
//    Draw wireframe triStrips with vertex normals, vertex colors,
//    and textures.
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Arguments:  
//    aPrim         A cell-array of the geometric primitives to be drawn.
//    <not_named> 
//    <not_named> 
//    p             The vertices of the primitives.
//    n             The normals for the vertices.
//    c             The colors for the  vertices.
//    t             The texture coordinates.
//   
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//    Use vtkIdType for pts, npts, use vtkUnsignedCharArray for colors, to
//    match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals and vtkTCoords have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Tue Apr 23 19:25:33 PDT 2002
//    Made function static.
//
// ****************************************************************************

static void 
DrawNSTW(vtkCellArray *aPrim, GLenum, int &, vtkPoints *p, vtkDataArray *n, 
         vtkUnsignedCharArray *c, vtkDataArray *t) 
{
    int j; 
    vtkIdType npts, *pts;
    int count = 0;
  
    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); count++)
    { 
        // draw first line
        glBegin(GL_LINE_STRIP);
        for (j = 0; j < npts; j += 2) 
        {
            glColor4ubv(c->GetPointer(4*pts[j]));
            glNormal3fv(n->GetTuple3(pts[j]));
            glTexCoord2fv(t->GetTuple2(pts[j]));
            glVertex3fv(p->GetPoint(pts[j]));
        }
        glEnd();
    
        // draw second line
        glBegin(GL_LINE_STRIP);
        for (j = 1; j < npts; j += 2) 
        {
            glColor4ubv(c->GetPointer(4*pts[j]));
            glNormal3fv(n->GetTuple3(pts[j]));
            glTexCoord2fv(t->GetTuple2(pts[j]));
            glVertex3fv(p->GetPoint(pts[j]));
        }
        glEnd();
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
//    Hank Childs, Mon May  5 18:17:23 PDT 2003
//    Do not assume that we have a valid VTK renderer.
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
    if (inputNum >= setupListId.size())
        setupListId.push_back(0);

    VTKRen->GetRenderWindow()->MakeCurrent();

    //
    //  See if display list needs to be regenerated. 
    //
    if (setupModified[inputNum] || (propMTime[inputNum] < prop->GetMTime()))
    {
        if (setupListId[inputNum])
        {
            glDeleteLists(setupListId[inputNum], 1);
        }
        if (!immediateModeRendering) 
        {
            // 
            // Get a unique display list id  
            // 
            setupListId[inputNum] = glGenLists(1);
            glNewList(setupListId[inputNum], GL_COMPILE);
            SetupGraphicsLibrary2();
            glEndList();
#ifdef DEBUG_GL_LIST_IDS
            if (!glIsList(setupListId[inputNum]))
               cerr << "calling invalid setup list" << endl;
#endif
            glCallList(setupListId[inputNum]);
        }
    }
    else
    {
        // 
        // If nothing changed but we are using display lists, set up.
        // 
        if (!immediateModeRendering)
        {
#ifdef DEBUG_GL_LIST_IDS
            if (!glIsList(setupListId[inputNum]))
               cerr << "calling invalid setup list" << endl;
#endif
            glCallList(setupListId[inputNum]);
        }
    }

    // 
    // If we are in immediate mode rendering we always
    // want to set up the graphics lib here.
    // 
    if (immediateModeRendering) 
    {
        SetupGraphicsLibrary2();
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
    float  color[4];

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
    float coeff = prop->GetAmbient();
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
    glColor4fv(color);

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
//    Hank Childs, Mon May  5 18:17:23 PDT 2003
//    Do not assume that we have a valid VTK renderer.
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
    if (inputNum >= surfaceListId.size())
        surfaceListId.push_back(0);

    VTKRen->GetRenderWindow()->MakeCurrent();

    //
    //  See if display list needs to be regenerated. 
    //
    if (  lutColorsChanged ||
        ((propMTime[inputNum] < prop->GetMTime()) &&
        ((lastRep[inputNum]   != prop->GetRepresentation()) ||
         (lastInterp[inputNum] != prop->GetInterpolation()) )))
           surfaceModified[inputNum] = true;

    if (surfaceModified[inputNum])
    {
        if (surfaceListId[inputNum])
        {
            glDeleteLists(surfaceListId[inputNum], 1);
        }
        if (!immediateModeRendering)
        {
            // 
            // Get a unique display list id  
            // 
            surfaceListId[inputNum] = glGenLists(1);
            glNewList(surfaceListId[inputNum], GL_COMPILE);
            DrawSurface2();
            glEndList();
#ifdef DEBUG_GL_LIST_IDS
            if (!glIsList(surfaceListId[inputNum]))
               cerr << "calling invalid surface list" << endl;
#endif
            glCallList(surfaceListId[inputNum]);
        }
    }
    else 
    {
        // 
        // If nothing changed but we are using display lists, draw it.
        // 
        if (!immediateModeRendering)
        {
#ifdef DEBUG_GL_LIST_IDS
            if (!glIsList(surfaceListId[inputNum]))
               cerr << "calling invalid surface list" << endl;
#endif
            glCallList(surfaceListId[inputNum]);
        }
    }

    // 
    // If we are in immediate mode rendering we always
    // want to draw the primitives here.
    // 
    if (immediateModeRendering)
    {
        DrawSurface2();
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
// ****************************************************************************

void 
avtOpenGLSurfaceAndWireframeRenderer::DrawSurface2()
{
    int rep;
    GLenum glFunction[4], aGlFunction;
    vtkPoints *p;
    vtkCellArray *aPrim;
    vtkUnsignedCharArray *c=NULL;
    vtkDataArray *n;
    vtkDataArray *t;
    int tDim;
    int cellScalars = 0;
    int cellNum = 0;
    int cellNormals = 0;
  
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
             debug5 << "Bad representation sent\n";
             glFunction[0] = GL_POINTS;
             glFunction[1] = GL_LINE_STRIP;
             glFunction[2] = GL_TRIANGLE_STRIP;
             glFunction[3] = GL_POLYGON;
             break;
    }

    // and draw the display list
    p = input->GetPoints();
  
    // are they cell or point scalars
    if (this->Colors && scalarVisibility)
    {
        c = this->Colors;
        if (!input->GetPointData()->GetScalars() &&
             input->GetCellData()->GetScalars())
        {
            cellScalars = 1;
        }
    }
    
    t = input->GetPointData()->GetTCoords();
    if (t) 
    {
        tDim = t->GetNumberOfComponents();
        if (tDim != 2)
        {
            debug5 << "Currently only 2d textures are supported.\n";
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
    //  Create an index that helps determine which drawing function to use.
    //
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

    // how do we draw points
    void (*draw0)(vtkCellArray *, GLenum, int &, vtkPoints *, vtkDataArray *, 
                  vtkUnsignedCharArray *, vtkDataArray *);

    switch (idx) 
    {
        case  0: draw0 = Draw01;       break;
        case  1: draw0 = DrawN013;     break;
        case  2: draw0 = DrawS01;      break;
        case  3: draw0 = DrawNS013;    break;
        case  4: draw0 = DrawT01;      break;
        case  5: draw0 = DrawNT013;    break;
        case  6: draw0 = DrawST01;     break;
        case  7: draw0 = DrawNST013;   break;
        case 10: draw0 = DrawCS01;     break;
        case 11: draw0 = DrawNCS013;   break;
        case 14: draw0 = DrawCST01;    break;
        case 15: draw0 = DrawNCST013;  break;
        case 16: draw0 = DrawCN013;    break;
        case 18: draw0 = DrawCNS013;   break;
        case 20: draw0 = DrawCNT013;   break;
        case 22: draw0 = DrawCNST013;  break;
        case 26: draw0 = DrawCNCS013;  break;
        case 30: draw0 = DrawCNCST013; break;
    }

    // how do we draw lines
    void (*draw1)(vtkCellArray *, GLenum, int &, vtkPoints *, vtkDataArray *, 
                  vtkUnsignedCharArray *, vtkDataArray *);
    switch (idx) 
    {
        case  0: draw1 = Draw01;       break; 
        case  1: draw1 = DrawN013;     break; 
        case  2: draw1 = DrawS01;      break; 
        case  3: draw1 = DrawNS013;    break; 
        case  4: draw1 = DrawT01;      break; 
        case  5: draw1 = DrawNT013;    break; 
        case  6: draw1 = DrawST01;     break; 
        case  7: draw1 = DrawNST013;   break; 
        case 10: draw1 = DrawCS01;     break; 
        case 11: draw1 = DrawNCS013;   break; 
        case 14: draw1 = DrawCST01;    break; 
        case 15: draw1 = DrawNCST013;  break; 
        case 16: draw1 = DrawCN013;    break; 
        case 18: draw1 = DrawCNS013;   break; 
        case 20: draw1 = DrawCNT013;   break; 
        case 22: draw1 = DrawCNST013;  break; 
        case 26: draw1 = DrawCNCS013;  break; 
        case 30: draw1 = DrawCNCST013; break; 
    }

    // how do we draw tstrips
    void (*draw2)(vtkCellArray *, GLenum, int &, vtkPoints *, vtkDataArray *, 
                  vtkUnsignedCharArray *, vtkDataArray *);
    void (*draw2W)(vtkCellArray *, GLenum, int &, vtkPoints *, vtkDataArray *, 
                   vtkUnsignedCharArray *, vtkDataArray *);
    switch (idx) 
    {
        case  0: draw2 = Draw2;       break;
        case  1: draw2 = DrawN013;    break;
        case  2: draw2 = DrawS2;      break;
        case  3: draw2 = DrawNS013;   break;
        case  4: draw2 = DrawT2;      break;
        case  5: draw2 = DrawNT013;   break;
        case  6: draw2 = DrawST2;     break;
        case  7: draw2 = DrawNST013;  break;
        case 10: draw2 = DrawCS2;     break;
        case 11: draw2 = DrawNCS013;  break;
        case 14: draw2 = DrawCST2;    break;
        case 15: draw2 = DrawNCST013; break;
        case 16: draw2 = Draw2;       break;
        case 18: draw2 = DrawS2;      break;
        case 20: draw2 = DrawT2;      break;
        case 22: draw2 = DrawST2;     break;
        case 26: draw2 = DrawCS2;     break;
        case 30: draw2 = DrawCST2;    break;
    }
    switch (idx)
    {
        case  0: draw2W = DrawW;    break;
        case  1: draw2W = DrawNW;   break;
        case  2: draw2W = DrawSW;   break;
        case  3: draw2W = DrawNSW;  break;
        case  4: draw2W = DrawTW;   break;
        case  5: draw2W = DrawNTW;  break;
        case  6: draw2W = DrawSTW;  break;
        case  7: draw2W = DrawNSTW; break;
        case 10: draw2W = DrawW;    break;
        case 11: draw2W = DrawNW;   break;
        case 14: draw2W = DrawTW;   break;
        case 15: draw2W = DrawNTW;  break;
        case 16: draw2W = DrawW;    break;
        case 18: draw2W = DrawSW;   break;
        case 20: draw2W = DrawTW;   break;
        case 22: draw2W = DrawSTW;  break;
        case 26: draw2W = DrawW;    break;
        case 30: draw2W = DrawTW;   break;
    }
  
    // how do we draw polys
    void (*draw3)(vtkCellArray *, GLenum, int &, vtkPoints *, vtkDataArray *, 
                   vtkUnsignedCharArray *, vtkDataArray *);
    switch (idx) 
    {
        case  0: draw3 = Draw3;        break;
        case  1: draw3 = DrawN013;     break;
        case  2: draw3 = DrawS3;       break;
        case  3: draw3 = DrawNS013;    break;
        case  4: draw3 = DrawT3;       break;
        case  5: draw3 = DrawNT013;    break;
        case  6: draw3 = DrawST3;      break;
        case  7: draw3 = DrawNST013;   break;
        case 10: draw3 = DrawCS3;      break;
        case 11: draw3 = DrawNCS013;   break;
        case 14: draw3 = DrawCST3;     break;
        case 15: draw3 = DrawNCST013;  break;
        case 16: draw3 = DrawCN013;    break;
        case 18: draw3 = DrawCNS013;   break;
        case 20: draw3 = DrawCNT013;   break;
        case 22: draw3 = DrawCNST013;  break;
        case 26: draw3 = DrawCNCS013;  break;
        case 30: draw3 = DrawCNCST013; break;
    }

    if (resolveTopology) 
    {
#ifdef GL_VERSION_1_1
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.,1.);
#endif      
    }


    // do verts
    aPrim = input->GetVerts();
    aGlFunction = glFunction[0];

    // For verts or lines that have no normals, disable shading.
    // This will fall back on the color set in the glColor4fv() 
    // call in vtkOpenGLProperty::Render() - the color returned
    // by vtkProperty::GetColor() with alpha set to 1.0.
    if (!n)
    {
        glDisable(GL_LIGHTING);
    }
  
    if (drawSurfaceVerts)
    {
        draw0(aPrim, aGlFunction, cellNum, p, n, c, t);
    }
  
    // do lines
    aPrim = input->GetLines();
    aGlFunction = glFunction[1];
  
    // draw all the elements
    if (drawSurfaceLines)
    {
        draw1(aPrim, aGlFunction, cellNum, p, n, c, t);
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
  
    // do tstrips
    aPrim = input->GetStrips();
    aGlFunction = glFunction[2];
    if (drawSurfaceStrips)
    {
        draw2(aPrim, aGlFunction, cellNum, p, n, c, t);
        if (rep == VTK_WIREFRAME)   
        {
            draw2W(aPrim, aGlFunction, cellNum, p, n, c, t);
        }
    }

    // do polys
    aPrim = input->GetPolys();
    aGlFunction = glFunction[3];
    if (drawSurfacePolys)
    {
        draw3(aPrim, aGlFunction, cellNum, p, n, c, t);
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
//    Hank Childs, Mon May  5 18:17:23 PDT 2003
//    Do not assume that we have a valid VTK renderer.
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
    if (inputNum >= edgesListId.size())
        edgesListId.push_back(0);

    VTKRen->GetRenderWindow()->MakeCurrent();

    //
    //  See if display list needs to be regenerated. 
    //
    if (propMTime[inputNum] < prop->GetMTime())
    {
        bool colorChanged = ((lastEdgeColor[inputNum].r != prop->GetEdgeColor()[0]) ||
                             (lastEdgeColor[inputNum].g != prop->GetEdgeColor()[1]) ||
                             (lastEdgeColor[inputNum].b != prop->GetEdgeColor()[2]) );
        if ( colorChanged || 
            (lastRep[inputNum] != prop->GetRepresentation()) ||
            (lastInterp[inputNum] != prop->GetInterpolation()))
           edgesModified[inputNum] = true;
    }
    
    if (edgesModified[inputNum]) 
    {
        if (edgesListId[inputNum])
        {
            glDeleteLists(edgesListId[inputNum], 1);
        }
        if (!immediateModeRendering)
        {
            // 
            // Get a unique display list id  
            // 
            edgesListId[inputNum] = glGenLists(1);
            glNewList(edgesListId[inputNum], GL_COMPILE);
            DrawEdges2();
            glEndList();
#ifdef DEBUG_GL_LIST_IDS
            if (!glIsList(edgesListId[inputNum]))
               cerr << "calling invalid edges list" << endl;
#endif
            glCallList(edgesListId[inputNum]);
        }
    }
    else 
    {
        // 
        // If nothing changed but we are using display lists, draw it.
        // 
        if (!immediateModeRendering)
        {
#ifdef DEBUG_GL_LIST_IDS
            if (!glIsList(edgesListId[inputNum]))
               cerr << "calling invalid edges list" << endl;
#endif
            glCallList(edgesListId[inputNum]);
        }
    }

    // if we are in immediate mode rendering we always
    // want to draw the primitives here
    if (immediateModeRendering) 
    {
        DrawEdges2();
    }
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
// ****************************************************************************

void
avtOpenGLSurfaceAndWireframeRenderer::DrawEdges2()
{
    GLenum aGlFunction;
    vtkPoints *p;
    vtkCellArray *aPrim;
    vtkDataArray *n;
    vtkDataArray *t;
    int tDim;
    int cellNum = 0;
    int cellNormals = 0;
  
    p = input->GetPoints();
  
    float edgeColor[4];
    prop->GetEdgeColor(edgeColor); 
    edgeColor[3] = 1.; 
    glColor4fv(edgeColor);

    t = input->GetPointData()->GetTCoords();
    if (t) 
    {
        tDim = t->GetNumberOfComponents();
        if (tDim != 2)
        {
            debug5 << "Currently only 2d textures are supported.\n";
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
    int idx;
    if (n && !cellNormals)
    {
        idx = 1;
    }
    else
    {
        idx = 0;
    }
    if (t)
    {
        idx += 4;
    }
    if (cellNormals)
    {
        idx += 16;
    }

    // how do we draw lines
    void (*draw1)(vtkCellArray *, GLenum, int &, vtkPoints *, vtkDataArray *, 
                  vtkUnsignedCharArray *, vtkDataArray *);
    switch (idx) 
    {
        case  0: draw1 = Draw01;     break; 
        case  1: draw1 = DrawN013;   break; 
        case  4: draw1 = DrawT01;    break; 
        case  5: draw1 = DrawNT013;  break; 
        case 16: draw1 = DrawCN013;  break; 
        case 20: draw1 = DrawCNT013; break; 
    }

    // how do we draw tstrips
    void (*draw2)(vtkCellArray *, GLenum, int &, vtkPoints *, vtkDataArray *, 
                  vtkUnsignedCharArray *, vtkDataArray *);
    void (*draw2W)(vtkCellArray *, GLenum, int &, vtkPoints *, vtkDataArray *, 
                   vtkUnsignedCharArray *, vtkDataArray *);
    switch (idx) 
    {
        case  0: draw2 = Draw2; break;
        case  1: draw2 = DrawN013; break;
        case  4: draw2 = DrawT2; break;
        case  5: draw2 = DrawNT013; break;
        case 16: draw2 = Draw2; break;
        case 20: draw2 = DrawT2; break;
    }
    switch (idx)
    {
        case  0: draw2W = DrawW; break;
        case  1: draw2W = DrawNW; break;
        case  4: draw2W = DrawTW; break;
        case  5: draw2W = DrawNTW; break;
        case 16: draw2W = DrawW; break;
        case 20: draw2W = DrawTW; break;
    }
  
    // how do we draw polys
    void (*draw3)(vtkCellArray *, GLenum, int &, vtkPoints *, vtkDataArray *, 
                  vtkUnsignedCharArray *, vtkDataArray *);
    switch (idx) 
    {
        case  0: draw3 = Draw3; break;
        case  1: draw3 = DrawN013; break;
        case  4: draw3 = DrawT3; break;
        case  5: draw3 = DrawNT013; break;
        case 16: draw3 = DrawCN013; break;
        case 20: draw3 = DrawCNT013; break;
    }

    glDisable(GL_LIGHTING);

    aPrim = input->GetLines();
    aGlFunction = GL_LINE_STRIP; 
  
    // draw all the elements
    if (drawEdgeLines)
    {
        draw1(aPrim, aGlFunction, cellNum, p, n, NULL, t); 
    }
  
        
    // do tstrips
    // Kat's note:  Not certain both of these draw methods are necesssary
    // here.  Pulled from the normal Draw method.  draw2 and draw2W
    // compute normals differently.
    aPrim = input->GetStrips();
    if (drawEdgeStrips)
    {
        draw2(aPrim, aGlFunction, cellNum, p, n, NULL, t);
        if (prop->GetRepresentation() == VTK_WIREFRAME)   
        {
            draw2W(aPrim, aGlFunction, cellNum, p, n, NULL, t);
        }
    }

    // do polys
    aPrim = input->GetPolys();
    aGlFunction = GL_LINE_LOOP;
    if (drawEdgePolys)
    {
        draw3(aPrim, aGlFunction, cellNum, p, n, NULL, t);
    }

    glEnable(GL_LIGHTING);
} // DrawEdges



