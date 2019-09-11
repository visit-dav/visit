// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          BadVectorException.C                           //
// ************************************************************************* //

#include <BadVectorException.h>

using std::string;


// ****************************************************************************
//  Method: BadVectorException constructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 20, 2003 
//
// ****************************************************************************

BadVectorException::BadVectorException(std::string type)
{
    if (type == "")
        msg = "A bad vector has been specified. Please supply proper values.";
    else
        msg = "A bad " + type + " has been specified. Please supply proper values.";
}


