// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             BadIndexException.h                           //
// ************************************************************************* //

#ifndef BAD_INDEX_EXCEPTION_H
#define BAD_INDEX_EXCEPTION_H
#include <avtexception_exports.h>


#include <PipelineException.h>


// ****************************************************************************
//  Class: BadIndexException
//
//  Purpose:
//      The exception that should be called when a bad index is encountered.
//
//  Programmer: Hank Childs
//  Creation:   November 29, 2000
//
// ****************************************************************************

class AVTEXCEPTION_API BadIndexException : public PipelineException
{
  public:
                    BadIndexException(int, int);
    virtual        ~BadIndexException() VISIT_THROW_NOTHING {;};
};


#endif


