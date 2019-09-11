// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       InvalidConversionException.C                        //
// ************************************************************************* //

#include <InvalidConversionException.h>


// ****************************************************************************
//  Method: InvalidConversionException constructor
//
//  Arguments:
//    setName    The set name.
//
//  Programmer: Cameron Christensen
//  Creation:   May 22, 2014
//
// ****************************************************************************

InvalidConversionException::InvalidConversionException()
{
    msg = "Impossible conversion was attempted.";
}


