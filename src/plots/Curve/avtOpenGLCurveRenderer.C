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

#include <avtOpenGLCurveRenderer.h>

#include <vtkPolyData.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

#include <LineAttributes.h>
#include <CurveAttributes.h>

#include <snprintf.h>

#include <DebugStream.h>
#include <TimingsManager.h>

#include <avtOpenGLExtensionManager.h>

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
//   Hank Childs, Thu Jul 15 18:20:26 PDT 2010
//   Add support for plotting cues for the current location.
//
//   Brad Whitlock, Fri Jul  5 16:46:04 PDT 2013
//   Fill in area under the curve.
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

    if(atts.GetFillMode() != CurveAttributes::NoFill)
        DrawCurveFill();

    if (atts.GetShowLines())
        DrawCurveAsLines();

    if (atts.GetShowPoints())
        DrawCurveAsSymbols();

    if (atts.GetDoLineTimeCue())
    {
        RenderLine();
    }
    if (atts.GetDoBallTimeCue())
    {
        RenderBall();
    }

    // Enable depth testing if it was on.
    if(enableDepthTest)
        glEnable(GL_DEPTH_TEST);

    // Enable lighting again if it was on.
    if(enableLighting)
        glEnable(GL_LIGHTING);
}


// ****************************************************************************
//  Method: avtOpenGLCurveRenderer::RenderBall
//
//  Purpose:
//      Render a ball to show the current location.
//
//  Programmer: Hank Childs
//  Creation:   July 15, 2010
//
//  Modifications:
//    Brad Whitlock, Fri Apr 20 16:19:17 PDT 2012
//    Support non-float coordinates.
//
// ****************************************************************************

void
avtOpenGLCurveRenderer::RenderBall(void)
{
    double ix = 0.;
    double iy = 0.;
    vtkIdType npts = input->GetPoints()->GetNumberOfPoints();
    for(vtkIdType i = 0; i < npts-1 ; i++)
    {
        double ptr[6];
        input->GetPoints()->GetPoint(i, ptr);
        input->GetPoints()->GetPoint(i+1, ptr+3);
        if (ptr[0] <= atts.GetTimeForTimeCue() && atts.GetTimeForTimeCue() <= ptr[3])
        {
            double lastX = ptr[0];
            double curX = ptr[3];
            double lastY = ptr[1];
            double curY = ptr[4];
            ix = atts.GetTimeForTimeCue();
            iy = (ix-lastX)/(curX-lastX)*(curY-lastY) + lastY;
        }
    }

    int    bin_x_n,      bin_y_n;
    double bin_x_size,   bin_y_size;
    double bin_x_offset, bin_y_offset;
    GetAspect(bin_x_n, bin_x_size, bin_x_offset,
              bin_y_n, bin_y_size, bin_y_offset);

    // Set the curve color.
    ColorAttribute curveColor(atts.GetBallTimeCueColor());
    curveColor.SetAlpha(255);
    glColor4ubv(curveColor.GetColor());

    int symbolNVerts = 100;
    glBegin(GL_TRIANGLE_FAN);
    double pt[3];
    pt[0] = ix;
    pt[1] = iy;
    pt[2] = 0.;
    glVertex3dv(pt);
    double REDUCE_SCALE = atts.GetTimeCueBallSize();
    for(int i = 0; i < symbolNVerts-1; ++i)
    {
        double t = double(i) / double(symbolNVerts-1-1);
        double angle = 2. * M_PI * t;
        pt[0] = ix + cos(angle) * REDUCE_SCALE * bin_x_size / 2.;
        pt[1] = iy + sin(angle) * REDUCE_SCALE * bin_y_size / 2.;
        pt[2] = 0.;                
        glVertex3dv(pt);
    }
    glEnd();
}


// ****************************************************************************
//  Method: avtOpenGLCurveRenderer::RenderLine
//
//  Purpose:
//      Render a vertical line to show the current location.
//
//  Programmer: Hank Childs
//  Creation:   July 15, 2010
//
//  Modifications:
//    Brad Whitlock, Fri Apr 20 16:31:19 PDT 2012
//    Support double coordinates.
//
// ****************************************************************************

void
avtOpenGLCurveRenderer::RenderLine(void)
{
    double max = -1e+30;
    double min = +1e+30;
    vtkIdType npts = input->GetPoints()->GetNumberOfPoints();
    for(vtkIdType i = 0; i < npts ; i++)
    {
        double pt[3];
        input->GetPoints()->GetPoint(i, pt);
        max = (max > pt[1] ? max : pt[1]);
        min = (min < pt[1] ? min : pt[1]);
    }
    double diff = max-min;
    max += diff;
    min -= diff;
    glLineWidth(LineWidth2Int(Int2LineWidth(atts.GetLineTimeCueWidth())));
    ColorAttribute curveColor(atts.GetLineTimeCueColor());
    curveColor.SetAlpha(255);
    glColor4ubv(curveColor.GetColor());
    glBegin(GL_LINES);
    double pt[3] = { atts.GetTimeForTimeCue(), min, 0. };
    glVertex3dv(pt);
    double pt2[3] = { atts.GetTimeForTimeCue(), max, 0. };
    glVertex3dv(pt2);
    glEnd();
}


