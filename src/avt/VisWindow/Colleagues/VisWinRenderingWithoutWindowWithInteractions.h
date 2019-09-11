// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       VisWinRenderingWithoutWindow.h                      //
// ************************************************************************* //

#ifndef VIS_WIN_RENDERING_WITHOUT_WINDOW_WITH_INTERACTIONS_H
#define VIS_WIN_RENDERING_WITHOUT_WINDOW_WITH_INTERACTIONS_H
#include <viswindow_exports.h>
#include <VisWinRenderingWithoutWindow.h>

class VisitHotPointInteractor;
class vtkRenderWindowInteractor;

// ****************************************************************************
//  Class: VisWinRenderingWithoutWindowWithInteractions
//
//  Purpose:
//      A derived type of VisWinRendering that assumes that there will be no
//      window on the screen but this version adds interactions.
//
//  Programmer: Hari Krishnan
//  Creation:
//
//  Modifications:
//
// ****************************************************************************

class VISWINDOW_API VisWinRenderingWithoutWindowWithInteractions : public VisWinRenderingWithoutWindow
{
  public:
    VisWinRenderingWithoutWindowWithInteractions(VisWindowColleagueProxy &);

    virtual ~VisWinRenderingWithoutWindowWithInteractions();
    virtual vtkRenderWindowInteractor* GetRenderWindowInteractor();

    void Initialize(VisitHotPointInteractor* i);

    static void SetInteractorCallback(vtkRenderWindowInteractor* (*cb)()) {
        createInteractor = cb;
    }

private:
    static vtkRenderWindowInteractor* (*createInteractor)();

    vtkRenderWindowInteractor* iren;
    bool ownsInteractor;
};
#endif
