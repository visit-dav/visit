// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              QtVisWindow.h                                //
// ************************************************************************* //

#ifndef VIS_WINDOW_WITH_INTERACTIONS_H
#define VIS_WINDOW_WITH_INTERACTIONS_H

#include <viswindow_exports.h>

#include <VisWindow.h>
#include <vtkRenderWindow.h>

//class vtkQtRenderWindow;

// ****************************************************************************
//  Class: VisWindowWithInteractions
//
//  Purpose:
//      A Vis Window that has support for interactions.
//      Should be using vtkRenderWindowInteractor, but it pops up a window.
//      TODO: create a test example and file a bug with VTK.
//
//  Programmer: Hari Krishnan
//  Creation:
//
//  Modifications:
//
// ****************************************************************************

class VISWINDOW_API VisWindowWithInteractions : public VisWindow
{
  public:
    VisWindowWithInteractions();
    virtual ~VisWindowWithInteractions() {}

    void UpdateMouseActions(std::string action,
                            double start_dx, double start_dy,
                            double end_dx, double end_dy,
                            bool ctrl, bool shift);
};

#endif


