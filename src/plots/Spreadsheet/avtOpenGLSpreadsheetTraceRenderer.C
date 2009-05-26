/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
#include <avtOpenGLSpreadsheetTraceRenderer.h>

#include <float.h>

#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkCell.h>

#include <DebugStream.h>

// Include OpenGL
#ifndef VTK_IMPLEMENT_MESA_CXX
  #if defined(__APPLE__) && (defined(VTK_USE_CARBON) || defined(VTK_USE_COCOA))
    #include <OpenGL/gl.h>
  #else
    #if defined(_WIN32)
       #include <windows.h>
    #endif
    #include <GL/gl.h>
  #endif
#else
  #include <GL/gl.h>
#endif

// ****************************************************************************
// Class: avtOpenGLSpreadsheetTraceRenderer::avtOpenGLSpreadsheetTraceRenderer
//
// Purpose:
//   Constructor for the avtOpenGLSpreadsheetTraceRenderer class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 21 09:57:23 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

avtOpenGLSpreadsheetTraceRenderer::avtOpenGLSpreadsheetTraceRenderer()
{
}

// ****************************************************************************
// Class: avtOpenGLSpreadsheetTraceRenderer::avtOpenGLSpreadsheetTraceRenderer
//
// Purpose:
//   Destructor for the avtOpenGLSpreadsheetTraceRenderer class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 21 09:57:23 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

avtOpenGLSpreadsheetTraceRenderer::~avtOpenGLSpreadsheetTraceRenderer()
{
}

// ****************************************************************************
// Method: avtOpenGLSpreadsheetTraceRenderer::Render
//
// Purpose: 
//   Draws the input dataset according to its type.
//
// Arguments:
//   ds      : The dataset to render.
//   bounds  : The bounds of the dataset.
//   atts    : The Spreadsheet plot's attributes.
//   fgColor : The foreground color.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 14:33:14 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtOpenGLSpreadsheetTraceRenderer::Render(vtkDataSet *ds, vtkDataArray *bounds,
    const SpreadsheetAttributes &atts, const double *fgColor)
{
    glDisable(GL_LIGHTING);

    if(ds->IsA("vtkRectilinearGrid"))
    {
        vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *)ds;
        DrawRectilinearGrid(rgrid, bounds, atts, fgColor);
    }
    else if(ds->IsA("vtkStructuredGrid"))
    {
        vtkStructuredGrid *sgrid = (vtkStructuredGrid *)ds;
        DrawStructuredGrid(sgrid, bounds, atts, fgColor);
    }
    else
        DrawBoundingBox(bounds, fgColor);

    glEnable(GL_LIGHTING);
}


// ****************************************************************************
// Method: avtOpenGLSpreadsheetTraceRenderer::RenderRectilinearGrid
//
// Purpose: 
//   Draws the input dataset as a rectilinear grid.
//
// Arguments:
//   rgrid  : The dataset to render.
//   bounds : The bounds of the dataset.
//   atts    : The Spreadsheet plot's attributes.
//   fgColor : The foreground color.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 14:33:14 PST 2007
//
// Modifications:
//   
//   Gunther H. Weber, Wed Oct 17 14:48:16 PDT 2007
//   Support toggling patch outline and tracer plane separately
//
//   Gunther H. Weber, Thu Nov 29 15:04:29 PST 2007
//   Support rendering outline of current cell
//
//   Brad Whitlock, Tue May 26 10:25:49 PDT 2009
//   We already have the cellId from the pick if it's a zone pick.
//
// ****************************************************************************

