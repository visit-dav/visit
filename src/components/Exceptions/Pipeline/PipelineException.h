// ************************************************************************* //
//                              PipelineException.h                          //
// ************************************************************************* //

#ifndef PIPELINE_EXCEPTION_H
#define PIPELINE_EXCEPTION_H
#include <avtexception_exports.h>


#include <VisItException.h>


// ****************************************************************************
//  Class: PipelineException
//
//  Purpose:
//      A base class for all pipeline exceptions.  Meant to make logic easier
//      on catches.
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
// ****************************************************************************

class AVTEXCEPTION_API PipelineException : public VisItException
{
  public:
                         ~PipelineException() VISIT_THROW_NOTHING {;};
};


#endif


