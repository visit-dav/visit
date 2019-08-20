// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtSpecMFExpression.h                             //
// ************************************************************************* //

#ifndef AVT_SPECMF_FILTER_H
#define AVT_SPECMF_FILTER_H

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;
class     ArgsExpr;
class     ExprPipelineState;
class     ConstExpr;

// ****************************************************************************
//  Class: avtSpecMFExpression
//
//  Purpose:
//      Creates the species mass fraction at each cell.
//          
//  Programmer: Jeremy Meredith
//  Creation:   June  8, 2004
//
//  Modifications:
//
// ****************************************************************************

class EXPRESSION_API avtSpecMFExpression : public avtSingleInputExpressionFilter
{
  public:
                              avtSpecMFExpression();
    virtual                  ~avtSpecMFExpression();

    virtual const char       *GetType(void) { return "avtSpecMFExpression"; };
    virtual const char       *GetDescription(void)
                                     {return "Calculating Species Mass Frac";};
    virtual void              ProcessArguments(ArgsExpr*, ExprPipelineState *);

  protected:
    bool                      issuedWarning;

    virtual avtContract_p
                              ModifyContract(avtContract_p);

    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual bool              IsPointVariable(void)  { return false; };
    virtual void              PreExecute(void);

    void                      AddMaterial(ConstExpr *);
    void                      AddSpecies(ConstExpr *);

    bool                      weightByVF;
    std::vector<std::string>  matNames;
    std::vector<int>          matIndices;
    std::vector<std::string>  specNames;
    std::vector<int>          specIndices;
};


#endif


