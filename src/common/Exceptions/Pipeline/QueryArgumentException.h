// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       QueryArgumentException.h                            //
// ************************************************************************* //

#ifndef QUERY_ARGUMENT_EXCEPTION_H
#define QUERY_ARGUMENT_EXCEPTION_H
#include <avtexception_exports.h>


#include <PipelineException.h>


// ****************************************************************************
//  Class: QueryArgumentException
//
//  Purpose:
//      The exception that should be called when a query's arguments could
//      not be properly parsed.
//
//  Programmer: Kathleen Biagas 
//  Creation:   August 23, 2011 
// 
//  Modifications:
//  
// ****************************************************************************

class AVTEXCEPTION_API QueryArgumentException : public PipelineException
{
  public:
                          QueryArgumentException(const std::string &);
                          QueryArgumentException(const std::string &,
                                                 const std::string &);
                          QueryArgumentException(const std::string &,
                                                 const int);
    virtual              ~QueryArgumentException() VISIT_THROW_NOTHING {;};
};


#endif


