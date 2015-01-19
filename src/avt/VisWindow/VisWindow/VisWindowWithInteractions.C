/*****************************************************************************
*
* Copyright (c) 2000 - 2014, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
//                               QtVisWindow.C                               //
// ************************************************************************* //
#include <VisWindowWithInteractions.h>
//#include <VisWinRenderingWithWindow.h>
#include <VisWinRenderingWithoutWindowWithInteractions.h>
#include <VisWinInteractions.h>

#include <VisitHotPointInteractor.h>
#include  <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

// ****************************************************************************
//  Method: VisWindowWithInteractions constructor
//
//  Programmer: Hari Krishnan
//  Creation:
//
//  Modifications:
// ****************************************************************************

VisWindowWithInteractions::VisWindowWithInteractions() : VisWindow(false)
{
    Initialize(new VisWinRenderingWithoutWindowWithInteractions(colleagueProxy));
}

void
VisWindowWithInteractions::UpdateMouseActions(std::string action,
                        double start_dx, double start_dy,
                        double end_dx, double end_dy,
                        bool ctrl, bool shift) {

    VisWinRenderingWithoutWindowWithInteractions* ren = dynamic_cast<VisWinRenderingWithoutWindowWithInteractions*>(rendering);

    if(rendering == NULL) {
        return;
    }

    vtkRenderWindowInteractor* iren = ren->GetRenderWindowInteractor();

    bool setupInteractor = false;
    if(iren == NULL) {
        ren->Initialize(interactions->GetHotPointInteractor());
        SetInteractionMode(GetInteractionMode());
        setupInteractor = true;
    }

    ren->UpdateMouseActions(action, start_dx, start_dy, end_dx, end_dy, ctrl, shift);

    if(setupInteractor) {
        ren->Render();
        ren->InvokeRenderCallback();
    }
}
