// ************************************************************************* //
//                         VisWinRenderingWithWindow.h                       //
// ************************************************************************* //

#ifndef VIS_WIN_RENDERING_WITH_WINDOW_H
#define VIS_WIN_RENDERING_WITH_WINDOW_H
#include <viswindow_exports.h>
#include <VisWinRendering.h>


class     vtkQtRenderWindow;
class     vtkQtRenderWindowInteractor;


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
// ****************************************************************************

class VISWINDOW_API VisWinRenderingWithWindow : public VisWinRendering
{
  public:
                                       VisWinRenderingWithWindow(
                                                    VisWindowColleagueProxy &);
    virtual                           ~VisWinRenderingWithWindow();

    virtual void                       GetWindowSize(int &w, int &h);
    virtual void                       Iconify(void);
    virtual void                       DeIconify(void);
    virtual void                       Show(void);
    virtual void                       Hide(void);
    virtual void                       Raise(void);
    virtual void                       Lower(void);
    virtual bool                       IsVisible() const;

    virtual void                       SetResizeEvent(void(*callback)(void *), void *);
    virtual void                       SetCloseCallback(void(*callback)(void *), void *);
    virtual void                       SetHideCallback(void(*callback)(void *), void *);
    virtual void                       SetShowCallback(void(*callback)(void *), void *);

    virtual void                      *CreateToolbar(const char *);
    virtual void                       SetLargeIcons(bool);
    virtual void                       SetCursorForMode(INTERACTION_MODE);

  protected:
    vtkQtRenderWindow                 *renWin;
    vtkQtRenderWindowInteractor       *iren;

    virtual vtkRenderWindow           *GetRenderWindow(void);
    virtual vtkRenderWindowInteractor *GetRenderWindowInteractor(void);

    virtual void                       RealizeRenderWindow(void);

    int                                cursorIndex;
};


#endif


