// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtMinCornerArea.h                            //
// ************************************************************************* //

#ifndef AVT_MIN_CORNER_AREA_H
#define AVT_MIN_CORNER_AREA_H


#include <avtSingleInputExpressionFilter.h>

class     vtkCell;


// ****************************************************************************
//  Class: avtMinCornerArea
//
//  Purpose:
//     Calculates the minimum corner area for a cell.
//
//  Programmer: Matthew Wheeler
//  Creation:   20 May, 2013
//  (Derived from avtCornerAngle.h)
//
//  Modifications:
//
//
// ****************************************************************************

class EXPRESSION_API avtMinCornerArea : public avtSingleInputExpressionFilter
{
  public:
                                avtMinCornerArea();

    virtual const char         *GetType(void) { return "avtMinCornerArea"; };
    virtual const char         *GetDescription(void)
                                 {return "Calculating minimum corner areas"; };

    void                        SetOrderCCW(bool ordr) { orderCCW = ordr; };

  protected:
    bool                        orderCCW;

    virtual vtkDataArray       *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual bool                IsPointVariable(void)  { return false; };
    virtual int                 GetVariableDimension()   { return 1; };

    double                      GetMinCornerArea(vtkCell *);
};


#endif
