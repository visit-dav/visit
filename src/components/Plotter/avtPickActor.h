#ifndef PICK_ACTOR_H
#define PICK_ACTOR_H
#include <plotter_exports.h>
#include <iostream.h>
#include <ref_ptr.h>

class vtkActor;
class vtkFollower;
class vtkLineSource;
class vtkPolyDataMapper;
class vtkRenderer;


// ****************************************************************************
//  Class:  avtPickActor
//
//  Purpose:  Responsible for creating the visual cue actors for a Pick. 
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 22, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Wed May  8 13:50:44 PDT 2002
//    Moved ComputeScaleFactor to VisWinRendering.
//
//    Kathleen Bonnell, Fri Jun  6 15:08:45 PDT 2003  
//    Added Translate and ResetPosition methods. 
//
// ****************************************************************************

class PLOTTER_API avtPickActor
{
  public:
                       avtPickActor();
    virtual           ~avtPickActor();

    void               Add(vtkRenderer *ren);
    void               Remove();
    void               Hide();
    void               UnHide();

    void               SetAttachmentPoint(const float newPos[3]);
    const float *      GetAttachmentPoint() { return attach; };
    void               SetMode3D(const bool);
    void               SetScale(float);
    void               SetDesignator(const char *l);
    void               SetForegroundColor(float fgr, float fgg, float fgb);
    void               SetForegroundColor(float fg[3]);
    void               UpdateView();
    void               Shift(const float vec[3]);
    void               Translate(const float vec[3]);
    void               ResetPosition(const float vec[3]);

  protected:
    bool               mode3D;
    float              attach[3];
    vtkFollower       *letterActor;

    vtkActor          *lineActor;
    vtkLineSource     *lineSource;
    vtkPolyDataMapper *lineMapper;

    vtkRenderer       *renderer; 

  private:
};

typedef ref_ptr<avtPickActor> avtPickActor_p;

#endif
