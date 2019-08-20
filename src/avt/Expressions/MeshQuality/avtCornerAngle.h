// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtCornerAngle.h                              //
// ************************************************************************* //

#ifndef AVT_CORNER_ANGLE_H
#define AVT_CORNER_ANGLE_H


#include <avtSingleInputExpressionFilter.h>

class     vtkCell;


// ****************************************************************************
//  Class: avtMinCornerAngle
//
//  Purpose:
//     Calculates the minimum or maximum corner angle for a cell.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2008
//
//  Modifications:
//
//    Hank Childs, Wed May 21 15:55:32 PDT 2008
//    Re-define GetVariableDimension.
//
// ****************************************************************************

class EXPRESSION_API avtCornerAngle : public avtSingleInputExpressionFilter
{
  public:
                                avtCornerAngle();

    virtual const char         *GetType(void) { return "avtCornerAngle"; };
    virtual const char         *GetDescription(void)
                                    { return "Calculating corner angles"; };

    void                        SetTakeMin(bool tm) { takeMin = tm; };
    
  protected:
    bool                        takeMin;

    virtual vtkDataArray       *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual bool                IsPointVariable(void)  { return false; };
    virtual int                 GetVariableDimension()   { return 1; };

    double                      GetCornerAngle(vtkCell *);
};


#endif


