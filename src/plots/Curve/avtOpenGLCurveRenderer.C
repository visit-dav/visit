/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#ifndef avtOpenGLCurveRenderer
#include <avtOpenGLCurveRenderer.h>
#else
#include <Init.h>
#endif

#include <vtkPolyData.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

#include <LineAttributes.h>
#include <CurveAttributes.h>

#include <snprintf.h>

#include <DebugStream.h>
#include <TimingsManager.h>

#ifndef VTK_IMPLEMENT_MESA_CXX
#if defined(__APPLE__) && (defined(VTK_USE_CARBON) || defined(VTK_USE_COCOA))
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#else
#include <GL/gl.h>
#endif


// ****************************************************************************
// Method: avtOpenGLCurveRenderer::avtOpenGLCurveRenderer
//
// Purpose: 
//   Constructor for the avtOpenGLCurveRenderer class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:14:10 PDT 2004
//
// Modifications:
//
// ****************************************************************************

avtOpenGLCurveRenderer::avtOpenGLCurveRenderer() : avtCurveRenderer()
{
}

// ****************************************************************************
// Method: avtOpenGLCurveRenderer::~avtOpenGLCurveRenderer
//
// Purpose: 
//   Destructor for the avtOpenGLCurveRenderer class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:14:32 PDT 2004
//
// Modifications:
//
// ****************************************************************************

avtOpenGLCurveRenderer::~avtOpenGLCurveRenderer()
{
}

// ****************************************************************************
// Method: avtOpenGLCurveRenderer::ReleaseGraphicsResources
//
// Purpose: 
//   Clears the character display lists.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 16:00:15 PST 2004
//
// Modifications:
//
// ****************************************************************************

void
avtOpenGLCurveRenderer::ReleaseGraphicsResources()
{
}

// ****************************************************************************
// Method: avtOpenGLCurveRenderer::SetupGraphicsLibrary
//
// Purpose: 
//   This method creates display lists to be used for drawing the letters.
//
// Note:       There is partial code for texture mapped fonts but the textures
//             have not been created.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:14:59 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
avtOpenGLCurveRenderer::SetupGraphicsLibrary()
{
}

// ****************************************************************************
// Method: avtOpenGLCurveRenderer::RenderCurves
//
// Purpose: 
//   Renders the Curves.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:18:49 PDT 2004
//
// Modifications:
//
// ****************************************************************************

void
avtOpenGLCurveRenderer::RenderCurves()
{
    //
    // Make the OpenGL calls to get it into the desired state.
    //
    SetupGraphicsLibrary();

    //
    // Set up the OpenGL state
    //

    // Turn off lighting if it's on.
    GLboolean enableLighting;
    glGetBooleanv(GL_LIGHTING, &enableLighting);
    if(enableLighting)
        glDisable(GL_LIGHTING);

    // Disable depth testing
    GLboolean enableDepthTest;
    glGetBooleanv(GL_DEPTH_TEST, &enableDepthTest);
    if(enableDepthTest)
        glDisable(GL_DEPTH_TEST);

    //
    // Now render the Curves.
    //
    if(atts.GetRenderMode() == CurveAttributes::RenderAsSymbols)
    {
        DrawCurveAsDynamicSymbols();
    }
    else
    {
        DrawCurveAsLines();
    }

    // Enable depth testing if it was on.
    if(enableDepthTest)
        glEnable(GL_DEPTH_TEST);

    // Enable lighting again if it was on.
    if(enableLighting)
        glEnable(GL_LIGHTING);
}

// ****************************************************************************
// Method: avtOpenGLCurveRenderer::DrawCurveAsLines
//
// Purpose: 
//   Draws all of the 2D Curves using the Curve cache.
//
// Arguments:
//   drawNodeCurves : Whether to draw node Curves.
//   drawCellLables : Whether to draw cell Curves.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:19:43 PDT 2004
//
// Modifications:
//
// ****************************************************************************

