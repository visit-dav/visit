// ************************************************************************* //
//                          InvalidMergeException.h                          //
// ************************************************************************* //

#ifndef INVALID_MERGE_EXCEPTION_H
#define INVALID_MERGE_EXCEPTION_H
#include <avtexception_exports.h>


#include <PipelineException.h>


// ****************************************************************************
//  Class: InvalidMergeException
//
//  Purpose:
//      The exception thrown when a dataset is not able to merge with another
//      dataset.
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2000
//
// ****************************************************************************

class AVTEXCEPTION_API InvalidMergeException : public PipelineException
{
  public:
                    InvalidMergeException(int, int);
                    InvalidMergeException();
    virtual        ~InvalidMergeException() VISIT_THROW_NOTHING {;};
};


#endif


