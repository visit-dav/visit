// ************************************************************************* //
//                      UnexpectedValueException.h                           //
// ************************************************************************* //

#ifndef UNEXPECTED_VALUE_EXCEPTION_H
#define UNEXPECTED_VALUE_EXCEPTION_H
#include <avtexception_exports.h>


#include <string>

#include <PipelineException.h>


// ****************************************************************************
//  Class: UnexpectedValueException 
//
//  Purpose:
//      The exception that should be called when an unexpected value is
//      encountered.
//
//  Programmer: Mark C. Miller 
//  Creation:   December 9, 2003 
//
// ****************************************************************************

class AVTEXCEPTION_API UnexpectedValueException : public PipelineException
{
  public:
                    UnexpectedValueException(int,int);
                    UnexpectedValueException(double,double);
                    UnexpectedValueException(int,string);
                    UnexpectedValueException(double,string);
                    UnexpectedValueException(std::string,std::string);
    virtual        ~UnexpectedValueException() VISIT_THROW_NOTHING {;};
};


#endif