void
avtOpenGLSpreadsheetTraceRenderer::DrawRectilinearGrid(vtkRectilinearGrid *rgrid,
    vtkDataArray *bounds, const SpreadsheetAttributes &atts, 
    const double *fgColor)
{
    // Get dimensions
    int dims[3];
    rgrid->GetDimensions(dims);

    // Compute id of current cell if necessary
    bool drawCellOutline = atts.GetShowCurrentCellOutline() &&
                           atts.GetCurrentPickValid() &&
                           atts.GetCurrentPickType() == 0;
    vtkIdType cellId = (vtkIdType)atts.GetCurrentPick();

    if(dims[2] < 2)
    {
        if (atts.GetShowTracerPlane())
        {
            // Draw the tracer plane.
            glBegin(GL_QUADS);
            glColor4ubv(atts.GetTracerColor().GetColor());
            glVertex3d(bounds->GetTuple1(0), bounds->GetTuple1(2), 0.);
            glVertex3d(bounds->GetTuple1(1), bounds->GetTuple1(2), 0.);
            glVertex3d(bounds->GetTuple1(1), bounds->GetTuple1(3), 0.);
            glVertex3d(bounds->GetTuple1(0), bounds->GetTuple1(3), 0.);         
            glEnd();
        }

        if (atts.GetShowPatchOutline())
        {
            // Draw the outline of the mesh
            glColor3dv(fgColor);
            glLineWidth(2.);
            glBegin(GL_LINE_LOOP);
            glVertex3d(bounds->GetTuple1(0), bounds->GetTuple1(2), 0.);
            glVertex3d(bounds->GetTuple1(1), bounds->GetTuple1(2), 0.);
            glVertex3d(bounds->GetTuple1(1), bounds->GetTuple1(3), 0.);
            glVertex3d(bounds->GetTuple1(0), bounds->GetTuple1(3), 0.);
            glEnd();
        }

        if (drawCellOutline)
        {
            vtkCell *cell = rgrid->GetCell(cellId);
            if (cell)
                Draw2DCell(cell, fgColor);
            else
                debug1 << "Internal error: Cannot locate cell." << std::endl;
        }
    }
    else
    {
        if (atts.GetShowTracerPlane())
        {

            vtkDataArray *arr = rgrid->GetPointData()->GetScalars();
            bool cellCentered = (arr == 0);

            if(atts.GetNormal() == SpreadsheetAttributes::X)
            {
                double X;

                vtkDataArray *xcoords = rgrid->GetXCoordinates();
                if(xcoords != 0 && atts.GetSliceIndex() < xcoords->GetNumberOfTuples())
                {
                    if(cellCentered)
                    {
                        X = (xcoords->GetTuple1(atts.GetSliceIndex()) + 
                                xcoords->GetTuple1(atts.GetSliceIndex() + 1)) / 2.;
                    }
                    else
                        X = xcoords->GetTuple1(atts.GetSliceIndex());
                }
                else
                    X = (bounds->GetTuple1(0) + bounds->GetTuple1(1)) / 2.;

                // Draw the tracer plane.
                if(atts.GetShowTracerPlane() && atts.GetTracerColor().Alpha() > 0)
                {
                    glBegin(GL_QUADS);
                    glColor4ubv(atts.GetTracerColor().GetColor());
                    glVertex3d(X, bounds->GetTuple1(2), bounds->GetTuple1(5));
                    glVertex3d(X, bounds->GetTuple1(2), bounds->GetTuple1(4));
                    glVertex3d(X, bounds->GetTuple1(3), bounds->GetTuple1(4));
                    glVertex3d(X, bounds->GetTuple1(3), bounds->GetTuple1(5));         
                    glEnd();
                }
                // Outline the plane
                glLineWidth(2.);
                glColor3ubv(atts.GetTracerColor().GetColor()); // 3ubv so opaque
                glBegin(GL_LINE_LOOP);
                glVertex3d(X, bounds->GetTuple1(2), bounds->GetTuple1(5));
                glVertex3d(X, bounds->GetTuple1(2), bounds->GetTuple1(4));
                glVertex3d(X, bounds->GetTuple1(3), bounds->GetTuple1(4));
                glVertex3d(X, bounds->GetTuple1(3), bounds->GetTuple1(5));         
                glEnd();
            }
            else if(atts.GetNormal() == SpreadsheetAttributes::Y)
            {
                double Y;

                vtkDataArray *ycoords = rgrid->GetYCoordinates();
                if(ycoords != 0 && atts.GetSliceIndex() < ycoords->GetNumberOfTuples())
                {
                    if(cellCentered)
                    {
                        Y = (ycoords->GetTuple1(atts.GetSliceIndex()) + 
                                ycoords->GetTuple1(atts.GetSliceIndex() + 1)) / 2.;
                    }
                    else
                        Y = ycoords->GetTuple1(atts.GetSliceIndex());
                }
                else
                    Y = (bounds->GetTuple1(2) + bounds->GetTuple1(3)) / 2.;

                // Draw the tracer plane.
                if(atts.GetTracerColor().Alpha() > 0)
                {
                    glBegin(GL_QUADS);
                    glColor4ubv(atts.GetTracerColor().GetColor());
                    glVertex3d(bounds->GetTuple1(0), Y, bounds->GetTuple1(4));
                    glVertex3d(bounds->GetTuple1(0), Y, bounds->GetTuple1(5));
                    glVertex3d(bounds->GetTuple1(1), Y, bounds->GetTuple1(5));
                    glVertex3d(bounds->GetTuple1(1), Y, bounds->GetTuple1(4));
                    glEnd();
                }
                // Outline the plane
                glLineWidth(2.);
                glColor3ubv(atts.GetTracerColor().GetColor()); // 3ubv so opaque
                glBegin(GL_LINE_LOOP);
                glVertex3d(bounds->GetTuple1(0), Y, bounds->GetTuple1(4));
                glVertex3d(bounds->GetTuple1(0), Y, bounds->GetTuple1(5));
                glVertex3d(bounds->GetTuple1(1), Y, bounds->GetTuple1(5));
                glVertex3d(bounds->GetTuple1(1), Y, bounds->GetTuple1(4));
                glEnd();        
            }
            else if(atts.GetNormal() == SpreadsheetAttributes::Z)
            {
                double Z;

                vtkDataArray *zcoords = rgrid->GetZCoordinates();
                if(zcoords != 0 && atts.GetSliceIndex() < zcoords->GetNumberOfTuples())
                {
                    if(cellCentered)
                    {
                        Z = (zcoords->GetTuple1(atts.GetSliceIndex()) + 
                                zcoords->GetTuple1(atts.GetSliceIndex() + 1)) / 2.;
                    }
                    else
                        Z = zcoords->GetTuple1(atts.GetSliceIndex());
                }
                else
                    Z = (bounds->GetTuple1(4) + bounds->GetTuple1(5)) / 2.;

                // Draw the tracer plane.
                if(atts.GetTracerColor().Alpha() > 0)
                {
                    glBegin(GL_QUADS);
                    glColor4ubv(atts.GetTracerColor().GetColor());
                    glVertex3d(bounds->GetTuple1(0), bounds->GetTuple1(2), Z);
                    glVertex3d(bounds->GetTuple1(1), bounds->GetTuple1(2), Z);
                    glVertex3d(bounds->GetTuple1(1), bounds->GetTuple1(3), Z);
                    glVertex3d(bounds->GetTuple1(0), bounds->GetTuple1(3), Z);
                    glEnd();
                }
                // Outline the plane
                glLineWidth(2.);
                glColor3ubv(atts.GetTracerColor().GetColor()); // 3ubv so opaque
                glBegin(GL_LINE_LOOP);
                glVertex3d(bounds->GetTuple1(0), bounds->GetTuple1(2), Z);
                glVertex3d(bounds->GetTuple1(1), bounds->GetTuple1(2), Z);
                glVertex3d(bounds->GetTuple1(1), bounds->GetTuple1(3), Z);
                glVertex3d(bounds->GetTuple1(0), bounds->GetTuple1(3), Z);
                glEnd();
            }
        }

        if (atts.GetShowPatchOutline())
        {
            DrawBoundingBox(bounds, fgColor);
        }

        if (drawCellOutline)
        {
            vtkCell *cell = rgrid->GetCell(cellId);
            if (cell)
                Draw3DCell(cell, fgColor);
            else
                debug1 << "Internal error: Cannot locate cell." << std::endl;
        }
    }
}

