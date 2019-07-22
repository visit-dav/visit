// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtCoordinateExtremaExpression.h                     //
// ************************************************************************* //

#ifndef AVT_COORDINATE_EXTREMA_FILTER_H
#define AVT_COORDINATE_EXTREMA_FILTER_H

#include <avtSingleInputExpressionFilter.h>


typedef enum
{
    CT_X = 0,
    CT_Y,
    CT_Z,
    CT_Radius,
    CT_Theta,
    CT_Phi
} CoordinateType;


// ****************************************************************************
//  Class: avtCoordinateExtremaExpression
//
//  Purpose:
//      Finds the extrema of a coordinate for a cell (example: minimum X 
//      coordinate)
//
//  Programmer: Hank Childs
//  Creation:   June 28, 2010
//
//  Modifications:
//
//    Hank Childs, Thu Jul  8 06:46:53 PDT 2010
//    Add support for polar coordinates; add parsing of arguments.
//
// ****************************************************************************

class EXPRESSION_API avtCoordinateExtremaExpression 
    : public avtSingleInputExpressionFilter
{
  public:
                              avtCoordinateExtremaExpression();
    virtual                  ~avtCoordinateExtremaExpression();

    virtual const char       *GetType(void)  
                                    { return "avtCoordinateExtremaExpression"; };
    virtual const char       *GetDescription(void)
                                    { return "Calculating coordinate extrema"; };

    virtual void              ProcessArguments(ArgsExpr*, ExprPipelineState *);

    void                      SetGetMinimum(bool gm) { getMinimum = gm; };

  protected:
    bool                      getMinimum;
    CoordinateType            coordinateType;

    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual int               GetVariableDimension() { return 1; }
    virtual bool              IsPointVariable()      { return false; }
};


#endif


