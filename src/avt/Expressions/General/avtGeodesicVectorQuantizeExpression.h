// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                    avtGeodesicVectorQuantizeExpression.h                  //
// ************************************************************************* //

#ifndef AVT_GEODESIC_VECTOR_QUANTIZE_EXPRESSION_H
#define AVT_GEODESIC_VECTOR_QUANTIZE_EXPRESSION_H

#include <avtMultipleInputExpressionFilter.h>

#include <vector>

class     vtkDataArray;
class     ArgsExpr;
class     ExprPipelineState;


// ****************************************************************************
//  Class: avtGeodesicVectorQuantizeExpression
//
//  Purpose:
//      Take vector variables and map it onto quantized geodesic coordinates.
//          
//  Programmer: Jeremy Meredith
//  Creation:   March 18, 2009
//
//  Modifications:
//    Jeremy Meredith, Fri Mar 20 15:55:17 EDT 2009
//    Allow a 0 spread (which means pick the single closest point).
//    Made the spread argument optional, with a default of 0.
//
// ****************************************************************************

class EXPRESSION_API avtGeodesicVectorQuantizeExpression 
    : public avtMultipleInputExpressionFilter
{
  public:
                              avtGeodesicVectorQuantizeExpression();
    virtual                  ~avtGeodesicVectorQuantizeExpression();

    virtual const char       *GetType(void) 
                          { return "avtGeodesicVectorQuantizeExpression"; }
    virtual const char       *GetDescription(void)
                          { return "Quantizing vector onto geodesic sphere"; };
    virtual void              ProcessArguments(ArgsExpr*, ExprPipelineState *);
    virtual int               NumVariableArguments(void) { return nargs; };

  protected:
    int       nargs;
    double    spread;

    virtual void              UpdateDataObjectInfo(void);
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual avtVarType        GetVariableType(void) { return AVT_ARRAY_VAR; };
};


#endif


