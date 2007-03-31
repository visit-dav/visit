#ifndef AVT_TIME_SLIDER_COLLEAGUE_H
#define AVT_TIME_SLIDER_COLLEAGUE_H
#include <viswindow_exports.h>
#include <avtAnnotationColleague.h>
#include <ColorAttribute.h>

class vtkTextActor;
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
//   
// ****************************************************************************

class VISWINDOW_API avtTimeSliderColleague : public avtAnnotationColleague
{
public:
    avtTimeSliderColleague(VisWindowColleagueProxy &);
    virtual ~avtTimeSliderColleague();

    virtual void AddToRenderer();
    virtual void RemoveFromRenderer();
    virtual void Hide();

    // Methods to set and get the annotation's properties.
    virtual void SetOptions(const AnnotationObject &annot);
    virtual void GetOptions(AnnotationObject &annot);

    // Methods that are called in response to vis window events.
    virtual void SetForegroundColor(float r, float g, float b);
    virtual void HasPlots(void);
    virtual void NoPlots(void);
    virtual void SetFrameAndState(int, int, int, int, int, int, int);
    virtual void UpdatePlotList(vector<avtActor_p> &lst);
private:
    bool ShouldBeAddedToRenderer() const;
    void SetText(const char *text);
    float SliderHeight(float height) const;
    void  GetTextRect(float, float, float, float, float *) const;
    void  GetSliderRect(float, float, float, float, float *) const;

    vtkTimeSliderActor *timeSlider;
    vtkTextActor       *textActor;
    char               *textFormatString;
    char               *textString;
    int                timeDisplayMode;

    double             currentTime;
    bool               useForegroundForTextColor;
    bool               addedToRenderer;
    ColorAttribute     textColor;
};

#endif
