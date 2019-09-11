// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtBinaryDivideExpression.h                         //
// ************************************************************************* //

#ifndef AVT_BINARY_DIVIDE_FILTER_H
#define AVT_BINARY_DIVIDE_FILTER_H

#include <avtBinaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtBinaryDivideExpression
//
//  Purpose:
//      A filter that calculates the quotient of its two inputs
//
//  Programmer: Sean Ahern
//  Creation:   Tue Jun 11 16:23:45 PDT 2002
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Hank Childs, Mon Jan 14 17:58:58 PST 2008
//    Allow constants to be created as singletons.
//
//    Eddie Rusu, Wed Sep 11 08:59:52 PDT 2019
//    Added the notion and logic of "smart division" or "guarded division".
//
// ****************************************************************************

class EXPRESSION_API avtBinaryDivideExpression : public avtBinaryMathExpression
{
  public:
                              avtBinaryDivideExpression();
                              avtBinaryDivideExpression(bool);
    virtual                  ~avtBinaryDivideExpression();

    virtual const char       *GetType(void) 
                                     { return "avtBinaryDivideExpression"; };
    virtual const char       *GetDescription(void) 
                                     { return "Calculating binary division"; };
    virtual int           NumVariableArguments() {
                              return nProcessedArgs > 4 ? 4 : nProcessedArgs;
                          };

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet*, int);
    virtual void              DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                  vtkDataArray *out, int ncomps, int ntuples);
    virtual bool              CanHandleSingletonConstants(void) {return true;};
  
  private:
    double tolerance;
    double value_if_zero;
    bool smart_division;

    vtkDataArray* DetermineCentering(avtCentering*, vtkDataSet*, const char*);
    double CheckZero(double, double);
};


#endif


