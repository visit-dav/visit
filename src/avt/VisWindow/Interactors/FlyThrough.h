// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               FlyThrough.h                                //
// ************************************************************************* //

#ifndef FLY_THROUGH_H
#define FLY_THROUGH_H
#include <viswindow_exports.h>


#include <VisitInteractor.h>


class VisWindowInteractorProxy;


// ****************************************************************************
//  Class: FlyThrough
//
//  Purpose:
//      Defines what Visit's 3D Fly Through interactions should look like.
//
//  Programmer: Eric Brugger
//  Creation:   October 28, 2004
//
//  Modifications:
//    Eric Brugger, Tue Dec 28 16:47:22 PST 2004
//    Moved RotateCamera, PanCamera and ZoomCamera to the VisitInterator
//    class as RotateAboutCamera3D, PanCamera3D and DollyCameraAndFocus3D.
//
//    Kathleen Biagas, Wed Mar 16, 2022
//    Added OnMouseWheelForward,OnMouseWheelbackward.
//
// ****************************************************************************

class VISWINDOW_API FlyThrough: public VisitInteractor
{
  public:
                        FlyThrough(VisWindowInteractorProxy &);
 
    virtual void        OnTimer(void);

    virtual void        StartLeftButtonAction();
    virtual void        EndLeftButtonAction();
    virtual void        StartMiddleButtonAction();
    virtual void        EndMiddleButtonAction();
    virtual void        OnMouseWheelForward();
    virtual void        OnMouseWheelBackward();

  protected:
    bool                ctrlOrShiftPushed;
    bool                shouldSpin;
    float               spinOldX, spinOldY;
    int                 spinNewX, spinNewY;

    void                EnableSpinMode(void);
    void                DisableSpinMode(void);
};

#endif
