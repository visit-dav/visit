// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_POINT_TOOL_H
#define VISIT_POINT_TOOL_H
#include <viswindow_exports.h>
#include <VisitInteractiveTool.h>
#include <avtPointToolInterface.h>

// Forward declarations
class VisWindow;
class vtkActor;
class vtkPolyDataMapper;
class vtkPolyData;
class vtkTextActor;

// ****************************************************************************
// Class: VisitPointTool
//
// Purpose:
//   This class contains an interactive point tool.
//
// Notes:      
//
// Programmer: Akira Haddox
// Creation:   Mon Jun  9 09:20:16 PDT 2003
//
// Modifications:
//   Akira Haddox, Wed Jul  2 14:53:50 PDT 2003
//   Added translation along an axis.
//
//   Akira Haddox, Mon Aug  4 12:48:02 PDT 2003
//   Removed unneeded point actor.
//
//   Jeremy Meredith, Fri Feb  1 18:07:16 EST 2008
//   Added new value to callback used to pass the hotpoint's "data" field.
//
//   Jeremy Meredith, Wed May 19 11:40:07 EDT 2010
//   Added full frame support.
//
//   Jeremy Meredith, Wed May 19 14:15:58 EDT 2010
//   Account for 3D axis scaling (3D equivalent of full-frame mode).
//
//   Burlen Loring, Mon Sep 28 16:04:23 PDT 2015
//   Added API for setting tool visibility
//
// ****************************************************************************

class VISWINDOW_API VisitPointTool : public VisitInteractiveTool
{
  public:
             VisitPointTool(VisWindowToolProxy &);
    virtual ~VisitPointTool();

    virtual void Enable();
    virtual void Disable();
    virtual bool IsAvailable() const;

    virtual void Start2DMode();
    virtual void Start3DMode();
    virtual void Stop3DMode();
    
    virtual void SetForegroundColor(double, double, double);
    
    virtual const char *  GetName() const { return "Point"; };
    virtual avtToolInterface &GetInterface() { return Interface; };

    virtual void UpdateView();
    virtual void UpdateTool();

    virtual void FullFrameOn(const double, const int);
    virtual void FullFrameOff(void);
    virtual void Set3DAxisScalingFactors(bool, const double[3]);

    virtual void SetVisibility(int);

  protected:
    // Callback functions for the tool's hot points.
    static void TranslateCallback(VisitInteractiveTool *, CB_ENUM,
                                  int, int, int, int, int);

    virtual void CallCallback();
    void Translate(CB_ENUM, int, int, int, int, int);

    avtVector ComputeTranslationDistance(int);

    void CreateGuide();
    void DeleteGuide();
    void AddGuide();
    void RemoveGuide();
    void UpdateGuide();
    void GetGuidePoints(avtVector *pts);

    void CreateTextActors();
    void DeleteTextActors();
    void AddText();
    void RemoveText();
    void UpdateText();

    void CreateSphere();
    void DeleteSphere();
    void UpdateSphere();

    void InitialActorSetup();
    void FinalActorSetup();

    double                focalDepth;
    avtVector            translationDistance;
    vtkActor            *guideActor;
    vtkPolyDataMapper   *guideMapper;
    vtkPolyData         *guideData;
    vtkActor            *sphereActor;
    vtkPolyDataMapper   *sphereMapper;
    vtkPolyData         *sphereData;
    vtkTextActor        *pointTextActor;

    avtPointToolInterface Interface;
    bool                 addedBbox;
    bool                 window3D;
    bool                 addedGuide;
    
    enum TranslateDirection
    {
        none = 0,
        leftAndRight,
        upAndDown,
        inAndOut
    };

    TranslateDirection   axisTranslate;
};

#endif
