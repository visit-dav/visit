// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       InvalidConversionException.h                        //
// ************************************************************************* //

#ifndef INVALID_CONVERSION_EXCEPTION_H
#define INVALID_CONVERSION_EXCEPTION_H
#include <avtexception_exports.h>


#include <PipelineException.h>


// ****************************************************************************
//  Class: InvalidConversionException
//
//  Purpose:
//     Called when attempting impossible datatype conversion.
//
//  Programmer: Cameron Christensen
//  Creation:   May 22, 2014
//
//  Modifications:
//
// ****************************************************************************

class AVTEXCEPTION_API InvalidConversionException : public PipelineException
{
  public:
                    InvalidConversionException();
    virtual        ~InvalidConversionException() VISIT_THROW_NOTHING {;};
};


#endif


