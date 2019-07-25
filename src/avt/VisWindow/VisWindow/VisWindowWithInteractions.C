// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
