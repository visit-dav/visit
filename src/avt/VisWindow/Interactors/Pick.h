// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                 Pick.h                                    //
// ************************************************************************* //

#ifndef PICK__H
#define PICK__H
#include <viswindow_exports.h>
#include <queue>


class VisWindowInteractorProxy;

#include <VisitInteractor.h>


// ****************************************************************************
//  Class: Pick
//
//  Purpose:
//      Defines what Visit's  Pick interactions should look like.  
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
//    Kathleen Bonnell, Thu Jan 12 14:03:39 PST 2006 
//    Renamed from Pick3D.  Added private members picking, handlingCache,
//    pickCache and methods HandlePickCache and DoPick.
//
//    Kathleen Bonnell, Tue Mar  7 08:27:25 PST 2006 
//    Added EndLeftButtonAction. 
// 
// ****************************************************************************

class VISWINDOW_API Pick : public VisitInteractor
{
  public:
                        Pick(VisWindowInteractorProxy &);
 
    virtual void        StartLeftButtonAction();
    virtual void        EndLeftButtonAction();

  private:
    bool                picking;
    bool                handlingCache;
    std::queue<int>     pickCache;
    void                HandlePickCache();
    void                DoPick(int x, int y);
};


#endif


