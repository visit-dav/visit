// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtText3DColleague.C                         //
// ************************************************************************* //
#include <avtText3DColleague.h>
#include <VisWindow.h>
#include <VisWindowColleagueProxy.h>

#include <AnnotationObject.h>

#include <vtkRenderer.h>
#include <vtkFollower.h>
#include <vtkLinearExtrusionFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkProperty.h>
#include <vtkVectorText.h>

#include <DebugStream.h>

// Macros that let us access the AnnotationObject and store the fields
// we care about into the available fields.
#define HasPreserveOrientation GetOptions().HasEntry("preserveOrientation")
#define GetPreserveOrientation GetOptions().GetEntry("preserveOrientation")->AsBool

#define GetRotations GetPosition2
#define SetRotations SetPosition2

// The flag that lets us switch between relative and fixed heights.
#define HasRelativeHeightMode GetOptions().HasEntry("useRelativeHeight")
#define GetRelativeHeightMode GetOptions().GetEntry("useRelativeHeight")->AsBool

// Relative scale will scale the text as a percentage of the bbox diagonal
#define HasRelativeHeight GetOptions().HasEntry("relativeHeight")
#define GetRelativeHeight GetOptions().GetEntry("relativeHeight")->AsInt

// Fixed height in world coordinates.
#define HasFixedHeight   GetOptions().HasEntry("fixedHeight")
#define GetFixedHeight   GetOptions().GetEntry("fixedHeight")->AsDouble

// ****************************************************************************
// Method: avtText3DColleague::avtText3DColleague
//
// Purpose: 
//   Constructor for the avtText3DColleague class.
//
// Arguments:
//   m : The vis window proxy.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 7 14:13:14 PST 2007
//
// Modifications:
//    Jeremy Meredith, Wed Mar 11 12:33:20 EDT 2009
//    Added $cycle support.
//   
// ****************************************************************************

avtText3DColleague::avtText3DColleague(VisWindowColleagueProxy &m) 
    : avtAnnotationWithTextColleague(m)
{
    info = new Text3DInformation;
    info->positionInitialized = false;
    info->scaleInitialized = false;

    info->useForegroundForTextColor = true;
    info->useRelativeHeight = true;
    info->relativeHeight = 3;
    info->fixedHeight = 1.;
    info->rotations[0] = info->rotations[1] = info->rotations[2] = 0.;
    info->addedToRenderer = false;

    // Set the initial text for the actor.
    info->textSource = vtkVectorText::New();
    SetText("3D text annotation");

    info->extrude = vtkLinearExtrusionFilter::New();
    info->extrude->SetInputConnection(info->textSource->GetOutputPort());
    info->extrude->SetExtrusionTypeToNormalExtrusion();
    info->extrude->SetVector(0., 0., 1.);
    info->extrude->SetScaleFactor(0.5);

    info->normals = vtkPolyDataNormals::New();
    info->normals->SetInputConnection(info->extrude->GetOutputPort());

    info->mapper = vtkPolyDataMapper::New();
    info->mapper->SetInputConnection(info->normals->GetOutputPort());

    info->textActor = vtkFollower::New();
    info->textActor->SetMapper(info->mapper);

    // Make sure that the actor initially has the right fg color.
    double fgColor[3];
    mediator.GetForegroundColor(fgColor);
    SetForegroundColor(fgColor[0], fgColor[1], fgColor[2]);
    info->textActor->GetProperty()->SetOpacity(1.);
    info->textActor->GetProperty()->SetInterpolationToPhong();

    // Store the foreground color into the text color.
    int ifgColor[3];
    ifgColor[0] = int((float)fgColor[0] * 255.f);
    ifgColor[1] = int((float)fgColor[1] * 255.f);
    ifgColor[2] = int((float)fgColor[2] * 255.f);
    info->textColor.SetRgb(ifgColor[0], ifgColor[1], ifgColor[2]);
}

// ****************************************************************************
// Method: avtText3DColleague::~avtText3DColleague
//
// Purpose: 
//   Destructor for the avtText3DColleague class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 7 14:18:31 PST 2007
//
// Modifications:
//   
// ****************************************************************************

