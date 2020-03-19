// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       StubReferencedException.C                          //
// ************************************************************************* //
#include <StubReferencedException.h>

// ****************************************************************************
//  Method: StubReferencedException constructor
//
//  Purpose: handle stub reference exception
//
//  Programmer: Mark C. Miller
//  Creation:   October 25, 2005 
//
// ****************************************************************************
StubReferencedException::StubReferencedException(const char *stubName)
{
    msg = stubName;
}
