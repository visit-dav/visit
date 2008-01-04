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

#ifndef avtOpenGLParallelAxisRenderer
#include <avtOpenGLParallelAxisRenderer.h>
#else
#include <Init.h>
#endif

#include <vtkCellData.h>
#include <vtkPolyData.h>
#include <vtkIdList.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

#include <ParallelAxisAttributes.h>
#include <avtParallelAxisFilter.h>
#include <VisitExtentsTool.h>
#include <LineAttributes.h>

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
// Method: avtOpenGLParallelAxisRenderer::avtOpenGLParallelAxisRenderer
//
// Purpose: Constructor for the avtOpenGLParallelAxisRenderer class.
//
// Programmer: Mark Blair
// Creation:   Thu Jul  5 19:06:33 PDT 2007
//
// Modifications:
//
// ****************************************************************************

avtOpenGLParallelAxisRenderer::avtOpenGLParallelAxisRenderer() : avtParallelAxisRenderer()
{
}


// ****************************************************************************
// Method: avtOpenGLParallelAxisRenderer::~avtOpenGLParallelAxisRenderer
//
// Purpose: Destructor for the avtOpenGLParallelAxisRenderer class.
//
// Programmer: Mark Blair
// Creation:   Thu Jul  5 19:06:33 PDT 2007
//
// Modifications:
//
// ****************************************************************************

avtOpenGLParallelAxisRenderer::~avtOpenGLParallelAxisRenderer()
{
}


// ****************************************************************************
// Method: avtOpenGLParallelAxisRenderer::ReleaseGraphicsResources
//
// Purpose: (Place holder for future development)
//
// Programmer: Mark Blair
// Creation:   Thu Jul  5 19:06:33 PDT 2007
//
// Modifications:
//
// ****************************************************************************

void
avtOpenGLParallelAxisRenderer::ReleaseGraphicsResources()
{
}


// ****************************************************************************
// Method: avtOpenGLParallelAxisRenderer::SetupGraphicsLibrary
//
// Purpose: Initializes information needed for all subsequent drawing performed
//          by this renderer.
//
// Programmer: Mark Blair
// Creation:   Thu Jul  5 19:06:33 PDT 2007
//
// Modifications:
//   
//     Mark Blair, Wed Oct 24 14:38:54 PDT 2007
//     Gets rendering window size, which is needed for text rendering.
//
// ****************************************************************************

void
avtOpenGLParallelAxisRenderer::SetupGraphicsLibrary()
{
    if (VTKRen == NULL)
    {
        renWinWidth  = 1.0;
        renWinHeight = 1.0;
    }
    else
    {
        renWinWidth  = (double)(VTKRen->GetVTKWindow()->GetSize()[0]);
        renWinHeight = (double)(VTKRen->GetVTKWindow()->GetSize()[1]);
    }
}


// ****************************************************************************
// Method: avtOpenGLParallelAxisRenderer::RenderPlotComponents
//
// Purpose: Renders components of a ParallelAxis plot.
//
// Programmer: Mark Blair
// Creation:   Thu Jul  5 19:06:33 PDT 2007
//
// Modifications:
//
//     Mark Blair, Wed Aug 22 15:56:42 PDT 2007
//     Modified to correct an oversight in parallel rendering.
//
//     Mark Blair, Tue Oct 16 19:33:48 PDT 2007
//     Upgraded to plot axis bounds and titles as true text.
//
// ****************************************************************************

void
avtOpenGLParallelAxisRenderer::RenderPlotComponents()
{
    int pointCount;
    if ((pointCount = input->GetNumberOfPoints()) < 2) return;

    double point0[3];
    double point1[3];
    input->GetPoint(0, point0);
    input->GetPoint(1, point1);
    int point0z = int(point0[2]);
    int point1z = int(point1[2]);
    int inputType = point0z >> 10;
    int processorRank = ((point0z & 0x000001ff) << 7) | (point1z >> 16);
    int partitionSize = point1z & 0x0000ffff;
    bool sameInput = ((point0z & PCP_RENDERER_SAME_CACHED_INPUT) != 0);

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
    // Now render the plot components.
    //
    if (inputType == PCP_RENDERER_DATA_CURVE_INPUT)
        DrawDataCurves(pointCount, sameInput, processorRank, partitionSize);
    else if (inputType == PCP_RENDERER_AXIS_LINE_INPUT)
        DrawAxisLines(pointCount);
    else if (inputType == PCP_RENDERER_AXIS_BOUND_INPUT)
        DrawAxisBounds(pointCount);
    else if (inputType == PCP_RENDERER_AXIS_TITLE_INPUT)
        DrawAxisTitles(pointCount);
    else
        DrawContextQuads(pointCount, inputType);

    input->GetPoint(0, point0);
    point0[2] = double(int(point0[2]) | PCP_RENDERER_SAME_CACHED_INPUT);
    input->GetPoints()->SetPoint(0, point0);

    // Enable depth testing if it was on.
    if(enableDepthTest)
        glEnable(GL_DEPTH_TEST);

    // Enable lighting again if it was on.
    if(enableLighting)
        glEnable(GL_LIGHTING);
}


