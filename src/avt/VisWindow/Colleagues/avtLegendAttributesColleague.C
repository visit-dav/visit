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
//                              avtLegendAttributesColleague.C               //
// ************************************************************************* //
#include <avtLegendAttributesColleague.h>
#include <VisWindow.h>
#include <VisWindowColleagueProxy.h>

#include <AnnotationObject.h>
#include <snprintf.h>

#include <DebugStream.h>
#include <vtkSystemIncludes.h>

#define LEGEND_MANAGE_POSITION 0
#define LEGEND_DRAW_BOX        1
#define LEGEND_DRAW_LABELS     2
#define LEGEND_ORIENTATION0    3
#define LEGEND_ORIENTATION1    4
#define LEGEND_DRAW_TITLE      5

//
// Helper functions to get bits out of the annotation attributes.
//

bool
GetBool(const AnnotationObject &annot, int bit)
{
    return (annot.GetIntAttribute1() & (1 << bit)) != 0;
}

void
SetBool(AnnotationObject &annot, int bit, bool val)
{
    int shifted = (1 << bit);
    int mask = ~shifted;
    int data = annot.GetIntAttribute1();
    annot.SetIntAttribute1((data & mask) | (val ? shifted : 0));
}

// ****************************************************************************
// Method: avtLegendAttributesColleague::avtLegendAttributesColleague
//
// Purpose: 
//   Constructor for the avtLegendAttributesColleague class.
//
// Arguments:
//   m : The vis window proxy.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 20 10:57:05 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

avtLegendAttributesColleague::avtLegendAttributesColleague(VisWindowColleagueProxy &m) 
    : avtAnnotationColleague(m), atts()
{
    // Populate atts with some legend defaults.
    SetBool(atts, LEGEND_MANAGE_POSITION, true);
    SetBool(atts, LEGEND_DRAW_BOX,        false);
    SetBool(atts, LEGEND_DRAW_LABELS,     true);
    SetBool(atts, LEGEND_ORIENTATION0,    false);
    SetBool(atts, LEGEND_ORIENTATION1,    false);
    SetBool(atts, LEGEND_DRAW_TITLE,      true);

    // Set the format string for the legend into the text.
    stringVector text;
    text.push_back("%# -9.4g");
    atts.SetText(text);

    // Set the default position.
    const double defaultPosition[2] = {0.05, 0.9};
    atts.SetPosition(defaultPosition);

    // Set the default scale.
    const double defaultScale[2] = {1.,1.};
    atts.SetPosition2(defaultScale);

    // Set the default font height.
    atts.SetDoubleAttribute1(0.015);

    // Set the default bounding box color.
    atts.SetColor1(ColorAttribute(0,0,0,50));

    // Set the default font properties.
    atts.SetFontFamily(AnnotationObject::Arial);
    atts.SetFontBold(false);
    atts.SetFontItalic(false);
    atts.SetFontShadow(false);
}

// ****************************************************************************
// Method: avtLegendAttributesColleague::~avtLegendAttributesColleague
//
// Purpose: 
//   Destructor for the avtLegendAttributesColleague class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 20 10:57:05 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

avtLegendAttributesColleague::~avtLegendAttributesColleague()
{
}

// ****************************************************************************
// Method: avtLegendAttributesColleague::AddToRenderer
//
// Purpose: 
//   This method adds the text actor to the renderer.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 20 10:57:05 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void 
avtLegendAttributesColleague::AddToRenderer()
{
    // Legends are not added to the renderer via this object.
}

// ****************************************************************************
// Method: avtLegendAttributesColleague::RemoveFromRenderer
//
// Purpose: 
//   This method removes the text actor from the renderer.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 20 10:57:05 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtLegendAttributesColleague::RemoveFromRenderer()
{
    // Legends are not added to the renderer via this object.
}

// ****************************************************************************
// Method: avtLegendAttributesColleague::Hide
//
// Purpose: 
//   This method toggles the visible flag and either adds or removes the text
//   actor to/from the renderer.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 20 10:57:05 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtLegendAttributesColleague::Hide()
{
    SetVisible(!GetVisible());
}

// ****************************************************************************
// Method: avtLegendAttributesColleague::SetOptions
//
// Purpose: 
//   This method sets the text actor's properties from the values in the
//   annotation object.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 20 10:57:05 PDT 2007
//
// Modifications:
//
// ****************************************************************************

void
avtLegendAttributesColleague::SetOptions(const AnnotationObject &annot)
{
    //
    // Set the object's visibility.
    //
    if(atts.GetVisible() != annot.GetVisible())
        SetVisible(annot.GetVisible());

    atts = annot;
}

// ****************************************************************************
// Method: avtLegendAttributesColleague::GetOptions
//
// Purpose: 
//   This method stores the text label's attributes in an object that can
//   be passed back to the client.
//
// Arguments:
//   annot : The AnnotationObject to populate.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 20 10:57:05 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtLegendAttributesColleague::GetOptions(AnnotationObject &annot)
{
    annot = atts;

    // Make sure that the name is set properly.
    annot.SetObjectType(AnnotationObject::LegendAttributes);
    annot.SetVisible(GetVisible());
    annot.SetActive(GetActive());
}

