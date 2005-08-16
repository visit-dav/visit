// ************************************************************************* //
//                          avtMeanFilterExpression.h                        //
// ************************************************************************* //

#ifndef AVT_MEAN_FILTER_EXPRESSION_H
#define AVT_MEAN_FILTER_EXPRESSION_H

#include <avtUnaryMathFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtMeanFilterExpression
//
//  Purpose:
//      A filter that calculates the mean of a zone and its surrounding 
//      neighbors.
//
//  Programmer: Hank Childs
//  Creation:   August 14, 2005
//
// ****************************************************************************

class EXPRESSION_API avtMeanFilterExpression : public avtUnaryMathFilter
{
  public:
                              avtMeanFilterExpression();
    virtual                  ~avtMeanFilterExpression();

    virtual const char       *GetType(void) 
                                 { return "avtMeanFilterExpression"; };
    virtual const char       *GetDescription(void)
                                 { return "Taking mean filter"; };

  protected:
    bool             haveIssuedWarning;

    virtual void     PreExecute(void);
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *out, 
                                 int ncomps, int ntuples);
};


#endif


