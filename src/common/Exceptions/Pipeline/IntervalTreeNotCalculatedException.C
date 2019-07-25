// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                   IntervalTreeNotCalculatedException.C                    //
// ************************************************************************* //

#include <IntervalTreeNotCalculatedException.h>


// ****************************************************************************
//  Method: IntervalTreeNotCalculatedException constructor
//
//  Programmer: Hank Childs
//  Creation:   August 8, 2000
//
// ****************************************************************************

IntervalTreeNotCalculatedException::IntervalTreeNotCalculatedException()
{
    msg = "The interval tree was accessed even though it has not yet been"
          " calculated";
}


