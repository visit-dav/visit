// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                  avtCylindricalCoordinatesExpression.h                    //
// ************************************************************************* //

#ifndef AVT_CYLINDRICAL_COORDINATES_FILTER_H
#define AVT_CYLINDRICAL_COORDINATES_FILTER_H


#include <avtSingleInputExpressionFilter.h>



// ****************************************************************************
//  Class: avtCylindricalCoordinatesExpression
//
//  Purpose:
//      A filter that calculates the cylindrical coordinates for each point.
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2005
//
// ****************************************************************************

class EXPRESSION_API avtCylindricalCoordinatesExpression 
    : public avtSingleInputExpressionFilter
{
  public:
                              avtCylindricalCoordinatesExpression();
    virtual                  ~avtCylindricalCoordinatesExpression();

    virtual const char       *GetType(void)   
                                { return "avtCylindricalCoordinatesExpression"; };
    virtual const char       *GetDescription(void)
                           { return "Calculating cylindrical coordinates."; };

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual bool              IsPointVariable(void)  { return true; };  
    virtual int               GetVariableDimension() { return 3; }
};


#endif


