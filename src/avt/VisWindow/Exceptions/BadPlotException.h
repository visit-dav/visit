// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             BadPlotException.h                            //
// ************************************************************************* //

#ifndef BAD_PLOT_EXCEPTION_H
#define BAD_PLOT_EXCEPTION_H
#include <viswindow_exports.h>

#include <VisWindowException.h>


// ****************************************************************************
//  Class: PlotDimensionalityException
//
//  Purpose:
//      The exception that should be thrown when a bad plot is specified.
//
//  Programmer: Hank Childs
//  Creation:   May 17, 2000
//
// ****************************************************************************

class VISWINDOW_API2 BadPlotException : public VisWindowException
{
  public:
                    BadPlotException();
    virtual        ~BadPlotException() VISIT_THROW_NOTHING {;};
};


#endif


