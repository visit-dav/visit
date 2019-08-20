// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      PlotDimensionalityException.h                        //
// ************************************************************************* //

#ifndef PLOT_DIMENSIONALITY_EXCEPTION_H
#define PLOT_DIMENSIONALITY_EXCEPTION_H
#include <viswindow_exports.h>

#include <VisWindowException.h>
#include <VisWindowTypes.h>


// ****************************************************************************
//  Method: PlotDimensionalityException
//
//  Purpose:
//      The exception that should be thrown when a plot is added whose
//      dimensionality (ie 2D or 3D) is not the same as the current plots in
//      the window.
//
//  Programmer: Hank Childs
//  Creation:   May 16, 2000
//
// ****************************************************************************

class VISWINDOW_API2 PlotDimensionalityException : public VisWindowException
{
  public:
                    PlotDimensionalityException(WINDOW_MODE, WINDOW_MODE, int);
    virtual        ~PlotDimensionalityException() VISIT_THROW_NOTHING {;};
};


#endif