// ****************************************************************************
// Method: avtLegendAttributesColleague::UpdatePlotList
//
// Purpose: 
//   This method is called when the plot list changes. Its job is to make sure
//   that the time slider always shows the right time.
//
// Arguments:
//   lst : The plot list.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 3 12:46:37 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
avtLegendAttributesColleague::UpdatePlotList(std::vector<avtActor_p> &lst)
{
    // Could always make some of the property setting happen here. Maybe even
    // cache pointer to the legend.
}

// ****************************************************************************
// Method: avtLegendAttributesColleague::ManageLayout
//
// Purpose: 
//   Sets the legend scale and returns whether the legend's layout is managed
//   by VisWinAnnotations.
//
// Arguments:
//   legend : The legend in question.
//
// Returns:    True if the legend's position will be managed.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 02:12:54 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

bool
avtLegendAttributesColleague::ManageLayout(avtLegend_p legend) const
{
    // Set the legend scale so we can manage its layout properly.
    legend->SetLegendScale(atts.GetPosition2()[0],atts.GetPosition2()[1]);

    return GetBool(atts, LEGEND_MANAGE_POSITION);
}

// ****************************************************************************
// Method: avtLegendAttributesColleague::CustomizeLegend
//
// Purpose: 
//   This method customizes the legend properties.
//
// Arguments:
//   legend : The legend to be customized.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 02:13:46 PDT 2007
//
// Modifications:
//   Brad Whitlock, Mon Mar 26 13:47:55 PST 2007
//   Added title visibility.
//
//   Dave Bremer, Wed Oct 15 16:37:37 PDT 2008
//   I set the orientation first now, because it will affect the 
//   call to legend->GetLegendSize()
// ****************************************************************************

void
avtLegendAttributesColleague::CustomizeLegend(avtLegend_p legend)
{
    const char *mName = "avtLegendAttributesColleague::CustomizeLegend: ";

    // Set the legend orientation.
    bool b0 = GetBool(atts,LEGEND_ORIENTATION0);
    bool b1 = GetBool(atts,LEGEND_ORIENTATION1);
    avtLegend::LegendOrientation orientation;
    if(b0 == false)
    {
        // Vertical
        if(b1 == false)
            orientation = avtLegend::VerticalTextOnRight;
        else
            orientation = avtLegend::VerticalTextOnLeft;
    }
    else
    {
        // Horizontal
        if(b1 == false)
            orientation = avtLegend::HorizontalTextOnTop;
        else
            orientation = avtLegend::HorizontalTextOnBottom;
    }
    legend->SetOrientation(orientation);

    // If the layout for the legend is not being managed then set the legend
    // position here so we can position it where we want.
    if(!GetBool(atts, LEGEND_MANAGE_POSITION))
    {
        double xLeft = atts.GetPosition()[0];
        double yTop  = atts.GetPosition()[1];
        double width, height;
        legend->GetLegendSize(yTop, width, height);
        legend->SetLegendPosition(xLeft, yTop - height);
    }

    // Set the legend's foreground color.
    double textColor[3];
    if(atts.GetUseForegroundForTextColor())
    {
        mediator.GetForegroundColor(textColor);
    }
    else
    {
        textColor[0] = float(atts.GetTextColor().Red()) / 255.f;
        textColor[1] = float(atts.GetTextColor().Green()) / 255.f;
        textColor[2] = float(atts.GetTextColor().Blue()) / 255.f;
    }
    legend->SetForegroundColor(textColor);

    // Set the legend font height.
    legend->SetFontHeight(atts.GetDoubleAttribute1());

    // Set whether the title is drawn.
    legend->SetTitleVisibility(GetBool(atts, LEGEND_DRAW_TITLE));

    // Set whether the labels are drawn.
    legend->SetLabelVisibility(GetBool(atts, LEGEND_DRAW_LABELS));

    // Set the format string used for numbers.
    if(atts.GetText().size() > 0)
        legend->SetNumberFormat(atts.GetText()[0].c_str());

    // Set bounding box properties.
    legend->SetBoundingBoxVisibility(GetBool(atts,LEGEND_DRAW_BOX));
    double bboxColor[4];
    bboxColor[0] = double(atts.GetColor1().Red()) / 255.;
    bboxColor[1] = double(atts.GetColor1().Green()) / 255.;
    bboxColor[2] = double(atts.GetColor1().Blue()) / 255.;
    bboxColor[3] = double(atts.GetColor1().Alpha()) / 255.;
    legend->SetBoundingBoxColor(bboxColor);

    // Set the font properties.
    int family;
    if(atts.GetFontFamily() == AnnotationObject::Arial)
        family = VTK_ARIAL;
    else if(atts.GetFontFamily() == AnnotationObject::Courier)
        family = VTK_COURIER;
    else
        family = VTK_TIMES;
    legend->SetFont(family, atts.GetFontBold(), atts.GetFontItalic(),
        atts.GetFontShadow());

    legend->Update();
}

