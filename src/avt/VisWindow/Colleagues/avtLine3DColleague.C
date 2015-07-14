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
//                              avtLine3DColleague.C                         //
// ************************************************************************* //
#include <avtLine3DColleague.h>

#include <math.h>

#include <snprintf.h>

#include <vtkActor.h>
#include <vtkLineSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>

#include <AnnotationObject.h>
#include <LineAttributes.h>
#include <VisWindow.h>
#include <VisWindowColleagueProxy.h>

#include <avtVector.h>


// ****************************************************************************
// Method: avtLine3DColleague::avtLine3DColleague
//
// Purpose: 
//   Constructor for the avtLine3DColleague class.
//
// Arguments:
//   m : The vis window proxy.
//
// Programmer: Kathleen Biagas 
// Creation:   July 13, 2015 
//
// Modifications:
//
// ****************************************************************************

avtLine3DColleague::avtLine3DColleague(VisWindowColleagueProxy &m):
    avtAnnotationColleague(m)
{
    addedToRenderer = false;
    useForegroundForLineColor = true;

    lineSource = vtkLineSource::New();
    lineSource->SetResolution(1);
    lineSource->SetPoint1(0, 0, 0);
    lineSource->SetPoint2(1, 1, 0);

    lineMapper  = vtkPolyDataMapper::New();
    lineMapper->SetInputConnection(lineSource->GetOutputPort());
    lineActor   = vtkActor::New();
    lineActor->SetMapper(lineMapper); 
    lineActor->PickableOff(); 

    // Set a default color.
    double fgColor[3];
    mediator.GetForegroundColor(fgColor);
    SetForegroundColor(fgColor[0], fgColor[1], fgColor[2]);

    // Set default opacity.
    lineActor->GetProperty()->SetOpacity(1.);

    // Set a default line width.
    lineActor->GetProperty()->SetLineWidth(1);
    lineActor->GetProperty()->SetLineStipplePattern(0xFFFF);
}


// ****************************************************************************
// Method: avtLine3DColleague::~avtLine3DColleague
//
// Purpose: 
//   Destructor for the avtLine3DColleague class.
//
// Programmer: Kathleen Biagas 
// Creation:   July 13, 2015
//
// Modifications:
//   
// ****************************************************************************

avtLine3DColleague::~avtLine3DColleague()
{
    if(lineActor != NULL)
    {
        lineActor->Delete();
        lineActor = NULL;
    }

    if(lineMapper != NULL)
    {
        lineMapper->Delete();
        lineMapper = NULL;
    }

    if(lineSource != NULL)
    {
        lineSource->Delete();
        lineSource = NULL;
    }
}


// ****************************************************************************
// Method: avtLine3DColleague::AddToRenderer
//
// Purpose: 
//   This method adds the lineActor to the renderer.
//
// Programmer: Kathleen Biagas 
// Creation:   July 13, 2015 
//
// Modifications:
//   
// ****************************************************************************

void 
avtLine3DColleague::AddToRenderer()
{
    if(!addedToRenderer && ShouldBeAddedToRenderer())
    {
        mediator.GetCanvas()->AddActor(lineActor);
        addedToRenderer = true;
    }
}


// ****************************************************************************
// Method: avtLine3DColleague::RemoveFromRenderer
//
// Purpose: 
//   This method removes the lineActor from the renderer.
//
// Programmer: Kathleen Biagas 
// Creation:   July 13, 2015 
//
// Modifications:
//   
// ****************************************************************************

void
avtLine3DColleague::RemoveFromRenderer()
{
    if(addedToRenderer)
    {
        mediator.GetCanvas()->RemoveActor(lineActor);
        addedToRenderer = false;
    }
}


// ****************************************************************************
// Method: avtLine3DColleague::Hide
//
// Purpose: 
//   This method toggles the visible flag and either adds or removes the
//   lineActor to/from the renderer.
//
// Programmer: Kathleen Biagas 
// Creation:   July 13, 2015 
//
// Modifications:
//   
// ****************************************************************************

void
avtLine3DColleague::Hide()
{
    SetVisible(!GetVisible());

    if(addedToRenderer)
        RemoveFromRenderer();
    else
        AddToRenderer();
}


// ****************************************************************************
// Method: avtLine3DColleague::ShouldBeAddedToRenderer
//
// Purpose: 
//   This method returns whether or not the lineActor should be added to the
//   renderer.
//
// Programmer: Kathleen Biagas 
// Creation:   July 13, 2015 
//
// Modifications:
//   
// ****************************************************************************

bool
avtLine3DColleague::ShouldBeAddedToRenderer() const
{
    return GetVisible() && mediator.HasPlots();
}