// ****************************************************************************
// Method: avtOpenGLParallelAxisRenderer::DrawDataCurves
//
// Purpose: Draws data curves in a ParallelAxis plot using data curve
//          coordinates from the ParallelAxis cache.
//
// Arguments:
//    pointCount : Number of points in all the curves
//     sameInput : Nonzero if input is same as last time and if ParallelAxis
//                 recalculation of this input was skipped in engine
//
// Programmer: Mark Blair
// Creation:   Thu Jul  5 19:06:33 PDT 2007
//
// Modifications:
//
//     Mark Blair, Wed Aug 22 15:56:42 PDT 2007
//     Modified to correct an oversight in parallel rendering.
//
// ****************************************************************************

void
avtOpenGLParallelAxisRenderer::DrawDataCurves(
    int pointCount, bool sameInput, int processorRank, int partitionSize)
{
    ColorAttribute dataCurveColor;
    
    if (colorAtts.GetNumColors() < PCP_CTX_BRIGHTNESS_LEVELS)
        dataCurveColor = colorAtts.GetColors(0);
    else
        dataCurveColor = colorAtts.GetColors(PCP_CTX_BRIGHTNESS_LEVELS);

    glColor4ubv(dataCurveColor.GetColor());

    // Set up the line properties.
    glLineWidth(1.0);
    glDisable(GL_LINE_STIPPLE);

    // Determine which curves to draw.
    if (procOrderedAxisNames.size() != partitionSize)
        InitializeCurveComparisonAtts(partitionSize);

    float *curveCoords = (float *)input->GetPoints()->GetVoidPointer(0);
    float point0Metadata = curveCoords[2];
    float point1Metadata = curveCoords[5];

    if (NeedToFindDrawableCurves(sameInput, processorRank))
    {
        ComputeDrawableCurveTemplates();
        FindDrawableCurves(pointCount, processorRank);
    }
    else
    {
        curveCoords[2] = drawableCoord2s[processorRank];
        curveCoords[5] = drawableCoord5s[processorRank];
    }

    // Draw the curves.
    float *pointXYZ;
    unsigned int hiNextCoordIndex, loNextCoordIndex;
    unsigned int drawCoordIndex = firstDrawCoordIndexs[processorRank];
    int pointNum;

    while (drawCoordIndex != PCP_END_OF_DRAWABLE_CURVE_LIST)
    {
        hiNextCoordIndex = (unsigned int)curveCoords[drawCoordIndex+2];
        loNextCoordIndex = (unsigned int)curveCoords[drawCoordIndex+5];
        curveCoords[drawCoordIndex+2] = 0.0;
        curveCoords[drawCoordIndex+5] = 0.0;

        pointXYZ = &curveCoords[drawCoordIndex];

        glBegin(GL_LINE_STRIP);

        for (pointNum = 0; pointNum < dataCurveLength; pointNum++)
        {
            glVertex3fv(pointXYZ);
            pointXYZ += 3;
        }

        glEnd();

        curveCoords[drawCoordIndex+2] = (float)hiNextCoordIndex;
        curveCoords[drawCoordIndex+5] = (float)loNextCoordIndex;

        drawCoordIndex = (hiNextCoordIndex << 16) | loNextCoordIndex;
    }

    drawableCoord2s[processorRank] = curveCoords[2];
    drawableCoord5s[processorRank] = curveCoords[5];

    curveCoords[2] = point0Metadata;
    curveCoords[5] = point1Metadata;
}


// ****************************************************************************
// Method: avtOpenGLParallelAxisRenderer::DrawAxisLines
//
// Purpose: Draws axes in a ParallelAxis plot using coordinates from the
//          ParallelAxis cache.
//
// Arguments:
//    pointCount : Number of points in all the axis lines and tick mark lines
//
// Programmer: Mark Blair
// Creation:   Tue Jul 24 17:36:07 PDT 2007
//
// Modifications:
//
//     Mark Blair, Wed Aug 22 15:56:42 PDT 2007
//     Modified to correct an oversight in parallel rendering.
//
//     Mark Blair, Tue Oct 16 19:33:48 PDT 2007
//     Now only draws axis lines and not also the stroked lines that formed
//     axis bound "text".
//
// ****************************************************************************

void
avtOpenGLParallelAxisRenderer::DrawAxisLines(int pointCount)
{
    ColorAttribute axisColor;
    
    if (colorAtts.GetNumColors() < PCP_CTX_BRIGHTNESS_LEVELS)
        axisColor = colorAtts.GetColors(1);
    else
        axisColor = colorAtts.GetColors(PCP_CTX_BRIGHTNESS_LEVELS+1);

    glColor4ubv(axisColor.GetColor());

    // Set up the line properties.
    glLineWidth(1.0);
    glDisable(GL_LINE_STIPPLE);

    // Draw the axis lines.
    float *axisCoords = (float *)input->GetPoints()->GetVoidPointer(0);
    float point0Metadata = axisCoords[2];

    axisCoords[2] = 0.0;

    glBegin(GL_LINES);

    for (int pointNum = 0; pointNum < pointCount; pointNum++)
	glVertex3fv(&axisCoords[pointNum*3]);

    glEnd();

    axisCoords[2] = point0Metadata;
}


