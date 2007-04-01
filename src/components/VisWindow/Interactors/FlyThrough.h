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

  protected:
    bool                ctrlOrShiftPushed;
    bool                shouldSpin;
    float               spinOldX, spinOldY;
    int                 spinNewX, spinNewY;

    void                EnableSpinMode(void);
    void                DisableSpinMode(void);

  private:
    void                RotateCamera(const int x, const int y);
    void                PanCamera(const int x, const int y);
    void                ZoomCamera(const int x, const int y);
};

#endif
