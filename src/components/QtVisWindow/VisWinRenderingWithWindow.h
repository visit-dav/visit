/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                         VisWinRenderingWithWindow.h                       //
// ************************************************************************* //

#ifndef VIS_WIN_RENDERING_WITH_WINDOW_H
#define VIS_WIN_RENDERING_WITH_WINDOW_H
#include <qtviswindow_exports.h>
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
//    Jeremy Meredith, Tue Jul 17 16:37:04 EDT 2007
//    Added fullscreen support.
//
// ****************************************************************************

class QTVISWINDOW_API VisWinRenderingWithWindow : public VisWinRendering
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
    virtual void                       SetFullScreenMode(bool);

  protected:
    vtkQtRenderWindow                 *renWin;
    vtkQtRenderWindowInteractor       *iren;

    virtual vtkRenderWindow           *GetRenderWindow(void);
    virtual vtkRenderWindowInteractor *GetRenderWindowInteractor(void);

    virtual void                       RealizeRenderWindow(void);

    int                                cursorIndex;
    bool                               fullScreenMode;
};


#endif


