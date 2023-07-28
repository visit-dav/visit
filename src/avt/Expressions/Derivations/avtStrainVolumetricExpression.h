// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtStrainVolumetricExpression.h                     //
// ************************************************************************* //

#ifndef AVT_STRAIN_VOLUMETRIC_FILTER_H
#define AVT_STRAIN_VOLUMETRIC_FILTER_H


#include <avtStrainExpression.h>


// ****************************************************************************
//  Class: avtStrainVolumetricExpression
//
//  Purpose:
//      Calculates the volumetric strain.
//
//  Programmer: Justin Privitera
//  Creation:   Fri Sep  9 10:37:12 PDT 2022
//
// ****************************************************************************

class EXPRESSION_API avtStrainVolumetricExpression
    : public avtStrainExpression
{
  public:
                               avtStrainVolumetricExpression();
    virtual                   ~avtStrainVolumetricExpression();

    virtual const char       *GetType(void)  
                               { return "avtStrainVolumetricExpression"; };
    virtual const char       *GetDescription(void)
                               {return "Calculating volumetric strain";};
    virtual int               NumVariableArguments() { return 2; }

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *in, 
                                             int currentDomainsIndex);
    virtual avtVarType        GetVariableType(void) { return AVT_SCALAR_VAR; };
    virtual int               GetNumberOfComponentsInOutput(int ncompsIn1,
                                                            int ncompsIn2)
                                         { return (ncompsIn1 > ncompsIn2
                                                    ? ncompsIn1
                                                    : ncompsIn2);
                                         };
};

#endif


