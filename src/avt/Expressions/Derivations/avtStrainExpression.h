// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtStrainExpression.h                               //
// ************************************************************************* //

#ifndef AVT_STRAIN_TENSOR_FILTER_H
#define AVT_STRAIN_TENSOR_FILTER_H

#include <avtMultipleInputExpressionFilter.h>


// ****************************************************************************
//  Class: avtStrainExpression
//
//  Purpose:
//      Calculates the strain tensor.
//
//  Programmer: Thomas R. Treadway
//  Creation:   Tue Nov  7 15:59:56 PST 2006
// 
//  Modifications:
//    Justin Privitera, Fri Sep 16 11:58:19 PDT 2022
//    Renamed avtStrainTensorExpression to avtStrainExpression.
//    Added CalculateEvolOrRelvol.
//
// ****************************************************************************

class EXPRESSION_API avtStrainExpression
    : public avtMultipleInputExpressionFilter
{
  public:
                           avtStrainExpression();
    virtual                ~avtStrainExpression();

    virtual const char     *GetType(void)  
                               { return "avtStrainExpression"; };
    virtual const char     *GetDescription(void)
                               {return "Calculating strain tensor";};
    virtual int            NumVariableArguments() { return 2; }

  protected:
    void                   HexPartialDerivative
                               (double dNx[8], double dNy[8], double dNz[8],
                                double coorX[8], double coorY[8], 
                                double coorZ[8]);
    vtkDataArray           *CalculateEvolOrRelvol(vtkDataSet *in, 
                                                  bool vol_strain);
    virtual avtVarType     GetVariableType(void) { return AVT_TENSOR_VAR; };
};

#endif


