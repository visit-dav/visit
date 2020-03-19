// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            BadDomainException.h                           //
// ************************************************************************* //

#ifndef BAD_DOMAIN_EXCEPTION_H
#define BAD_DOMAIN_EXCEPTION_H
#include <avtexception_exports.h>


#include <PipelineException.h>


// ****************************************************************************
//  Class: BadDomainException
//
//  Purpose:
//      The exception that should be called when a bad domain is encountered.
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
// ****************************************************************************

class AVTEXCEPTION_API BadDomainException : public PipelineException
{
  public:
                    BadDomainException(int, int);
    virtual        ~BadDomainException() VISIT_THROW_NOTHING {;};
};


#endif


