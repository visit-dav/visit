// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         NoDefaultVariableException.h                      //
// ************************************************************************* //

#ifndef NO_DEFAULT_VARIABLE_EXCEPTION_H
#define NO_DEFAULT_VARIABLE_EXCEPTION_H
#include <avtexception_exports.h>


#include <PipelineException.h>


// ****************************************************************************
//  Class: NoDefaultVariableException
//
//  Purpose:
//      This is the exception to be thrown when the active variable is
//      specified as 'default', but there is no active variable to work on.
//
//  Programmer: Hank Childs
//  Creation:   August 28, 2002
//
// ****************************************************************************

class AVTEXCEPTION_API NoDefaultVariableException : public PipelineException
{
  public:
                          NoDefaultVariableException(std::string);
    virtual              ~NoDefaultVariableException() VISIT_THROW_NOTHING {;};
};


#endif