// ****************************************************************************
// Method: avtOpenGLParallelAxisRenderer::DrawAxisBounds
//
// Purpose: Draws axis bounds in a ParallelAxis plot using bound values and
//          coordinates stored in the ParallelAxis cache.
//
// Arguments:
//    pointCount : Number of axis bound reference points (which is twice the
//                 number of axis bounds)
//
// Note: Data bound values in the ParallelAxis attributes are NOT used as the
//       axis bound values beacuase they are not always up to date.
//
// Programmer: Mark Blair
// Creation:   Tue Jul 24 17:36:07 PDT 2007
//
// Modifications:
//
//     Mark Blair, Wed Aug 22 15:56:42 PDT 2007
//     Modified to correct an oversight in parallel rendering.
//
//     Mark Blair, Wed Oct 24 14:38:54 PDT 2007
//     Now draws axis bounds as true Arial text (defined as triangles), same
//     font used by the Label plot.
//
// ****************************************************************************

void
avtOpenGLParallelAxisRenderer::DrawAxisBounds(int pointCount)
{
    int axisCount = atts.GetOrderedAxisNames().size();

    if (pointCount != axisCount * 3)
    {
        debug3 << "PCP/aOGLPAR/DAT1: ParallelAxis plot axis bound "
               << "information is inconsistent." << endl;
        return;
    }

    bool boundsAreHorizontal =
        ((atts.GetPlotToolModeFlags() & EA_VERTICAL_TEXT_AXIS_INFO_FLAG) == 0);
    double boundSize;
    Arial_Direction direction;
    Arial_Horiz_Justify horizJust;
    Arial_Vert_Justify vertJust;
    ColorAttribute boundColor;
    
    if (boundsAreHorizontal)
    {
        boundSize = VET_H_BOUND_SIZE_MIN_DIM_RATIO * ARIAL_FONT_H_SCALE_ADJUST;
        direction = RIGHTWARD_DIRECTION;
    }
    else
    {
        boundSize = VET_V_BOUND_SIZE_MIN_DIM_RATIO * ARIAL_FONT_V_SCALE_ADJUST;
        direction = UPWARD_DIRECTION;
    }

    if (colorAtts.GetNumColors() < PCP_CTX_BRIGHTNESS_LEVELS)
        boundColor = colorAtts.GetColors(2);
    else
        boundColor = colorAtts.GetColors(PCP_CTX_BRIGHTNESS_LEVELS+2);

    glColor4ubv(boundColor.GetColor());
    
    // Draw the axes and axis bounds.
    float *boundCoords = (float *)input->GetPoints()->GetVoidPointer(0);
    float point0Metadata = boundCoords[2];
    char boundValText[VET_H_BOUND_MAX_CHARS + 1];

    boundCoords[2] = 0.0;
    
    glBegin(GL_TRIANGLES);
    
    for (int axisNum = 0; axisNum < axisCount; axisNum++)
    {
        if (boundsAreHorizontal)
        {
            if (axisNum == 0)
                horizJust = LEFT_HORIZ_JUSTIFY;
            else if (axisNum == axisCount-1)
                horizJust = RIGHT_HORIZ_JUSTIFY;
            else
                horizJust = CENTER_HORIZ_JUSTIFY;
        }

        if (!boundsAreHorizontal)
        {
            horizJust = RIGHT_HORIZ_JUSTIFY;
            vertJust  = CENTER_VERT_JUSTIFY;
        }
        MakeAxisBoundText(boundValText, (double)boundCoords[axisNum*9+6]);

        DrawArialTextString(boundValText,
            (double)boundCoords[axisNum*9], (double)boundCoords[axisNum*9+1],
            boundSize, direction, horizJust, vertJust);

        if (!boundsAreHorizontal)
        {
            horizJust = LEFT_HORIZ_JUSTIFY;
            vertJust  = BOTTOM_VERT_JUSTIFY;
        }
        MakeAxisBoundText(boundValText, (double)boundCoords[axisNum*9+7]);

        DrawArialTextString(boundValText,
            (double)boundCoords[axisNum*9+3], (double)boundCoords[axisNum*9+4],
            boundSize, direction, horizJust, vertJust);
    }

    glEnd();

    boundCoords[2] = point0Metadata;
}


// ****************************************************************************
// Method: avtOpenGLParallelAxisRenderer::DrawAxisTitles
//
// Purpose: Draws axis titles in a ParallelAxis plot using titles in current
//          attributes and coordinates from the ParallelAxis cache.
//
// Arguments:
//    pointCount : Number of axis title reference points (and titles)
//
// Programmer: Mark Blair
// Creation:   Tue Jul 24 17:36:07 PDT 2007
//
// Modifications:
//
//     Mark Blair, Wed Aug 22 15:56:42 PDT 2007
//     Modified to correct an oversight in parallel rendering.
//
//     Mark Blair, Wed Oct 24 14:38:54 PDT 2007
//     Now draws axis titles as true Arial text (defined as triangles), same
//     font used by the Label plot.
//
// ****************************************************************************

