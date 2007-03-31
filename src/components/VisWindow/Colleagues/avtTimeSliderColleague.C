#include <avtTimeSliderColleague.h>

#include <ColorAttribute.h>
#include <AnnotationObject.h>
#include <VisWindowColleagueProxy.h>

#include <vtkRenderer.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkTimeSliderActor.h>

#include <snprintf.h>

#define DEFAULT_X       0.01
#define DEFAULT_Y       0.01
#define DEFAULT_WIDTH   0.4
#define DEFAULT_HEIGHT  0.04
#define TIME_IDENTIFIER "$time"

// ****************************************************************************
// Method: avtTimeSliderColleague::avtTimeSliderColleague
//
// Purpose: 
//   Constructor for the avtTimeSliderColleague class.
//
// Arguments:
//   m : Reference to the colleague proxy.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 6 14:25:58 PST 2003
//
// Modifications:
//   
// ****************************************************************************

avtTimeSliderColleague::avtTimeSliderColleague(VisWindowColleagueProxy &m) :
    avtAnnotationColleague(m)
{
    useForegroundForTextColor = true;
    addedToRenderer = false;
    textFormatString = 0;
    textString = 0;
    timeDisplayMode = 0;
    currentTime = 0.;

    //
    // Create and position the time slider actor
    //
    float rect[4];
    GetSliderRect(DEFAULT_X, DEFAULT_Y, DEFAULT_WIDTH, DEFAULT_HEIGHT, rect);
    timeSlider = vtkTimeSliderActor::New();
    timeSlider->GetPositionCoordinate()->SetValue(rect[0], rect[1]);
    timeSlider->GetPosition2Coordinate()->SetValue(rect[0] + rect[2],
        rect[1] + rect[3]);
    timeSlider->SetStartColor(0.,1.,1.,1.);
    timeSlider->SetEndColor(1.,1.,1.,0.6);

    //
    // Create and position the start text actor.
    //
    textActor = vtkTextActor::New();
    textActor->ScaledTextOn();
    std::string defaultString("Time="); defaultString += TIME_IDENTIFIER;
    SetText(defaultString.c_str());
    vtkCoordinate *pos = textActor->GetPositionCoordinate();
    pos->SetCoordinateSystemToNormalizedViewport();
    GetTextRect(DEFAULT_X, DEFAULT_Y, DEFAULT_WIDTH, DEFAULT_HEIGHT, rect);
    pos->SetValue(rect[0], rect[1], 0.);
    textActor->SetWidth(rect[2]);
    textActor->SetHeight(rect[3]);

    // Make sure that the text actors initially has the right fg color.
    float fgColor[3];
    mediator.GetForegroundColor(fgColor);
    SetForegroundColor(fgColor[0], fgColor[1], fgColor[2]);
    textActor->GetTextProperty()->SetOpacity(1.);

    // Store the foreground color into the text color.
    int ifgColor[3];
    ifgColor[0] = int(fgColor[0] * 255.f);
    ifgColor[1] = int(fgColor[1] * 255.f);
    ifgColor[2] = int(fgColor[2] * 255.f);
    textColor = ColorAttribute(ifgColor[0], ifgColor[1], ifgColor[2], 255);

    // Make sure that the time slider initially shows the right time.
    int a,b,c,d,e,f,g;
    mediator.GetFrameAndState(a, b, c, d, e, f, g);
    SetFrameAndState(a, b, c, d, e, f, g);
}

// ****************************************************************************
// Method: avtTimeSliderColleague::~avtTimeSliderColleague
//
// Purpose: 
//   Destructor for the avtTimeSliderColleague class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 6 14:26:36 PST 2003
//
// Modifications:
//   
// ****************************************************************************

avtTimeSliderColleague::~avtTimeSliderColleague()
{
    if(timeSlider != 0)
    {
        timeSlider->Delete();
        timeSlider = 0;
    }

    if(textActor != 0)
    {
        textActor->Delete();
        textActor = 0;
    }

    if(textFormatString != 0)
        delete textFormatString;
}

