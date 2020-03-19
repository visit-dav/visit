// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       VisWinRenderingWithoutWindow.h                      //
// ************************************************************************* //

#ifndef VIS_WIN_RENDERING_WITHOUT_WINDOW_H
#define VIS_WIN_RENDERING_WITHOUT_WINDOW_H
#include <viswindow_exports.h>
#include <VisWinRendering.h>


class     vtkRenderWindow;


// ****************************************************************************
//  Class: VisWinRenderingWithoutWindow
//
//  Purpose:
//      A derived type of VisWinRendering that assumes that there will be no
//      window on the screen.
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2002
//
//  Modifications:
//      Sean Ahern, Mon May 20 13:35:16 PDT 2002
//      Added empty functions for Raise/Lower.
//
//      Kathleen Bonnell, Tue Feb 11 11:28:03 PST 2003  
//      Removed member 'iren'.  Made GetRenderWindowInteractor return NULL. 
//
//      Tom Fogal, Wed Dec  9 15:44:56 MST 2009
//      Define RealizeRenderWindow.
//
//      Gunther H. Weber, Mon Jul 18 16:18:35 PDT 2011
//      Add ActivateWindow.
//
//      Brad Whitlock, Fri Sep 30 18:13:20 PDT 2011
//      Override SetImmediateModeRendering.
//
//      Brad Whitlock, Wed Mar 13 16:08:08 PDT 2013
//      Add RenderRenderWindow.
//
//      Burlen Loring, Fri Oct 16 15:42:50 PDT 2015
//      Fix a couple of compiler warnings
//
// ****************************************************************************

class VISWINDOW_API VisWinRenderingWithoutWindow : public VisWinRendering
{
  public:
    VisWinRenderingWithoutWindow(VisWindowColleagueProxy &);
    virtual ~VisWinRenderingWithoutWindow();

    virtual void Iconify(void) {;};
    virtual void DeIconify(void) {;};
    virtual void Show(void) {;};
    virtual void Hide(void) {;};
    virtual void Raise(void) {;};
    virtual void Lower(void) {;};
    virtual void ActivateWindow(void) {;};

    virtual void SetResizeEvent(void(*)(void *), void *) {;};
    virtual void SetCloseCallback(void(*)(void *), void *) {;};

  protected:
    vtkRenderWindow                   *renWin;
    int                                displayStatus;

    virtual vtkRenderWindow           *GetRenderWindow(void);
    virtual vtkRenderWindowInteractor *GetRenderWindowInteractor(void)
                                           { return NULL; };

    virtual void                       RealizeRenderWindow(void);
    virtual void                       RenderRenderWindow(void);
};
#endif
