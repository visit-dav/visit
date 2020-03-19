// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                SiloException.C                            //
// ************************************************************************* //

// for sprintf
#include <stdio.h> 

#include <SiloException.h>


// ****************************************************************************
//  Method: SiloException constructor
//
//  Arguments:
//      filename    The name of an invalid file.
//
//  Programmer: Hank Childs
//  Creation:   April 9, 2001
//
// ****************************************************************************

SiloException::SiloException(const char *filename)
{
    char str[1024];
    sprintf(str, "A Silo error occurred.\nThe error is \"%s.\"", filename);

    msg = str;
}


