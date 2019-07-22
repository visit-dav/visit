// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_BOX_TOOL_H
#define VISIT_BOX_TOOL_H
#include <viswindow_exports.h>
#include <VisitInteractiveTool.h>
#include <avtBoxToolInterface.h>
#include <avtMatrix.h>
#include <avtQuaternion.h>
#include <avtTrackball.h>

// Forward declarations
class VisWindow;
class vtkActor;
class vtkPolyDataMapper;
class vtkPolyData;
class vtkTextActor;

// ****************************************************************************
// Class: VisitBoxTool
//
// Purpose:
//   This class contains an interactive plane tool that can be used to define
//   a slice plane.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 23 07:13:08 PDT 2002
//
// Modifications:
//   Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002
//   Replace vtkActor2d/vtkTextMapper pairs with vtkTextActor.
//
//   Kathleen Bonnell, Wed May 28 16:09:47 PDT 2003 
//   Add method ReAddToWindow.
//
//   Brad Whitlock, Tue Jul 13 14:12:33 PST 2004
//   Added new handlers for the new hotpoints.
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

class VISWINDOW_API VisitBoxTool : public VisitInteractiveTool
{
  public:
             VisitBoxTool(VisWindowToolProxy &);
    virtual ~VisitBoxTool();

    virtual void Enable();
    virtual void Disable();
    virtual bool IsAvailable() const;

    virtual void Start2DMode();
    virtual void Stop3DMode();
    virtual void UpdateView();

    virtual void SetForegroundColor(double, double, double);

    virtual const char *  GetName() const { return "Box"; };
    virtual avtToolInterface &GetInterface() { return Interface; };

    virtual void UpdateTool();
    virtual void ReAddToWindow();
    virtual void Set3DAxisScalingFactors(bool, const double[3]);
    void SetActiveHotPoint(int v) { activeHotPoint = v; };

    virtual void SetVisibility(int val);

  protected:
    // Callback functions for the tool's hot points.
    static void TranslateCallback(VisitInteractiveTool *, CB_ENUM,
                                  int, int, int, int, int);
    static void ResizeCallback(VisitInteractiveTool *, CB_ENUM,
                               int, int, int, int, int);
    static void XMINCallback(VisitInteractiveTool *, CB_ENUM,
                             int, int, int, int, int);
    static void XMAXCallback(VisitInteractiveTool *, CB_ENUM,
                             int, int, int, int, int);
    static void YMINCallback(VisitInteractiveTool *, CB_ENUM,
                             int, int, int, int, int);
    static void YMAXCallback(VisitInteractiveTool *, CB_ENUM,
                             int, int, int, int, int);
    static void ZMINCallback(VisitInteractiveTool *, CB_ENUM,
                             int, int, int, int, int);
    static void ZMAXCallback(VisitInteractiveTool *, CB_ENUM,
                             int, int, int, int, int);

    virtual void CallCallback();
    void Translate(CB_ENUM, int, int, int, int);
    void Resize(CB_ENUM, int, int, int, int);

    void CreateBoxActor();

    void CreateTextActors();
    void DeleteTextActors();
    void AddText();
    void RemoveText();
    void UpdateText();
    void GetHotPointLabel(int index, char *str);

    void CreateOutline();
    void DeleteOutline();
    void AddOutline();
    void RemoveOutline();
    void UpdateOutline();
    void GetBoundingBoxOutline(int a, avtVector *verts, bool giveMin);

    void InitialActorSetup();
    void FinalActorSetup();

    void DoTransformations();

    vtkActor           *boxActor;
    vtkPolyDataMapper  *boxMapper;
    vtkPolyData        *boxData;

    vtkActor           *outlineActor[3];
    vtkPolyDataMapper  *outlineMapper[3];
    vtkPolyData        *outlineData[3];
    vtkTextActor       *outlineTextActor[4];

    vtkTextActor       *originTextActor;
    vtkTextActor       *labelTextActor[7];

    avtBoxToolInterface Interface;

    avtTrackball        trackball;
    HotPointVector      origHotPoints;
    avtMatrix           SMtx;
    avtMatrix           TMtx;

    bool                addedOutline;
    bool                addedBbox;
    bool                depthTranslate;
    int                 activeHotPoint;
    double               focalDepth;
    double               originalDistance;
    avtVector           depthTranslationDistance;

};

#endif