// ****************************************************************************
// Method: avtLine3DColleague::SetOptions
//
// Purpose: 
//   This method sets the lineActor's properties from the values in the
//   annotation object.
//
// Programmer: Kathleen Biagas 
// Creation:   July 13, 2015 
//
// Modifications:
//
// ****************************************************************************

void
avtLine3DColleague::SetOptions(const AnnotationObject &annot)
{
    // Get the current options.
    AnnotationObject currentOptions;
    GetOptions(currentOptions);

    lineSource->SetPoint1(const_cast<double*>(annot.GetPosition()));
    lineSource->SetPoint2(const_cast<double*>(annot.GetPosition2()));

    lineActor->GetProperty()->SetLineWidth(annot.GetIntAttribute1()+1);
    lineActor->GetProperty()->SetLineStipplePattern(LineStyle2StipplePattern(
        Int2LineStyle(annot.GetIntAttribute2())));

    //
    // The line color has changed or the useForegroundForTextColor flag
    // has changed.
    //
    if(annot.GetUseForegroundForTextColor() != useForegroundForLineColor ||
       annot.GetColor1() != lineColor)
    {
        // Record the line color that should be used when we're not using
        // the foreground text color.
        lineColor = annot.GetColor1();
        useForegroundForLineColor = annot.GetUseForegroundForTextColor();

        // Compute the line opacity.
        double lc[4];
        lc[3] = double(lineColor.Alpha()) / 255.;

        // Set the line color using the foreground color or the line color.
        if(useForegroundForLineColor)
        {
            // Get the foreground color.
            double fgColor[3];
            mediator.GetForegroundColor(fgColor);
            lineActor->GetProperty()->SetColor(fgColor[0], fgColor[1], fgColor[2]);
        }
        else
        {
            // Compute the line color as double.
            lc[0] = double(lineColor.Red()) / 255.;
            lc[1] = double(lineColor.Green()) / 255.;
            lc[2] = double(lineColor.Blue()) / 255.;
            lineActor->GetProperty()->SetColor(lc[0], lc[1], lc[2]);
        }

        lineActor->GetProperty()->SetOpacity(double(lc[3]));
    }

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
// Method: avtLine3DColleague::GetOptions
//
// Purpose: 
//   This method stores the attributes in an object that can
//   be passed back to the client.
//
// Arguments:
//   annot : The AnnotationObject to populate.
//
// Programmer: Kathleen Biagas 
// Creation:   July 13, 2015
//
// Modifications:
//   
// ****************************************************************************

void
avtLine3DColleague::GetOptions(AnnotationObject &annot)
{
    annot.SetObjectType(AnnotationObject::Line3D);
    annot.SetVisible(GetVisible());
    annot.SetActive(GetActive());

    annot.SetPosition(lineSource->GetPoint1());
    annot.SetPosition2(lineSource->GetPoint2());

    annot.SetColor1(lineColor);
    annot.SetUseForegroundForTextColor(useForegroundForLineColor);
    annot.SetIntAttribute1(lineActor->GetProperty()->GetLineWidth()-1);
    annot.SetIntAttribute2(LineStyle2Int(StipplePattern2LineStyle(
        lineActor->GetProperty()->GetLineStipplePattern())));
}


// ****************************************************************************
// Method: avtLine3DColleague::HasPlots
//
// Purpose: 
//   This method is called when the vis window gets some plots. We use this
//   signal to add the lineActor to the renderer.
//
// Programmer: Kathleen Biagas 
// Creation:   July 13, 2015
//
// Modifications:
//   
// ****************************************************************************

void
avtLine3DColleague::HasPlots(void)
{
    AddToRenderer();
}


// ****************************************************************************
// Method: avtLine3DColleague::NoPlots
//
// Purpose: 
//   This method is called when the vis window has no plots. We use this signal
//   to remove the lineActor from the renderer.
//
// Programmer: Kathleen Biagas 
// Creation:   July 13, 2015 
//
// Modifications:
//   
// ****************************************************************************

void
avtLine3DColleague::NoPlots(void)
{
    RemoveFromRenderer();
}


// ****************************************************************************
// Method: avtLine3DColleague::SetForegroundColor
//
// Purpose: 
//   This method is called when the vis window's foreground color changes.
//
// Arguments:
//   r,g,b : The new foreground color.
//
// Programmer: Kathleen Biagas 
// Creation:   July 13, 2015 
//
// Modifications:
//
// ****************************************************************************

void
avtLine3DColleague::SetForegroundColor(double r, double g, double b)
{
    if(useForegroundForLineColor)
        lineActor->GetProperty()->SetColor(r, g, b);
}