// ****************************************************************************
// Method: avtTimeSliderColleague::AddToRenderer
//
// Purpose: 
//   Adds the time slider to the renderer if it should be added.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 6 14:26:55 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void 
avtTimeSliderColleague::AddToRenderer()
{
    if(!addedToRenderer && ShouldBeAddedToRenderer())
    {
        mediator.GetForeground()->AddActor2D(timeSlider);
        mediator.GetForeground()->AddActor2D(textActor);
        addedToRenderer = true;
    }
}

// ****************************************************************************
// Method: avtTimeSliderColleague::RemoveFromRenderer
//
// Purpose: 
//   Removes the time slider from the renderer.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 6 14:27:22 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
avtTimeSliderColleague::RemoveFromRenderer()
{
    if(addedToRenderer)
    {
        mediator.GetForeground()->RemoveActor2D(timeSlider);
        mediator.GetForeground()->RemoveActor2D(textActor);
        addedToRenderer = false;
    }
}

// ****************************************************************************
// Method: avtTimeSliderColleague::Hide
//
// Purpose: 
//   Hides the time slider or shows it if it is already hidden.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 6 14:27:56 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
avtTimeSliderColleague::Hide()
{
    SetVisible(!GetVisible());

    if(addedToRenderer)
        RemoveFromRenderer();
    else
        AddToRenderer();
}

// ****************************************************************************
// Method: avtTimeSliderColleague::ShouldBeAddedToRenderer
//
// Purpose: 
//   Returns when the time slider should be added to the renderer.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 6 14:28:38 PST 2003
//
// Modifications:
//   
// ****************************************************************************

bool
avtTimeSliderColleague::ShouldBeAddedToRenderer() const
{
    return GetVisible() && mediator.HasPlots();
}

