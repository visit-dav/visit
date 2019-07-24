// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          ImproperUseException.C                           //
// ************************************************************************* //

#include <ImproperUseException.h>

using std::string;


// ****************************************************************************
//  Method: ImproperUseException constructor
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
//  Modifications:
//
//    Tom Fogal, Sat Jun 14 20:22:32 EDT 2008
//    Changed the argument to be a constant reference.  It needs to be const to
//    be usable with string constants.
//
// ****************************************************************************

ImproperUseException::ImproperUseException(const std::string & reason)
{
    if (reason == "")
        msg = "The pipeline object is being used improperly.";
    else
        msg = "The pipeline object is being used improperly: " + reason;
}


