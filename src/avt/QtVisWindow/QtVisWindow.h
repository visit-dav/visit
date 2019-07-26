// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              QtVisWindow.h                                //
// ************************************************************************* //

#ifndef QT_VIS_WINDOW_H
#define QT_VIS_WINDOW_H
#include <qtviswindow_exports.h>

#include <VisWindow.h>

class vtkQtRenderWindow;

// ****************************************************************************
//  Class: QtVisWindow
//
//  Purpose:
//      A vis window that uses Qt to do its windowing.
//
//  Programmer: Hank Childs
//  Creation:   March 4, 2004
//
//  Modifications:
//    Brad Whitlock, Wed Mar 24 12:23:47 PDT 2004
//    I made it build on Windows.
//
//    Jeremy Meredith, Tue Jul 17 16:35:37 EDT 2007
//    Added fullscreen support.
//
//    Brad Whitlock, Mon Aug 18 16:23:28 PDT 2008
//    Added a window creation callback.
//
// ****************************************************************************

class QTVISWINDOW_API QtVisWindow : public VisWindow
{
  public:
    QtVisWindow(bool fullScreenMode = false);

    static void SetWindowCreationCallback(vtkQtRenderWindow *(*wcc)(void*), void *wccdata);
    static void SetOwnerShipOfAllWindows(bool owner);
  protected:
    virtual void CreateToolColleague();

    static vtkQtRenderWindow* (*windowCreationCallback)(void *);
    static void                *windowCreationData;
    static bool               ownsAllWindows;
};

#endif


