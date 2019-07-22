// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       NonQueryableInputException.h                        //
// ************************************************************************* //

#ifndef NON_QUERYABLE_INPUT_EXCEPTION_H
#define NON_QUERYABLE_INPUT_EXCEPTION_H
#include <avtexception_exports.h>


#include <PipelineException.h>


// ****************************************************************************
//  Class: NonQueryableInputException
//
//  Purpose:
//      The exception that should be called when attempting to query a 
//      non-queryable object. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 22, 2002 
// 
//  Modifications:
//    Kathleen Bonnell, Fri Sep  3 10:10:28 PDT 2004
//    Added constructor with string argument.
//  
// ****************************************************************************

class AVTEXCEPTION_API NonQueryableInputException : public PipelineException
{
  public:
                          NonQueryableInputException();
                          NonQueryableInputException(const std::string &);
    virtual              ~NonQueryableInputException() VISIT_THROW_NOTHING {;};
};


#endif


