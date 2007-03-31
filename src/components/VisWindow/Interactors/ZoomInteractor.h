// ************************************************************************* //
//                             ZoomInteractor.h                              //
// ************************************************************************* //

#ifndef ZOOM_INTERACTOR_H
#define ZOOM_INTERACTOR_H
#include <viswindow_exports.h>
#include <VisitInteractor.h>

class vtkActor2D;
class vtkPolyData;
class vtkPolyDataMapper2D;
class VisWindowInteractorProxy;


// ****************************************************************************
//  Class: ZoomInteractor
//
//  Purpose:
//      An abstract type (conceptually) that has two concrete types, Zoom2D
//      and Zoom3D.  It captures the common routines for the two and defines
//      them in one spot.
//
//  Programmer: Hank Childs
//  Creation:   May 22, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Dec 21 08:35:12 PST 2001
//    Add OnTimer.
//
//    Hank Childs, Mon Mar 25 16:06:00 PST 2002
//    Remove lastX and lastY.  They were defined by the base class and being
//    erroneously re-defined here.
//
//    Eric Brugger, Tue Mar 26 14:33:43 PST 2002
//    Remove UpdateViewport and make ZoomCamera pure virtual.
//
//    Eric Brugger, Fri Apr 12 14:10:16 PDT 2002
//    Add an overloaded pure virtual ZoomCamera.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Removed arguments from all OnMouseMove, in order to match
//    vtk's new interactor api.
//
//    Akira Haddox, Thu Jul  3 14:15:48 PDT 2003
//    Added two flags for when control and shift are set.
//    Made Start/End/UpdateRubberBand virtual (overloaded in Zoom2D).
//
// ****************************************************************************

class VISWINDOW_API ZoomInteractor : public VisitInteractor
{
  public:
                           ZoomInteractor(VisWindowInteractorProxy &);
    virtual               ~ZoomInteractor();

    virtual void           OnMouseMove();
    virtual void           OnTimer(void);
 
  protected:
    int                    anchorX, anchorY;

    float                  canvasDeviceMinX, canvasDeviceMaxX;
    float                  canvasDeviceMinY, canvasDeviceMaxY;

    vtkPolyData           *rubberBand;
    vtkPolyDataMapper2D   *rubberBandMapper;
    vtkActor2D            *rubberBandActor;

    bool                   rubberBandMode;
    bool                   rubberBandDrawn;
        
    bool                   shiftKeyDown;
    bool                   controlKeyDown;

    virtual void           StartRubberBand(int, int);
    virtual void           EndRubberBand();
    virtual void           UpdateRubberBand(int, int, int, int, int, int);
    virtual void           DrawRubberBandLine(int, int, int, int);

    void                   SetCanvasViewport(void);
    void                   ForceCoordsToViewport(int &, int &);

    virtual void           ZoomCamera(void)=0;
    virtual void           ZoomCamera(const int x, const int y)=0;

};


#endif


