// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtRelativeVolumeExpression.h                       //
// ************************************************************************* //

#ifndef AVT_RELATIVE_VOLUME_FILTER_H
#define AVT_RELATIVE_VOLUME_FILTER_H


#include <avtStrainExpression.h>


// ****************************************************************************
//  Class: avtRelativeVolumeExpression
//
//  Purpose:
//      Calculates the relative volume.
//
//  Programmer: Justin Privitera
//  Creation:   Fri Sep  9 10:37:12 PDT 2022
//
// ****************************************************************************

class EXPRESSION_API avtRelativeVolumeExpression
    : public avtStrainExpression
{
  public:
                               avtRelativeVolumeExpression();
    virtual                   ~avtRelativeVolumeExpression();

    virtual const char       *GetType(void)  
                               { return "avtRelativeVolumeExpression"; };
    virtual const char       *GetDescription(void)
                               {return "Calculating relative volume";};
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


