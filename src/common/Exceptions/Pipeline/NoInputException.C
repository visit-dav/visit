// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             NoInputException.C                            //
// ************************************************************************* //

#include <NoInputException.h>


// ****************************************************************************
//  Method: NoInputException constructor
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
// ****************************************************************************

NoInputException::NoInputException()
{
    msg = "No input was defined for this filter.";
}