// ****************************************************************************
// Method: avtTimeSliderColleague::SetOptions
//
// Purpose: 
//   Sets the time slider's attributes from the annotation object's options.
//
// Arguments:
//   annot : The annotation object that we'll use to set the options.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 6 14:29:01 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
avtTimeSliderColleague::SetOptions(const AnnotationObject &annot)
{
    float fColor[4];

#define SetColorUsingColorAttribute(Source, Dest) {\
    fColor[0] = float(annot.Get##Source().Red()) / 255.f; \
    fColor[1] = float(annot.Get##Source().Green()) / 255.f; \
    fColor[2] = float(annot.Get##Source().Blue()) / 255.f; \
    fColor[3] = float(annot.Get##Source().Alpha()) / 255.f; \
    timeSlider->Set##Dest(fColor); }

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
        float tc[4];
        tc[3] = float(textColor.Alpha()) / 255.f;

        // Set the text color using the foreground color or the text color.
        if(useForegroundForTextColor)
        {
            // Get the foreground color.
            float fgColor[3];
            mediator.GetForegroundColor(fgColor);
            textActor->GetTextProperty()->SetColor(fgColor[0], fgColor[1], fgColor[2]);
        }
        else
        {
            // Compute the text color as floats.
            tc[0] = float(textColor.Red()) / 255.f;
            tc[1] = float(textColor.Green()) / 255.f;
            tc[2] = float(textColor.Blue()) / 255.f;
            textActor->GetTextProperty()->SetColor(tc[0], tc[1], tc[2]);
        }

        // Set the text opacity.
        textActor->GetTextProperty()->SetOpacity(tc[3]);
    }

    //
    // Set the colors if the new colors are different
    //
    if(currentOptions.GetColor1() != annot.GetColor1())
        SetColorUsingColorAttribute(Color1, StartColor);
    if(currentOptions.GetColor2() != annot.GetColor2())
        SetColorUsingColorAttribute(Color2, EndColor);

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
    // Set the position coordinates if they are different
    //
    if(!currentOptions.FieldsEqual(3, &annot) ||
       !currentOptions.FieldsEqual(4, &annot))
    {
        const float *p1 = annot.GetPosition();
        const float *p2 = annot.GetPosition2();

        // Set the time slider's coordinates.
        float rect[4];
        GetSliderRect(p1[0], p1[1], p2[0], p2[1], rect);
        timeSlider->GetPositionCoordinate()->SetValue(rect[0], rect[1], 0.f);
        timeSlider->GetPosition2Coordinate()->SetValue(rect[0] + rect[2],
            rect[1] + rect[3], 0.f);

        // Set the text actor's coordinates.
        vtkCoordinate *pos = textActor->GetPositionCoordinate();
        pos->SetCoordinateSystemToNormalizedViewport();
        GetTextRect(p1[0], p1[1], p2[0], p2[1], rect);
        pos->SetValue(rect[0], rect[1], 0.);
        textActor->SetWidth(rect[2]);
        textActor->SetHeight(rect[3]);
    }

    //
    // Set the rounded, shaded settings if they are different.
    //
    bool timeDisplayModeChanged = false;
    if(currentOptions.GetIntAttribute1() != annot.GetIntAttribute1())
    {
        int rounded =  annot.GetIntAttribute1() & 1;
        int shaded  = ((annot.GetIntAttribute1() >> 1) & 1);
        timeSlider->SetDrawEndCaps(rounded);
        timeSlider->SetDraw3D(shaded);

        // Set the time display mode.
        int newTimeDisplayMode = ((annot.GetIntAttribute1() >> 2) & 3);
        if(newTimeDisplayMode != timeDisplayMode)
        {
            timeDisplayMode = newTimeDisplayMode;
            timeDisplayModeChanged = true;
        }
    }

    //
    // If we changed time display modes or if we're showing user-defined
    // time and the percent complete is different then update the
    // time slider's parametric time.
    //
    if(timeDisplayModeChanged ||
       (timeDisplayMode == 3 &&
        currentOptions.GetFloatAttribute1() != annot.GetFloatAttribute1())
      )
    {
        if(timeDisplayMode == 3)
        {
            timeSlider->SetParametricTime(annot.GetFloatAttribute1());
        }
        else
        {
            int a,b,c,d,e,f,g;
            mediator.GetFrameAndState(a, b, c, d, e, f, g);
            SetFrameAndState(a, b, c, d, e, f, g);
        }
    }
}

// ****************************************************************************
// Method: avtTimeSliderColleague::GetOptions
//
// Purpose: 
//   This method stores the time slider's attributes in an object that can
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
avtTimeSliderColleague::GetOptions(AnnotationObject &annot)
{
    annot.SetObjectType(AnnotationObject::TimeSlider);
    annot.SetVisible(GetVisible());
    annot.SetActive(GetActive());

    const float *p1 = textActor->GetPosition();
    const float *p2 = timeSlider->GetPosition2();
    annot.SetPosition(p1);
    // Store the width and height in position2.
    float p2wh[3];
    p2wh[0] = p2[0] - p1[0];
    p2wh[1] = p2[1] - p1[1];
    p2wh[2] = p2[2];
    annot.SetPosition2(p2wh);

    float fColor[4]; int iColor[4];
#define FloatsToColorAttribute(Source, Dest) \
    timeSlider->Get##Source(fColor); \
    iColor[0] = int(fColor[0] * 255.f); \
    iColor[1] = int(fColor[1] * 255.f); \
    iColor[2] = int(fColor[2] * 255.f); \
    iColor[3] = int(fColor[3] * 255.f); \
    annot.Set##Dest(ColorAttribute(iColor[0],iColor[1],iColor[2],iColor[3]));

    // Store the text color and opacity.
    annot.SetTextColor(textColor);
    annot.SetUseForegroundForTextColor(useForegroundForTextColor);
    FloatsToColorAttribute(StartColor, Color1);
    FloatsToColorAttribute(EndColor, Color2);
    
    stringVector text;
    text.push_back(textFormatString);
    annot.SetText(text);

    // Store the rounded and shaded settings in int attribute 1.
    int rounded = (timeSlider->GetDrawEndCaps() & 1);
    int shaded  = ((timeSlider->GetDraw3D() & 1) << 1);
    int tdsm    = ((timeDisplayMode & 3) << 2);
    annot.SetIntAttribute1(rounded | shaded | tdsm);

    // Store the parametricTime from the time slider into float attribute 1.
    annot.SetFloatAttribute1(timeSlider->GetParametricTime());
}

// ****************************************************************************
// Method: avtTimeSliderColleague::SetForegroundColor
//
// Purpose: 
//   This method is called when the vis window changes foreground colors so we
//   can update the text color in the time slider.
//
// Arguments:
//   r,g,b : The color components.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 6 14:30:04 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
avtTimeSliderColleague::SetForegroundColor(float r, float g, float b)
{
    if(useForegroundForTextColor)
        textActor->GetTextProperty()->SetColor(r, g, b);
}

// ****************************************************************************
// Method: avtTimeSliderColleague::HasPlots
//
// Purpose: 
//   Called to add the time slider to the renderer when there are plots.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 6 14:30:56 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
avtTimeSliderColleague::HasPlots(void)
{
    AddToRenderer();
}

// ****************************************************************************
// Method: avtTimeSliderColleague::NoPlots
//
// Purpose: 
//   Called to remove the time slider from the renderer when there are no plots.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 6 14:30:56 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
avtTimeSliderColleague::NoPlots(void)
{
    RemoveFromRenderer();
}

// ****************************************************************************
// Method: avtTimeSliderColleague::SetFrameAndState
//
// Purpose: 
//   This method is called when the vis window's frame/states change so the
//   time slider can show the current time in the animation.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 6 14:32:32 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
avtTimeSliderColleague::SetFrameAndState(int nFrames,
    int startFrame, int curFrame, int endFrame,
    int startState, int curState, int endState)
{
    double parametricTime;

    if(timeDisplayMode == 0)
    {
        int dFrame = (nFrames > 1) ? (nFrames - 1) : 1;
        parametricTime = double(curFrame) / double(dFrame);
    }
    else if(timeDisplayMode == 1)
    {
        int dFrame = endFrame - startFrame;
        dFrame = (dFrame == 0) ? 1 : dFrame;
        parametricTime = double(curFrame - startFrame) / double(dFrame);
    }
    else if(timeDisplayMode == 2)
    {
        int dState = endState - startState;
        dState = (dState == 0) ? 1 : dState;
        parametricTime = double(curState - startState) / double(dState);
    }
    else
    {
        // The time display mode is user-specified. Return without setting
        // the parametric time in the time slider.
        return;
    }

    //
    // If the parametric time is different from the time slider's parametric
    // time then set the new time and render the window.
    //
    if(timeSlider->GetParametricTime() != parametricTime)
        timeSlider->SetParametricTime(parametricTime);
}

// ****************************************************************************
// Method: avtTimeSliderColleague::UpdatePlotList
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
avtTimeSliderColleague::UpdatePlotList(vector<avtActor_p> &lst)
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
// Method: avtTimeSliderColleague::SetText
//
// Purpose: 
//   Updates the text string.
//
// Note:       This code matches avtText2DColleague::SetText.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 14:17:22 PST 2003
//
// Modifications:
//
// ****************************************************************************

void
avtTimeSliderColleague::SetText(const char *formatString)
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

// ****************************************************************************
// Method: avtTimeSliderColleague::SliderHeight
//
// Purpose: 
//   Returns how much of the total height should be devoted to the slider.
//
// Arguments:
//   height : The total time slider colleague height.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 3 11:18:22 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

float
avtTimeSliderColleague::SliderHeight(float height) const
{
    return height * 0.6f;
}

// ****************************************************************************
// Method: avtTimeSliderColleague::GetTextRect
//
// Purpose: 
//   Returns the rectangle that the text should fit in.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 3 11:18:57 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
avtTimeSliderColleague::GetTextRect(float x, float y, float width,
    float height, float *rect) const
{
    rect[0] = x;
    rect[1] = y;
    rect[2] = width;
    rect[3] = height - SliderHeight(height);
}

// ****************************************************************************
// Method: avtTimeSliderColleague::GetSliderRect
//
// Purpose: 
//   Returns the rectangle that the slider should fit in.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 3 11:18:57 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
avtTimeSliderColleague::GetSliderRect(float x, float y, float width,
    float height, float *rect) const
{
    rect[0] = x;
    rect[1] = y + height - SliderHeight(height);
    rect[2] = width;
    rect[3] = height - SliderHeight(height);
}

