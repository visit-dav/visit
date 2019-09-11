// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               Navigate2D.h                                //
// ************************************************************************* //

#ifndef NAVIGATE_2D_H
#define NAVIGATE_2D_H
#include <viswindow_exports.h>


#include <VisitInteractor.h>


class VisWindowInteractorProxy;


// ****************************************************************************
//  Class: Navigate2D
//
//  Purpose:
//      Defines what Visit's 2D Navigation interactions should look like.  
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Mar 18 13:47:00 PST 2002
//    Comply with new interface from base class for better buttonpress control.
// 
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Removed arguments from all ButtonAction methods, in order to match
//    vtk's new interactor api.
//
//    Eric Brugger, Fri Oct 10 08:52:11 PDT 2003
//    I added OnTimer, PanCamera and ZoomCamera.
//
//    Gunther H. Weber, Tue Aug  7 13:46:29 PDT 2007
//    Added methods for mouse wheel interactions
//
// ****************************************************************************

class VISWINDOW_API Navigate2D : public VisitInteractor
{
  public:
                        Navigate2D(VisWindowInteractorProxy &);
 
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
};


#endif


