// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                     IncompatibleDomainListsException.h                    //
// ************************************************************************* //

#ifndef INCOMPATIBLE_DOMAIN_LISTS_EXCEPTION_H
#define INCOMPATIBLE_DOMAIN_LISTS_EXCEPTION_H
#include <avtexception_exports.h>


#include <PipelineException.h>


// ****************************************************************************
//  Class: IncompatibleDomainListsException
//
//  Purpose:
//      The exception thrown when two domain lists try to merge, but do not
//      have the same number of domains.
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
// ****************************************************************************

class AVTEXCEPTION_API IncompatibleDomainListsException : public PipelineException
{
  public:
                    IncompatibleDomainListsException(int, int);
    virtual        ~IncompatibleDomainListsException() VISIT_THROW_NOTHING {;};
};


#endif


