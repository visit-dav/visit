// ************************************************************************* //
//                            avtBinaryPowerFilter.h                         //
// ************************************************************************* //

#ifndef AVT_BINARY_POWER_FILTER_H
#define AVT_BINARY_POWER_FILTER_H

#include <avtBinaryMathFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtBinaryPowerFilter
//
//  Purpose:
//      A filter that calculates the first variable raised to the power of
//      the second filter.
//
//  Programmer: Sean Ahern
//  Creation:   Wed Jun 26 17:05:31 PDT 2002
//
// ****************************************************************************

class EXPRESSION_API avtBinaryPowerFilter : public avtBinaryMathFilter
{
  public:
                              avtBinaryPowerFilter() {;};
    virtual                  ~avtBinaryPowerFilter() {;};

    virtual const char       *GetType(void) { return "avtBinaryPowerFilter"; };
    virtual const char       *GetDescription(void)
                                            { return "Calculating power"; };

  protected:
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                 vtkDataArray *out, int ncomps, int ntuples);
};


#endif