// ****************************************************************************
// Method: avtOpenGLCurveRenderer::DrawCurveAsLines
//
// Purpose: 
//   Draws all of the 2D Curves using the Curve cache.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:19:43 PDT 2004
//
// Modifications:
//    Kathleen Bonnell, Fri Aug 13 13:28:39 PDT 2010
//    Moved drawing of points to DrawCurveAsSymbols.
//
//    Brad Whitlock, Fri Apr 20 16:37:28 PDT 2012
//    Convert to GetPoint so we can use different coordinate precisions.
//
// ****************************************************************************

void
avtOpenGLCurveRenderer::DrawCurveAsLines()
{
    // Set the curve color.
    ColorAttribute curveColor(atts.GetCurveColor());
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
    bool lastWasGood = false;
    vtkIdType npts = input->GetPoints()->GetNumberOfPoints();
    for(vtkIdType i = 0; i < npts ; i++)
    {
        double pt[3];
        input->GetPoints()->GetPoint(i, pt);

        if (atts.GetDoCropTimeCue() && atts.GetTimeForTimeCue() < pt[0])
        {
            if (lastWasGood)
            {
                double prev[3];
                input->GetPoints()->GetPoint(i-1, prev);

                double p2[3];
                p2[0] = atts.GetTimeForTimeCue();
                p2[1] = (p2[0]-prev[0])/(pt[0]-prev[0])*(pt[1]-prev[1]) + prev[1];
                p2[2] = 0.;
                glVertex3dv(p2);
            }
            input->GetPoints()->GetPoint(i+1, pt);
            lastWasGood = false;
            continue;
        }
        lastWasGood = true;
        glVertex3dv(pt);
    }
    glEnd();

    if (stipplePattern != 0xFFFF)
        glDisable(GL_LINE_STIPPLE);
}

// ****************************************************************************
// Method: avtOpenGLCurveRenderer::SetColor
//
// Purpose: 
//   Set the color based on the x,y location within the specified box.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul  5 21:35:42 PDT 2013
//
// Modifications:
//   
// ****************************************************************************

void
avtOpenGLCurveRenderer::SetColor(double x, double y, double minX, double maxX, double minY, double maxY) const
{
    if(atts.GetFillMode() == CurveAttributes::VerticalGradient)
    {
        double dY = maxY - minY;
        double t = 1. - ((dY > 0.) ? ((y - minY) / dY) : 0.);
        glColor4ubv(ColorAttribute::Blend(atts.GetFillColor1(), atts.GetFillColor2(), t).GetColor());
    }
    else if(atts.GetFillMode() == CurveAttributes::HorizontalGradient)
    {
        double dX = maxX - minX;
        double t = (dX > 0.) ? ((x - minX) / dX) : 0.;
        glColor4ubv(ColorAttribute::Blend(atts.GetFillColor1(), atts.GetFillColor2(), t).GetColor());
    }
}

// ****************************************************************************
// Method: avtOpenGLCurveRenderer::DrawCurveFill
//
// Purpose: 
//   Draw in the filled area under the curve.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul  5 16:44:22 PDT 2013
//
// Modifications:
//   
// ****************************************************************************

