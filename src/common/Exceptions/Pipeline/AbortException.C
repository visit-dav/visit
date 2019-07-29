// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              AbortException.C                             //
// ************************************************************************* //

#include <AbortException.h>


// ****************************************************************************
//  Method: AbortException constructor
//
//  Programmer: Hank Childs
//  Creation:   June 20, 2001
//
// ****************************************************************************

AbortException::AbortException()
{
    msg = "The pipeline was ordered to abort.";
}