void
avtOpenGLParallelAxisRenderer::DrawAxisTitles(int pointCount)
{
    const stringVector axisNames = atts.GetOrderedAxisNames();

    if (pointCount != axisNames.size())
    {
        debug3 << "PCP/aOGLPAR/DAT1: ParallelAxis plot axis title "
               << "information is inconsistent." << endl;
        return;
    }

    bool titlesAreHorizontal =
        ((atts.GetPlotToolModeFlags() & EA_VERTICAL_TEXT_AXIS_INFO_FLAG) == 0);
    double titleSize;
    int maxChars;
    Arial_Direction direction;
    Arial_Horiz_Justify horizJust;
    Arial_Vert_Justify vertJust;
    ColorAttribute titleColor;
    
    if (titlesAreHorizontal)
    {
        titleSize = VET_H_TITLE_SIZE_MIN_DIM_RATIO * ARIAL_FONT_H_SCALE_ADJUST;
        maxChars  = VET_H_TITLE_MAX_CHARS;
        direction = RIGHTWARD_DIRECTION;
        vertJust  = BOTTOM_VERT_JUSTIFY;
    }
    else
    {
        titleSize = VET_V_TITLE_SIZE_MIN_DIM_RATIO * ARIAL_FONT_V_SCALE_ADJUST;
        maxChars  = VET_V_TITLE_MAX_CHARS;
        direction = UPWARD_DIRECTION;
        horizJust = RIGHT_HORIZ_JUSTIFY;
        vertJust  = TOP_VERT_JUSTIFY;
    }

    if (colorAtts.GetNumColors() < PCP_CTX_BRIGHTNESS_LEVELS)
        titleColor = colorAtts.GetColors(3);
    else
        titleColor = colorAtts.GetColors(PCP_CTX_BRIGHTNESS_LEVELS+3);

    glColor4ubv(titleColor.GetColor());
    
    // Draw the axes and axis bounds.
    float *titleCoords = (float *)input->GetPoints()->GetVoidPointer(0);
    float point0Metadata = titleCoords[2];
    char axisTitle[VET_H_TITLE_MAX_CHARS + 1];

    titleCoords[2] = 0.0;
    
    glBegin(GL_TRIANGLES);

    for (int pointNum = 0; pointNum < pointCount; pointNum++)
    {
        if (titlesAreHorizontal)
        {
            if (pointNum == 0)
                horizJust = LEFT_HORIZ_JUSTIFY;
            else if (pointNum == pointCount-1)
                horizJust = RIGHT_HORIZ_JUSTIFY;
            else
                horizJust = CENTER_HORIZ_JUSTIFY;
        }
        
        MakeAxisTitleText(axisTitle, axisNames[pointNum], maxChars);
        
        DrawArialTextString(axisTitle,
            (double)titleCoords[pointNum*3], (double)titleCoords[pointNum*3+1],
            titleSize, direction, horizJust, vertJust);
    }

    glEnd();

    titleCoords[2] = point0Metadata;
}


// ****************************************************************************
// Method: avtOpenGLParallelAxisRenderer::DrawContextQuads
//
// Purpose: Draws "context" quadrilaterals of one color in a ParallelAxis plot
//          using coordinates from the ParallelAxis cache.
//
// Arguments:
//    pointCount : Number of vertices in all the quads to be drawn
//       colorID : Index of the color of the quads to be drawn
//
// Programmer: Mark Blair
// Creation:   Tue Jul 24 17:36:07 PDT 2007
//
// Modifications:
//
//     Mark Blair, Wed Aug 22 15:56:42 PDT 2007
//     Modified to correct an oversight in parallel rendering.
//
// ****************************************************************************

void
avtOpenGLParallelAxisRenderer::DrawContextQuads(int pointCount, int brightLevel)
{
    glColor4ubv(colorAtts.GetColors(brightLevel).GetColor());

    float *vertexCoords = (float *)input->GetPoints()->GetVoidPointer(0);
    float point0Metadata = vertexCoords[2];

    vertexCoords[2] = 0.0;

    glBegin(GL_QUADS);

    for (int pointNum = 0; pointNum < pointCount; pointNum++)
	glVertex3fv(&vertexCoords[pointNum*3]);

    glEnd();

    vertexCoords[2] = point0Metadata;
}


// ****************************************************************************
// Method: avtOpenGLParallelAxisRenderer::NeedToFindDrawableCurves
//
// Purpose: Returns true if no plot input data or Extents tool extents have
//          changed since last rendering, which means that the same drawable
//          curves can be rendered.
//
// Arguments:
// numProcessors : Number of parallel processors contributing to plot
//     sameInput : Nonzero if input is same as last time and if ParallelAxis
//                 recalculation of this input was skipped in engine
//
// Programmer: Mark Blair
// Creation:   Tue Jul 24 17:36:07 PDT 2007
//
// Modifications:
//
//     Mark Blair, Wed Aug 22 15:56:42 PDT 2007
//     Modified to correct an oversight in parallel rendering.
//
// ****************************************************************************