void
avtOpenGLCurveRenderer::DrawCurveFill()
{
    // Set the curve fill color.
    ColorAttribute fillColor(atts.GetFillColor1());
    glColor4ubv(fillColor.GetColor());

    // Draw the curve.
    glBegin(GL_QUAD_STRIP);
    bool lastWasGood = false;
    vtkIdType npts = input->GetPoints()->GetNumberOfPoints();

    // Figure out X and Y extents
    double minX = 0., maxX = 0.;
    double minY = -1.e30, maxY = 0.;
    if(atts.GetFillMode() == CurveAttributes::HorizontalGradient ||
       atts.GetFillMode() == CurveAttributes::VerticalGradient)
    {
        for(vtkIdType i = 0; i < npts ; i++)
        {
            double pt[3];
            input->GetPoints()->GetPoint(i, pt);

            if(i == 0 || pt[0] < minX)
                minX = pt[0];
            if(i == 0 || pt[0] > maxX)
                maxX = pt[0];
            if(atts.GetFillMode() == CurveAttributes::VerticalGradient)
            {
                if(i == 0 || pt[1] < minY)
                    minY = pt[1];
            }
            if(i == 0 || pt[1] > maxY)
                maxY = pt[1];
        }
    }

    double clipX = maxX;
    for(vtkIdType i = 0; i < npts ; i++)
    {
        double pt[3];
        input->GetPoints()->GetPoint(i, pt);

        if (atts.GetDoCropTimeCue() && atts.GetTimeForTimeCue() < pt[0])
        {
            if (lastWasGood)
            {
                double prev[3];
                input->GetPoints()->GetPoint(i-1, prev);

                double p2[3];
                p2[0] = atts.GetTimeForTimeCue();
                p2[1] = (p2[0]-prev[0])/(pt[0]-prev[0])*(pt[1]-prev[1]) + prev[1];
                p2[2] = 0.;

                clipX = atts.GetTimeForTimeCue();

                SetColor(p2[0], p2[1], minX, maxX, minY, maxY);
                glVertex3dv(p2);

                p2[1] = minY;
                SetColor(p2[0], p2[1], minX, maxX, minY, maxY);
                glVertex3dv(p2);
            }
            input->GetPoints()->GetPoint(i+1, pt);
            lastWasGood = false;
            continue;
        }
        lastWasGood = true;
        SetColor(pt[0], pt[1], minX, maxX, minY, maxY);
        glVertex3dv(pt);

        pt[1] = minY;
        SetColor(pt[0], pt[1], minX, maxX, minY, maxY);
        glVertex3dv(pt);
    }
    glEnd();

    if(atts.GetFillMode() == CurveAttributes::VerticalGradient)
    {
        glColor4ubv(atts.GetFillColor2().GetColor());
        glBegin(GL_QUADS);
        glVertex2d(minX, minY);
        glVertex2d(minX, -1.e30);
        glVertex2d(clipX, -1.e30);
        glVertex2d(clipX, minY);
        glEnd();
    }
}

// ****************************************************************************
//  Method: avtOpenGLCurveRenderer::GetAspect
//
//  Purpose:
//      Gets the aspect ratio of the view.
//
//  Programmer: Hank Childs
//  Creation:   July 15, 2010
//
// ****************************************************************************

void 
avtOpenGLCurveRenderer::GetAspect(int &bin_x_n, double &bin_x_size, double &bin_x_offset,
                                  int &bin_y_n, double &bin_y_size, double &bin_y_offset)
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
    double bin_size;
    if(win_dy > win_dx)
        bin_size = win_dy / n_bins;
    else
        bin_size = win_dx / n_bins;

    bin_x_n = int(win_dx / (bin_size / m[0])) + 1;
    bin_x_size = bin_size / m[0];
    bin_x_offset = lowerleft[0];

    bin_y_n = int(win_dy / (bin_size / m[5])) + 1;
    bin_y_size = bin_size / m[5];
    bin_y_offset = lowerleft[1];
}

// ****************************************************************************
// Method: avtOpenGLCurveRenderer::DrawCurveAsSymbols
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
//    Hank Childs, Thu Jul 15 18:20:26 PDT 2010
//    Moved some code to another method so that it could be reused in the
//    RenderBall method.
//
// ****************************************************************************

