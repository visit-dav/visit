// ************************************************************************* //
//                              NavigateCurve.h                              //
// ************************************************************************* //

#ifndef NAVIGATE_CURVE_H
#define NAVIGATE_CURVE_H
#include <viswindow_exports.h>


#include <VisitInteractor.h>


class VisWindowInteractorProxy;


// ****************************************************************************
//  Class: NavigateCurve
//
//  Purpose:
//      Defines what Visit's Curve Navigation interactions should look like.  
//
//  Programmer: Eric Brugger
//  Creation:   October 15, 2003
//
// ****************************************************************************

class VISWINDOW_API NavigateCurve : public VisitInteractor
{
  public:
                        NavigateCurve(VisWindowInteractorProxy &);
 
    virtual void        OnTimer(void);

    virtual void        StartLeftButtonAction();
    virtual void        EndLeftButtonAction();
    virtual void        StartMiddleButtonAction();
    virtual void        EndMiddleButtonAction();

  private:
    void                PanCamera(const int x, const int y);
    void                ZoomCamera(const int x, const int y);
};


#endif


