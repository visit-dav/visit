// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_AXIS_RESTRICTION_TOOL_H
#define VISIT_AXIS_RESTRICTION_TOOL_H
#include <viswindow_exports.h>
#include <VisitInteractiveTool.h>
#include <avtAxisRestrictionToolInterface.h>
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
// Class: VisitAxisRestrictionTool
//
// Purpose:
//   This class contains an interactive tool that can be used to define
//   restrictions along arrays of parallel axes.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   February  1, 2008
//
// Modifications:
//    Jeremy Meredith, Thu Feb  7 17:59:55 EST 2008
//    Added support for array variables and bin-defined x positions.
//
//    Jeremy Meredith, Fri Feb 15 13:21:20 EST 2008
//    Added axis names to the axis restriction tool.
//
//    Burlen Loring, Mon Sep 28 16:04:23 PDT 2015
//    Added API for setting tool visibility
//
// ****************************************************************************

class VISWINDOW_API VisitAxisRestrictionTool : public VisitInteractiveTool
{
  public:
             VisitAxisRestrictionTool(VisWindowToolProxy &);
    virtual ~VisitAxisRestrictionTool();

    virtual void Enable();
    virtual void Disable();
    virtual bool IsAvailable() const;

    virtual void StopAxisArrayMode();

    virtual void SetForegroundColor(double, double, double);

    virtual const char *  GetName() const { return "AxisRestriction"; };
    virtual avtToolInterface &GetInterface() { return Interface; };

    virtual void UpdateView();
    virtual void UpdateTool();
    virtual void ReAddToWindow();
    virtual void UpdatePlotList(std::vector<avtActor_p> &list);
    virtual void FullFrameOn(const double, const int);
    virtual void FullFrameOff();

    virtual void SetVisibility(int);

  protected:
    // Callback functions for the tool's hot points.
    static void MoveCallback(VisitInteractiveTool *, CB_ENUM,
                             int, int, int, int, int);

    virtual void CallCallback();
    void Move(CB_ENUM, int, int, int, int, int);

    void CreateTextActors();
    void DeleteTextActors();
    void AddText();
    void RemoveText();
    void UpdateText();

    void InitialActorSetup();
    void FinalActorSetup();

    void DoClampAndTransformations();

    static const float         radius;
    double                     focalDepth;
    std::vector<vtkTextActor*> posTextActors;
    std::vector<std::string>   axesNames;
    std::vector<double>        axesMin;
    std::vector<double>        axesMax;
    std::vector<double>        axesXPos;
    double                     color[3];

    avtAxisRestrictionToolInterface Interface;

    HotPointVector     origHotPoints;

    bool               addedBbox;
    bool               textAdded;
};

#endif
