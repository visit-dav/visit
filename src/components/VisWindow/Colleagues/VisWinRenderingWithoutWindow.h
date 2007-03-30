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
// ****************************************************************************

class VISWINDOW_API VisWinRenderingWithoutWindow : public VisWinRendering
{
  public:
                                       VisWinRenderingWithoutWindow(
                                                    VisWindowColleagueProxy &);
    virtual                           ~VisWinRenderingWithoutWindow();

    virtual void                       Iconify(void) {;};
    virtual void                       DeIconify(void) {;};
    virtual void                       Show(void) {;};
    virtual void                       Hide(void) {;};
    virtual void                       Raise(void) {;};
    virtual void                       Lower(void) {;};

    virtual void                       SetResizeEvent(void(*callback)(void *), void *){;};
    virtual void                       SetCloseCallback(void(*callback)(void *), void *)
                                                                           {;};

  protected:
    vtkRenderWindow                   *renWin;

    virtual vtkRenderWindow           *GetRenderWindow(void);
    virtual vtkRenderWindowInteractor *GetRenderWindowInteractor(void)
                                           { return NULL; };

    virtual void                       RealizeRenderWindow(void) {;};
};


#endif


