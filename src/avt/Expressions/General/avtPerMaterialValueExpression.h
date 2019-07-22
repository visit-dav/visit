// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                   avtPerMaterialValueExpression.h                         //
// ************************************************************************* //

#ifndef AVT_PER_MATERIAL_VALUE_FILTER_H
#define AVT_PER_MATERIAL_VALUE_FILTER_H

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;
class     ArgsExpr;
class     ExprPipelineState;

// ****************************************************************************
//  Class: avtPerMaterialValueExpression
//
//  Purpose:
//      Extracts the correct per material zonal value for a mixed variable.
//          
//  Programmer: Cyrus Harrison
//  Creation:   January 29, 2008
//
//  Modifications:
//    Cyrus Harrison, Tue Feb 19 13:20:50 PST 2008
//    Added doPostGhost memeber. 
//
//    Cyrus Harrison, Fri May 26 16:35:57 PDT 2017
//    Added preexecute. 
//
// ****************************************************************************

class EXPRESSION_API avtPerMaterialValueExpression
: public avtSingleInputExpressionFilter
{
  public:
                              avtPerMaterialValueExpression();
    virtual                  ~avtPerMaterialValueExpression();

    virtual const char       *GetType(void) 
                                  { return "avtPerMaterialValueExpression"; };
    virtual const char       *GetDescription(void)
                                  {return "Extracting mixed material value.";};
    virtual void              ProcessArguments(ArgsExpr*, ExprPipelineState *);

  protected:
    virtual int               GetVariableDimension(void) { return 1; };
    
    virtual void              PreExecute(void);

    virtual avtContract_p
                              ModifyContract(avtContract_p);

    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual bool              IsPointVariable(void)  { return false; };
    
    bool                      doPostGhost;
    int                       matNumber;
    std::string               matName;
};


#endif


