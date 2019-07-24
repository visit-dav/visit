// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            BadPlotException.C                             //
// ************************************************************************* //

#include <BadPlotException.h>


// ****************************************************************************
//  Method: BadPlotException constructor
//
//  Programmer: Hank Childs
//  Creation:   May 17, 2000
//
// ****************************************************************************

BadPlotException::BadPlotException()
{
    msg  = "Tried to access non-existant plot.";
}