avtText3DColleague::~avtText3DColleague()
{
    if (info->textActor != NULL)
    {
        info->textActor->Delete();
        info->textActor = NULL;
    }

    if (info->mapper != NULL)
    {
        info->mapper->Delete();
        info->mapper = NULL;
    } 

    if (info->normals != NULL)
    {
        info->normals->Delete();
        info->normals = NULL;
    }

    if (info->extrude != NULL)
    {
        info->extrude->Delete();
        info->extrude = NULL;
    } 

    if (info->textSource != NULL)
    {
        info->textSource->Delete();
        info->textSource = NULL;
    } 

    delete info;
}

// ****************************************************************************
// Method: avtText3DColleague::AddToRenderer
//
// Purpose: 
//   This method adds the text actor to the renderer.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 7 15:52:19 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void 
avtText3DColleague::AddToRenderer()
{
    if(!info->addedToRenderer && ShouldBeAddedToRenderer())
    {
        mediator.GetCanvas()->AddActor(info->textActor);
        info->addedToRenderer = true;
    }
}

// ****************************************************************************
// Method: avtText3DColleague::RemoveFromRenderer
//
// Purpose: 
//   This method removes the text actor from the renderer.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 7 15:52:38 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtText3DColleague::RemoveFromRenderer()
{
    if(info->addedToRenderer)
    {
        mediator.GetCanvas()->RemoveActor(info->textActor);
        info->addedToRenderer = false;
    }
}

// ****************************************************************************
// Method: avtText3DColleague:SetVisibility
//
// Purpose: 
//   Sets visibility of the tool. Use this if you need to temporarily
//   take the tool out of the scene during transparent rendering.
//
// Programmer: Burlen Loring 
// Creation:   Mon Sep 28 16:06:19 PDT 2015
//
// Modifications:
//
// ****************************************************************************

void
avtText3DColleague::SetVisibility(int val)
{
    if (GetActive() && GetVisible())
    {
        info->textActor->SetVisibility(val);
    }
}

// ****************************************************************************
// Method: avtText3DColleague::Hide
//
// Purpose: 
//   This method toggles the visible flag and either adds or removes the text
//   actor to/from the renderer.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 7 15:52:57 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtText3DColleague::Hide()
{
    SetVisible(!GetVisible());

    if(info->addedToRenderer)
        RemoveFromRenderer();
    else
        AddToRenderer();
}

// ****************************************************************************
// Method: avtText3DColleague::ShouldBeAddedToRenderer
//
// Purpose: 
//   This method returns whether or not the text actor should be added to the
//   renderer.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 7 15:53:36 PST 2007
//
// Modifications:
//   
// ****************************************************************************

bool
avtText3DColleague::ShouldBeAddedToRenderer() const
{
    return GetVisible() && mediator.HasPlots();
}

// ****************************************************************************
// Method: avtText3DColleague::UpdateActorScale
//
// Purpose: 
//   This method updates the actor's scale according to the preferred 
//   scaling method and the current plot size.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 7 15:53:36 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtText3DColleague::UpdateActorScale()
{
    const char *mName = "avtText3DColleague::UpdateActorScale: ";
    if(info->useRelativeHeight)
    {
        double bounds[6];
        mediator.GetBounds(bounds);
        double dX = bounds[1] - bounds[0];
        double dY = bounds[3] - bounds[2];
        double dZ = bounds[5] - bounds[4];
        double diagonal = sqrt(dX*dX + dY*dY + dZ*dZ);
        double s = (double(info->relativeHeight) / 100.) * diagonal;
        info->textActor->SetScale(s,s,s);
        debug5 << mName << "Setting scale relative to diagonal: " << diagonal << endl;
    } 
    else
    {
        info->textActor->SetScale(
            info->fixedHeight, info->fixedHeight, info->fixedHeight);
    }
}

// ****************************************************************************
// Method: avtText3DColleague::SetOptions
//
// Purpose: 
//   This method sets the text actor's properties from the values in the
//   annotation object.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 7 15:54:02 PST 2007
//
// Modifications:
//
// ****************************************************************************

