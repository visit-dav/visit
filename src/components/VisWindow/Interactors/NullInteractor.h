// ************************************************************************* //
//                             NullInteractor.h                              //
// ************************************************************************* //

#ifndef NULL_INTERACTOR_H
#define NULL_INTERACTOR_H
#include <viswindow_exports.h>


#include <VisitInteractor.h>


class VisWindowInteractorProxy;


// ****************************************************************************
//  Class: NullInteractor
//
//  Purpose:
//      Defines a null interactions should look like (does nothing).
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Removed arguments from OnMouseMove, in order to match
//    vtk's new interactor api.
//
// ****************************************************************************

class VISWINDOW_API NullInteractor : public VisitInteractor
{
  public:
                        NullInteractor(VisWindowInteractorProxy &);
 
    virtual void        OnMouseMove();
};


#endif


