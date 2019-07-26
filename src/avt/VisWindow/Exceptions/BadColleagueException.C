// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          BadColleagueException.C                          //
// ************************************************************************* //


#include <BadColleagueException.h>


// ****************************************************************************
//  Method: BadColleagueException constructor
//
//  Programmer: Hank Childs
//  Creation:   June 8, 2000
//
// ****************************************************************************

BadColleagueException::BadColleagueException()
{
    msg = "The VisWindow colleague could not be found.\n";
}


