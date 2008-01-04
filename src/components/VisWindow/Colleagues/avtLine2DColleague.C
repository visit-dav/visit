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

// ************************************************************************* //
//                              avtLine2DColleague.C                         //
// ************************************************************************* //
#include <avtLine2DColleague.h>

#include <math.h>

#include <snprintf.h>

#include <vtkActor2D.h>
#include <vtkAppendPolyData.h>
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>

#include <AnnotationObject.h>
#include <VisWindow.h>
#include <VisWindowColleagueProxy.h>

#include <avtVector.h>

#define ARROW_SIZE       0.01F
#define ARROW_POINTINESS 3.f

// ****************************************************************************
// Method: avtLine2DColleague::avtLine2DColleague
//
// Purpose: 
//   Constructor for the avtLine2DColleague class.
//
// Arguments:
//   m : The vis window proxy.
//
// Programmer: John C. Anderson
// Creation:   Mon Jul 12 15:55:58 PDT 2004
//
// Modifications:
//   Brad Whitlock, Tue Jun 28 15:32:03 PST 2005
//   Removed the cone sources.
//
// ****************************************************************************
avtLine2DColleague::avtLine2DColleague(VisWindowColleagueProxy &m):
    avtAnnotationColleague(m),
    actor(NULL),
    mapper(NULL),
    allData(NULL),
    lineData(NULL),
    beginArrowSolid(NULL),
    endArrowSolid(NULL),
    beginArrowLine(NULL),
    endArrowLine(NULL),
    beginArrowStyle(0),
    endArrowStyle(0)
{
    addedToRenderer = false;

    // Create a float array which represents the points.
    vtkFloatArray* pcoords = vtkFloatArray::New();
    
    // Set number of components and tuples.
    pcoords->SetNumberOfComponents(3);
    pcoords->SetNumberOfTuples(2);

    double pts[2][3] = { {0.25, 0.25, 0.0}, {0.75, 0.75, 0.0} };
    for(int i = 0; i < 2; i++)
        pcoords->SetTuple(i, pts[i]);

    // Create vtkPoints and assign pcoords as the internal data array.
    vtkPoints* points = vtkPoints::New();
    points->SetData(pcoords);

    // Create the line.
    vtkCellArray* lines = vtkCellArray::New();
    lines->InsertNextCell(2);
    lines->InsertCellPoint(0);
    lines->InsertCellPoint(1);

    // Create the line polydata.
    lineData = vtkPolyData::New();

    // Assign points and cells.
    lineData->SetPoints(points);
    lineData->SetLines(lines);
    
    // Delete stuff that is now in the polydata.
    //pcoords->Delete();
    //points->Delete();
    //lines->Delete();

    // Setup the solid and line arrow object.
    beginArrowSolid = vtkPolyData::New();
    endArrowSolid = vtkPolyData::New();
    makeArrows(beginArrowSolid, endArrowSolid, false);
    beginArrowLine = vtkPolyData::New();
    endArrowLine = vtkPolyData::New();
    makeArrows(beginArrowLine, endArrowLine, true);

    // Create the mapper.
    mapper = vtkPolyDataMapper2D::New();
    mapper->SetInput(lineData);

    //
    // Create and position the actor.
    //
    actor = vtkActor2D::New();
    actor->SetMapper(mapper);

    vtkCoordinate *pos = actor->GetPositionCoordinate();
    pos->SetCoordinateSystemToNormalizedViewport();
    actor->SetPosition(0., 0.);

    pos = actor->GetPosition2Coordinate();
    pos->SetCoordinateSystemToNormalizedViewport();
    actor->SetPosition2(1., 1.);

    mapper->SetTransformCoordinate(pos);

    // Set a default color.
    double fgColor[3];
    mediator.GetForegroundColor(fgColor);
    actor->GetProperty()->SetColor(fgColor[0], fgColor[1], fgColor[2]);

    // Set default opacity.
    actor->GetProperty()->SetOpacity(1.);

    // Set a default line width.
    actor->GetProperty()->SetLineWidth(1);
}

