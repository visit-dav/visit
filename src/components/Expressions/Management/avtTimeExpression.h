// ************************************************************************* //
//                             avtTimeExpression.h                           //
// ************************************************************************* //

#ifndef AVT_TIME_EXPRESSION_H
#define AVT_TIME_EXPRESSION_H

#include <avtUnaryMathFilter.h>


// ****************************************************************************
//  Class: avtTimeExpression
//
//  Purpose:
//      Creates a constant, where the constant is the time.
//
//  Programmer: Hank Childs
//  Creation:   March 5, 2006
//
// ****************************************************************************

class EXPRESSION_API avtTimeExpression : public avtUnaryMathFilter
{
  public:
                             avtTimeExpression();
    virtual                 ~avtTimeExpression();

    virtual const char *     GetType(void) 
                                         { return "avtTimeExpression"; };
    virtual const char *     GetDescription(void) 
                                     { return "Generating constant (time)"; };

  protected:
    virtual void             DoOperation(vtkDataArray *in, vtkDataArray *out,
                                         int ncomponents, int ntuples);
    virtual int              GetNumberOfComponentsInOutput(int) { return 1; };
    virtual int              GetVariableDimension(void) { return 1; };
    virtual vtkDataArray    *CreateArray(vtkDataArray *);
};


#endif


