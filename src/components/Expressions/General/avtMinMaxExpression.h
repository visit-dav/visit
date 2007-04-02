// ************************************************************************* //
//                             avtMinMaxExpression.h                         //
// ************************************************************************* //

#ifndef AVT_MINMAX_EXPRESSION_H
#define AVT_MINMAX_EXPRESSION_H

#include <avtBinaryMathFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtMinMaxExpression
//
//  Purpose:
//      A filter that calculates resrad -- which adjusts the resolution
//      using a monte carlo resampling with a given radius.
//
//  Programmer: Hank Childs
//  Creation:   March 13, 2006
//
// ****************************************************************************

class EXPRESSION_API avtMinMaxExpression : public avtBinaryMathFilter
{
  public:
                              avtMinMaxExpression();
    virtual                  ~avtMinMaxExpression();

    virtual const char       *GetType(void) 
                                 { return "avtMinMaxExpression"; };
    virtual const char       *GetDescription(void)
                                 { return "Calculating min or max"; };
    bool                      SetDoMinimum(bool b) { doMin = b; };

  protected:
    bool             doMin;

    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                 vtkDataArray *out, int ncomps, int ntuples);
};


#endif


