// ************************************************************************* //
//                                  Zoom3D.h                                   //
// ************************************************************************* //

#ifndef ZOOM_3D_H
#define ZOOM_3D_H
#include <viswindow_exports.h>


#include <ZoomInteractor.h>


class VisWindowInteractorProxy;


// ****************************************************************************
//  Class: Zoom3D
//
//  Purpose:
//      Defines what Visit's 3D Zoom interactions should look like.  
//
//  Programmer: Hank Childs
//  Creation:   May 22, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Mar 18 13:47:00 PST 2002
//    Comply with new interface from base class for better buttonpress control.
//
//    Eric Brugger, Tue Mar 26 14:52:43 PST 2002
//    Add ZoomCamera.
//    
//    Eric Brugger, Fri Apr 12 12:31:05 PDT 2002
//    Add an overloaded ZoomCamera.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Removed arguments from all ButtonAction methods, in order to match
//    vtk's new interactor api.
//
// ****************************************************************************

class VISWINDOW_API Zoom3D : public ZoomInteractor
{
  public:
                        Zoom3D(VisWindowInteractorProxy &);

    virtual void        StartLeftButtonAction();
    virtual void        EndLeftButtonAction();
    virtual void        AbortLeftButtonAction();
    virtual void        StartMiddleButtonAction();
    virtual void        EndMiddleButtonAction();

  protected:
    void                ZoomCamera(void);
    void                ZoomCamera(const int x, const int y);
};


#endif


