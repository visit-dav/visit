// ************************************************************************* //
//                                 Dolly3D.h                                 //
// ************************************************************************* //

#ifndef DOLLY_3D_H
#define DOLLY_3D_H
#include <viswindow_exports.h>


#include <VisitInteractor.h>


class VisWindowInteractorProxy;


// ****************************************************************************
//  Class: Dolly3D
//
//  Purpose:
//      Defines what Visit's 3D Dolly interactions should look like.
//
//  Programmer: Eric Brugger
//  Creation:   December 27, 2004
//
// ****************************************************************************

class VISWINDOW_API Dolly3D: public VisitInteractor
{
  public:
                        Dolly3D(VisWindowInteractorProxy &);
 
    virtual void        OnTimer(void);

    virtual void        StartLeftButtonAction();
    virtual void        EndLeftButtonAction();
    virtual void        StartMiddleButtonAction();
    virtual void        EndMiddleButtonAction();

  protected:
    bool                ctrlOrShiftPushed;
    bool                shouldSpin;

    void                EnableSpinMode(void);
    void                DisableSpinMode(void);
};

#endif
