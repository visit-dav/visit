// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      NonQueryableInputException.C                         //
// ************************************************************************* //

#include <NonQueryableInputException.h>


// ****************************************************************************
//  Method: NonQueryableInputException constructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 22, 2002 
//
// ****************************************************************************

NonQueryableInputException::NonQueryableInputException()
{
    msg =  "The input is not queryable.";
}

// ****************************************************************************
//  Method: NonQueryableInputException constructor
//  
//  Programmer: Kathleen Bonnell 
//  Creation:   September 3, 2004 
//
// ****************************************************************************

NonQueryableInputException::NonQueryableInputException(const std::string &reason)
{
    msg =  "The input is not queryable.  " + reason;
}


