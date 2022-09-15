// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtStrainGreenLagrangeExpression.h                  //
// ************************************************************************* //

#ifndef AVT_STRAIN_GREEN_LAGRANGE_FILTER_H
#define AVT_STRAIN_GREEN_LAGRANGE_FILTER_H


#include <avtStrainExpression.h>


// ****************************************************************************
//  Class: avtStrainGreenLagrangeExpression
//
//  Purpose:
//      Calculates the Green Lagrange based strain tensor.
//
//  Programmer: Thomas R. Treadway
//  Creation:   Tue Nov 14 12:59:38 PST 2006
//
// ****************************************************************************

class EXPRESSION_API avtStrainGreenLagrangeExpression
    : public avtStrainExpression
{
  public:
                               avtStrainGreenLagrangeExpression();
    virtual                   ~avtStrainGreenLagrangeExpression();

    virtual const char       *GetType(void)  
                               { return "avtStrainGreenLagrangeExpression"; };
    virtual const char       *GetDescription(void)
                               {return "Calculating strain tensor";};
    virtual int               NumVariableArguments() { return 2; }

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *in, int currentDomainsIndex);
    virtual avtVarType        GetVariableType(void) { return AVT_TENSOR_VAR; };
    virtual int               GetNumberOfComponentsInOutput(int ncompsIn1,
                                                            int ncompsIn2)
                                         { return (ncompsIn1 > ncompsIn2
                                                    ? ncompsIn1
                                                    : ncompsIn2);
                                         };
    virtual int               GetVariableDimension(void); 
};

#endif


