#ifndef AVT_ANNOTATION_COLLEAGUE_H
#define AVT_ANNOTATION_COLLEAGUE_H
#include <VisWinColleague.h>

class AnnotationObject;

// ****************************************************************************
// Class: avtAnnotationColleague
//
// Purpose: 
//   Base class for certain annotations that can live in the window.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 16:06:04 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VISWINDOW_API avtAnnotationColleague : protected VisWinColleague
{
public:
    avtAnnotationColleague(VisWindowColleagueProxy &);
    virtual ~avtAnnotationColleague();

    virtual void AddToRenderer() = 0;
    virtual void RemoveFromRenderer() = 0;
    virtual void Hide() = 0;

    // Methods that return a little info about the annotation.
    void SetActive(bool val) { active = val; }
    bool GetActive() const   { return active; }
    void SetVisible(bool val) { visible = val; }
    bool GetVisible() const   { return visible; }

    // Methods to set and get the annotation's properties.
    virtual void SetOptions(const AnnotationObject &annot) = 0;
    virtual void GetOptions(AnnotationObject &annot) = 0;

    // Methods that are called in response to vis window events. These
    // method can be overridden to let the annotation decide what to do when
    // these vis window events happen.
    virtual void SetBackgroundColor(double r, double g, double b) { };
    virtual void SetForegroundColor(double r, double g, double b) { };
    virtual void Start2DMode(void) { };
    virtual void Start3DMode(void) { };
    virtual void StartCurveMode(void) { };
    virtual void Stop2DMode(void) { };
    virtual void Stop3DMode(void) { };
    virtual void StopCurveMode(void) { };
    virtual void HasPlots(void) { };
    virtual void NoPlots(void) { };
    virtual void MotionBegin(void) { };
    virtual void MotionEnd(void) { };
    virtual void UpdateView(void) { };
    virtual void UpdatePlotList(std::vector<avtActor_p> &p) { };
    virtual void SetFrameAndState(int, int, int, int, int, int, int) { };

private:
    bool active;
    bool visible;
};

#endif
