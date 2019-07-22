// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtMinSinCorner.h                             //
// ************************************************************************* //

#ifndef AVT_MIN_SIN_CORNER_H
#define AVT_MIN_SIN_CORNER_H


#include <avtSingleInputExpressionFilter.h>

class     vtkCell;


// ****************************************************************************
//  Class: avtMinSinCorner
//
//  Purpose:
//     Calculates the minimum Sin(corner-angle) for a cell.
//
//  Programmer: Matthew Wheeler
//  Creation:   20 May, 2013
//  (Derived from avtCornerAngle.h)
//
//  Modifications:
//
//
// ****************************************************************************

class EXPRESSION_API avtMinSinCorner : public avtSingleInputExpressionFilter
{
  public:
                                avtMinSinCorner();

    virtual const char         *GetType(void) { return "avtMinSinCorner"; };
    virtual const char         *GetDescription(void)
                                  { return "Calculating Sin(corner angles)"; };

    void                        SetOrderCCW(bool ordr) { orderCCW = ordr; };

  protected:
    bool                        orderCCW;

    virtual vtkDataArray       *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual bool                IsPointVariable(void)  { return false; };
    virtual int                 GetVariableDimension()   { return 1; };

    double                      GetMinSinCorner(vtkCell *);
};


#endif