bool
avtOpenGLParallelAxisRenderer::NeedToFindDrawableCurves(
    bool sameInput, int processorRank)
{
    stringVector curOrderedAxisNames = atts.GetOrderedAxisNames();
    doubleVector curExtentMinima     = atts.GetExtentMinima();
    doubleVector curExtentMaxima     = atts.GetExtentMaxima();
    intVector    curAxisInfoFlagSets = atts.GetAxisInfoFlagSets();

    bool needNewCurves = false;

    if (!sameInput)
        needNewCurves = true;
    else if (curOrderedAxisNames != procOrderedAxisNames[processorRank])
        needNewCurves = true;
    else if (curExtentMinima != procExtentMinima[processorRank])
        needNewCurves = true;
    else if (curExtentMaxima != procExtentMaxima[processorRank])
        needNewCurves = true;
    else
    {
        needNewCurves = false;

        if ((atts.GetPlotToolModeFlags() & EA_THRESHOLD_SELECTED_ONLY_FLAG) != 0)
        {
            intVector procFlagSets = procAxisInfoFlagSets[processorRank];
            int axisCount = curAxisInfoFlagSets.size();
            int axisID, flagDiffs;

            for (axisID = 0; axisID < axisCount; axisID++)
            {
                flagDiffs =  curAxisInfoFlagSets[axisID] ^ procFlagSets[axisID];
                if ((flagDiffs & EA_SELECTED_AXIS_FLAGS) != 0) break;
            }

            if (axisID < axisCount) needNewCurves = true;
        }
    }

    procOrderedAxisNames[processorRank] = curOrderedAxisNames;
    procExtentMinima[processorRank]     = curExtentMinima;
    procExtentMaxima[processorRank]     = curExtentMaxima;
    procAxisInfoFlagSets[processorRank] = curAxisInfoFlagSets;

    return needNewCurves;
}


// ****************************************************************************
// Method: avtOpenGLParallelAxisRenderer::InitializeCurveComparisonAtts
//
// Purpose: For each processor contributing to the plot, initialize attributes
//          that will be compared to current attributes to determine whether
//          the set of drawable data curves has potentially changed.
//
// Programmer: Mark Blair
// Creation:   Wed Aug 22 15:56:42 PDT 2007
//
// Modifications:
//
// ****************************************************************************

void
avtOpenGLParallelAxisRenderer::InitializeCurveComparisonAtts(int partitionSize)
{
    procOrderedAxisNames.clear();
    procExtentMinima.clear();
    procExtentMaxima.clear();
    procAxisInfoFlagSets.clear();

    firstDrawCoordIndexs.clear();
    drawableCoord2s.clear();
    drawableCoord5s.clear();

    for (int procNum = 0; procNum < partitionSize; procNum++)
    {
        procOrderedAxisNames.push_back(*(new stringVector));
        procExtentMinima.push_back(*(new doubleVector));
        procExtentMaxima.push_back(*(new doubleVector));
        procAxisInfoFlagSets.push_back(*(new intVector));

        firstDrawCoordIndexs.push_back(PCP_END_OF_DRAWABLE_CURVE_LIST);
        drawableCoord2s.push_back(0.0);
        drawableCoord5s.push_back(0.0);
    }
}


// ****************************************************************************
// Method: avtOpenGLParallelAxisRenderer::ComputeDrawableCurveTemplates
//
// Purpose: Determines which sequence of the current plot axes corresponds to
//          the input data and which subsequence of that sequence corresponds
//          to extents that should be applied to each input data curve to
//          determine whether the curve should be drawn or not.
//
// Programmer: Mark Blair
// Creation:   Tue Jul 24 17:36:07 PDT 2007
//
// Modifications:
//
// ****************************************************************************

