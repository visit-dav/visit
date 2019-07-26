// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtMIRvfExpression.h                             //
// ************************************************************************* //

#ifndef AVT_MIRVF_FILTER_H
#define AVT_MIRVF_FILTER_H

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;
class     ArgsExpr;
class     ExprPipelineState;
class     ConstExpr;


// ****************************************************************************
//  Class: avtMIRvfExpression
//
//  Purpose:
//      Calculates the volume fraction as computed by the MIR.
//          
//  Programmer: Hank Childs
//  Creation:   December 31, 2004
//
// ****************************************************************************

class EXPRESSION_API avtMIRvfExpression : public avtSingleInputExpressionFilter
{
  public:
                              avtMIRvfExpression();
    virtual                  ~avtMIRvfExpression();

    virtual const char       *GetType(void) { return "avtMIRvfExpression"; };
    virtual const char       *GetDescription(void)
                                            {return "Calculating MIR VF";};
    virtual void              ProcessArguments(ArgsExpr*, ExprPipelineState *);
    virtual int               NumVariableArguments(void) { return 3; };

  protected:
    bool                      issuedWarning;
    std::vector<std::string>  matNames;
    std::vector<int>          matIndices;
    std::string               zoneid_name;
    std::string               volume_name;

    virtual avtContract_p
                              ModifyContract(avtContract_p);

    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual bool              IsPointVariable(void)  { return false; };
    virtual void              PreExecute(void);
    virtual void              UpdateDataObjectInfo(void);

    void                      AddMaterial(ConstExpr *);
    void                      GetMaterialList(std::vector<bool> &, int);
};


#endif


