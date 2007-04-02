// ************************************************************************* //
//                       StubReferencedException.h                           //
// ************************************************************************* //

#ifndef STUB_REFERENCED_EXCEPTION_H
#define STUB_REFERENCED_EXCEPTION_H
#include <avtexception_exports.h>

#include <PipelineException.h>

// ****************************************************************************
//  Class: UnexpectedValueException 
//
//  Purpose: The exception that all pipeline stub functions should throw
//
//  Programmer: Mark C. Miller 
//  Creation:   October 25, 2005 
//
// ****************************************************************************

class AVTEXCEPTION_API StubReferencedException: public PipelineException
{
  public:
                    StubReferencedException(const char*);
    virtual        ~StubReferencedException() VISIT_THROW_NOTHING {;};
};


#endif