void
avtOpenGLParallelAxisRenderer::ComputeDrawableCurveTemplates()
{
    intVector axisInfoFlagSets = atts.GetAxisInfoFlagSets();
    int axisCount = axisInfoFlagSets.size();
    int plotToolModeFlags = atts.GetPlotToolModeFlags();
    int leftDrawAxisID, rightDrawAxisID, axisID;
    int axisInfoFlagSet;
    bool badDrawAxes = false;
    bool badTestAxes = false;

    if ((plotToolModeFlags & EA_TOOL_DRAWS_AXIS_INFO_FLAG) == 0)
    {
        applyExtents = false;

        leftDrawAxisID = 0; rightDrawAxisID = axisCount - 1;
        leftTestAxisID = 0; rightTestAxisID = axisCount - 1;
    }
    else
    {
        applyExtents = true;
        ComputeWorldCoordinateExtents();

        leftDrawAxisID = -1; rightDrawAxisID = -1;
        leftTestAxisID = -1; rightTestAxisID = -1;
    
        for (axisID = 0; axisID < axisCount; axisID++)
        {
            axisInfoFlagSet = axisInfoFlagSets[axisID];
        
            if ((axisInfoFlagSet & EA_LEFT_SHOWN_AXIS_FLAG) != 0)
                leftDrawAxisID = axisID;
            if ((axisInfoFlagSet & EA_RIGHT_SHOWN_AXIS_FLAG) != 0)
                rightDrawAxisID = axisID;
        
            if ((axisInfoFlagSet & EA_LEFT_SELECTED_AXIS_FLAG) != 0)
                leftTestAxisID = axisID;
            if ((axisInfoFlagSet & EA_RIGHT_SELECTED_AXIS_FLAG) != 0)
                rightTestAxisID = axisID;
        }
    
        if (leftDrawAxisID  == -1)
	    badDrawAxes = true;
        else if (rightDrawAxisID == -1)
	    badDrawAxes = true;
        else if (leftDrawAxisID >= rightDrawAxisID)
	    badDrawAxes = true;

        if (badDrawAxes)
        {
            leftDrawAxisID  = 0;
            rightDrawAxisID = axisCount - 1;
        }
    
        if (leftTestAxisID  == -1)
	    badTestAxes = true;
        else if (rightTestAxisID == -1)
	    badTestAxes = true;
        else if (leftTestAxisID >= rightTestAxisID)
	    badTestAxes = true;
        else if (leftTestAxisID  <  leftDrawAxisID)
	    badTestAxes = true;
        else if (rightTestAxisID > rightDrawAxisID)
	    badTestAxes = true;

        if (badTestAxes ||
            ((plotToolModeFlags & EA_THRESHOLD_SELECTED_ONLY_FLAG) == 0))
        {
            leftTestAxisID  = leftDrawAxisID;
            rightTestAxisID = rightDrawAxisID;
        }
    }

    if (badDrawAxes || badTestAxes)
    {
        debug3 << "PCP/aOGLPAR/CST1: ParallelAxis plot shown/selected "
               << "axis marks missing or out of order." << endl;
    }

    dataCurveLength = rightDrawAxisID - leftDrawAxisID + 1;
    testAxisOffset = leftTestAxisID - leftDrawAxisID;
}


// ****************************************************************************
// Method: avtOpenGLParallelAxisRenderer::FindDrawableCurves
//
// Purpose: Constructs a linked list of input data curves to be drawn.
//
// Programmer: Mark Blair
// Creation:   Tue Jul 24 17:36:07 PDT 2007
//
// Modifications:
//
//     Mark Blair, Wed Aug 22 15:56:42 PDT 2007
//     Modified to correct an oversight in parallel rendering.
//
// ****************************************************************************

void
avtOpenGLParallelAxisRenderer::FindDrawableCurves(
    int pointCount, int processorRank)
{
    unsigned int curveCount = (unsigned int)(pointCount / dataCurveLength);
    unsigned int coordsPerCurve = (unsigned int)dataCurveLength * 3;
    unsigned int testCoordIndex = (unsigned int)testAxisOffset * 3;
    unsigned int prevDrawCoordIndex = PCP_END_OF_DRAWABLE_CURVE_LIST;
    unsigned int drawCoordIndex, hiDrawCoordIndex, loDrawCoordIndex;
    unsigned int curveNum;
    int axisID;

    float *curveCoords = (float *)input->GetPoints()->GetVoidPointer(0);
    float *pointXYZ;
    double pointY;

    for (curveNum = 0; curveNum < curveCount; curveNum++)
    {
        if (applyExtents)
        {
            pointXYZ = &curveCoords[testCoordIndex];

            for (axisID = leftTestAxisID; axisID <= rightTestAxisID; axisID++)
            {
                pointY = (double)pointXYZ[1];

                if (pointY < worldExtentMinima[axisID]) break;
                if (pointY > worldExtentMaxima[axisID]) break;

                pointXYZ += 3;
            }
        }
        else
        {
            axisID = rightTestAxisID + 1;
        }

        if (axisID > rightTestAxisID)
        {
            drawCoordIndex = curveNum * coordsPerCurve;

            if (prevDrawCoordIndex == PCP_END_OF_DRAWABLE_CURVE_LIST)
                firstDrawCoordIndexs[processorRank] = drawCoordIndex;
            else
            {
                hiDrawCoordIndex = (drawCoordIndex >> 16) & 0x0000ffff;
                loDrawCoordIndex =  drawCoordIndex        & 0x0000ffff;

                curveCoords[prevDrawCoordIndex+2] = (float)hiDrawCoordIndex;
                curveCoords[prevDrawCoordIndex+5] = (float)loDrawCoordIndex;
            }

            prevDrawCoordIndex = drawCoordIndex;
        }

        testCoordIndex += coordsPerCurve;
    }

    if (prevDrawCoordIndex == PCP_END_OF_DRAWABLE_CURVE_LIST)
        firstDrawCoordIndexs[processorRank] = PCP_END_OF_DRAWABLE_CURVE_LIST;
    else
    {
        hiDrawCoordIndex = (PCP_END_OF_DRAWABLE_CURVE_LIST >> 16) & 0x0000ffff;
        loDrawCoordIndex =  PCP_END_OF_DRAWABLE_CURVE_LIST        & 0x0000ffff;

        curveCoords[prevDrawCoordIndex+2] = (float)hiDrawCoordIndex;
        curveCoords[prevDrawCoordIndex+5] = (float)loDrawCoordIndex;
    }
}


