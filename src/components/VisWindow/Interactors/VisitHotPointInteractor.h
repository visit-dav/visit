#ifndef VISIT_HOT_POINT_INTERACTOR_H
#define VISIT_HOT_POINT_INTERACTOR_H
#include <viswindow_exports.h>
#include <VisitInteractor.h>
#include <VisWindowTypes.h>

// Forward declarations.
struct HotPoint;
class Lineout2D;
class Navigate2D;
class Navigate3D;
class NullInteractor;
class Pick2D;
class Pick3D;
class Zoom2D;
class Zoom3D;
class ZoomCurve;

// ****************************************************************************
// Class: VisitHotPointInteractor
//
// Purpose:
//   This interactor is the main interactor that is installed in the VisWindow.
//   It keeps a set of other interactors around and delegates interaction
//   responsibilities to them or to tools whose hotpoints are clicked.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 2 13:57:17 PST 2001
//
// Modifications:
//
//   Hank Childs, Mon Mar 18 13:47:00 PST 2002
//   Comply with new interface from base class for better buttonpress control.
//
//   Kathleen Bonnell, Tue Apr 16 15:07:38 PDT 2002  
//   Added Lineout2D, ZoomCurve and Start/Stop CurveMode.
//   
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Removed arguments from all ButtonAction methods and OnMouseMove, 
//    in order to match vtk's new interactor api.  Added SetInteractor method.
//
// ****************************************************************************

class VISWINDOW_API VisitHotPointInteractor : public VisitInteractor
{
public:
                      VisitHotPointInteractor(VisWindowInteractorProxy &);
    virtual          ~VisitHotPointInteractor();

    virtual void      SetInteractor(vtkRenderWindowInteractor *rwi);
 
    virtual void      StartLeftButtonAction();
    virtual void      EndLeftButtonAction();
    virtual void      AbortLeftButtonAction();
    virtual void      StartMiddleButtonAction();
    virtual void      EndMiddleButtonAction();
    virtual void      AbortMiddleButtonAction();

    virtual void      OnMouseMove();
    virtual void      OnTimer();

    void              SetNullInteractor();

    void              Start2DMode(INTERACTION_MODE mode);
    void              Start3DMode(INTERACTION_MODE mode);
    void              StartCurveMode(INTERACTION_MODE mode);
    void              Stop2DMode();
    void              Stop3DMode();
    void              StopCurveMode();
private:
    VisitInteractor  *currentInteractor;

    bool             hotPointMode;
    HotPoint         currentHotPoint;

    Lineout2D        *lineout2D;
    Navigate3D       *navigate3D;
    Navigate2D       *navigate2D;
    NullInteractor   *nullInteractor;
    Pick2D           *pick2D;
    Pick3D           *pick3D;
    Zoom2D           *zoom2D;
    Zoom3D           *zoom3D;
    ZoomCurve        *zoomCurve;

    void              SetInteractor(VisitInteractor *);
};

#endif
