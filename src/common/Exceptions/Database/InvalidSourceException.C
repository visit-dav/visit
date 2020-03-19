// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           InvalidSourceException.C                        //
// ************************************************************************* //

#include <InvalidSourceException.h>


// ****************************************************************************
//  Method: InvalidSourceException constructor
//
//  Programmer: Hank Childs
//  Creation:   August 10, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Aug 14 13:54:21 PDT 2000
//    Renamed InvalidSourceException from NoSourceException and changed log
//    message.
//
// ****************************************************************************

InvalidSourceException::InvalidSourceException()
{
    msg = "The source object specified is invalid.";
}


