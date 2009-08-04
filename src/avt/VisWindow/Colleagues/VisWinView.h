/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                               VisWinView.h                                //
// ************************************************************************* //

#ifndef VIS_WIN_VIEW_H
#define VIS_WIN_VIEW_H
#include <viswindow_exports.h>


#include <VisWinColleague.h>


// ****************************************************************************
//  Class: VisWinView
//
//  Purpose:
//      Handles the view information and resetting of cameras.
//
//  Programmer: Hank Childs
//  Creation:   November 8, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Nov 10 10:19:14 PST 2000
//    Added code to support turning perspective projections on and off.
//
//    Hank Childs, Fri Jan  5 17:55:42 PST 2001
//    Removed antiquated method SetCameraToView.
//
//    Eric Brugger, Mon Mar 12 14:48:34 PST 2001
//    I removed the Get/SetPerspectiveProjection methods.
//
//    Kathleen Bonnell, Wed May  8 14:06:50 PDT 2002 
//    Added support for curve mode. 
//
//    Jeremy Meredith, Thu Jan 31 14:41:50 EST 2008
//    Added new AxisArray window mode.
//
//    Eric Brugger, Tue Dec  9 14:26:32 PST 2008
//    Added the AxisParallel window mode.
//
// ****************************************************************************

class VISWINDOW_API VisWinView : public VisWinColleague
{
  public:
                         VisWinView(VisWindowColleagueProxy &);
    virtual             ~VisWinView() {;};

    const avtViewInfo   &GetViewInfo(void);
    void                 SetViewInfo(const avtViewInfo &);

    void                 ResetView(void);

    virtual void         Start2DMode(void);
    virtual void         Stop2DMode(void);

    virtual void         StartCurveMode(void);
    virtual void         StopCurveMode(void);

    virtual void         StartAxisArrayMode(void);
    virtual void         StopAxisArrayMode(void);

    virtual void         StartAxisParallelMode(void);
    virtual void         StopAxisParallelMode(void);

  protected:
    avtViewInfo          viewInfo;
};


#endif


