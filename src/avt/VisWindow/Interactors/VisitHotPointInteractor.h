// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_HOT_POINT_INTERACTOR_H
#define VISIT_HOT_POINT_INTERACTOR_H
#include <viswindow_exports.h>
#include <VisitInteractor.h>
#include <VisWindowTypes.h>

// Forward declarations.
struct HotPoint;
class Dolly3D;
class FlyThrough;
class Lineout2D;
class Navigate2D;
class Navigate3D;
class NavigateCurve;
class NavigateAxisArray;
class NullInteractor;
class Pick;
class Zoom2D;
class Zoom3D;
class ZoomCurve;
class ZoomAxisArray;

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
//    Hank Childs, Mon Mar 18 13:47:00 PST 2002
//    Comply with new interface from base class for better buttonpress control.
//
//    Kathleen Bonnell, Tue Apr 16 15:07:38 PDT 2002  
//    Added Lineout2D, ZoomCurve and Start/Stop CurveMode.
//   
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Removed arguments from all ButtonAction methods and OnMouseMove, 
//    in order to match vtk's new interactor api.  Added SetInteractor method.
//
//    Eric Brugger, Wed Oct 15 17:32:48 PDT 2003
//    Added NavigateCurve interactor.
//
//    Eric Brugger, Thu Oct 28 15:31:02 PDT 2004
//    Added FlyThrough interactor.
//
//    Eric Brugger, Tue Dec 28 07:50:45 PST 2004
//    Added Dolly3D interactor.
//
//    Kathleen Bonnell, Thu Jan 12 13:58:25 PST 2006 
//    Replaced Pick3D and Pick2D interactors with Pick.
//
//    Jeremy Meredith, Thu Jan 31 14:41:50 EST 2008
//    Added new AxisArray window mode.
//
//    Jeremy Meredith, Mon Feb  4 13:24:08 EST 2008
//    Added zoom interactor for AxisArray mode.
//
//    Eric Brugger, Tue Dec  9 14:44:12 PST 2008
//    Added the ParallelAxes window mode.
//
//    Eric Brugger, Mon Jun 24 13:36:10 PDT 2013
//    I modified the 2d and 3d zoom interactors to once again constrain the
//    zoom rectangle to a 1:1 ratio when zooming with the shift key and left
//    mouse button pressed. Pressing the ctrl key and the left mouse button
//    still pans the image. I corrected a bug where pressing the ctrl key and
//    the left mouse button would result in the window being stuck in pan mode
//    if the shift key was released before the left mouse button.
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
    virtual void      StartRightButtonAction();
    virtual void      EndRightButtonAction();
    virtual void      AbortRightButtonAction();

    virtual void      OnMouseMove();
    virtual void      OnTimer();

    virtual void      OnMouseWheelForward();
    virtual void      OnMouseWheelBackward();

    void              SetNullInteractor();

    void              Start2DMode(INTERACTION_MODE mode);
    void              Start3DMode(INTERACTION_MODE mode);
    void              StartCurveMode(INTERACTION_MODE mode);
    void              StartAxisArrayMode(INTERACTION_MODE mode);
    void              StartParallelAxesMode(INTERACTION_MODE mode);
    void              Stop2DMode();
    void              Stop3DMode();
    void              StopCurveMode();
    void              StopAxisArrayMode();
    void              StopParallelAxesMode();
private:
    VisitInteractor  *currentInteractor;

    bool             hotPointMode;
    HotPoint         currentHotPoint;

    Dolly3D          *dolly3D;
    FlyThrough       *flyThrough;
    Lineout2D        *lineout2D;
    Navigate3D       *navigate3D;
    Navigate2D       *navigate2D;
    NavigateCurve    *navigateCurve;
    NavigateAxisArray *navigateAxisArray;
    NullInteractor   *nullInteractor;
    Pick             *pick;
    Zoom2D           *zoom2D;
    Zoom3D           *zoom3D;
    ZoomCurve        *zoomCurve;
    ZoomAxisArray    *zoomAxisArray;

    void              SetInteractor(VisitInteractor *);
};

#endif
