// ************************************************************************* //
//                                 ZoomCurve.h                                  //
// ************************************************************************* //

#ifndef ZOOM_CURVE_H
#define ZOOM_CURVE_H
#include <viswindow_exports.h>


#include <ZoomInteractor.h>


class VisWindowInteractorProxy;


// ****************************************************************************
//  Class: ZoomCurve
//
//  Purpose:
//      Defines what Visit's Curve Zoom interactions should look like.  
//
//  Programmer: Hank Childs
//  Creation:   May 22, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Mar 18 13:47:00 PST 2002
//    Comply with new interface from base class for better buttonpress control.
//
//    Eric Brugger, Tue Mar 26 14:45:12 PST 2002
//    Remove UpdateViewport and add ZoomCamera.
//
//    Eric Brugger, Fri Apr 12 12:58:37 PDT 2002
//    Add an overloaded ZoomCamera.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Removed arguments from all ButtonAction methods, in order to match
//    vtk's new interactor api.
//
// ****************************************************************************

class VISWINDOW_API ZoomCurve : public ZoomInteractor
{
  public:
                        ZoomCurve(VisWindowInteractorProxy &);
 
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


