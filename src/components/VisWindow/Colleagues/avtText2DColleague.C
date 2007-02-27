/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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

// ************************************************************************* //
//                              avtText2DColleague.C                         //
// ************************************************************************* //
#include <avtText2DColleague.h>
#include <VisWindow.h>
#include <VisWindowColleagueProxy.h>

#include <AnnotationObject.h>
#include <snprintf.h>

#include <vtkRenderer.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>

#define TIME_IDENTIFIER "$time"

// ****************************************************************************
// Method: avtText2DColleague::avtText2DColleague
//
// Purpose: 
//   Constructor for the avtText2DColleague class.
//
// Arguments:
//   m : The vis window proxy.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 14:13:14 PST 2003
//
// Modifications:
//   
// ****************************************************************************

avtText2DColleague::avtText2DColleague(VisWindowColleagueProxy &m) 
    : avtAnnotationColleague(m)
{
    useForegroundForTextColor = true;
    addedToRenderer = false;
    textFormatString = 0;
    textString = 0;
    currentTime = 0.;

    //
    // Create and position the actor.
    //
    textActor = vtkTextActor::New();
    textActor->ScaledTextOn();
    SetText("2D text annotation");
    vtkCoordinate *pos = textActor->GetPositionCoordinate();
    pos->SetCoordinateSystemToNormalizedViewport();
    pos->SetValue(0.5, 0.5, 0.);
    textActor->SetWidth(0.25);
    textActor->SetHeight(0.1);

    // Make sure that the actor initially has the right fg color.
    double fgColor[3];
    mediator.GetForegroundColor(fgColor);
    SetForegroundColor(fgColor[0], fgColor[1], fgColor[2]);
    textActor->GetTextProperty()->SetOpacity(1.);

    // Store the foreground color into the text color.
    int ifgColor[3];
    ifgColor[0] = int((float)fgColor[0] * 255.f);
    ifgColor[1] = int((float)fgColor[1] * 255.f);
    ifgColor[2] = int((float)fgColor[2] * 255.f);
    textColor = ColorAttribute(ifgColor[0], ifgColor[1], ifgColor[2], 255);
}

// ****************************************************************************
// Method: avtText2DColleague::~avtText2DColleague
//
// Purpose: 
//   Destructor for the avtText2DColleague class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 14:18:31 PST 2003
//
// Modifications:
//   
// ****************************************************************************

avtText2DColleague::~avtText2DColleague()
{
    if (textActor != NULL)
    {
        textActor->Delete();
        textActor = NULL;
    }

    if (textString != NULL)
    {
        delete [] textString;
        textString = NULL;
    }

    if(textFormatString != NULL)
    {
        delete textFormatString;
        textFormatString = NULL;
    }
}

// ****************************************************************************
// Method: avtText2DColleague::AddToRenderer
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
avtText2DColleague::AddToRenderer()
{
    if(!addedToRenderer && ShouldBeAddedToRenderer())
    {
        mediator.GetForeground()->AddActor2D(textActor);
        addedToRenderer = true;
    }
}

// ****************************************************************************
// Method: avtText2DColleague::RemoveFromRenderer
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
avtText2DColleague::RemoveFromRenderer()
{
    if(addedToRenderer)
    {
        mediator.GetForeground()->RemoveActor2D(textActor);
        addedToRenderer = false;
    }
}

// ****************************************************************************
// Method: avtText2DColleague::Hide
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
avtText2DColleague::Hide()
{
    SetVisible(!GetVisible());

    if(addedToRenderer)
        RemoveFromRenderer();
    else
        AddToRenderer();
}

// ****************************************************************************
// Method: avtText2DColleague::ShouldBeAddedToRenderer
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
avtText2DColleague::ShouldBeAddedToRenderer() const
{
    return GetVisible() && mediator.HasPlots();
}

// ****************************************************************************
// Method: avtText2DColleague::SetOptions
//
// Purpose: 
//   This method sets the text actor's properties from the values in the
//   annotation object.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 6 15:54:02 PST 2003
//
// Modifications:
//    Brad Whitlock, Tue Aug 31 11:56:56 PDT 2004
//    I added code to set the object's visibility so objects that are not
//    visible remain invisible when restoring a session file.
//
// ****************************************************************************

