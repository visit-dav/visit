// ************************************************************************* //
//                              NoCurveException.h                           //
// ************************************************************************* //

#ifndef NO_CURVE_EXCEPTION_H
#define NO_CURVE_EXCEPTION_H

#include <avtexception_exports.h>

#include <PipelineException.h>


// ****************************************************************************
//  Class: NoCurveException
//
//  Purpose:
//      The exception that should be called when a curve is expected, but none
//      is found.
//
//  Programmer: Hank Childs
//  Creation:   May 28, 2002
//
// ****************************************************************************

class AVTEXCEPTION_API NoCurveException : public PipelineException
{
  public:
                          NoCurveException();
    virtual              ~NoCurveException() VISIT_THROW_NOTHING {;};
};


#endif


