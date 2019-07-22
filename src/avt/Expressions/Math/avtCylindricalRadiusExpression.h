// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtCylindricalRadiusExpression.h                    //
// ************************************************************************* //

#ifndef AVT_CYLINDRICAL_RADIUS_FILTER_H
#define AVT_CYLINDRICAL_RADIUS_FILTER_H


#include <avtSingleInputExpressionFilter.h>



// ****************************************************************************
//  Class: avtCylindricalRadiusExpression
//
//  Purpose:
//      A filter that calculates the cylindrical radius for each mesh 
//      coordinate. 
//
//  Programmer: Cyrus Harrison
//  Creation:   April 2, 2008
//
//  Modifications:
//    Kathleen Bonnell, Fri May  8 12:59:29 PDT 2009
//    Moved GetNumericVal to avtExpressionFilter so it could be used by 
//    other expressions.
//
// ****************************************************************************

class EXPRESSION_API avtCylindricalRadiusExpression 
    : public avtSingleInputExpressionFilter
{
  public:
                              avtCylindricalRadiusExpression();
    virtual                  ~avtCylindricalRadiusExpression();

    virtual const char       *GetType(void)   
                                { return "avtCylindricalRadiusExpression"; };
    virtual const char       *GetDescription(void)
                           { return "Calculating cylindrical radius."; };
    virtual void              ProcessArguments(ArgsExpr*, ExprPipelineState *);

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual bool              IsPointVariable(void)  { return true; };  
    virtual int               GetVariableDimension() { return 1; }

  private:
    double                    axisVector[3];
    
};


#endif