void
avtOpenGLCurveRenderer::DrawCurveAsLines()
{
    // Set the curve color.
    ColorAttribute curveColor(atts.GetColor());
    curveColor.SetAlpha(255);
    glColor4ubv(curveColor.GetColor());

    // Set up the line properties.
    glLineWidth(LineWidth2Int(Int2LineWidth(atts.GetLineWidth())));

    // Set the LineStipple
    int stipplePattern = LineStyle2StipplePattern(Int2LineStyle(
        atts.GetLineStyle()));
    if (stipplePattern != 0xFFFF)
    {
        glEnable(GL_LINE_STIPPLE);
        glLineStipple(1, stipplePattern);
    }
    else
    {
        // In case it was set elsewhere
        glDisable(GL_LINE_STIPPLE);
    }

    // Draw the curve.
    glBegin(GL_LINE_STRIP);
    const float *pt = (const float *)input->GetPoints()->GetVoidPointer(0);
    for(int i = 0; i < input->GetPoints()->GetNumberOfPoints(); ++i)
    {
        glVertex3fv(pt);
        pt += 3;
    }
    glEnd();

    // Draw the points on the curve.
    if(atts.GetShowPoints())
    {
        glPointSize(atts.GetPointSize());
        glBegin(GL_POINTS);
        const float *pt = (const float *)input->GetPoints()->GetVoidPointer(0);
        for(int i = 0; i < input->GetPoints()->GetNumberOfPoints(); ++i)
        {
            glVertex3fv(pt);
            pt += 3;
        }
        glEnd();
    }

    if (stipplePattern != 0xFFFF)
        glDisable(GL_LINE_STIPPLE);
}

// ****************************************************************************
// Method: avtOpenGLCurveRenderer::DrawCurveAsDynamicSymbols
//
// Purpose: 
//   Bins up 2D world space to make sure that only a subset of the 2D Curves
//   are drawn.
//
// Arguments:
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:20:29 PDT 2004
//
// Modifications:
//
// ****************************************************************************

