// ************************************************************************* //
//                              avtBase10LogFilter.h                         //
// ************************************************************************* //

#ifndef AVT_BASE10LOG_FILTER_H
#define AVT_BASE10LOG_FILTER_H


#include <avtUnaryMathFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtBase10LogFilter
//
//  Purpose:
//      A filter that calculates the base 10 logarithm of its input.
//
//  Programmer: Sean Ahern
//  Creation:   Tue Jun 11 16:23:45 PDT 2002
//
//  Modifications:
//
//    Hank Childs, Fri Nov 15 15:23:54 PST 2002
//    Added support for vectors and non-float arrays.
//
// ****************************************************************************

class EXPRESSION_API avtBase10LogFilter : public avtUnaryMathFilter
{
  public:
                              avtBase10LogFilter() {;};
    virtual                  ~avtBase10LogFilter() {;};

    virtual const char       *GetType(void)   { return "avtBase10LogFilter"; };
    virtual const char       *GetDescription(void) 
                                   { return "Calculating base 10 logarithm"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples);
};


#endif


