// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIS_WIN_ANNOTATIONS_H
#define VIS_WIN_ANNOTATIONS_H
#include <viswindow_exports.h>
#include <VisWinColleague.h>

class AnnotationObjectList;
class avtAnnotationColleague;

// ****************************************************************************
// Class: VisWinAnnotations
//
// Purpose: 
//   Contains some of the annotations for the vis window and manages them.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 14:21:00 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Mar 20 10:10:34 PDT 2007
//   Name the annotation objects.
//
//   Brad Whitlock, Mon Mar  2 14:17:08 PST 2009
//   I added SetTimeScaleAndOffset.
//
//   Burlen Loring, Mon Sep 28 16:04:23 PDT 2015
//   Added API for setting visibility
//
// ****************************************************************************

class VISWINDOW_API VisWinAnnotations : public VisWinColleague
{
public:
                 VisWinAnnotations(VisWindowColleagueProxy &);
    virtual      ~VisWinAnnotations();

    // Overrides from VisWinColleague
    virtual void SetBackgroundColor(double, double, double);
    virtual void SetForegroundColor(double, double, double);

    virtual void Start2DMode(void);
    virtual void Start3DMode(void);
    virtual void StartCurveMode(void);
    virtual void Stop2DMode(void);
    virtual void Stop3DMode(void);
    virtual void StopCurveMode(void);

    virtual void HasPlots(void);
    virtual void NoPlots(void);

    virtual void MotionBegin(void);
    virtual void MotionEnd(void);

    virtual void UpdateView(void);

    virtual void UpdatePlotList(std::vector<avtActor_p> &);

    virtual void SetFrameAndState(int, int, int, int, int, int, int);

    // New methods for annotations
    bool         AddAnnotationObject(int annotType, const std::string &annotName);
    void         HideActiveAnnotationObjects();
    void         DeleteActiveAnnotationObjects();
    bool         DeleteAnnotationObject(const std::string &);
    void         DeleteAllAnnotationObjects();
    void         RaiseActiveAnnotationObjects();
    void         LowerActiveAnnotationObjects();
    void         SetAnnotationObjectOptions(const AnnotationObjectList &al);
    void         UpdateAnnotationObjectList(AnnotationObjectList &al);
    void         CreateAnnotationObjectsFromList(const AnnotationObjectList &al);

    void         SetTimeScaleAndOffset(double,double);

    void         SetVisibility(int val);
private:
    void UpdateLegends();

    std::vector<avtAnnotationColleague *> annotations;
    double                                timeScale;
    double                                timeOffset;

    // DO NOT USE avtActor_p BECAUSE WHEN WE CLEAR THE VECTOR, THE ACTORS GET 
    // DELETED AGAIN!
    std::vector<avtActor *>               actorList;
};

#endif
