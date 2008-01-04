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
//                               VisWinFrame.h                               //
// ************************************************************************* //

#ifndef VIS_WIN_FRAME_H
#define VIS_WIN_FRAME_H
#include <viswindow_exports.h>


#include <VisWinColleague.h>


class vtkVisItAxisActor2D;


// ****************************************************************************
//  Class: VisWinFrame
//
//  Purpose:
//      This is a concrete colleague for the mediator VisWindow.  It creates
//      a frame around the viewport when the VisWindow is in 2D mode.
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Jul 25 13:30:59 PDT 2000
//    Added SetViewport method and put frame on background instead of canvas.
//
//    Kathleen Bonnell, Mon Jun 18 14:56:09 PDT 2001 
//    Added methods related to user-interaction: UpdateView, SetVisibility,
//    SetTopRightTickVisibility, SetTickLocation, GetRange. 
//
//    Eric Brugger, Tue Nov  6 12:38:05 PST 2002
//    Added methods SetAutoSetTicks, SetMajorTickMinimum, SetMajorTickMaximum,
//    SetMajorTickSpacing and SetMinorTickSpacing.
//    
//    Eric Brugger, Wed Jun 25 15:36:16 PDT 2003
//    I added SetLineWidth.
//
// ****************************************************************************

class VISWINDOW_API VisWinFrame : public VisWinColleague
{
  public:
                                 VisWinFrame(VisWindowColleagueProxy &);
    virtual                     ~VisWinFrame();

    virtual void                 SetForegroundColor(double, double, double);
    virtual void                 SetViewport(double, double, double, double);
    virtual void                 UpdateView(void);

    virtual void                 Start2DMode(void);
    virtual void                 Stop2DMode(void);

    virtual void                 HasPlots(void);
    virtual void                 NoPlots(void);

    void                         SetVisibility(int);
    void                         SetTopRightTickVisibility(int);
    void                         SetTickLocation(int);
    void                         SetAutoSetTicks(int);
    void                         SetMajorTickMinimum(double, double);
    void                         SetMajorTickMaximum(double, double);
    void                         SetMajorTickSpacing(double, double);
    void                         SetMinorTickSpacing(double, double);
    void                         SetLineWidth(int);

  protected:
    vtkVisItAxisActor2D          *leftBorder, *rightBorder, *topBorder,
                                *bottomBorder;

    bool                         addedFrame;

    void                         AddFrameToWindow(void);
    void                         RemoveFrameFromWindow(void);
    void                         GetRange(double &, double &, double &, double &); 
    bool                         ShouldAddFrame(void);
};


#endif


