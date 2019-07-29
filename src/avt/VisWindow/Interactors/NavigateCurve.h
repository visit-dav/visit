// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//  Modifications:
//    Brad Whitlock, Fri Mar  2 14:21:58 PST 2012
//    I added mouse wheel functions.
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
    virtual void        OnMouseWheelForward();
    virtual void        OnMouseWheelBackward();

  private:
    void                PanCamera(const int x, const int y);
    void                ZoomCamera(const int x, const int y);
    void                ZoomCamera(double);
};


#endif


