// ************************************************************************* //
//                                 Pick2D.h                                  //
// ************************************************************************* //

#ifndef PICK_2D_H
#define PICK_2D_H
#include <viswindow_exports.h>


class VisWindowInteractorProxy;

#include <VisitInteractor.h>


// ****************************************************************************
//  Class: Pick2D
//
//  Purpose:
//      Defines what Visit's 2D Pick interactions should look like.  
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Mar 18 13:47:00 PST 2002
//    Comply with new interface from base class for better buttonpress control.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Removed arguments from StartLeftButtonAction, in order to match
//    vtk's new interactor api.
//
// ****************************************************************************

class VISWINDOW_API Pick2D : public VisitInteractor
{
  public:
                        Pick2D(VisWindowInteractorProxy &);
 
    virtual void        StartLeftButtonAction();
};


#endif


