// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_LINE_TOOL_H
#define VISIT_LINE_TOOL_H
#include <viswindow_exports.h>
#include <VisitInteractiveTool.h>
#include <avtLineToolInterface.h>
#include <avtMatrix.h>
#include <avtQuaternion.h>
#include <avtTrackball.h>

// Forward declarations
class VisWindow;
class vtkActor;
class vtkLineSource;
class vtkPolyDataMapper;
class vtkPolyData;
class vtkTextActor;

// ****************************************************************************
// Class: VisitLineTool
//
// Purpose:
//   This class contains an interactive line tool that can be used to define
//   a lineout curve.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 18 15:23:54 PST 2002
//
// Modifications:
//   Brad Whitlock, Tue Jul 23 10:23:18 PDT 2002
//   I removed the NoPlots redeclaration since it now uses the base class's
//   method implementation.
//
//   Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002
//   Replace vtkActor2d/vtkTextMapper pairs with vtkTextActor.
//
//   Kathleen Bonnell, Wed May 28 16:14:22 PDT 2003 
//   Add method ReAddToWindow.
//
//   Kathleen Bonnell, Fri Jun  6 15:36:24 PDT 2003 
//   Add FullFrameOn/Off methods. 
//
//   Jeremy Meredith, Fri Feb  1 18:07:16 EST 2008
//   Added new value to callback used to pass the hotpoint's "data" field.
//
//   Jeremy Meredith, Wed May 19 14:15:58 EDT 2010
//   Account for 3D axis scaling (3D equivalent of full-frame mode).
//
//   Burlen Loring, Mon Sep 28 16:04:23 PDT 2015
//   Added API for setting tool visibility
//
// ****************************************************************************

class VISWINDOW_API VisitLineTool : public VisitInteractiveTool
{
  public:
             VisitLineTool(VisWindowToolProxy &);
    virtual ~VisitLineTool();

    virtual void Enable();
    virtual void Disable();
    virtual bool IsAvailable() const;

    virtual void Start2DMode();
    virtual void Start3DMode();
    virtual void Stop3DMode();

    virtual void SetForegroundColor(double, double, double);

    virtual const char *  GetName() const { return "Line"; };
    virtual avtToolInterface &GetInterface() { return Interface; };

    virtual void UpdateView();
    virtual void UpdateTool();
    virtual void ReAddToWindow();

    virtual void FullFrameOn(const double, const int);
    virtual void FullFrameOff(void);
    virtual void Set3DAxisScalingFactors(bool, const double[3]);

    virtual void SetVisibility(int val);

  protected:
    // Callback functions for the tool's hot points.
    static void TranslatePoint1Callback(VisitInteractiveTool *, CB_ENUM,
                                        int, int, int, int, int);
    static void TranslatePoint2Callback(VisitInteractiveTool *, CB_ENUM,
                                        int, int, int, int, int);
    static void TranslateCallback(VisitInteractiveTool *, CB_ENUM,
                                  int, int, int, int, int);

    virtual void CallCallback();
    void Translate(CB_ENUM, int, int, int, int, int);
    void InitializePoints();

    void CreateLineActor();
    void UpdateLine();

    void CreateGuide();
    void DeleteGuide();
    void AddGuide(int);
    void RemoveGuide();
    void UpdateGuide(int);
    void GetGuidePoints(int pi, avtVector *pts);

    void CreateTextActors();
    void DeleteTextActors();
    void AddText();
    void RemoveText();
    void UpdateText();

    void InitialActorSetup(int);
    void FinalActorSetup();

    double                focalDepth;
    double                translationDistance;
    avtVector            depthTranslationDistance;
    vtkLineSource       *lineSource;
    vtkActor            *lineActor;
    vtkPolyDataMapper   *lineMapper;
    vtkPolyData         *lineData;
    vtkActor            *guideActor;
    vtkPolyDataMapper   *guideMapper;
    vtkPolyData         *guideData;
    vtkTextActor        *pointTextActor[2];

    avtLineToolInterface Interface;
    bool                 addedBbox;
    bool                 window3D;
    bool                 addedGuide;
    bool                 enlongating;
    bool                 depthTranslate;
};

#endif
