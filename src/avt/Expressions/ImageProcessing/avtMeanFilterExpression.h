// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtMeanFilterExpression.h                        //
// ************************************************************************* //

#ifndef AVT_MEAN_FILTER_EXPRESSION_H
#define AVT_MEAN_FILTER_EXPRESSION_H

#include <avtUnaryMathExpression.h>

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
//  Modifications:
//    Gunther H. Weber, Thu Mar  5 13:13:49 PST 2009
//    Added ability to specify filter width as argument (adding private
//    variable width[3] and the method ProcessArguments())
//    
//
// ****************************************************************************

class EXPRESSION_API avtMeanFilterExpression : public avtUnaryMathExpression
{
  public:
                              avtMeanFilterExpression();
    virtual                  ~avtMeanFilterExpression();

    virtual const char       *GetType(void) 
                                 { return "avtMeanFilterExpression"; };
    virtual const char       *GetDescription(void)
                                 { return "Taking mean filter"; };
    virtual void              ProcessArguments(ArgsExpr*, ExprPipelineState *);

  protected:
    bool             haveIssuedWarning;

    virtual void     PreExecute(void);
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *out, 
                                 int ncomps, int ntuples, vtkDataSet *in_ds);

  private:
    int              width[3];
};


#endif