void
avtText2DColleague::SetOptions(const AnnotationObject &annot)
{
    // Get the current options.
    AnnotationObject currentOptions;
    GetOptions(currentOptions);

    //
    // The text color has changed or the useForegroundForTextColor flag
    // has changed.
    //
    if(annot.GetUseForegroundForTextColor() != useForegroundForTextColor ||
       annot.GetTextColor() != textColor)
    {
        // Record the text color that should be used when we're not using
        // the foreground text color.
        textColor = annot.GetTextColor();
        useForegroundForTextColor = annot.GetUseForegroundForTextColor();

        // Compute the text opacity.
        double tc[4];
        tc[3] = double(textColor.Alpha()) / 255.;

        // Set the text color using the foreground color or the text color.
        if(useForegroundForTextColor)
        {
            // Get the foreground color.
            double fgColor[3];
            mediator.GetForegroundColor(fgColor);
            textActor->GetTextProperty()->SetColor(fgColor[0], fgColor[1], fgColor[2]);
        }
        else
        {
            // Compute the text color as double.
            tc[0] = double(textColor.Red()) / 255.;
            tc[1] = double(textColor.Green()) / 255.;
            tc[2] = double(textColor.Blue()) / 255.;
            textActor->GetTextProperty()->SetColor(tc[0], tc[1], tc[2]);
        }

        // Set the text opacity.
        textActor->GetTextProperty()->SetOpacity(tc[3]);
    }

    //
    // Set the labels if the text vector is different
    //
    bool textChanged = false;
    if(currentOptions.GetText() != annot.GetText())
    {
        const stringVector &text = annot.GetText();
        if(text.size() > 0)
            SetText(text[0].c_str());
        else
            SetText("");
        // Set the width to a size we don't want so we can update it later.
        textActor->SetWidth(0.1);
        textActor->SetHeight(0.1);
        textChanged = true;
    }

    //
    // Set the font properties if they are different.
    //
    if(currentOptions.GetFontFamily() != annot.GetFontFamily())
    {
        int ff = annot.GetFontFamily();
        if(ff == 0)
            textActor->GetTextProperty()->SetFontFamilyToArial();
        else if(ff == 1)
            textActor->GetTextProperty()->SetFontFamilyToCourier();
        else if(ff == 2)
            textActor->GetTextProperty()->SetFontFamilyToTimes();
    }
    if(currentOptions.GetFontBold() != annot.GetFontBold())
        textActor->GetTextProperty()->SetBold(annot.GetFontBold()?1:0);
    if(currentOptions.GetFontItalic() != annot.GetFontItalic())
        textActor->GetTextProperty()->SetItalic(annot.GetFontItalic()?1:0);
    if(currentOptions.GetFontShadow() != annot.GetFontShadow())
        textActor->GetTextProperty()->SetShadow(annot.GetFontShadow()?1:0);

    //
    // Set the position coordinates if they are different
    //
    if(!currentOptions.FieldsEqual(3, &annot) ||
       !currentOptions.FieldsEqual(4, &annot) || textChanged)
    {
        const double *p1 = annot.GetPosition();
        const double *p2 = annot.GetPosition2();
        vtkCoordinate *pos = textActor->GetPositionCoordinate();
        pos->SetCoordinateSystemToNormalizedViewport();
        pos->SetValue(p1[0], p1[1], 0.);
        textActor->SetWidth(p2[0]);
        textActor->SetHeight(p2[0]);
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
// Method: avtText2DColleague::GetOptions
//
// Purpose: 
//   This method stores the text label's attributes in an object that can
//   be passed back to the client.
//
// Arguments:
//   annot : The AnnotationObject to populate.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 30 14:13:21 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
avtText2DColleague::GetOptions(AnnotationObject &annot)
{
    annot.SetObjectType(AnnotationObject::Text2D);
    annot.SetVisible(GetVisible());
    annot.SetActive(GetActive());

    annot.SetPosition(textActor->GetPosition());
    // Store the width and height in position2.
    double p2wh[3];
    p2wh[0] = textActor->GetWidth();
    p2wh[1] = textActor->GetHeight();
    p2wh[2] = 0.f;
    annot.SetPosition2(p2wh);

    // Store the text color and opacity.
    annot.SetTextColor(textColor);
    annot.SetUseForegroundForTextColor(useForegroundForTextColor);

    // Store the font properties into the annotation object.
    int ff = textActor->GetTextProperty()->GetFontFamily();
    AnnotationObject::FontFamily aff;
    if(ff == VTK_ARIAL)
        aff = AnnotationObject::Arial;
    else if(ff == VTK_COURIER)
        aff = AnnotationObject::Courier;
    else if(ff == VTK_TIMES)
        aff = AnnotationObject::Times;
    else
        aff = AnnotationObject::Arial;
    annot.SetFontFamily(aff);
    annot.SetFontBold(textActor->GetTextProperty()->GetBold() > 0);
    annot.SetFontItalic(textActor->GetTextProperty()->GetItalic() > 0);
    annot.SetFontShadow(textActor->GetTextProperty()->GetShadow() > 0);

    stringVector text;
    text.push_back(textFormatString);
    annot.SetText(text);
}

// ****************************************************************************
// Method: avtText2DColleague::SetForegroundColor
//
// Purpose: 
//   This method is called when the vis window's foreground color changes.
//
// Arguments:
//   r,g,b : The new foreground color.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 14:18:20 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
avtText2DColleague::SetForegroundColor(double r, double g, double b)
{
    if(useForegroundForTextColor)
        textActor->GetTextProperty()->SetColor(r, g, b);
}

// ****************************************************************************
// Method: avtText2DColleague::HasPlots
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
avtText2DColleague::HasPlots(void)
{
    AddToRenderer();
}

// ****************************************************************************
// Method: avtText2DColleague::NoPlots
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
avtText2DColleague::NoPlots(void)
{
    RemoveFromRenderer();
}

// ****************************************************************************
// Method: avtText2DColleague::UpdatePlotList
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
avtText2DColleague::UpdatePlotList(std::vector<avtActor_p> &lst)
{
    if(lst.size() > 0 && textFormatString != 0)
    {
        avtDataAttributes &atts = lst[0]->GetBehavior()->GetInfo().GetAttributes();
        currentTime = atts.GetTime();

        std::string formatString(textFormatString);
        std::string::size_type pos;
        if((pos = formatString.find(TIME_IDENTIFIER)) != std::string::npos)
            SetText(textFormatString);
    }
}

// ****************************************************************************
// Method: avtText2DColleague::SetText
//
// Purpose: 
//   Updates the text string.
//
// Note:       This code matches avtTimeSliderColleague::SetText.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 14:17:22 PST 2003
//
// Modifications:
//
// ****************************************************************************

void
avtText2DColleague::SetText(const char *formatString)
{
    if(formatString == 0)
        return;

    // Save the format string. Don't do it in the case that the formatString
    // pointer is the same as textFormatString, which is how we get here from
    // UpdatePlotList.
    int len = strlen(formatString);
    if(textFormatString != formatString)
    {
        delete [] textFormatString;
        textFormatString = new char[len + 1];
        strcpy(textFormatString, formatString);
    }

    // Replace $time with the time if the format string contains $time.
    delete [] textString;
    std::string fmtStr(textFormatString);
    std::string::size_type pos = fmtStr.find(TIME_IDENTIFIER);
    if(pos != std::string::npos)
    {
        int tlen = strlen(TIME_IDENTIFIER);
        std::string left(fmtStr.substr(0, pos));
        std::string right(fmtStr.substr(pos + tlen, fmtStr.size() - pos - tlen));
        char tmp[100];
        SNPRINTF(tmp, 100, "%g", currentTime);
        len = left.size() + strlen(tmp) + right.size() + 1;
        textString = new char[len];
        SNPRINTF(textString, len, "%s%s%s", left.c_str(), tmp, right.c_str());
    }
    else
    {
        textString = new char[len + 1];
        strcpy(textString, formatString);
    }

    if(textActor)
        textActor->SetInput(textString);
}
