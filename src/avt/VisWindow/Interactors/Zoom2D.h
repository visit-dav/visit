// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                 Zoom2D.h                                  //
// ************************************************************************* //

#ifndef ZOOM_2D_H
#define ZOOM_2D_H
#include <viswindow_exports.h>


#include <ZoomInteractor.h>

class vtkActor2D;
class vtkPolyData;
class vtkPolyDataMapper2D;
class VisWindowInteractorProxy;


// ****************************************************************************
//  Class: Zoom2D
//
//  Purpose:
//      Defines what Visit's 2D Zoom interactions should look like.  
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
//    Akira Haddox, Thu Jul  3 13:58:11 PDT 2003
//    Added drawing of guidelines.
//
//    Eric Brugger, Mon Jun 24 13:24:51 PDT 2013
//    I modified the 2d and 3d zoom interactors to once again constrain the
//    zoom rectangle to a 1:1 ratio when zooming with the shift key and left
//    mouse button pressed. Pressing the ctrl key and the left mouse button
//    still pans the image. I corrected a bug where pressing the ctrl key and
//    the left mouse button would result in the window being stuck in pan mode
//    if the shift key was released before the left mouse button.
//
//    Eric Brugger, Wed Oct  2 16:54:48 PDT 2024
//    I modified the class to use the APPLE path in all cases.
//
// ****************************************************************************

class VISWINDOW_API Zoom2D : public ZoomInteractor
{
  public:
                        Zoom2D(VisWindowInteractorProxy &);
    virtual             ~Zoom2D();
 
    virtual void        OnTimer(void);

    virtual void        StartLeftButtonAction();
    virtual void        EndLeftButtonAction();
    virtual void        AbortLeftButtonAction();
    virtual void        StartMiddleButtonAction();
    virtual void        EndMiddleButtonAction();
    virtual void        OnMouseWheelForward();
    virtual void        OnMouseWheelBackward();

  protected:
    vtkPolyData                 *guideLines;
    vtkPolyDataMapper2D         *guideLinesMapper;
    vtkActor2D                  *guideLinesActor;
    
    virtual void        StartRubberBand(int, int);
    virtual void        EndRubberBand();
    virtual void        UpdateRubberBand(int, int, int, int, int, int);
   
    void                DrawGuideLines(int, int, int, int, const bool which[8]);
    void                DrawGuideLine(int, int, int, int);

    void                PanCamera(const int x, const int y);
    void                ZoomCamera(void);
    void                ZoomCamera(const int x, const int y);

    bool                shiftPressed;
};


#endif


