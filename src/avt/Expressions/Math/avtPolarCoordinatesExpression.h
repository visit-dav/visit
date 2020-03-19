// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                     avtPolarCoordinatesExpression.h                       //
// ************************************************************************* //

#ifndef AVT_POLAR_COORDINATES_FILTER_H
#define AVT_POLAR_COORDINATES_FILTER_H


#include <avtSingleInputExpressionFilter.h>




// ****************************************************************************
//  Class: avtPolarCoordinatesExpression
//
//  Purpose:
//      A filter that calculates the polar coordinates for each point.
//
//  Programmer: Hank Childs
//  Creation:   November 18, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class EXPRESSION_API avtPolarCoordinatesExpression 
    : public avtSingleInputExpressionFilter
{
  public:
                              avtPolarCoordinatesExpression();
    virtual                  ~avtPolarCoordinatesExpression();

    virtual const char       *GetType(void)   
                                  { return "avtPolarCoordinatesExpression"; };
    virtual const char       *GetDescription(void)
                                  { return "Calculating polar coordinates."; };

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual bool              IsPointVariable(void)  { return true; };  
    virtual int               GetVariableDimension() { return 3; }
};


#endif


