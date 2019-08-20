// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              NoInputException.h                           //
// ************************************************************************* //

#ifndef NO_INPUT_EXCEPTION_H
#define NO_INPUT_EXCEPTION_H
#include <avtexception_exports.h>


#include <PipelineException.h>


// ****************************************************************************
//  Class: NoInputException
//
//  Purpose:
//      The exception that should be called when there is no input to a filter.
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
// ****************************************************************************

class AVTEXCEPTION_API NoInputException : public PipelineException
{
  public:
                          NoInputException();
    virtual              ~NoInputException() VISIT_THROW_NOTHING {;};
};


#endif


