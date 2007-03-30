// ************************************************************************* //
//                            VisWindowManager.h                             //
// ************************************************************************* //

#ifndef VIS_WINDOW_MANAGER_H
#define VIS_WINDOW_MANAGER_H
#include <viswindow_exports.h>

#include <X11/Intrinsic.h>

#include <VisWindow.h>


// ****************************************************************************
//  Class: VisWindow
//
//  Purpose:
//      Manages the visualization windows.
//
//  Programmer: Hank Childs
//  Creation:   May 15, 2000
//
// ****************************************************************************

class VISWINDOW_API
VisWindowManager
{
  public:
                                  VisWindowManager(int *, char **);
    virtual                      ~VisWindowManager();

    int                           AddWindow();
    VisWindow                    *GetWindow(int);

    void                          Start();

  protected:
    VisWindow                   **windows;
    int                           iWindows;
    int                           nWindows;
    XtAppContext                  app;
};


#endif


