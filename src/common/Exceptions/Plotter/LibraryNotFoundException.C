// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         LibraryNotFoundException.C                        //
// ************************************************************************* //

#include <LibraryNotFoundException.h>

// ****************************************************************************
//  Method: LibraryNotFoundException constructor
//
//  Programmer: Tom Fogal
//  Creation:   Tue Dec  8 14:14:00 MST 2009
//
//  Modifications:
//
// ****************************************************************************

LibraryNotFoundException::LibraryNotFoundException(const std::string &lib):
    PlotterException()
{
    msg = std::string("Could not find a required library: ") + lib;
}