// ****************************************************************************
// Method: avtOpenGLParallelAxisRenderer::ComputeWorldCoordinateExtents
//
// Purpose: Translates current extents from [0,1] to world coordinate range
//
// Programmer: Mark Blair
// Creation:   Tue Jul 24 17:36:07 PDT 2007
//
// Modifications:
//
// ****************************************************************************

void
avtOpenGLParallelAxisRenderer::ComputeWorldCoordinateExtents()
{
    doubleVector curExtentMinima = atts.GetExtentMinima();
    doubleVector curExtentMaxima = atts.GetExtentMaxima();
    int axisCount = curExtentMinima.size();
    double bottomAxisY, topAxisY, axisHeight;

    if ((atts.GetPlotToolModeFlags() & EA_VERTICAL_TEXT_AXIS_INFO_FLAG) != 0)
    {
        bottomAxisY = PCP_V_BOTTOM_AXIS_Y_FRACTION;
        topAxisY    = PCP_V_TOP_AXIS_Y_FRACTION;
    }
    else
    {
        bottomAxisY = PCP_H_BOTTOM_AXIS_Y_FRACTION;
        topAxisY    = PCP_H_TOP_AXIS_Y_FRACTION;
    }

    if ((atts.GetPlotToolModeFlags() & EA_TOOL_DRAWS_AXIS_INFO_FLAG) != 0)
    {
        bottomAxisY += EA_TOOL_BUTTON_MARGIN_FRACTION;
        topAxisY    -= EA_TOOL_MARK_MARGIN_FRACTION;
    }   // Test is redundant.  This method is called only when tool is enabled.

    axisHeight = topAxisY - bottomAxisY;

    worldExtentMinima.clear(); worldExtentMaxima.clear();

    for (int axisID = 0; axisID < axisCount; axisID++)
    {
        worldExtentMinima.push_back(
            curExtentMinima[axisID]*axisHeight + bottomAxisY);
        worldExtentMaxima.push_back(
            curExtentMaxima[axisID]*axisHeight + bottomAxisY);
    }
}


// *****************************************************************************
//  Method: avtOpenGLParallelAxisRenderer::MakeAxisBoundText
//
//  Purpose: Converts a double-precision floating-point axis bound value to a
//           VisIt-style C string representation.
//
//  Programmer: Mark Blair
//  Creation:   Wed Oct 24 14:38:54 PDT 2007
//
//  Modifications:
//
// *****************************************************************************

void
avtOpenGLParallelAxisRenderer::MakeAxisBoundText(char boundText[], double boundValue)
{
    int textLen, charNum;

    if (boundValue < -9e+36) {
        strcpy(boundText, "min");
        return;
    }

    if (boundValue > +9e+36) {
        strcpy(boundText, "max");
        return;
    }

    sprintf (boundText, "%g", boundValue);

    if (strchr(boundText, 'e') != NULL) return;
    if (strchr(boundText, 'E') != NULL) return;
    
    if ((textLen = strlen(boundText)) < 3) return;

    for (charNum = textLen - 1; charNum > 1; charNum--)
    {
        if (boundText[charNum  ] != '0') break;
        if (boundText[charNum-1] == '.') break;
        if (boundText[charNum-1] == '+') break;
        if (boundText[charNum-1] == '-') break;
    }

    boundText[charNum + 1] = '\0';
}


// *****************************************************************************
//  Method: avtOpenGLParallelAxisRenderer::MakeAxisTitleText
//
//  Purpose: Creates a displayable axis title from an axis variable name.  Long
//           titles and compound titles are shortened in a meaningful way.
//
//  Programmer: Mark Blair
//  Creation:   Wed Oct 24 14:38:54 PDT 2007
//
//  Modifications:
//
// *****************************************************************************

void
avtOpenGLParallelAxisRenderer::MakeAxisTitleText(
    char titleText[], const std::string &axisTitle, int maxTitleChars)
{
    int rawTitleLen;
    char rawTitle[121];
    
    strncpy(rawTitle, axisTitle.c_str(), 120);
    
    if ((rawTitleLen = strlen(rawTitle)) <= maxTitleChars)
        strcpy(titleText, rawTitle);
    else
    {
        rawTitle[maxTitleChars-4] = '\0';
        sprintf(titleText,"%s..%s", rawTitle, &rawTitle[rawTitleLen-2]);
    }
}


// ****************************************************************************
// Method: avtOpenGLParallelAxisRenderer::DrawArialTextString
//
// Purpose: Draws a text string in the ParallelAxis plot using Arial font
//
// Arguments:
//      text: The string to be drawn
// xPosition: X coordinate of string reference position
// yPosition: Y coordinate of string reference position
//  charSize: Height of a line of characters of the desired size
//    direct: Direction the string follows (left, right, down, up) from first
//            character to last
//     hJust: String's horizontal position relative to reference position
//     vJust: String's vertical position relative to reference position
//
// Programmer: Mark Blair
// Creation:   Tue Oct 16 19:33:48 PDT 2007
//
// Modifications:
//
// ****************************************************************************

