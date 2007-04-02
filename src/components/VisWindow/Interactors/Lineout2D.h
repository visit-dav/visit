// ************************************************************************* //
//                                 Lineout2D.h                               //
// ************************************************************************* //

#ifndef LINEOUT_2D_H
#define LINEOUT_2D_H

#include <VisitInteractor.h>


class vtkActor2D;
class vtkPolyData;
class vtkPolyDataMapper2D;

class VisWindowInteractorProxy;


// ****************************************************************************
//  Class: Lineout2D
//
//  Purpose:
//      Defines what Visit's 2D Lineout interactions should look like.  
//
//  Note:  Modified from Zoom2D.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 16, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Removed arguments from all ButtonAction methods and OnMouseMove, 
//    in order to match vtk's new interactor api.
//
//    Kathleen Bonnell, Tue Feb 18 15:14:15 PST 2003     
//    Added OnTimer method.
//    
// ****************************************************************************

class VISWINDOW_API Lineout2D : public VisitInteractor
{
  public:
                        Lineout2D(VisWindowInteractorProxy &);
    virtual            ~Lineout2D();
 
    virtual void        OnMouseMove();
    virtual void        OnTimer();
    virtual void        StartLeftButtonAction();
    virtual void        EndLeftButtonAction();
    virtual void        AbortLeftButtonAction();

  protected:
    int                    anchorX, anchorY;

    double                  canvasDeviceMinX, canvasDeviceMaxX;
    double                  canvasDeviceMinY, canvasDeviceMaxY;

    vtkPolyData           *rubberBand;
    vtkPolyDataMapper2D   *rubberBandMapper;
    vtkActor2D            *rubberBandActor;

    bool                   rubberBandMode;
    bool                   doAlign;
      
    void                   StartRubberBand(int, int);
    void                   EndRubberBand();
    void                   UpdateRubberBand(int, int, int, int, int, int);
    void                   DrawRubberBandLine(int, int, int, int);

    void                   SetCanvasViewport(void);
    void                   ForceCoordsToViewport(int &, int &);
    void                   Lineout(void);
    void                   AlignToAxis(int &, int &);

};


#endif




