// ************************************************************************* //
//                               Navigate3D.h                                //
// ************************************************************************* //

#ifndef NAVIGATE_3D_H
#define NAVIGATE_3D_H
#include <viswindow_exports.h>


#include <VisitInteractor.h>


class VisWindowInteractorProxy;


// ****************************************************************************
//  Class: Navigate3D
//
//  Purpose:
//      Defines what Visit's 3D Navigation interactions should look like.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//
//    Eric Brugger, Fri Aug 10 11:37:14 PDT 2001
//    I added the OnTimer method to override vtk's default trackball
//    behaviour.  I added a bunch of other private methods to support
//    trackball.
//
//    Hank Childs, Mon Mar 18 13:47:00 PST 2002
//    Comply with new interface from base class for better buttonpress control.
//
//    Eric Brugger, Thu Apr 11 16:10:06 PDT 2002
//    I moved the vector and matrix manipulation methods from Navigate3D
//    to VisitInteractor.
//
//    Hank Childs, Wed May 29 10:03:18 PDT 2002
//    Added support for a 'spin' mode.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Removed arguments from all ButtonAction methods, in order to match
//    vtk's new interactor api.
//
//    Eric Brugger, Tue Feb 10 09:08:06 PST 2004
//    Removed the quaternion methods since they are in the matrix class.
//
// ****************************************************************************

class VISWINDOW_API Navigate3D : public VisitInteractor
{
  public:
                        Navigate3D(VisWindowInteractorProxy &);
 
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