// ****************************************************************************
// Method: avtLine2DColleague::~avtLine2DColleague
//
// Purpose: 
//   Destructor for the avtLine2DColleague class.
//
// Programmer: John C. Anderson
// Creation:   Mon Jul 12 16:24:34 PDT 2004
//
// Modifications:
//   
// ****************************************************************************
avtLine2DColleague::~avtLine2DColleague()
{
    if(actor != NULL)
    {
        actor->Delete();
        actor = NULL;
    }

    if(mapper != NULL)
    {
        mapper->Delete();
        mapper = NULL;
    }

    if(lineData != NULL)
    {
        lineData->Delete();
        lineData = NULL;
    }
}

// ****************************************************************************
// Method:  avtLine2DColleague::makeArrows
//
// Purpose:
//   Make the beginning and ending line arrows for a line.
//
// Arguments:
//   a0        : Polydata for the first arrow.
//   a1        : Polydata for the second arrow.
//   makeLines : Tells the routine to make the polydata be lines.
//
// Programmer: John C. Anderson
// Creation:   Thu Jul 15 09:15:36 PDT 2004
//
// Modifications:
//   Brad Whitlock, Tue Jun 28 15:33:11 PST 2005
//   I made the polydata that we're creating get passed in so we can initialize
//   both lines and solids so they have the same geometry.
//
// ****************************************************************************

void
avtLine2DColleague::makeArrows(vtkPolyData *a0, vtkPolyData *a1, bool makeLines)
{
    //
    // Make the begin line arrow.
    //

    // Create a float array which represents the points.
    vtkFloatArray* pcoordsBegin = vtkFloatArray::New();
    
    // Set number of components and tuples.
    pcoordsBegin->SetNumberOfComponents(3);
    pcoordsBegin->SetNumberOfTuples(3);

    double ptsBegin[3][3] = { {0., 0., 0.}, {1., 1., 0.}, {1., 0., 0.} };
    int i;
    for(i = 0; i < 3; i++)
        pcoordsBegin->SetTuple(i, ptsBegin[i]);

    // Create vtkPoints and assign pcoords as the internal data array.
    vtkPoints* pointsBegin = vtkPoints::New();
    pointsBegin->SetData(pcoordsBegin);

    // Create the line.
    vtkCellArray* linesBegin = vtkCellArray::New();
    linesBegin->InsertNextCell(3);
    linesBegin->InsertCellPoint(0);
    linesBegin->InsertCellPoint(1);
    linesBegin->InsertCellPoint(2);

    // Assign points and cells.
    a0->SetPoints(pointsBegin);
    if(makeLines)
        a0->SetLines(linesBegin);
    else
        a0->SetPolys(linesBegin);
    
    // Delete stuff that is now in the polydata.
    //pcoordsBegin->Delete();
    //pointsBegin->Delete();
    //linesBegin->Delete();

    //
    // Make the end line arrow.
    //

    // Create a float array which represents the points.
    vtkFloatArray* pcoordsEnd = vtkFloatArray::New();
    
    // Set number of components and tuples.
    pcoordsEnd->SetNumberOfComponents(3);
    pcoordsEnd->SetNumberOfTuples(3);

    double ptsEnd[3][3] = { {0., 1., 0.}, {0., 0., 0.}, {1., 1., 0.} };
    for(i = 0; i < 3; i++)
        pcoordsEnd->SetTuple(i, ptsEnd[i]);

    // Create vtkPoints and assign pcoords as the internal data array.
    vtkPoints* pointsEnd = vtkPoints::New();
    pointsEnd->SetData(pcoordsEnd);

    // Create the line.
    vtkCellArray* linesEnd = vtkCellArray::New();
    linesEnd->InsertNextCell(3);
    linesEnd->InsertCellPoint(0);
    linesEnd->InsertCellPoint(1);
    linesEnd->InsertCellPoint(2);

    // Assign points and cells.
    a1->SetPoints(pointsEnd);
    if(makeLines)
        a1->SetLines(linesEnd);
    else
        a1->SetPolys(linesEnd);

    // Delete stuff that is now in the polydata.
    //pcoordsEnd->Delete();
    //pointsEnd->Delete();
    //linesEnd->Delete();
}

// ****************************************************************************
// Method: avtLine2DColleague::updateArrows
//
// Purpose:
//   Update the line arrows for a line.
//
// Arguments:
//
// Returns:
//
// Note:
//
// Programmer: John C. Anderson
// Creation:   Thu Jul 15 09:16:38 PDT 2004
//
// Modifications:
//   Brad Whitlock, Tue Jun 28 14:49:58 PST 2005
//   Rewrote using avtVector to make a pointier arrow.
//
// ****************************************************************************

