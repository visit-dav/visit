// ************************************************************************* //
//                              avtLabelActor.h                              //
// ************************************************************************* //


#ifndef AVT_LABEL_ACTOR_H
#define AVT_LABEL_ACTOR_H
#include <plotter_exports.h>
#include <iostream.h>
#include <ref_ptr.h>

class vtkFollower;
class vtkPolyDataMapper;
class vtkRenderer;


// ****************************************************************************
//  Class:  avtLabelActor
//
//  Purpose:  Responsible for creating label actors used in decorations. 
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    July 12, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Jul 19 08:39:04 PDT 2002
//    Add ComputeScaleFactor.
//
// ****************************************************************************

class PLOTTER_API avtLabelActor
{
  public:
                       avtLabelActor();
    virtual           ~avtLabelActor();

    void               Add(vtkRenderer *ren);
    void               Remove();
    void               Hide();
    void               UnHide();

    void               SetAttachmentPoint(const float newPos[3]);
    const float *      GetAttachmentPoint() { return attach; };
    void               SetScale(float);
    void               SetDesignator(const char *l);
    void               SetForegroundColor(float fgr, float fgg, float fgb);
    void               SetForegroundColor(float fg[3]);
    void               Shift(const float vec[3]);
    float              ComputeScaleFactor();

  protected:
    float              attach[3];
    vtkFollower       *labelActor;

    vtkRenderer       *renderer; 

  private:
};

typedef ref_ptr<avtLabelActor> avtLabelActor_p;

#endif