void
avtOpenGLCurveRenderer::DrawCurveAsDynamicSymbols()
{

    //
    // Figure out the world coordinates of the window that is being displayed.
    //
    float m[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, m);
    double lowerleft[3] = {-1., -1., 0.};
    VTKRen->ViewToWorld(lowerleft[0], lowerleft[1], lowerleft[2]);
    double upperright[3] = {1., 1., 0.};
    VTKRen->ViewToWorld(upperright[0], upperright[1], upperright[2]);
    lowerleft[0] /= m[0];
    upperright[0] /= m[0];
    lowerleft[1] /= m[5];
    upperright[1] /= m[5];

//    cerr << "x range = (" << lowerleft[0] << ", " << upperright[0] << ")"
//         << "   y range = (" << lowerleft[1] << ", " << upperright[1] << ")\n";

    //
    // figure out the size and number of bins that we want in X, Y.
    //
    double win_dx = upperright[0] - lowerleft[0];
    double win_dy = upperright[1] - lowerleft[1];

    int n_bins = atts.GetSymbolDensity() > 10 ? atts.GetSymbolDensity() : 10;
    float bin_size;
    if(win_dy > win_dx)
        bin_size = win_dy / n_bins;
    else
        bin_size = win_dx / n_bins;

    int bin_x_n = int(win_dx / (bin_size / m[0])) + 1;
    float bin_x_size = bin_size / m[0];
    float bin_x_offset = lowerleft[0];

    int bin_y_n = int(win_dy / (bin_size / m[5])) + 1;
    float bin_y_size = bin_size / m[5];
    float bin_y_offset = lowerleft[1];

#ifdef VISUALIZE_DYNAMIC_BINS
    //
    // Draw the grid that we use to dscretize space.
    //
    glColor3f(0.4,0.4,0.4);
    glBegin(GL_LINES);
    for(int i = 0; i < bin_x_n; ++i)
    {
        float v0[3], v1[3];
        v0[0] = bin_x_offset + float(i) * bin_x_size;
        v0[1] = bin_y_offset;
        v0[2] = 0.;

        v1[0] = bin_x_offset + float(i) * bin_x_size;
        v1[1] = bin_y_offset + win_dy;
        v1[2] = 0.;

        glVertex3fv(v0);
        glVertex3fv(v1);
    }
    glEnd();

    glBegin(GL_LINES);
    for(int i = 0; i < bin_y_n; ++i)
    {
        float v0[3], v1[3];
        v0[0] = bin_x_offset;
        v0[1] = bin_y_offset + float(i) * bin_y_size;
        v0[2] = 0.;

        v1[0] = bin_x_offset + win_dx;
        v1[1] = bin_y_offset + float(i) * bin_y_size;
        v1[2] = 0.;

        glVertex3fv(v0);
        glVertex3fv(v1);
    }
    glEnd();
#endif

    // Set the curve color.
    ColorAttribute curveColor(atts.GetColor());
    curveColor.SetAlpha(255);
    glColor4ubv(curveColor.GetColor());

#define SWAP(a,b) { int tmp = a; a = b; b = tmp; }
#define ONE_THIRD_ROUND (2. * M_PI / 3.)
#define REDUCE_SCALE 0.8
#define MAX_SYMBOL_VERTS 25

    float symbolPoints[MAX_SYMBOL_VERTS][2];
    int symbolNVerts;
    if(atts.GetSymbol() == CurveAttributes::TriangleUp)
    {
        symbolNVerts = 3;
        symbolPoints[0][0] = cos(M_PI/2.) * REDUCE_SCALE * bin_x_size / 2.;
        symbolPoints[0][1] = sin(M_PI/2.) * REDUCE_SCALE * bin_y_size / 2.;
        symbolPoints[1][0] = cos(M_PI/2. + ONE_THIRD_ROUND) * REDUCE_SCALE * bin_x_size / 2.;
        symbolPoints[1][1] = sin(M_PI/2. + ONE_THIRD_ROUND) * REDUCE_SCALE * bin_y_size / 2.;
        symbolPoints[2][0] = cos(M_PI/2. + 2.*ONE_THIRD_ROUND) * REDUCE_SCALE * bin_x_size / 2.;
        symbolPoints[2][1] = sin(M_PI/2. + 2.*ONE_THIRD_ROUND) * REDUCE_SCALE * bin_y_size / 2.;
        glBegin(GL_TRIANGLES);
    }
    else if(atts.GetSymbol() == CurveAttributes::TriangleDown)
    {
        symbolNVerts = 3;
        symbolPoints[0][0] = cos(3*M_PI/2.) * REDUCE_SCALE * bin_x_size / 2.;
        symbolPoints[0][1] = sin(3*M_PI/2.) * REDUCE_SCALE * bin_y_size / 2.;
        symbolPoints[1][0] = cos(3*M_PI/2. + ONE_THIRD_ROUND) * REDUCE_SCALE * bin_x_size / 2.;
        symbolPoints[1][1] = sin(3*M_PI/2. + ONE_THIRD_ROUND) * REDUCE_SCALE * bin_y_size / 2.;
        symbolPoints[2][0] = cos(3*M_PI/2. + 2.*ONE_THIRD_ROUND) * REDUCE_SCALE * bin_x_size / 2.;
        symbolPoints[2][1] = sin(3*M_PI/2. + 2.*ONE_THIRD_ROUND) * REDUCE_SCALE * bin_y_size / 2.;
        glBegin(GL_TRIANGLES);
    }
    else if(atts.GetSymbol() == CurveAttributes::Square)
    {
        symbolNVerts = 4;
        symbolPoints[0][0] = REDUCE_SCALE * bin_x_size / 2.;
        symbolPoints[0][1] = REDUCE_SCALE * bin_y_size / 2.;
        symbolPoints[1][0] = -REDUCE_SCALE * bin_x_size / 2.;
        symbolPoints[1][1] = REDUCE_SCALE * bin_y_size / 2.;
        symbolPoints[2][0] = -REDUCE_SCALE * bin_x_size / 2.;
        symbolPoints[2][1] = -REDUCE_SCALE * bin_y_size / 2.;
        symbolPoints[3][0] = REDUCE_SCALE * bin_x_size / 2.;
        symbolPoints[3][1] = -REDUCE_SCALE * bin_y_size / 2.;
        glBegin(GL_QUADS);
    }
    else if(atts.GetSymbol() == CurveAttributes::Circle)
    {
        symbolNVerts = MAX_SYMBOL_VERTS;
        symbolPoints[0][0] = 0.f;
        symbolPoints[0][1] = 0.f;
        for(int i = 0; i < MAX_SYMBOL_VERTS-1; ++i)
        {
            double t = double(i) / double(MAX_SYMBOL_VERTS-1-1);
            double angle = 2. * M_PI * t;
            symbolPoints[i+1][0] = cos(angle) * REDUCE_SCALE * bin_x_size / 2.;
            symbolPoints[i+1][1] = sin(angle) * REDUCE_SCALE * bin_y_size / 2.;
        }
    }
    else if(atts.GetSymbol() == CurveAttributes::Plus)
    {
        symbolNVerts = 4;
        symbolPoints[0][0] = 0.f;
        symbolPoints[0][1] = -REDUCE_SCALE * bin_y_size / 2.;
        symbolPoints[1][0] = 0.f;
        symbolPoints[1][1] = REDUCE_SCALE * bin_y_size / 2.;
        symbolPoints[2][0] = -REDUCE_SCALE * bin_x_size / 2.;
        symbolPoints[2][1] = 0.f;
        symbolPoints[3][0] = REDUCE_SCALE * bin_x_size / 2.;
        symbolPoints[3][1] = 0.f;
        glLineWidth(2.);
        glBegin(GL_LINES);
    }
    else if(atts.GetSymbol() == CurveAttributes::X)
    {
        symbolNVerts = 4;
        symbolPoints[0][0] = cos(M_PI/4. + M_PI/2.) * REDUCE_SCALE * bin_x_size / 2.;
        symbolPoints[0][1] = sin(M_PI/4. + M_PI/2.) * REDUCE_SCALE * bin_y_size / 2.;
        symbolPoints[1][0] = cos(M_PI/4. + M_PI + M_PI/2.) * REDUCE_SCALE * bin_x_size / 2.;
        symbolPoints[1][1] = sin(M_PI/4. + M_PI + M_PI/2.) * REDUCE_SCALE * bin_y_size / 2.;
        symbolPoints[2][0] = cos(M_PI/4. + 0.) * REDUCE_SCALE * bin_x_size / 2.;
        symbolPoints[2][1] = sin(M_PI/4. + 0.) * REDUCE_SCALE * bin_y_size / 2.;
        symbolPoints[3][0] = cos(M_PI/4. + M_PI) * REDUCE_SCALE * bin_x_size / 2.;
        symbolPoints[3][1] = sin(M_PI/4. + M_PI) * REDUCE_SCALE * bin_y_size / 2.;
        glLineWidth(2.);
        glBegin(GL_LINES);
    }

    // Now iterate over the line segments and draw the symbols on them.
    for(int i = 1; i < input->GetPoints()->GetNumberOfPoints(); ++i)
    {
        const float *pt = (const float *)input->GetPoints()->GetVoidPointer(0);
        const float *A = pt + (i-1) * 3;
        const float *B = pt + i * 3;
        // Determine the grid cells that contain the points.
        int x0 = int((A[0] - lowerleft[0]) / bin_x_size);
        int y0 = int((A[1] - lowerleft[1]) / bin_y_size);

        int x1 = int((B[0] - lowerleft[0]) / bin_x_size);
        int y1 = int((B[1] - lowerleft[1]) / bin_y_size);

        // Use Bresenham's line algorithm to produce a number of
        // cells encountered along the line segment.
        bool steep = abs(y1 - y0) > abs(x1 - x0);
        if(steep)
        {
            SWAP(x0, y0);
            SWAP(x1, y1);
        }
        if(x0 > x1)
        {
            SWAP(x0, x1);
            SWAP(y0, y1);
        }
        int deltax = x1 - x0;
        int deltay = y1 - y0;
        if(deltay < 0)
            deltay = -deltay;
        int err = 0;
        int y = y0;
        int ystep = (y0 < y1) ? 1 : -1;
        int cells_in_line = 0;
        for(int x = x0; x <= x1; ++x)
        {
            cells_in_line++;
            err = err + deltay;
            if((err << 1) >= deltax)
            {
                y += ystep;
                err -= deltax;
            }
        }

        // Use the number of cells along the way between the line
        // end points to calculate intermediate points at which to 
        // put symbols.
        for(int pindex = 0; pindex < cells_in_line; ++pindex)
        {
            float t = (cells_in_line == 1) ? 0. : (float(pindex) / float(cells_in_line-1));

                        float ix = (1.-t)*A[0] + t*B[0];
            float iy = (1.-t)*A[1] + t*B[1];

            if(atts.GetSymbol() == CurveAttributes::Circle)
                glBegin(GL_TRIANGLE_FAN);

            for(int j = 0; j < symbolNVerts; ++j)
            {
                float pt[3];
                pt[0] = ix + symbolPoints[j][0];
                pt[1] = iy + symbolPoints[j][1];
                pt[2] = 0.;                
                glVertex3fv(pt);
            }

            if(atts.GetSymbol() == CurveAttributes::Circle)
                glEnd();
        }
    }

    if(atts.GetSymbol() != CurveAttributes::Circle)
        glEnd();
}

