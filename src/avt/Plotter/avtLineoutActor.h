// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef LINEOUT_ACTOR_H
#define LINEOUT_ACTOR_H

#include <plotter_exports.h>
#include <visitstream.h>
#include <ref_ptr.h>
#include <string>

class vtkActor;
class vtkLineSource;
class vtkPolyDataMapper;
class vtkRenderer;
class vtkFollower;


// ****************************************************************************
//  Class:  avtLineoutActor
//
//  Purpose:  Responsible for creating the visual cue actors for a Lineout. 
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 22, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Wed Jun 19 18:05:04 PDT 2002
//    Added SetLineWidth/Style, GetDesignator, and overloade Set Point methods
//    for doubles.
//
//    Kathleen Bonnell, Tue Oct  1 16:07:40 PDT 2002   
//    Added members 'hidden' and showLabels, methods GetAttachmentPoint, 
//    SetShowLabels.  Added two actors for labels to be positioned at the 
//    endpoints.
//
//    Kathleen Bonnell, Fri Jun  6 15:08:45 PDT 2003 
//    Added Translate and ResetPosition methods. 
//    
//    Kathleen Bonnell, Wed Jun 25 15:16:42 PDT 2003  
//    Changed arguments to SetAttachmentPoint, SetPoint2 from an array to
//    3 values.
//    
// ****************************************************************************

class PLOTTER_API avtLineoutActor
{
  public:
                       avtLineoutActor();
    virtual           ~avtLineoutActor();

    void               Add(vtkRenderer *ren);
    void               Remove();
    void               Hide();
    void               UnHide();

    void               SetAttachmentPoint(double x, double y, double z);
    const double      *GetAttachmentPoint() { return attach; };
    void               SetPoint2(double x, double y, double z);
    void               SetMode3D(const bool);
    void               SetScale(double);
    void               SetDesignator(const std::string &designator_);
    const std::string  GetDesignator() const;
          std::string  GetDesignator();
    void               SetForegroundColor(double fgr, double fgg, double fgb);
    void               SetForegroundColor(double fg[3]);
    void               UpdateView();
    void               Shift(const double vec[3]);
    void               Translate(const double vec[3]);
    void               ResetPosition(void);

    void               SetLineWidth(const int);
    void               SetShowLabels(const bool);

  protected:
    bool               hidden;
    bool               mode3D;
    bool               showLabels;
    double             attach[3];
    double             pt2[3];
    std::string        designator; 
    vtkFollower       *labelActor1;
    vtkFollower       *labelActor2;

    vtkActor          *lineActor;
    vtkLineSource     *lineSource;
    vtkPolyDataMapper *lineMapper;

    vtkRenderer       *renderer; 

};

typedef ref_ptr<avtLineoutActor> avtLineoutActor_p;

#endif
