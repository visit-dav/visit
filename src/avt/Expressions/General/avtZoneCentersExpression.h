// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtZoneCentersExpression.h                         //
// ************************************************************************* //

#ifndef AVT_ZONE_CENTERS_EXPRESSION_H
#define AVT_ZONE_CENTERS_EXPRESSION_H

#include <avtSingleInputExpressionFilter.h>


// ****************************************************************************
//  Class: avtZoneCentersExpression
//
//  Purpose:
//      Gets the cell center coordinates for the mesh
//
//  Programmer: Chris Laganella
//  Creation:   Mon Jan 31 15:54:58 EST 2022
//
//  Modifications:
//
// ****************************************************************************

class EXPRESSION_API avtZoneCentersExpression
    : public avtSingleInputExpressionFilter
{
  public:
                              avtZoneCentersExpression();
    virtual                  ~avtZoneCentersExpression();

    virtual const char       *GetType(void)
                                    { return "avtZoneCentersExpression"; };
    virtual const char       *GetDescription(void)
                                    { return "Calculates the zone center "
                                             "coordinates of the mesh."; };

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual int               GetVariableDimension() { return 3; }
    virtual bool              IsPointVariable()      { return false; }
};


#endif


