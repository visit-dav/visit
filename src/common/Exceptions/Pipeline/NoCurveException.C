// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             NoCurveException.C                            //
// ************************************************************************* //

#include <NoCurveException.h>


// ****************************************************************************
//  Method: NoCurveException constructor
//
//  Programmer: Hank Childs
//  Creation:   May 28, 2002
//
// ****************************************************************************

NoCurveException::NoCurveException()
{
    msg = "The operation expected a curve, but none was found.  This can sometimes happen when the active window is not set to the one containing curves.";
}


