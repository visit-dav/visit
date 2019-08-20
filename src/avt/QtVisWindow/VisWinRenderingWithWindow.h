// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         VisWinRenderingWithWindow.h                       //
// ************************************************************************* //

#ifndef VIS_WIN_RENDERING_WITH_WINDOW_H
#define VIS_WIN_RENDERING_WITH_WINDOW_H
#include <qtviswindow_exports.h>
#include <VisWinRendering.h>

class     vtkQtRenderWindow;

// ****************************************************************************
//  Class: VisWinRenderingWithWindow
//
//  Purpose:
//      A derived type of VisWinRendering that assumes that there will be a
//      window on the screen.
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2002
//
//  Modifications:
//    Sean Ahern, Mon May 20 13:33:37 PDT 2002
//    Added ability to raise/lower windows.
//
//    Brad Whitlock, Wed Jan 29 14:38:35 PST 2003
//    I added a method to create a toolbar.
//
//    Brad Whitlock, Wed Mar 12 09:25:18 PDT 2003
//    I added IsVisible, SetHideCallback, and SetShowCallback.
//
//    Brad Whitlock, Wed May 21 12:05:20 PDT 2003
//    I added an override of GetSize.
//
//    Mark C. Miller, 07Jul03
//    Renamed GetSize to GetWindowSize
//
//    Brad Whitlock, Wed Jan 7 14:43:19 PST 2004
//    I added SetCursorForMode.
//
//    Brad Whitlock, Tue Mar 16 09:46:11 PDT 2004
//    I added SetLargeIcons.
//
//    Jeremy Meredith, Tue Jul 17 16:37:04 EDT 2007
//    Added fullscreen support.
//
//    Brad Whitlock, Mon Aug 18 14:44:46 PDT 2008
//    Pass in the vtkQtRenderWindow that we'll be using.
//
//    Brad Whitlock, Tue Feb 22 15:07:17 PST 2011
//    Added methods that let us control the render window in more ways since
//    it is no longer a subclass of vtkRenderWindow.
//
//    Gunther H. Weber, Mon Jul 18 16:14:56 PDT 2011
//    Added ActivateWindow method
//
//    Brad Whitlock, Fri Oct 14 16:31:08 PDT 2011
//    Add mapper creation methods.
//
// ****************************************************************************

class QTVISWINDOW_API VisWinRenderingWithWindow : public VisWinRendering
{
  public:
                                       VisWinRenderingWithWindow(
                                           vtkQtRenderWindow *rw, 
                                           bool own,
                                           VisWindowColleagueProxy &);
    virtual                           ~VisWinRenderingWithWindow();

    virtual void                       SetSize(int, int);
    virtual void                       GetWindowSize(int&, int&);
    virtual void                       SetLocation(int, int);
    virtual void                       GetLocation(int&, int&);
    virtual void                       SetTitle(const char *);
    virtual void                       Iconify(void);
    virtual void                       DeIconify(void);
    virtual void                       Show(void);
    virtual void                       Hide(void);
    virtual void                       Raise(void);
    virtual void                       Lower(void);
    virtual void                       ActivateWindow(void);
    virtual bool                       IsVisible() const;

    virtual void                       SetResizeEvent(void(*callback)(void *), void *);
    virtual void                       SetCloseCallback(void(*callback)(void *), void *);
    virtual void                       SetHideCallback(void(*callback)(void *), void *);
    virtual void                       SetShowCallback(void(*callback)(void *), void *);

    virtual void                      *CreateToolbar(const char *);
    virtual void                       SetLargeIcons(bool);
    virtual void                       SetCursorForMode(INTERACTION_MODE);
    virtual void                       SetFullScreenMode(bool);

    virtual vtkPolyDataMapper2D *CreateRubberbandMapper();
    virtual vtkPolyDataMapper2D *CreateXorGridMapper();

  protected:
    vtkQtRenderWindow                 *renWin;

    virtual vtkRenderWindow           *GetRenderWindow(void);
    virtual vtkRenderWindowInteractor *GetRenderWindowInteractor(void);

    virtual void                       RealizeRenderWindow(void);

    int                                cursorIndex;
    bool                               fullScreenMode;
    bool                               ownRenderWindow;
    bool                               useLargeIcons;
};


#endif


