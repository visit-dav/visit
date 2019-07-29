// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtDisplacementExpression.h                         //
// ************************************************************************* //

#ifndef AVT_DISPLACEMENT_FILTER_H
#define AVT_DISPLACEMENT_FILTER_H

#include <avtMultipleInputExpressionFilter.h>

// ****************************************************************************
//  Class: avtDisplacementExpression
//
//  Purpose:
//      Calculates the displacements.
//
//  Programmer: Thomas R. Treadway
//  Creation:   Tue Nov 28 14:17:06 PST 2006
//
// ****************************************************************************

class EXPRESSION_API avtDisplacementExpression
    : public avtMultipleInputExpressionFilter
{
  public:
                               avtDisplacementExpression();
    virtual                   ~avtDisplacementExpression();

    virtual const char       *GetType(void)  
                               { return "avtDisplacementExpression"; };
    virtual const char       *GetDescription(void)
                               {return "Calculating the displacement";};
    virtual int               NumVariableArguments() { return 2; }

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *in, int currentDomainsIndex);
    virtual avtVarType        GetVariableType(void) { return AVT_VECTOR_VAR; };
    virtual int               GetNumberOfComponentsInOutput(int ncompsIn1,
                                                            int ncompsIn2)
                                         { return (ncompsIn1 > ncompsIn2
                                                    ? ncompsIn1
                                                    : ncompsIn2);
                                         };
    virtual int               GetVariableDimension(void); 
};

#endif
