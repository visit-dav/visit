// ************************************************************************* //
//                           LogicalIndexException.h                         //
// ************************************************************************* //

#ifndef LOGICAL_INDEX_EXCEPTION_H
#define LOGICAL_INDEX_EXCEPTION_H
#include <avtexception_exports.h>


#include <PipelineException.h>


// ****************************************************************************
//  Class: LogicalIndexException
//
//  Purpose:
//      The exception that should be called when a logical index of 2 or
//      3 components is expected, but only 1 component was given.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 12, 2002 
//
// ****************************************************************************

class AVTEXCEPTION_API LogicalIndexException : public PipelineException
{
  public:
                    LogicalIndexException();
                    LogicalIndexException(const char *);
    virtual        ~LogicalIndexException() VISIT_THROW_NOTHING {;};
};


#endif


