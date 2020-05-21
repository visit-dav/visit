// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              TyphonIOException.C                          //
// ************************************************************************* //

// for sprintf
#include <stdio.h> 

#include <TyphonIOException.h>


// ****************************************************************************
//  Method: TyphonIOException constructor
//
//  Arguments:
//      code    The TyphonIO error code
//
//  Programmer: Paul Selby
//  Creation:   March 16, 2015
//
// ****************************************************************************

TyphonIOException::TyphonIOException(TIO_t code)
{
    char str[1024];
    char tiostr[TIO_STRLEN];

    //
    // Use empty message if TyphonIO fails to retrieve it
    //
    if (TIO_Get_Error(code, tiostr) != TIO_SUCCESS)
    {
        tiostr[0] = '\0';
    }
    sprintf(str, "A TyphonIO error occurred.\nThe error is \"%s.\" (%d)",
            tiostr, code);

    msg = str;
}


