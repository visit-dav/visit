// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtSmartDivideExpression.h                       //
// ************************************************************************* //

#ifndef AVT_SMART_DIVIDE_EXPRESSION_H
#define AVT_SMART_DIVIDE_EXPRESSION_H

#include <avtMultipleInputMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtSmartDivideExpression
//
//  Purpose:
//      A filter that calculates the quotient of two inputs. Two additional
//      inputs supported for setting the value when encountering a divide-
//      by-zero and for defining a tolerance for zero.
//
//  Programmer: Eddie Rusu
//  Creation:   Tue Sep 24 09:07:44 PDT 2019
//
// ****************************************************************************

class EXPRESSION_API avtSmartDivideExpression
    : public avtMultipleInputMathExpression
{
  public:
             avtSmartDivideExpression();
    virtual ~avtSmartDivideExpression();

    virtual const char *GetType(void) 
                                     { return "avtSmartDivideExpression"; };
    virtual const char *GetDescription(void) 
                                     { return "Calculating smart division"; };
    virtual int         NumVariableArguments() {
                            return nProcessedArgs > 4 ? 4 : nProcessedArgs;
                          };

  protected:
    virtual vtkDataArray *DoOperation();
    virtual void          RecenterData(vtkDataSet*);
    virtual bool          CanHandleSingletonConstants(void) {return true;};
  
  private:
    double CheckZero(double, double);

    double tolerance;
    double value_if_zero;
    
};


#endif


