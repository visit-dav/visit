// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtStrainInfinitesimalExpression.h                  //
// ************************************************************************* //

#ifndef AVT_STRAIN_INFINITESIMAL_FILTER_H
#define AVT_STRAIN_INFINITESIMAL_FILTER_H


#include <avtStrainExpression.h>


// ****************************************************************************
//  Class: avtStrainInfinitesimalExpression
//
//  Purpose:
//      Calculates the Infinitesimal based strain tensor.
//
//  Programmer: Thomas R. Treadway
//  Creation:   Wed Nov 15 12:57:36 PST 2006
//
// ****************************************************************************

class EXPRESSION_API avtStrainInfinitesimalExpression
    : public avtStrainExpression
{
  public:
                               avtStrainInfinitesimalExpression();
    virtual                   ~avtStrainInfinitesimalExpression();

    virtual const char       *GetType(void)  
                               { return "avtStrainInfinitesimalExpression"; };
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


