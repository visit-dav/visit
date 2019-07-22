// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_LINE3D_COLLEAGUE_H
#define AVT_LINE3D_COLLEAGUE_H

#include <ColorAttribute.h>
#include <avtAnnotationColleague.h>
#include <viswindow_exports.h>

class vtkActor;
class vtkConeSource;
class vtkLineSource;
class vtkPolyDataMapper;
class vtkTubeFilter;

// ****************************************************************************
// Class: avtLine3DColleague
//
// Purpose:
//   This colleague is a 3D line that can be shown in the vis window.
//
// Notes:      
//
// Programmer: Kathleen Biagas 
// Creation:   July 13, 2015 
//
// Modifications:
//   Kathleen Biagas, Tue Jul 14 16:35:47 PDT 2015
//   Add support for arrows and tube.
//
// ****************************************************************************

class VISWINDOW_API avtLine3DColleague : public avtAnnotationColleague
{
public:
    avtLine3DColleague(VisWindowColleagueProxy &);
    virtual ~avtLine3DColleague();

    virtual void AddToRenderer();
    virtual void RemoveFromRenderer();
    virtual void Hide();

    virtual std::string TypeName() const { return "Line3D"; }

    // Methods to set and get the annotation's properties.
    virtual void SetOptions(const AnnotationObject &annot);
    virtual void GetOptions(AnnotationObject &annot);

    // Methods that are called in response to vis window events.
    virtual void SetForegroundColor(double r, double g, double b);
    virtual void HasPlots(void);
    virtual void NoPlots(void);

protected:
    vtkLineSource       *lineSource;
    vtkConeSource       *arrow1Source;
    vtkConeSource       *arrow2Source;
    vtkPolyDataMapper   *lineMapper;
    vtkPolyDataMapper   *arrow1Mapper;
    vtkPolyDataMapper   *arrow2Mapper;
    vtkActor            *lineActor;
    vtkActor            *arrow1Actor;
    vtkActor            *arrow2Actor;
    vtkTubeFilter       *tubeFilter;

    bool                 addedToRenderer;
    bool                 useForegroundForLineColor;
    bool                 useArrow1;
    bool                 useArrow2;
    bool                 arrow1Added;
    bool                 arrow2Added;
    int                  lineType;
    ColorAttribute       lineColor;

    bool ShouldBeAddedToRenderer() const;

};


#endif


