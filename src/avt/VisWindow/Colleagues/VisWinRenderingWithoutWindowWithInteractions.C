// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      VisWinRenderingWithoutWindow.C                       //
// ************************************************************************* //

#include <VisWinRenderingWithoutWindowWithInteractions.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
//#include <QVTKInteractor.h>
#include <VisWindowColleagueProxy.h>
#include <VisitHotPointInteractor.h>

vtkRenderWindowInteractor* (*VisWinRenderingWithoutWindowWithInteractions::createInteractor)()  = 0;
// ****************************************************************************
//  Method: VisWinRenderingWithoutWindow constructor
//
//  Programmer: Hari Krishnan
//  Creation:
//
//  Modifications:
//
// ****************************************************************************

VisWinRenderingWithoutWindowWithInteractions::VisWinRenderingWithoutWindowWithInteractions(
                                                    VisWindowColleagueProxy &p)
   : VisWinRenderingWithoutWindow(p)
{
    iren = renWin->GetInteractor();
    ownsInteractor = false;
}

VisWinRenderingWithoutWindowWithInteractions::~VisWinRenderingWithoutWindowWithInteractions()
{
    if(ownsInteractor) {
        iren->Delete();
    }
}

vtkRenderWindowInteractor*
VisWinRenderingWithoutWindowWithInteractions::GetRenderWindowInteractor() {
    return renWin->GetInteractor();
}

void
VisWinRenderingWithoutWindowWithInteractions::Initialize(VisitHotPointInteractor *i) {

    /// using QVTKInteractor since it does not popup a window
    /// TODO: replace with vtkRenderWindowInteractor when the
    /// vtkRenderWindowInteractor does not pop up a window..
    /// under nowin mode vtkRenderWindowInteractor crashes.

    if(iren == NULL) {
        ownsInteractor = true;
        if(createInteractor) {
            iren = createInteractor();
            iren->SetInteractorStyle(i);
            renWin->SetInteractor(iren);
        } else {
            iren = vtkRenderWindowInteractor::New();
            //iren = QVTKInteractor::New();
            iren->SetInteractorStyle(i);
            renWin->SetInteractor(iren);
        }
    }
}
