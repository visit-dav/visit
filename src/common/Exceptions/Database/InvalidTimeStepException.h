// ************************************************************************* //
//                          InvalidTimeStepException.h                       //
// ************************************************************************* //

#ifndef INVALID_TIME_STEP_EXCEPTION_H
#define INVALID_TIME_STEP_EXCEPTION_H
#include <avtexception_exports.h>


#include <DatabaseException.h>


// ****************************************************************************
//  Class: InvalidTimeStepException
//
//  Purpose:
//      The exception that should be called when an invalid time step is 
//      encountered.
//
//  Programmer: Hank Childs
//  Creation:   September 13, 2000
//
// ****************************************************************************

class AVTEXCEPTION_API InvalidTimeStepException : public DatabaseException
{
  public:
                    InvalidTimeStepException(int, int);
    virtual        ~InvalidTimeStepException() VISIT_THROW_NOTHING {;};
};


#endif


