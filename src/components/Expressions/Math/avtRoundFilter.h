// ************************************************************************* //
//                              avtRoundFilter.h                             //
// ************************************************************************* //

#ifndef AVT_ROUND_FILTER_H
#define AVT_ROUND_FILTER_H


#include <avtUnaryMathFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtRoundFilter
//
//  Purpose:
//      A filter that rounds up or down a floating point quantity.
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2005
//
// ****************************************************************************

class EXPRESSION_API avtRoundFilter : public avtUnaryMathFilter
{
  public:
                              avtRoundFilter();
    virtual                  ~avtRoundFilter();

    virtual const char       *GetType(void)  { return "avtRoundFilter"; };
    virtual const char       *GetDescription(void) 
                                         { return "Rounding"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples);
};


#endif