void
avtOpenGLParallelAxisRenderer::DrawArialTextString(
    const char *text, double xPosition, double yPosition, double charSize,
    Arial_Direction direct, Arial_Horiz_Justify hJust, Arial_Vert_Justify vJust)
{
    int charCount = strlen(text);
    if (charCount == 0) return;

    double tst[3][2];
    
    ConstructTextStringTransform(
        tst, text, xPosition, yPosition, charSize, direct, hJust, vJust);
        
    double xBottomLeft = 0.0;
    double xCoord, yCoord;
    int charID, triCoordID, coordCount;
    unsigned int charCode;
    
    float triCoords[3];
    triCoords[2] = 0.0;

    for (charID = 0; charID < charCount; charID++)
    {
        charCode = (unsigned int)text[charID];
        triCoordID = arial_triangle_start[charCode];
        coordCount = arial_triangle_ntriangles[charCode] * 3;
        
        while (coordCount-- > 0)
        {
            xCoord = (double)arial_font_x[arial_triangle_x_index[triCoordID]] +
                     xBottomLeft;
            yCoord = (double)arial_font_y[arial_triangle_y_index[triCoordID++]];
            
            triCoords[0] = float(tst[0][0]*xCoord + tst[1][0]*yCoord + tst[2][0]);
            triCoords[1] = float(tst[0][1]*xCoord + tst[1][1]*yCoord + tst[2][1]);

	    glVertex3fv(triCoords);
        }
        
        xBottomLeft += arial_triangle_spacing[charCode];
    }
}


// ****************************************************************************
// Method: avtOpenGLParallelAxisRenderer::ConstructTexStringtTransform
//
// Purpose: Constructs matrix of the transformation needed to draw an Arial
//          text string in the ParallelAxis plot
//
// Arguments:
//       tst: The constructed transformation matrix
//      text: The string to be drawn
// xPosition: X coordinate of string reference position
// yPosition: Y coordinate of string reference position
//  charSize: Height of a line of characters of the desired size
//    direct: Direction the string follows (left, right, down, up) from first
//            character to last
//     hJust: String's horizontal position relative to reference position
//     vJust: String's vertical position relative to reference position
//
// Programmer: Mark Blair
// Creation:   Tue Oct 16 19:33:48 PDT 2007
//
// Modifications:
//
// ****************************************************************************

void
avtOpenGLParallelAxisRenderer::ConstructTextStringTransform(double tst[3][2],
    const char *text, double xPosition, double yPosition, double charSize,
    Arial_Direction direct, Arial_Horiz_Justify hJust, Arial_Vert_Justify vJust)
{
    double textWidth = 0.0;
    double saveCoeff, distortX, distortY;
    int textLen = strlen(text);

    for (int charID = 0; charID < textLen; charID++)
        textWidth += (double)arial_triangle_spacing[(unsigned int)text[charID]];

    if (hJust == LEFT_HORIZ_JUSTIFY)
        tst[2][0] = 0.0;
    else if (hJust == CENTER_HORIZ_JUSTIFY)
        tst[2][0] = -textWidth * 0.5;
    else  // RIGHT_HORIZ_JUSTIFY
        tst[2][0] = -textWidth;

    if (vJust == BOTTOM_VERT_JUSTIFY)
        tst[2][1] = 0.0;
    else if (vJust == CENTER_VERT_JUSTIFY)
        tst[2][1] = -0.35791;
    else  // TOP_VERT_JUSTIFY
        tst[2][1] = -0.71582;
        
    tst[0][0] =  charSize; tst[0][1]  = 0.0;
    tst[1][0] =  0.0;      tst[1][1]  = charSize;
    tst[2][0] *= charSize; tst[2][1] *= charSize;

    if (direct == LEFTWARD_DIRECTION)
    {
        tst[0][0] = -tst[0][0]; tst[1][1] = -tst[1][1];
        tst[2][0] = -tst[2][0]; tst[2][1] = -tst[2][1];
    }
    else if (direct == DOWNWARD_DIRECTION)
    {
        tst[0][1] = -tst[0][0]; tst[0][0] = 0.0;
        tst[1][0] =  tst[1][1]; tst[1][1] = 0.0;
        saveCoeff =  tst[2][0]; tst[2][0] = tst[2][1]; tst[2][1] = -saveCoeff;
    }
    else if (direct == UPWARD_DIRECTION)
    {
        tst[0][1] =  tst[0][0]; tst[0][0] = 0.0;
        tst[1][0] = -tst[1][1]; tst[1][1] = 0.0;
        saveCoeff =  tst[2][0]; tst[2][0] = -tst[2][1]; tst[2][1] = saveCoeff;
    }
    
    if (renWinWidth > renWinHeight)
    {
        distortX = renWinHeight / renWinWidth;
        distortY = 1.0;
    }
    else
    {
        distortX = 1.0;
        distortY = renWinWidth / renWinHeight;
    }
    
    tst[0][0] *= distortX; tst[1][0] *= distortX; tst[2][0] *= distortX;
    tst[0][1] *= distortY; tst[1][1] *= distortY; tst[2][1] *= distortY;
    
    tst[2][0] += xPosition; tst[2][1] += yPosition;
}
