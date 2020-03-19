// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtDominantMaterialExpression.h                     //
// ************************************************************************* //

#ifndef AVT_DOMINANT_MATERIAL_EXPRESSION_H
#define AVT_DOMINANT_MATERIAL_EXPRESSION_H

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;

// ****************************************************************************
//  Class: avtDominantMaterialExpression
//
//  Purpose:
//      Returns the material number in a zone.  For mixed cells, returns the
//      one with the dominant volume fraction.
//          
//  Programmer: Hank Childs
//  Creation:   December 29, 2008
//
// ****************************************************************************

class EXPRESSION_API avtDominantMaterialExpression 
    : public avtSingleInputExpressionFilter
{
  public:
                              avtDominantMaterialExpression();
    virtual                  ~avtDominantMaterialExpression();

    virtual const char       *GetType(void) { return "avtDominantMaterialExpression"; };
    virtual const char       *GetDescription(void)
                                           {return "Finding dominant material";};
  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual bool              IsPointVariable(void)  { return false; };

    virtual avtContract_p     ModifyContract(avtContract_p);
};


#endif