void
avtLine2DColleague::updateArrows(vtkPolyData *a0, vtkPolyData *a1,
    double *p0, double *p1)
{
    if(beginArrowStyle > 0)
    {
        avtVector v0(p0[0] - p1[0], p0[1] - p1[1], 0.);
        avtVector v0n(v0 * -1.);
        v0n.normalize();
        avtVector Z(0., 0., 1.);
        avtVector perp(v0n % Z);
        avtVector initial(p0[0], p0[1], 0.);
        avtVector dx(perp * ARROW_SIZE);
        avtVector dy(v0n  * ARROW_SIZE * ARROW_POINTINESS);
        avtVector point0 = initial + dx + dy;
        avtVector point2 = initial - dx + dy;

        a0->GetPoints()->SetPoint(1, p0[0], p0[1], 0.);
        a0->GetPoints()->SetPoint(0, point0.x, point0.y, 0);
        a0->GetPoints()->SetPoint(2, point2.x, point2.y, 0);
    }

    if(endArrowStyle > 0)
    {
        avtVector v0(p0[0] - p1[0], p0[1] - p1[1], 0.);
        avtVector v0n(v0);
        v0n.normalize();
        avtVector Z(0., 0., 1.);
        avtVector perp(v0n % Z);
        avtVector initial(p1[0], p1[1], 0.);
        avtVector dx(perp * ARROW_SIZE);
        avtVector dy(v0n  * ARROW_SIZE * ARROW_POINTINESS);
        avtVector point0 = initial + dx + dy;
        avtVector point2 = initial - dx + dy;

        a1->GetPoints()->SetPoint(1, p1[0], p1[1], 0.);
        a1->GetPoints()->SetPoint(0, point0.x, point0.y, 0);
        a1->GetPoints()->SetPoint(2, point2.x, point2.y, 0);
    }
}

// ****************************************************************************
// Method: avtLine2DColleague::AddToRenderer
//
// Purpose: 
//   This method adds the text actor to the renderer.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 6 15:52:19 PST 2003
//
// Modifications:
//   
// ****************************************************************************
void 
avtLine2DColleague::AddToRenderer()
{
    if(!addedToRenderer && ShouldBeAddedToRenderer())
    {
        mediator.GetForeground()->AddActor2D(actor);
        addedToRenderer = true;
    }
}

// ****************************************************************************
// Method: avtLine2DColleague::RemoveFromRenderer
//
// Purpose: 
//   This method removes the text actor from the renderer.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 6 15:52:38 PST 2003
//
// Modifications:
//   
// ****************************************************************************
void
avtLine2DColleague::RemoveFromRenderer()
{
    if(addedToRenderer)
    {
        mediator.GetForeground()->RemoveActor2D(actor);
        addedToRenderer = false;
    }
}

// ****************************************************************************
// Method: avtLine2DColleague::Hide
//
// Purpose: 
//   This method toggles the visible flag and either adds or removes the text
//   actor to/from the renderer.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 6 15:52:57 PST 2003
//
// Modifications:
//   
// ****************************************************************************
void
avtLine2DColleague::Hide()
{
    SetVisible(!GetVisible());

    if(addedToRenderer)
        RemoveFromRenderer();
    else
        AddToRenderer();
}

// ****************************************************************************
// Method: avtLine2DColleague::ShouldBeAddedToRenderer
//
// Purpose: 
//   This method returns whether or not the text actor should be added to the
//   renderer.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 6 15:53:36 PST 2003
//
// Modifications:
//   
// ****************************************************************************
bool
avtLine2DColleague::ShouldBeAddedToRenderer() const
{
    return GetVisible() && mediator.HasPlots();
}

// ****************************************************************************
// Method: avtLine2DColleague::SetOptions
//
// Purpose: 
//   This method sets the text actor's properties from the values in the
//   annotation object.
//
// Programmer: John C. Anderson
// Creation:   Fri Sep 03 09:03:25 PDT 2004
//
// Modifications:
//    Brad Whitlock, Tue Aug 31 11:56:56 PDT 2004
//    I added code to set the object's visibility so objects that are not
//    visible remain invisible when restoring a session file.
//
//    Brad Whitlock, Tue Jun 28 15:38:30 PST 2005
//    I made it use updateArrows.
//
// ****************************************************************************

