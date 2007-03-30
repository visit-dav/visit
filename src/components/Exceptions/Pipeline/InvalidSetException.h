// ************************************************************************* //
//                          InvalidSetException.h                            //
// ************************************************************************* //

#ifndef INVALID_SET_EXCEPTION_H
#define INVALID_SET_EXCEPTION_H
#include <avtexception_exports.h>


#include <PipelineException.h>


// ****************************************************************************
//  Class: InvalidSetException
//
//  Purpose:
//      The exception that should be called when an invalid SIL set is
//      encountered for a particular SIL Category.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 12, 2002 
//
// ****************************************************************************

class AVTEXCEPTION_API InvalidSetException : public PipelineException
{
  public:
                    InvalidSetException(const char *, const char *);
    virtual        ~InvalidSetException() VISIT_THROW_NOTHING {;};
};


#endif


