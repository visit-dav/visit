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
//                          VisWinInteractions.h                             //
// ************************************************************************* //

#ifndef VIS_WIN_INTERACTIONS_H
#define VIS_WIN_INTERACTIONS_H
#include <viswindow_exports.h>


#include <VisWinColleague.h>
#include <VisWindowInteractorProxy.h>

#include <VisWindowTypes.h>


class VisWindowColleagueProxy;
class VisWindowInteractorProxy;
class VisitHotPointInteractor;

// ****************************************************************************
//  Class: VisWinInteractions
//
//  Purpose:
//      This is a concrete colleague for the mediator VisWindow.  It controls
//      the interactor to the plots.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//     Brad Whitlock, Tue Nov 7 16:11:20 PST 2000
//     I added a method to query the interaction mode.
//
//     Brad Whitlock, Thu Nov 9 15:50:02 PST 2000
//     I added a method to set the bbox mode.
//
//     Hank Childs, Tue Sep 18 10:41:30 PDT 2001
//     Make destructor virtual.
//
//     Brad Whitlock, Mon Oct 1 14:41:22 PST 2001
//     Replaced all of the interactors with a single VisitHotPointInteractor.
//
//     Kathleen Bonnell, Wed May  8 14:06:50 PDT 2002
//     Add support for curve mode. 
//
//     Brad Whitlock, Wed Sep 10 15:27:35 PST 2003
//     Added support for temporarily suspending spin mode.
//
//     Jeremy Meredith, Thu Jan 31 14:41:50 EST 2008
//     Added new AxisArray window mode.
//
//     Eric Brugger, Tue Dec  9 14:17:09 PST 2008
//     Added the AxisParallel window mode.
//
// ****************************************************************************

class VISWINDOW_API VisWinInteractions : public VisWinColleague
{
  public:                    
                             VisWinInteractions(VisWindowColleagueProxy &,
                                                VisWindowInteractorProxy &);
    virtual                 ~VisWinInteractions();  
   
    void                     SetInteractionMode(INTERACTION_MODE);
    INTERACTION_MODE         GetInteractionMode() const;
    void                     SetBoundingBoxMode(bool);
    bool                     GetBoundingBoxMode() const;
    void                     SetSpinMode(bool);
    bool                     GetSpinMode() const;
    void                     SetSpinModeSuspended(bool);
    bool                     GetSpinModeSuspended() const;

    virtual void             Start2DMode();
    virtual void             Stop2DMode();
    virtual void             Start3DMode();
    virtual void             Stop3DMode();
    virtual void             StartCurveMode();
    virtual void             StopCurveMode();
    virtual void             StartAxisArrayMode();
    virtual void             StopAxisArrayMode();
    virtual void             StartAxisParallelMode();
    virtual void             StopAxisParallelMode();

    virtual void             NoPlots();
    virtual void             HasPlots();

  protected:
    INTERACTION_MODE         mode;
    bool                     bboxMode;
    bool                     spinMode;
    bool                     spinModeSuspended;
    VisitHotPointInteractor *hotPointInteractor;
};


#endif