// ****************************************************************************
// Method: avtOpenGLSpreadsheetTraceRenderer::DrawStructuredGrid
//
// Purpose: 
//   Renders the highlight plane for the input structured grid dataset.
//
// Arguments:
//   sgrid   : The input structured grid.
//   bounds  : The bounds of the dataset.
//   atts    : The Spreadsheet plot's attributes.
//   fgColor : The foreground color.
//
// Returns:    
//
// Note:       Polygons for each cell are drawn for this method since each
//             node in the mesh can have its own location. Get the vertex
//             array coding working!
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 14:34:11 PST 2007
//
// Modifications:
//   
//   Gunther H. Weber, Wed Oct 17 14:48:16 PDT 2007
//   Support toggling patch outline and tracer plane separately
//
//   Gunther H. Weber, Thu Nov 29 15:04:29 PST 2007
//   Support rendering outline of current cell
//
//   Brad Whitlock, Tue May 26 10:28:03 PDT 2009
//   I removed the locator code since we already know the cellId.
//
// ****************************************************************************

void
avtOpenGLSpreadsheetTraceRenderer::DrawStructuredGrid(vtkStructuredGrid *sgrid, 
    vtkDataArray *bounds, const SpreadsheetAttributes &atts, 
    const double *fgColor)
{
    int dims[3];
    sgrid->GetDimensions(dims);

    vtkIdType cellId = (vtkIdType)atts.GetCurrentPick();
    bool drawCellOutline = atts.GetShowCurrentCellOutline() &&
                           atts.GetCurrentPickValid() &&
                           atts.GetCurrentPickType() == 0;

    if(dims[2] < 2)
    {
        if (atts.GetShowTracerPlane())
        {
            // Draw the tracer plane.
            glBegin(GL_QUADS);
            glColor4ubv(atts.GetTracerColor().GetColor());
            for(int j = 0; j < dims[1]-1; ++j)
            {
                vtkIdType row0 = j * dims[0];
                vtkIdType row1 = (j+1) * dims[0];
                for(int i = 0; i < dims[0]-1; ++i)
                {
                    glVertex3dv(sgrid->GetPoint(row0));
                    glVertex3dv(sgrid->GetPoint(row0+1));
                    glVertex3dv(sgrid->GetPoint(row1+1));
                    glVertex3dv(sgrid->GetPoint(row1));

                    ++row0;
                    ++row1;
                }
            }
            glEnd();
        }

        if (atts.GetShowPatchOutline())
        {
            // Draw the outline of the mesh
            glColor3dv(fgColor);
            glLineWidth(2.);
            glBegin(GL_LINE_LOOP);
            glVertex3d(bounds->GetTuple1(0), bounds->GetTuple1(2), 0.);
            glVertex3d(bounds->GetTuple1(1), bounds->GetTuple1(2), 0.);
            glVertex3d(bounds->GetTuple1(1), bounds->GetTuple1(3), 0.);
            glVertex3d(bounds->GetTuple1(0), bounds->GetTuple1(3), 0.);
            glEnd();
        }

        if (drawCellOutline)
        {
            vtkCell *cell = sgrid->GetCell(cellId);
            if (cell)
                Draw2DCell(cell, fgColor);
            else
                debug1 << "Internal error: Cannot locate cell." << std::endl;
        }
    }
    else
    {
        if (atts.GetShowTracerPlane())
        {
            if(atts.GetNormal() == SpreadsheetAttributes::X)
            {
                int i = atts.GetSliceIndex();
                if(i < 0) i = 0;
                if(i >= dims[0]) i = dims[0];

                glBegin(GL_QUADS);
                glColor4ubv(atts.GetTracerColor().GetColor());
                int nxny = dims[1]*dims[0];
                for(int j = 0; j < dims[1]-1; ++j)
                {
                    int jnx = j * dims[0];
                    int j1nx = (j+1) * dims[0];
                    for(int k = 0; k < dims[2]-1; ++k)
                    {
                        vtkIdType p0 = k * nxny + jnx + i;
                        vtkIdType p1 = (k+1) * nxny + jnx + i;
                        vtkIdType p2 = (k+1) * nxny + j1nx + i;
                        vtkIdType p3 = k * nxny + j1nx + i;

                        glVertex3dv(sgrid->GetPoint(p0));
                        glVertex3dv(sgrid->GetPoint(p1));
                        glVertex3dv(sgrid->GetPoint(p2));
                        glVertex3dv(sgrid->GetPoint(p3));
                    }
                }
                glEnd();
            }
            else if(atts.GetNormal() == SpreadsheetAttributes::Y)
            {
                int j = atts.GetSliceIndex();
                if(j < 0) j = 0;
                if(j >= dims[1]) j = dims[1];

                glBegin(GL_QUADS);
                glColor4ubv(atts.GetTracerColor().GetColor());
                int jnx = j * dims[0];
                for(int k = 0; k < dims[2]-1; ++k)
                {
                    int knxny = k * dims[1] * dims[0];
                    int k1nxny = (k+1) * dims[1] * dims[0];
                    for(int i = 0; i < dims[0]-1; ++i)
                    {
                        vtkIdType p0 = knxny + jnx + i;
                        vtkIdType p1 = knxny + jnx + (i+1);
                        vtkIdType p2 = k1nxny + jnx + (i+1);
                        vtkIdType p3 = k1nxny + jnx + i;

                        glVertex3dv(sgrid->GetPoint(p0));
                        glVertex3dv(sgrid->GetPoint(p1));
                        glVertex3dv(sgrid->GetPoint(p2));
                        glVertex3dv(sgrid->GetPoint(p3));
                    }
                }
                glEnd();
            }
            else if(atts.GetNormal() == SpreadsheetAttributes::Z)
            {
#ifdef USE_VERTEX_ARRAYS
                //
                // This is an experiment that does not work yet for some reason. The plan was to
                // expose the VTK point data to GL and then create an index array that would
                // let me create quads while using the VTK points as the vertices. Theoretically,
                // this should be pretty fast.
                //
                int nxny = dims[0] * dims[1];
                int nCells = (dims[0]-1) * (dims[1]-1);
                static int *quadIndices = 0;
                if(quadIndices == 0)
                {
                    // We create indices for the quads given that we'll make all of 
                    // the VTK points in the object available as a vertex array in 
                    // the client state.
                    quadIndices = new int[nCells * 4];
                }

                // if(k != lastK)
                {
                    int *iptr = quadIndices;

                    int k = atts.GetSliceIndex();
                    if(k < 0) k = 0;
                    if(k >= dims[2]) k = dims[2];
                    int knxny = k * dims[1] * dims[0];
                    for(int j = 0; j < dims[1]-1; ++j)
                    {
                        int row0 = knxny + j * dims[0];
                        int row1 = knxny + (j+1) * dims[0];
                        for(int i = 0; i < dims[0]-1; ++i)
                        {
                            iptr[0] = row0;
                            iptr[1] = row0+1;
                            iptr[2] = row1+1;
                            iptr[3] = row1;

                            //                        if(iptr < quadIndices + 40)
                            //                            cerr << "quad = {" << iptr[0] << ", " << iptr[1] << ", " << iptr[2] << ", " << iptr[3] << "}" << endl;

                            iptr += 4;
                            row0++; row1++;
                        }
                    }
                }

                glEnableClientState(GL_VERTEX_ARRAY);
                if(sgrid->GetPoints()->GetDataType() == VTK_FLOAT)
                {
                    glVertexPointer(3, GL_FLOAT, 0, sgrid->GetPoints()->GetData()->GetVoidPointer(0));

                    const float *fptr = (const float *)sgrid->GetPoints()->GetData()->GetVoidPointer(0);
                    //                for(int i = 0; i < 10; ++i)
                    //                {
                    //                    cerr << "pt[" << i << "] = {" << fptr[0] << ", " << fptr[1] << ", " << fptr[2] << "}" << endl;
                    //                    fptr += 3;
                    //                }

                }
                else if(sgrid->GetPoints()->GetDataType() == VTK_DOUBLE)
                    glVertexPointer(3, GL_DOUBLE, 0, sgrid->GetPoints()->GetData()->GetVoidPointer(0));
                else
                    debug1 << "VTK points are: " << sgrid->GetPoints()->GetDataType() << endl;
                glColor4ubv(atts.GetTracerColor().GetColor());

                // Use the connectivity that we computed for slicing in 
                glDrawElements(GL_QUADS, nCells*4, GL_INT, quadIndices);
#else
                int k = atts.GetSliceIndex();
                if(k < 0) k = 0;
                if(k >= dims[2]) k = dims[2];

                glBegin(GL_QUADS);
                glColor4ubv(atts.GetTracerColor().GetColor());
                int knxny = k * dims[1] * dims[0];
                for(int j = 0; j < dims[1]-1; ++j)
                {
                    vtkIdType row0 = knxny + j * dims[0];
                    vtkIdType row1 = knxny + (j+1) * dims[0];
                    for(int i = 0; i < dims[0]-1; ++i)
                    {
                        glVertex3dv(sgrid->GetPoint(row0));
                        glVertex3dv(sgrid->GetPoint(row0+1));
                        glVertex3dv(sgrid->GetPoint(row1+1));
                        glVertex3dv(sgrid->GetPoint(row1));

                        ++row0;
                        ++row1;
                    }
                }
                glEnd();
#endif
            }
        }

        if (atts.GetShowPatchOutline())
        {
            DrawBoundingBox(bounds, fgColor);
        }

        if (drawCellOutline)
        {
            vtkCell *cell = sgrid->GetCell(cellId);
            if (cell)
                Draw3DCell(cell, fgColor);
            else
                debug1 << "Internal error: Cannot locate cell." << std::endl;
        }
    }
}