void
avtOpenGLCurveRenderer::DrawCurveAsSymbols()
{
    int bin_x_n,         bin_y_n;
    double bin_x_size,   bin_y_size;
    double bin_x_offset, bin_y_offset;
    GetAspect(bin_x_n, bin_x_size, bin_x_offset,
              bin_y_n, bin_y_size, bin_y_offset);

#ifdef VISUALIZE_DYNAMIC_BINS
    //
    // Draw the grid that we use to dscretize space.
    //
    glColor3f(0.4,0.4,0.4);
    glBegin(GL_LINES);
    for(int i = 0; i < bin_x_n; ++i)
    {
        double v0[3], v1[3];
        v0[0] = bin_x_offset + double(i) * bin_x_size;
        v0[1] = bin_y_offset;
        v0[2] = 0.;

        v1[0] = bin_x_offset + double(i) * bin_x_size;
        v1[1] = bin_y_offset + win_dy;
        v1[2] = 0.;

        glVertex3dv(v0);
        glVertex3dv(v1);
    }
    glEnd();

    glBegin(GL_LINES);
    for(int i = 0; i < bin_y_n; ++i)
    {
        double v0[3], v1[3];
        v0[0] = bin_x_offset;
        v0[1] = bin_y_offset + double(i) * bin_y_size;
        v0[2] = 0.;

        v1[0] = bin_x_offset + win_dx;
        v1[1] = bin_y_offset + double(i) * bin_y_size;
        v1[2] = 0.;

        glVertex3dv(v0);
        glVertex3dv(v1);
    }
    glEnd();
#endif

    // Set the curve color.
    ColorAttribute curveColor(atts.GetCurveColor());
    curveColor.SetAlpha(255);
    glColor4ubv(curveColor.GetColor());

#define SWAP(a,b) { int tmp = a; a = b; b = tmp; }
#define ONE_THIRD_ROUND (2. * M_PI / 3.)
#define REDUCE_SCALE 0.8
#define MAX_SYMBOL_VERTS 25

    double symbolPoints[MAX_SYMBOL_VERTS][2];
    int symbolNVerts = 0;
    if (atts.GetSymbol() == CurveAttributes::Point)
    {
        symbolNVerts = 1;
        glPointSize(atts.GetPointSize());
        symbolPoints[0][0] = 0.;
        symbolPoints[0][1] = 0.;
        glBegin(GL_POINTS);
    }
    else if(atts.GetSymbol() == CurveAttributes::TriangleUp)
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
    else // shouldn't happen, but let's default to Point, for completeness
    {
        debug3 << "avtOpenGLCurveRenderer encountered invalid symbol type, reverting"
               << " to Point" << endl;
        symbolNVerts = 1;
        glPointSize(atts.GetPointSize());
        symbolPoints[0][0] = 0.;
        symbolPoints[0][1] = 0.;
        glBegin(GL_POINTS);
    }

    // static case
    if (atts.GetPointFillMode() == CurveAttributes::Static)
    {
        vtkIdType stride = atts.GetPointStride();
        vtkIdType nPts = input->GetPoints()->GetNumberOfPoints();
        double pts[3] = {0., 0., 0.};
        for(vtkIdType i = 0; i < nPts-1; i+=stride)
        {
            input->GetPoints()->GetPoint(i, pts);

            if (atts.GetDoCropTimeCue() && atts.GetTimeForTimeCue() < pts[0])
                continue;

            if(atts.GetSymbol() == CurveAttributes::Circle)
                glBegin(GL_TRIANGLE_FAN);

            for(int j = 0; j < symbolNVerts; ++j)
            {
                double pt[3];
                pt[0] = pts[0] + symbolPoints[j][0];
                pt[1] = pts[1] + symbolPoints[j][1];
                pt[2] = 0.;
                glVertex3dv(pt);
            }

            if(atts.GetSymbol() == CurveAttributes::Circle)
                glEnd();
        }
        // add the last point.
        input->GetPoints()->GetPoint(nPts-1, pts);
        if (!atts.GetDoCropTimeCue() || 
           (atts.GetDoCropTimeCue() && atts.GetTimeForTimeCue() >= pts[0]))
        {
            if(atts.GetSymbol() == CurveAttributes::Circle)
                glBegin(GL_TRIANGLE_FAN);
            for(int j = 0; j < symbolNVerts; ++j)
            {
                double pt[3];
                pt[0] = pts[0] + symbolPoints[j][0];
                pt[1] = pts[1] + symbolPoints[j][1];
                pt[2] = 0.;
                glVertex3dv(pt);
            }

            if(atts.GetSymbol() == CurveAttributes::Circle)
                glEnd();
        }
    }
    else
    {
        // Now iterate over the line segments and draw the symbols on them.
        for(vtkIdType i = 1; i < input->GetPoints()->GetNumberOfPoints(); ++i)
        {
            double A[3], B[3];
            input->GetPoints()->GetPoint(i-1, A);
            input->GetPoints()->GetPoint(i,   B);

            // Determine the grid cells that contain the points.
            int x0 = int((A[0] - bin_x_offset) / bin_x_size);
            int y0 = int((A[1] - bin_y_offset) / bin_y_size);

            int x1 = int((B[0] - bin_x_offset) / bin_x_size);
            int y1 = int((B[1] - bin_y_offset) / bin_y_size);

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
                double t = (cells_in_line == 1) ? 0. : (double(pindex) / double(cells_in_line-1));

                double ix = (1.-t)*A[0] + t*B[0];
                if (atts.GetDoCropTimeCue() && atts.GetTimeForTimeCue() < ix)
                    continue;
                double iy = (1.-t)*A[1] + t*B[1];

                if(atts.GetSymbol() == CurveAttributes::Circle)
                    glBegin(GL_TRIANGLE_FAN);

                for(int j = 0; j < symbolNVerts; ++j)
                {
                    double pt[3];
                    pt[0] = ix + symbolPoints[j][0];
                    pt[1] = iy + symbolPoints[j][1];
                    pt[2] = 0.;                
                    glVertex3dv(pt);
                }

                if(atts.GetSymbol() == CurveAttributes::Circle)
                    glEnd();
            }
        }
    }

    if(atts.GetSymbol() != CurveAttributes::Circle)
        glEnd();
}

