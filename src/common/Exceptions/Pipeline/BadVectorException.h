// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           BadVectorException.h                          //
// ************************************************************************* //

#ifndef BAD_VECTOR_EXCEPTION_H
#define BAD_VECTOR_EXCEPTION_H
#include <avtexception_exports.h>


#include <PipelineException.h>


// ****************************************************************************
//  Class: BadVectorException
//
//  Purpose:
//      Thrown when a bad vector has been specified. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 20, 2003 
//
// **************************************************************************** 

class AVTEXCEPTION_API BadVectorException : public PipelineException
{
  public:
                          BadVectorException(std::string = "");
    virtual              ~BadVectorException() VISIT_THROW_NOTHING {;};
};


#endif


