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
//   
// ****************************************************************************

class VISWINDOW_API VisWinAnnotations : public VisWinColleague
{
public:
                 VisWinAnnotations(VisWindowColleagueProxy &);
    virtual      ~VisWinAnnotations();

    // Overrides from VisWinColleague
    virtual void SetBackgroundColor(float, float, float);
    virtual void SetForegroundColor(float, float, float);

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
    bool         AddAnnotationObject(int annotType);
    void         HideActiveAnnotationObjects();
    void         DeleteActiveAnnotationObjects();
    void         DeleteAllAnnotationObjects();
    void         RaiseActiveAnnotationObjects();
    void         LowerActiveAnnotationObjects();
    void         SetAnnotationObjectOptions(const AnnotationObjectList &al);
    void         UpdateAnnotationObjectList(AnnotationObjectList &al);
    void         CreateAnnotationObjectsFromList(const AnnotationObjectList &al);
private:
    std::vector<avtAnnotationColleague *> annotations;
};

#endif
