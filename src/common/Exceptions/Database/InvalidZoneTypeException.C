// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          InvalidZoneTypeException.C                       //
// ************************************************************************* //

#include <stdio.h>                     // for sprintf
#include <InvalidZoneTypeException.h>


// ****************************************************************************
//  Method: InvalidZoneTypeException constructor
//
//  Arguments:
//      zonetype    The zone type that was invalid.
//
//  Programmer: Hank Childs
//  Creation:   August 15, 2000
//
// ****************************************************************************

InvalidZoneTypeException::InvalidZoneTypeException(int zonetype)
{
    char smsg[1024];
    sprintf(smsg, "Zone type %d is not supported by VTK.", zonetype);
    msg = smsg;
}


