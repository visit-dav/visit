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
// ****************************************************************************

class AVTEXCEPTION_API NonQueryableInputException : public PipelineException
{
  public:
                          NonQueryableInputException();
    virtual              ~NonQueryableInputException() VISIT_THROW_NOTHING {;};
};


#endif


