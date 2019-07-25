// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtViscousStressExpression.h                         //
// ************************************************************************* //

#ifndef AVT_VISCOUS_STRESS_FILTER_H
#define AVT_VISCOUS_STRESS_FILTER_H

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;

// ****************************************************************************
//  Class: avtViscousStressExpression
//
//  Purpose:
//      Constructs a viscous stress tensor given a velocity vector.
//
//  Programmer: Cyrus Harrison
//  Creation:   June  5, 2007
//
//  Modifications:
//   Cyrus Harrison, Sun Jun 10 14:18:08 PDT 2007
//   Added CalculateVStress2D helper function.
//
// ****************************************************************************

class EXPRESSION_API avtViscousStressExpression
    : public avtSingleInputExpressionFilter
{
  public:
                             avtViscousStressExpression();
    virtual                 ~avtViscousStressExpression();

    virtual const char      *GetType(void)
                             { return "avtViscousStressExpression"; };
    virtual const char      *GetDescription(void)
                             { return "Calculating viscous stress";};

  protected:
    virtual vtkDataArray    *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual avtVarType       GetVariableType(void) { return AVT_TENSOR_VAR; };
    virtual bool             IsPointVariable(void) { return false; };
    virtual void             CalculateVStress2D(vtkDataSet *ds,
                                                vtkDataArray *vel,
                                                int idx,
                                                bool rz_mesh,
                                                double *vstress);


};

#endif
