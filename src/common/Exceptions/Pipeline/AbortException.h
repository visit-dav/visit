// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               AbortException.h                            //
// ************************************************************************* //

#ifndef ABORT_EXCEPTION_H
#define ABORT_EXCEPTION_H
#include <avtexception_exports.h>


#include <PipelineException.h>


// ****************************************************************************
//  Class: AbortException
//
//  Purpose:
//      The exception thrown when the pipeline is ordered to abort mid-stream.
//
//  Programmer: Hank Childs
//  Creation:   June 20, 2001
//
// **************************************************************************** 

class AVTEXCEPTION_API AbortException : public PipelineException
{
  public:
                          AbortException();
    virtual              ~AbortException() VISIT_THROW_NOTHING {;};
};


#endif


