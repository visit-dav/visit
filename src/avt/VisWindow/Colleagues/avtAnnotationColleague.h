// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_ANNOTATION_COLLEAGUE_H
#define AVT_ANNOTATION_COLLEAGUE_H
#include <VisWinColleague.h>
#include <avtLegend.h>

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
//   Brad Whitlock, Tue Mar 20 10:18:04 PDT 2007
//   Added support for setting legend properties.
//
//   Brad Whitlock, Mon Mar  2 14:19:55 PST 2009
//   I added SetTimeScaleAndOffset.
//
//   Burlen Loring, Mon Sep 28 16:04:23 PDT 2015
//   Added API for setting visibility
//
//   Burlen Loring, Thu Oct  8 13:43:48 PDT 2015
//   Fix a couple of compiler warnings
//
//   Mark C. Miller, Thu Oct  5 15:30:04 PDT 2023
//   Moved timeScale,timeOffset from avtTimeSliderCollegue
//   This is because by virtue of annotation macros, half the annotations
//   could potentially need to access this info when handling $time macros.
// ****************************************************************************

class VISWINDOW_API avtAnnotationColleague : protected VisWinColleague
{
public:
    avtAnnotationColleague(VisWindowColleagueProxy &);
    virtual ~avtAnnotationColleague();

    virtual void AddToRenderer() = 0;
    virtual void RemoveFromRenderer() = 0;
    virtual void Hide() = 0;

    virtual std::string TypeName() const = 0;

    // Methods that return a little info about the annotation.
    void SetName(const std::string &n) { name = n; }
    const std::string &GetName() const { return name; }
    void SetActive(bool val) { active = val; }
    bool GetActive() const   { return active; }
    void SetVisible(bool val) { visible = val; }
    bool GetVisible() const   { return visible; }

    // Methods to set and get the annotation's properties.
    virtual void SetOptions(const AnnotationObject &annot) = 0;
    virtual void GetOptions(AnnotationObject &annot) = 0;

    // Legend methods.
    virtual bool ManageLayout(avtLegend_p) const { return false; }
    virtual void CustomizeLegend(avtLegend_p) { };

    // Methods that are called in response to vis window events. These
    // method can be overridden to let the annotation decide what to do when
    // these vis window events happen.
    virtual void SetBackgroundColor(double, double, double) { };
    virtual void SetForegroundColor(double, double, double) { };
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
    virtual void UpdatePlotList(std::vector<avtActor_p> &) { };
    virtual void SetFrameAndState(int, int, int, int, int, int, int) { };

    virtual void SetTimeScaleAndOffset(double,double);

    virtual void SetVisibility(int) {}

protected:
    double      timeScale;
    double      timeOffset;

private:
    std::string name;
    bool        active;
    bool        visible;

};

#endif
