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
//  Modifications:
//
//    Hank Childs, Thu Aug  4 11:06:21 PDT 2005
//    Added new constructor that takes doubles.
//
//    Hank Childs, Mon Aug 29 15:49:32 PDT 2005
//    Added new constructor with a warning message.
//
// ****************************************************************************

class AVTEXCEPTION_API InvalidMergeException : public PipelineException
{
  public:
                    InvalidMergeException(int, int);
                    InvalidMergeException(double, double);
                    InvalidMergeException(const char *);
                    InvalidMergeException();
    virtual        ~InvalidMergeException() VISIT_THROW_NOTHING {;};
};


#endif


