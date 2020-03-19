// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
                    UnexpectedValueException(std::string, int);
                    UnexpectedValueException(std::string, double);
                    UnexpectedValueException(std::string,std::string);
    virtual        ~UnexpectedValueException() VISIT_THROW_NOTHING {;};
};


#endif


