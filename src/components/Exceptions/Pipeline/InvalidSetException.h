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
//  Modifications:
//    Kathleen Bonnell, Thu Feb 26 12:50:38 PST 2004
//    Added single-argument constructor. (Gives different message).
//
// ****************************************************************************

class AVTEXCEPTION_API InvalidSetException : public PipelineException
{
  public:
                    InvalidSetException(const char *, const char *);
                    InvalidSetException(const char *);
    virtual        ~InvalidSetException() VISIT_THROW_NOTHING {;};
};


#endif


