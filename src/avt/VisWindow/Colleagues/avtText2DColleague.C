// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtText2DColleague.C                         //
// ************************************************************************* //
#include <avtText2DColleague.h>
#include <VisWindow.h>
#include <VisWindowColleagueProxy.h>

#include <AnnotationObject.h>

#include <vtkRenderer.h>
#include <vtkVisItTextActor.h>
#include <vtkTextProperty.h>

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
//    Jeremy Meredith, Wed Mar 11 12:33:20 EDT 2009
//    Added $cycle support.
//
//    Tom Fogal, Fri Jan 28 15:26:59 MST 2011
//    VTK API change.
//   
// ****************************************************************************

avtText2DColleague::avtText2DColleague(VisWindowColleagueProxy &m) 
    : avtAnnotationWithTextColleague(m)
{
    useForegroundForTextColor = true;
    addedToRenderer = false;

    //
    // Create and position the actor.
    //
    textActor = vtkVisItTextActor::New();
    textActor->SetTextScaleMode(vtkTextActor::TEXT_SCALE_MODE_VIEWPORT);
    textActor->SetTextHeight(0.03);
    SetText("2D text annotation");
    vtkCoordinate *pos = textActor->GetPositionCoordinate();
    pos->SetCoordinateSystemToNormalizedViewport();
    pos->SetValue(0.5, 0.5, 0.);

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
//    Burlen Loring, Mon Jul 14 14:04:31 PDT 2014
//    fix alloc-dealloc-mismatch (operator new [] vs operator delete)
//
// ****************************************************************************

avtText2DColleague::~avtText2DColleague()
{
    if (textActor)
        textActor->Delete();
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
//    Brad Whitlock, Thu Mar 22 15:02:23 PST 2007
//    Changed FieldsEqual due to state object changes.
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
    if(!currentOptions.FieldsEqual(4, &annot) ||
       !currentOptions.FieldsEqual(5, &annot) || textChanged)
    {
        const double *p1 = annot.GetPosition();
        const double *p2 = annot.GetPosition2();
        vtkCoordinate *pos = textActor->GetPositionCoordinate();
        pos->SetCoordinateSystemToNormalizedViewport();
        pos->SetValue(p1[0], p1[1], 0.);
        textActor->SetTextHeight(p2[0]);
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
    p2wh[0] = textActor->GetTextHeight();
    p2wh[1] = 0.;
    p2wh[2] = 0.;
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
//    Jeremy Meredith, Wed Mar 11 12:33:20 EDT 2009
//    Added $cycle support.
//
//    Brad Whitlock, Mon Dec 10 11:41:13 PST 2012
//    Set initial values from the current values.
//
// ****************************************************************************

void
avtText2DColleague::UpdatePlotList(std::vector<avtActor_p> &lst)
{
    if (lst.size() > 0 && textFormatString != 0)
    {
        avtAnnotationWithTextColleague::UpdatePlotList(lst);
        SetText(textFormatString);
    }
}

void
avtText2DColleague::SetText(const char *formatString)
{
    if(formatString == 0)
        return;

    // Save the format string. Don't do it in the case that the formatString
    // pointer is the same as textFormatString, which is how we get here from
    // UpdatePlotList.
    size_t len = strlen(formatString);
    if (textFormatString != formatString)
    {
        delete [] textFormatString;
        textFormatString = new char[len + 1];
        strcpy(textFormatString, formatString);
    }

    delete [] textString;
    textString = CreateAnnotationString(textFormatString);

    if (textActor)
        textActor->SetInput(textString);
}