void
avtText3DColleague::SetOptions(const AnnotationObject &annot)
{
    // Get the current options.
    AnnotationObject currentOptions;
    GetOptions(currentOptions);

    //
    // The text color has changed or the useForegroundForTextColor flag
    // has changed.
    //
    if(annot.GetUseForegroundForTextColor() != info->useForegroundForTextColor ||
       annot.GetTextColor() != info->textColor)
    {
        // Record the text color that should be used when we're not using
        // the foreground text color.
        info->textColor = annot.GetTextColor();
        info->useForegroundForTextColor = annot.GetUseForegroundForTextColor();

        // Compute the text opacity.
        double tc[4];
        tc[3] = double(info->textColor.Alpha()) / 255.;

        // Set the text color using the foreground color or the text color.
        if(info->useForegroundForTextColor)
        {
            // Get the foreground color.
            double fgColor[3];
            mediator.GetForegroundColor(fgColor);
            info->textActor->GetProperty()->SetColor(fgColor[0], fgColor[1], fgColor[2]);
        }
        else
        {
            // Compute the text color as double.
            tc[0] = double(info->textColor.Red()) / 255.;
            tc[1] = double(info->textColor.Green()) / 255.;
            tc[2] = double(info->textColor.Blue()) / 255.;
            info->textActor->GetProperty()->SetColor(tc[0], tc[1], tc[2]);
        }

        // Set the text opacity.
        info->textActor->GetProperty()->SetOpacity(tc[3]);
    }

    //
    // Set the labels if the text vector is different
    //
    if(currentOptions.GetText() != annot.GetText())
    {
        const stringVector &text = annot.GetText();
        if(text.size() > 0)
            SetText(text[0].c_str());
        else
            SetText("");
    }

    //
    // If the rotations are different then set the rotations.
    //
    if(!currentOptions.FieldsEqual(5, &annot))
    {
        info->rotations[0] = annot.GetRotations()[0];
        info->rotations[1] = annot.GetRotations()[1];
        info->rotations[2] = annot.GetRotations()[2];
        info->textActor->SetOrientation(
            info->rotations[0], info->rotations[1], info->rotations[2]);
    }

    //
    // Set the actor's scale
    //
    if((annot.HasRelativeHeightMode &&  annot.HasRelativeHeight && annot.HasFixedHeight) &&
      (currentOptions.GetRelativeHeightMode() != annot.GetRelativeHeightMode() ||
       currentOptions.GetRelativeHeight() != annot.GetRelativeHeight() ||
       currentOptions.GetFixedHeight() != annot.GetFixedHeight()))
    {
        info->useRelativeHeight = annot.GetRelativeHeightMode();
        info->relativeHeight = annot.GetRelativeHeight();
        info->fixedHeight = annot.GetFixedHeight();
        UpdateActorScale();
    }

    //
    // Set the position coordinates if they are different
    //
    if(!currentOptions.FieldsEqual(4, &annot))
    {
        info->textActor->SetPosition((double *)annot.GetPosition());
    }

    //
    // The flag indicating whether the annotation faces the camera is stored
    // inside the GetFontBold flag in the annotation object.
    //
    if (annot.HasPreserveOrientation)
    {
        if(currentOptions.GetPreserveOrientation() != annot.GetPreserveOrientation())
        {
            if(annot.GetPreserveOrientation())
                info->textActor->SetCamera(mediator.GetCanvas()->GetActiveCamera());
            else
                info->textActor->SetCamera(NULL);
        }
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
// Method: avtText3DColleague::GetOptions
//
// Purpose: 
//   This method stores the text label's attributes in an object that can
//   be passed back to the client.
//
// Arguments:
//   annot : The AnnotationObject to populate.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 7 14:13:21 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtText3DColleague::GetOptions(AnnotationObject &annot)
{
    const char *mName = "avtText3DColleague::GetOptions: ";

    annot.SetObjectType(AnnotationObject::Text3D);
    annot.SetVisible(GetVisible());
    annot.SetActive(GetActive());

    // Get the actor's position.
    if(!info->positionInitialized)
    {
        // Let's put the annotation in the center of the 3D world by default.
        double bounds[6];
        mediator.GetBounds(bounds);
        double pos[3];
        pos[0] = (bounds[0] + bounds[1]) / 2.;
        pos[1] = (bounds[2] + bounds[3]) / 2.;
        pos[2] = (bounds[4] + bounds[5]) / 2.;
        annot.SetPosition(pos);
        info->positionInitialized = true;
        info->textActor->SetPosition(pos);
        debug5 << mName << "Default position: " << pos[0] << ", " << pos[1]
               << ", " << pos[2] << endl;
    }
    annot.SetPosition(info->textActor->GetPosition());

    // Store the text color and opacity.
    annot.SetTextColor(info->textColor);
    annot.SetUseForegroundForTextColor(info->useForegroundForTextColor);

    MapNode opts;
    // Store whether we're facing the camera.
    opts["preserveOrientation"] = (bool)(info->textActor->GetCamera() != NULL);

    // Store the rotations that we're applying to the text.
    annot.SetRotations(info->rotations);

    // Store the height information.
    opts["useRelativeHeight"] = info->useRelativeHeight;
    opts["relativeHeight"] = info->relativeHeight;
    opts["fixedHeight"] = info->fixedHeight;
    if(!info->scaleInitialized)
    {
        UpdateActorScale();
        info->scaleInitialized = true;
    }

    // Store the text
    stringVector text;
    text.push_back(textFormatString);
    annot.SetText(text);
    annot.SetOptions(opts);
}

// ****************************************************************************
// Method: avtText3DColleague::SetForegroundColor
//
// Purpose: 
//   This method is called when the vis window's foreground color changes.
//
// Arguments:
//   r,g,b : The new foreground color.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 7 14:18:20 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtText3DColleague::SetForegroundColor(double r, double g, double b)
{
    if(info->useForegroundForTextColor)
        info->textActor->GetProperty()->SetColor(r, g, b);
}

// ****************************************************************************
// Method: avtText3DColleague::HasPlots
//
// Purpose: 
//   This method is called when the vis window gets some plots. We use this
//   signal to add the text actor to the renderer.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 7 15:56:06 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtText3DColleague::HasPlots(void)
{
    AddToRenderer();
}

// ****************************************************************************
// Method: avtText3DColleague::NoPlots
//
// Purpose: 
//   This method is called when the vis window has no plots. We use this signal
//   to remove the text actor from the renderer.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 7 15:56:42 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtText3DColleague::NoPlots(void)
{
    RemoveFromRenderer();
}

// ****************************************************************************
// Method: avtText3DColleague::UpdatePlotList
//
// Purpose: 
//   This method is called when the plot list changes. Its job is to make sure
//   that the time slider always shows the right time.
//
// Arguments:
//   lst : The plot list.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 7 12:46:37 PDT 2007
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
avtText3DColleague::UpdatePlotList(std::vector<avtActor_p> &lst)
{
    if (lst.size() > 0 && textFormatString != 0)
    {   
        avtAnnotationWithTextColleague::UpdatePlotList(lst);
        SetText(textFormatString);
    }

    // Update the actor's scale
    if(info->addedToRenderer && info->useRelativeHeight)
       UpdateActorScale();
}

// ****************************************************************************
// Method: avtText3DColleague::SetText
//
// Purpose: 
//   Updates the text string.
//
// Note:       This code matches avtTimeSliderColleague::SetText.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 7 14:17:22 PST 2007
//
// Modifications:
//    Jeremy Meredith, Wed Mar 11 12:33:20 EDT 2009
//    Added $cycle support.
//    Fixed off-by-one error in length.
//
// ****************************************************************************

void
avtText3DColleague::SetText(const char *formatString)
{
    if (formatString == 0)
        return;

    // Save the format string. Don't do it in the case that the formatString
    // pointer is the same as textFormatString, which is how we get here from
    // UpdatePlotList.
    size_t len = strlen(formatString);
    if (textFormatString != formatString)
    {
        if (textFormatString != 0)
            delete [] textFormatString;
        textFormatString = new char[len + 1];
        strcpy(textFormatString, formatString);
    }

    if (textString != 0)
        delete [] textString;
    textString = CreateAnnotationString(textFormatString);

    if (info->textSource)
        info->textSource->SetText(textString);
}