// ****************************************************************************
// Method: avtOpenGLSpreadsheetTraceRenderer::DrawBoundingBox(vtkDataArray*)
//
// Purpose: 
//   Draws the 3D bounding box.
//
// Arguments:
//   bounds  : The bounds array.
//   fgColor : The foreground color.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 14:35:54 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtOpenGLSpreadsheetTraceRenderer::DrawBoundingBox(vtkDataArray *bounds,
    const double *fgColor)
{
    // Draw the outline of the mesh
    glColor3dv(fgColor);
    glLineWidth(2.);
    glBegin(GL_LINE_LOOP);
    glVertex3d(bounds->GetTuple1(0), bounds->GetTuple1(2), bounds->GetTuple1(4));
    glVertex3d(bounds->GetTuple1(1), bounds->GetTuple1(2), bounds->GetTuple1(4));
    glVertex3d(bounds->GetTuple1(1), bounds->GetTuple1(3), bounds->GetTuple1(4));
    glVertex3d(bounds->GetTuple1(0), bounds->GetTuple1(3), bounds->GetTuple1(4));
    glEnd();
    glBegin(GL_LINE_LOOP);
    glVertex3d(bounds->GetTuple1(0), bounds->GetTuple1(2), bounds->GetTuple1(5));
    glVertex3d(bounds->GetTuple1(1), bounds->GetTuple1(2), bounds->GetTuple1(5));
    glVertex3d(bounds->GetTuple1(1), bounds->GetTuple1(3), bounds->GetTuple1(5));
    glVertex3d(bounds->GetTuple1(0), bounds->GetTuple1(3), bounds->GetTuple1(5));
    glEnd();
    glBegin(GL_LINES);
    glVertex3d(bounds->GetTuple1(0), bounds->GetTuple1(2), bounds->GetTuple1(4));
    glVertex3d(bounds->GetTuple1(0), bounds->GetTuple1(2), bounds->GetTuple1(5));
    glVertex3d(bounds->GetTuple1(1), bounds->GetTuple1(2), bounds->GetTuple1(4));
    glVertex3d(bounds->GetTuple1(1), bounds->GetTuple1(2), bounds->GetTuple1(5));
    glVertex3d(bounds->GetTuple1(1), bounds->GetTuple1(3), bounds->GetTuple1(4));
    glVertex3d(bounds->GetTuple1(1), bounds->GetTuple1(3), bounds->GetTuple1(5));
    glVertex3d(bounds->GetTuple1(0), bounds->GetTuple1(3), bounds->GetTuple1(4));
    glVertex3d(bounds->GetTuple1(0), bounds->GetTuple1(3), bounds->GetTuple1(5));
    glEnd();
}

