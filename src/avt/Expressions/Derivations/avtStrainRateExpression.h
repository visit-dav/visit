// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtStrainRateExpression.h                           //
// ************************************************************************* //

#ifndef AVT_STRAIN_RATE_FILTER_H
#define AVT_STRAIN_RATE_FILTER_H


#include <avtStrainExpression.h>


// ****************************************************************************
//  Class: avtStrainRateExpression
//
//  Purpose:
//      Calculates the Rate based strain tensor.
//
//  Programmer: Thomas R. Treadway
//  Creation:   Wed Nov 15 12:57:36 PST 2006
// 
//  Modifications:
//    Justin Privitera, Fri Sep 16 11:58:19 PDT 2022
//    Renamed avtStrainTensorExpression to avtStrainExpression.
//
// ****************************************************************************

class EXPRESSION_API avtStrainRateExpression
    : public avtStrainExpression
{
  public:
                               avtStrainRateExpression();
    virtual                   ~avtStrainRateExpression();

    virtual const char       *GetType(void)  
                               { return "avtStrainRateExpression"; };
    virtual const char       *GetDescription(void)
                               {return "Calculating Rate based strain tensor";};
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


