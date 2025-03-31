// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//    Kathleen Bonnell, Wed Aug 18 10:10:35 PDT 2004 
//    Added two flags for when InteractorAtts 'ClampToSquare' and 
//    'ShowGuidelines' are set. 
//
//    Eric Brugger, Wed Oct  2 16:54:48 PDT 2024
//    I modified the class to use the APPLE path in all cases.
//
// ****************************************************************************

class VISWINDOW_API ZoomInteractor : public VisitInteractor
{
  public:
                           ZoomInteractor(VisWindowInteractorProxy &);
    virtual               ~ZoomInteractor();

    virtual void           OnMouseMove();
 
  protected:
    int                    anchorX, anchorY;

    double                 canvasDeviceMinX, canvasDeviceMaxX;
    double                 canvasDeviceMinY, canvasDeviceMaxY;

    vtkPolyData           *rubberBand;
    vtkPolyDataMapper2D   *rubberBandMapper;
    vtkActor2D            *rubberBandActor;

    bool                   rubberBandMode;
    bool                   rubberBandDrawn;
        
    bool                   shiftKeyDown;
    bool                   controlKeyDown;
    bool                   shouldClampSquare;
    bool                   shouldDrawGuides;

    virtual void           StartRubberBand(int, int);
    virtual void           EndRubberBand();
    virtual void           UpdateRubberBand(int, int, int, int, int, int);

    void                   SetCanvasViewport(void);
    void                   ForceCoordsToViewport(int &, int &);

    virtual void           ZoomCamera(void)=0;
    virtual void           ZoomCamera(const int x, const int y)=0;

    virtual bool           SufficientDistanceMoved();
};


#endif