void
avtLine2DColleague::SetOptions(const AnnotationObject &annot)
{
    // Get the current options.
    AnnotationObject currentOptions;
    GetOptions(currentOptions);

    double *p0 = (double *) annot.GetPosition();
    double *p1 = (double *) annot.GetPosition2();

    lineData->GetPoints()->SetPoint(0, p0[0], p0[1], 0.);
    lineData->GetPoints()->SetPoint(1, p1[0], p1[1], 0.);

    actor->GetProperty()->SetLineWidth(annot.GetColor2().Red());

    ColorAttribute ca = annot.GetColor1();
    actor->GetProperty()->SetColor(double(ca.Red()) / 255.,
                                   double(ca.Green()) / 255.,
                                   double(ca.Blue()) / 255.);
    actor->GetProperty()->SetOpacity(double(ca.Alpha()) / 255.);

    beginArrowStyle = annot.GetColor2().Green();
    endArrowStyle = annot.GetColor2().Blue();

    updateArrows(beginArrowLine, endArrowLine, p0, p1);
    updateArrows(beginArrowSolid, endArrowSolid, p0, p1);

    //
    // Aggregate the data if need be.
    //
 
    // Delete the polydata aggregator.
    if(allData)
    {
        allData->Delete();
        allData = NULL;
    }

    if(beginArrowStyle)
        allData = vtkAppendPolyData::New();

    if(beginArrowStyle == 1)
        allData->AddInput(beginArrowLine);
    else if(beginArrowStyle == 2)
        allData->AddInput(beginArrowSolid);

    if(endArrowStyle && !allData)
        allData = vtkAppendPolyData::New();

    if(endArrowStyle == 1)
        allData->AddInput(endArrowLine);
    else if(endArrowStyle == 2)
        allData->AddInput(endArrowSolid);

    //
    // Set the mapper.
    //
    if(allData)
    {
        allData->AddInput(lineData);
        mapper->SetInput(allData->GetOutput());
    }
    else mapper->SetInput(lineData);

    //
    // Set the object's visibility.
    //
    if(currentOptions.GetVisible() != annot.GetVisible())
    {
        SetVisible(annot.GetVisible());
        if(annot.GetVisible())
            AddToRenderer();
        else
            RemoveFromRenderer();
    }
}

// ****************************************************************************
// Method: avtLine2DColleague::GetOptions
//
// Purpose: 
//   This method stores the text label's attributes in an object that can
//   be passed back to the client.
//
// Arguments:
//   annot : The AnnotationObject to populate.
//
// Programmer: John C. Anderson
// Creation:   Fri Sep 03 09:03:39 PDT 2004
//
// Modifications:
//   
// ****************************************************************************
void
avtLine2DColleague::GetOptions(AnnotationObject &annot)
{
    annot.SetObjectType(AnnotationObject::Line2D);
    annot.SetVisible(GetVisible());
    annot.SetActive(GetActive());

    annot.SetPosition(lineData->GetPoints()->GetPoint(0));
    annot.SetPosition2(lineData->GetPoints()->GetPoint(1));

    double *currColor = actor->GetProperty()->GetColor();

    ColorAttribute ca;
    ca.SetRgba(int((float)currColor[0] * 255),
               int((float)currColor[1] * 255),
               int((float)currColor[2] * 255),
               int((float)actor->GetProperty()->GetOpacity() * 255));
    annot.SetColor1(ca);
    ca.SetRgb((int) actor->GetProperty()->GetLineWidth(),
              beginArrowStyle,
              endArrowStyle);
    annot.SetColor2(ca);
}

// ****************************************************************************
// Method: avtLine2DColleague::HasPlots
//
// Purpose: 
//   This method is called when the vis window gets some plots. We use this
//   signal to add the text actor to the renderer.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 6 15:56:06 PST 2003
//
// Modifications:
//   
// ****************************************************************************
void
avtLine2DColleague::HasPlots(void)
{
    AddToRenderer();
}

// ****************************************************************************
// Method: avtLine2DColleague::NoPlots
//
// Purpose: 
//   This method is called when the vis window has no plots. We use this signal
//   to remove the text actor from the renderer.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 6 15:56:42 PST 2003
//
// Modifications:
//   
// ****************************************************************************
void
avtLine2DColleague::NoPlots(void)
{
    RemoveFromRenderer();
}
