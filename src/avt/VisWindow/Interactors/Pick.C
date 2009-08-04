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
//                                  Pick.C                                   //
// ************************************************************************* //

#include <Pick.h>
#include <VisWindowInteractorProxy.h>
#include <vtkRenderWindowInteractor.h>

using std::queue;

// ****************************************************************************
//  Method: Pick constructor
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2000
//
//  Modifications:
//    Kathleen Bonnell, Thu Jan 12 14:03:39 PST 2006
//    Initialize picking and handlingCache.
//
// ****************************************************************************

Pick::Pick(VisWindowInteractorProxy &v) : VisitInteractor(v)
{
    picking = false;
    handlingCache = false;
}


// ****************************************************************************
//  Method: Pick::StartLeftButtonAction
//
//  Purpose:
//      Handles the left button being pushed down.  For Pick, this means
//      performing a pick.
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Jun 29 16:08:05 PDT 200
//    Added downcast to picker type from abstract picker from the render window
//    to account for VTK changes.
//
//    Hank Childs, Wed Jul  5 16:15:09 PDT 2000
//    Removed picker argument to VisWindow's pick.
//
//    Kathleen Bonnell, Wed Nov 12 10:54:55 PST 2001 
//    Uncomment proxy.Pick. 
//
//    Hank Childs, Mon Mar 18 13:48:55 PST 2002
//    Renamed from OnLeftButtonDown.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Removed arguments to match vtk's new interactor api, retrieve them
//    directly from the RenderWindowInteractor.
//
//    Kathleen Bonnell, Thu Jan 12 14:03:39 PST 2006 
//    Added logic to prevent a call to proxy->Pick when the current Pick has
//    not yet completed (can cause hang).  Added logic to cache picks and
//    handle them.
//
//    Kathleen Bonnell, Mon Mar  6 14:44:19 PST 2006 
//    Moved execution of the actual pick to EndLeftButtonAction. As pick
//    can reset InteractionMode, and it isn't good for that to happen 
//    in-between a Start and End action.
//
// ****************************************************************************

void
Pick::StartLeftButtonAction()
{
    int x, y;
    Interactor->GetEventPosition(x, y);
    pickCache.push(x);
    pickCache.push(y);
}


// ****************************************************************************
//  Method: Pick::DoPick
//
//  Purpose:
//    Call's the VisWindow's routine to handle pick. 
//
//  Arguments:
//    x         The x screen position that was picked.
//    y         The y screen position that was picked.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   January 12, 2006 
//
//  Modifications:
//
// ****************************************************************************

void
Pick::DoPick(int x, int y)
{
    picking = true;
    //
    // Just call the VisWindow's routine to do this.
    //
    proxy.Pick(x, y);
    picking = false;
}


// ****************************************************************************
//  Method: Pick::HandlePickCache
//
//  Purpose:
//    Performs pick for all the x,y values stored in the pick cache.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   January 12, 2006 
//
//  Modifications:
//
// ****************************************************************************

void
Pick::HandlePickCache()
{
    handlingCache = true;
    while (!pickCache.empty())
    {
        int x, y;
        x = pickCache.front();
        pickCache.pop();
        y = pickCache.front();
        pickCache.pop();
        DoPick(x, y);
    }
    handlingCache = false;
}


// ****************************************************************************
//  Method: Pick::EndLeftButtonAction
//
//  Purpose:
//    Handles the left button being released, meaning it is time to perform
//    the pick.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 6, 2006 
//
//  Modifications:
//    Kathleen Bonnell, Fri Apr 28 10:29:03 PDT 2006
//    Set leftButtonDown to false before handling pick cache.  (Normally set
//    to false by VisitInteractor after EndLeftButtonAction is complete.
//    If pick requires a re-execution, then SetInteractor will be called in the
//    course of the re-execution, and a second pick will be cached, 
//    unnecessarily, unless leftButtonDown is false.
//
// ****************************************************************************

void
Pick::EndLeftButtonAction()
{
    leftButtonDown = false;
    if (!picking && !handlingCache)
        HandlePickCache();
}


