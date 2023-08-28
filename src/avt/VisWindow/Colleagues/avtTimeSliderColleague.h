// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_TIME_SLIDER_COLLEAGUE_H
#define AVT_TIME_SLIDER_COLLEAGUE_H
#include <viswindow_exports.h>
#include <avtAnnotationWithTextColleague.h>
#include <ColorAttribute.h>

class vtkVisItTextActor;
class vtkTimeSliderActor;

// ****************************************************************************
// Class: avtTimeSliderColleague
//
// Purpose:
//   This colleague is a time slider actor that can exist in the window and
//   indicate how far we are through the animation.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 16:27:10 PST 2003
//
// Modifications:
//   Kathleen Bonnell, Thu Jan 13 08:39:30 PST 2005
//   Added timeFormatString and another char* arg to SetText.
//
//   Brad Whitlock, Mon Mar  2 14:21:17 PST 2009
//   Added SetTimeScaleAndOffset.
//
//   Jeremy Meredith, Wed Mar 11 12:33:20 EDT 2009
//   Added $cycle support.
//
//   Brad Whitlock, Wed Sep 28 15:24:47 PDT 2011
//   Change the text actor type.
//
// ****************************************************************************

class VISWINDOW_API avtTimeSliderColleague : public avtAnnotationWithTextColleague
{
public:
    avtTimeSliderColleague(VisWindowColleagueProxy &);
    virtual ~avtTimeSliderColleague();

    virtual void AddToRenderer();
    virtual void RemoveFromRenderer();
    virtual void Hide();

    virtual void SetTimeScaleAndOffset(double,double);

    virtual std::string TypeName() const { return "TimeSlider"; }

    // Methods to set and get the annotation's properties.
    virtual void SetOptions(const AnnotationObject &annot);
    virtual void GetOptions(AnnotationObject &annot);

    // Methods that are called in response to vis window events.
    virtual void SetForegroundColor(double r, double g, double b);
    virtual void HasPlots(void);
    virtual void NoPlots(void);
    virtual void SetFrameAndState(int, int, int, int, int, int, int);
    virtual void UpdatePlotList(std::vector<avtActor_p> &lst);

private:
    bool ShouldBeAddedToRenderer() const;
    void SetText(const char *text, const char *format);
    double SliderHeight(double height) const;
    void  GetTextRect(double, double, double, double, double *) const;
    void  GetSliderRect(double, double, double, double, double *) const;

    vtkTimeSliderActor *timeSlider;
    vtkVisItTextActor  *textActor;
    char               *timeFormatString;
    int                timeDisplayMode;

    double             timeScale;
    double             timeOffset;
    bool               useForegroundForTextColor;
    bool               addedToRenderer;
    ColorAttribute     textColor;
};

#endif
