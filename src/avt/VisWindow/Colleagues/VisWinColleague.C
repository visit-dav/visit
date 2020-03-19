// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            VisWinColleague.C                              //
// ************************************************************************* //

#include <VisWinColleague.h>
#include <VisWindowColleagueProxy.h>


// ****************************************************************************
//  Method: VisWinColleague constructor
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
// ****************************************************************************

VisWinColleague::VisWinColleague(VisWindowColleagueProxy &p)
    : mediator(p)
{
    ;
}