// ****************************************************************************
// Method: avtOpenGLSpreadsheetTraceRenderer::Draw2DCell
//
// Purpose: 
//   Draws the outline of a 2D cell.
//
// Arguments:
//   cell    : Pointer to the 2D VTK cell
//   fgColor : The foreground color.
//
// Programmer: Gunther H. Weber
// Creation:   Thu Nov 29 14:31:24 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void 
avtOpenGLSpreadsheetTraceRenderer::Draw2DCell(vtkCell *cell, const double *fgColor)
{
    glBegin(GL_LINES);
    for (int eNo=0; eNo<cell->GetNumberOfEdges(); ++eNo)
    {
        vtkCell *edge = cell->GetEdge(eNo);
        glVertex3d(
                edge->GetPoints()->GetPoint(0)[0],
                edge->GetPoints()->GetPoint(0)[1],
                edge->GetPoints()->GetPoint(0)[2]);
        glVertex3d(
                edge->GetPoints()->GetPoint(1)[0],
                edge->GetPoints()->GetPoint(1)[1],
                edge->GetPoints()->GetPoint(1)[2]);
    }
    glEnd();
}

// ****************************************************************************
// Method: avtOpenGLSpreadsheetTraceRenderer::Draw3DCell
//
// Purpose: 
//   Draws the outline of a 3D cell.
//
// Arguments:
//   cell    : Pointer to the 3D VTK cell
//   fgColor : The foreground color.
//
// Programmer: Gunther H. Weber
// Creation:   Thu Nov 29 14:31:24 PST 2007
//
//
// Modifications:
//   
// ****************************************************************************

void 
avtOpenGLSpreadsheetTraceRenderer::Draw3DCell(vtkCell *cell, const double *fgColor)
{
    glBegin(GL_LINES);
    for (int fNo =0; fNo<cell->GetNumberOfFaces(); ++fNo)
    {
        vtkCell *face = cell->GetFace(fNo);
        for (int eNo=0; eNo<face->GetNumberOfEdges(); ++eNo)
        {
            vtkCell *edge = face->GetEdge(eNo);
            glVertex3d(
                    edge->GetPoints()->GetPoint(0)[0],
                    edge->GetPoints()->GetPoint(0)[1],
                    edge->GetPoints()->GetPoint(0)[2]);
            glVertex3d(
                    edge->GetPoints()->GetPoint(1)[0],
                    edge->GetPoints()->GetPoint(1)[1],
                    edge->GetPoints()->GetPoint(1)[2]);
        }
    }
    glEnd();
}
