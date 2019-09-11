// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtLambda2Expression.h                               //
// ************************************************************************* //

#ifndef AVT_LAMBDA2_EXPRESSION_H
#define AVT_LAMBDA2_EXPRESSION_H

#include <avtMultipleInputExpressionFilter.h>


// ****************************************************************************
//  Class: avtLambda2Expression
//
//  Purpose:
//      Creates a scalar variable out of three gradient vectors.
//
//  Programmer: Kevin Griffin
//  Creation:   Mon Aug  4 15:15:38 PDT 2014
//
//  Modifications:
//
// ****************************************************************************

class EXPRESSION_API avtLambda2Expression 
    : public avtMultipleInputExpressionFilter
{
  public:
                              avtLambda2Expression();
    virtual                  ~avtLambda2Expression();

    virtual const char       *GetType(void)  
                                    { return "avtLambda2Expression"; };
    virtual const char       *GetDescription(void)
                                 {return "Creating a scalar from three gradient vectors";};
    virtual int               NumVariableArguments() { return 3; }
    static double             Lambda2(const double *, const double *, const double *);
        
  protected:
    static void               EigenValues(double **, double evec[3]);
#ifdef _OPENMP
    template <typename InputType>
    void CalculateLambda2(const InputType *, const InputType *, const InputType *,
                          double *lambda2, const int numTuples);
#endif
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual int               GetVariableDimension(void) { return 1; };
    virtual bool              CanHandleSingletonConstants(void) {return true;};
};


#endif


