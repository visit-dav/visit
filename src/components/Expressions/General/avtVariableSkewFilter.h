// ************************************************************************* //
//                           avtVariableSkewFilter.h                         //
// ************************************************************************* //

#ifndef AVT_VARIALE_SKEW_FILTER_H
#define AVT_VARIALE_SKEW_FILTER_H


#include <avtBinaryMathFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtVariableSkewFilter
//
//  Purpose:
//      A filter that calculates the skewed value of its input.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 2, 2005 
//
//  Modifications:
//
// ****************************************************************************

class EXPRESSION_API avtVariableSkewFilter : public avtBinaryMathFilter
{
  public:
                              avtVariableSkewFilter();
    virtual                  ~avtVariableSkewFilter();

    virtual const char       *GetType(void)   { return "avtVariableSkewFilter"; };
    virtual const char       *GetDescription(void) 
                                   { return "Calculating skew"; };

  protected:
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                 vtkDataArray *out, int ncomponents, int ntuples);
  private:
    double           SkewTheValue(double val, double min, double max, 
                                  double factor);
};


#endif


