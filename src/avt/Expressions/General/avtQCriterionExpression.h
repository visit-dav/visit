// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtQCriterionExpression.h                         //
// ************************************************************************* //

#ifndef AVT_Q_CRITERION_EXPRESSION_H
#define AVT_Q_CRITERION_EXPRESSION_H

#include <avtMultipleInputExpressionFilter.h>


// ****************************************************************************
//  Class: avtQCriterionExpression
//
//  Purpose:
//      Creates a scalar variable out of three gradient vectors. 
//
//  Programmer: Kevin Griffin
//  Creation:   Tue Jul 29 10:08:40 PDT 2014
//
//  Modifications:
//
// ****************************************************************************

class EXPRESSION_API avtQCriterionExpression 
    : public avtMultipleInputExpressionFilter
{
  public:
                              avtQCriterionExpression();
    virtual                  ~avtQCriterionExpression();

    virtual const char       *GetType(void)  
                                    { return "avtQCriterionExpression"; };
    virtual const char       *GetDescription(void)
                                 {return "Creating a vector from components";};
    virtual int               NumVariableArguments() { return 3; }
    static double             QCriterion(const double *, const double *, const double *);

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual int               GetVariableDimension(void) { return 1; };
    virtual bool              CanHandleSingletonConstants(void) {return true;};
};


#endif


