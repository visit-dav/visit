#ifndef LINEOUT_ACTOR_H
#define LINEOUT_ACTOR_H

#include <plotter_exports.h>
#include <iostream.h>
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

    void               SetAttachmentPoint(const float newPos[3]);
    void               SetAttachmentPoint(const double newPos[3]);
    const float       *GetAttachmentPoint() { return attach; };
    void               SetPoint2(const float newPos[3]);
    void               SetPoint2(const double newPos[3]);
    void               SetMode3D(const bool);
    void               SetScale(float);
    void               SetDesignator(const std::string &designator_);
    const std::string  GetDesignator() const;
          std::string  GetDesignator();
    void               SetForegroundColor(float fgr, float fgg, float fgb);
    void               SetForegroundColor(float fg[3]);
    void               UpdateView();
    void               Shift(const float vec[3]);
    void               Translate(const float vec[3]);
    void               ResetPosition(void);

    void               SetLineStyle(const int);
    void               SetLineWidth(const int);
    void               SetShowLabels(const bool);

  protected:
    bool               hidden;
    bool               mode3D;
    bool               showLabels;
    float              attach[3];
    float              pt2[3];
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
