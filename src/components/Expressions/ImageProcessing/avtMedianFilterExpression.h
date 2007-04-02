// ************************************************************************* //
//                        avtMedianFilterExpression.h                        //
// ************************************************************************* //

#ifndef AVT_MEDIAN_FILTER_EXPRESSION_H
#define AVT_MEDIAN_FILTER_EXPRESSION_H

#include <avtUnaryMathFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtMedianFilterExpression
//
//  Purpose:
//      A filter that calculates the median of a zone and its surrounding 
//      neighbors.
//
//  Programmer: Hank Childs
//  Creation:   August 14, 2005
//
// ****************************************************************************

class EXPRESSION_API avtMedianFilterExpression : public avtUnaryMathFilter
{
  public:
                              avtMedianFilterExpression();
    virtual                  ~avtMedianFilterExpression();

    virtual const char       *GetType(void) 
                                 { return "avtMedianFilterExpression"; };
    virtual const char       *GetDescription(void)
                                 { return "Taking median filter"; };

  protected:
    bool             haveIssuedWarning;

    virtual void     PreExecute(void);
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *out, 
                                 int ncomps, int ntuples);
};


#endif


