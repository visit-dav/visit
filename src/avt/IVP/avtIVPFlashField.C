// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtIVPFlashField.C                            //
// ************************************************************************* //

#include "avtIVPFlashField.h"

#include <DebugStream.h>

#include <vtkCellData.h>

// ****************************************************************************
//  Method: avtIVPFlashField constructor
//
//  Creationist: Allen Sanderson
//  Creation:   20 June 2011
//
// ****************************************************************************

avtIVPFlashField::avtIVPFlashField( vtkDataSet* dataset, 
                                    avtCellLocator* locator,
                                    double fact) : 
  avtIVPVTKField( dataset, locator ),
  factor(1.0/fact), B_vtkDataArray(0), E_vtkDataArray(0)
{
  order = 2;

  if( velCellBased )
  {
    B_vtkDataArray = dataset->GetCellData()->GetArray("B");
    E_vtkDataArray = dataset->GetCellData()->GetArray("E");
  }
  else
  {
    B_vtkDataArray = dataset->GetPointData()->GetArray("B");
    E_vtkDataArray = dataset->GetPointData()->GetArray("E");
  }

  if( B_vtkDataArray == 0 )
  {
    EXCEPTION1( ImproperUseException,
                "avtIVPFlashField: Can't locate 'B' vectors to interpolate." );
  }

  if( E_vtkDataArray == 0 )
  {
    EXCEPTION1( ImproperUseException,
                "avtIVPFlashField: Can't locate 'E' vectors to interpolate." );
  }
}


// ****************************************************************************
//  Method: avtIVPFlashField destructor
//
//  Creationist: Allen Sanderson
//  Creation:   20 June 2011
//
// ****************************************************************************

avtIVPFlashField::~avtIVPFlashField()
{
}


// ****************************************************************************
//  Method: avtIVPFlashField::operator
//
//  Evaluates the velocity via the Lorentz force.
//
//  Creationist: Allen Sanderson
//  Creation:   20 June 2011
//
//  Modifications:
//
// ****************************************************************************

avtIVPField::Result
avtIVPFlashField::operator()( const double &t,
                              const avtVector &p,
                              const avtVector &v,
                              avtVector& retV ) const
{
    if (FindCell(t, p) != OK)
        return OUTSIDE_SPATIAL;

  avtVector B, E;
  if (FindValue(B_vtkDataArray, B) && FindValue(E_vtkDataArray, E))
  {
      retV = factor * (E + Cross(v, B) );
      return OK;
  }
  else
      return OUTSIDE_SPATIAL;
}


// ****************************************************************************
//  Method: avtIVPFlashField::ConvertToCartesian
//
//  Purpose: Converts the coordinates from local cylindrical to
//      cartesian coordinates
//
//  Creationist: Allen Sanderson
//  Creation:   20 June 2011
//
// ****************************************************************************

avtVector 
avtIVPFlashField::ConvertToCartesian(const avtVector& pt) const
{
  return pt;
}

// ****************************************************************************
//  Method: avtIVPFlashField::ConvertToCylindrical
//
//  Purpose: Converts the coordinates from local cylindrical to
//      cartesian coordinates
//
//  Creationist: Allen Sanderson
//  Creation:   20 June 2011
//
// ****************************************************************************

avtVector 
avtIVPFlashField::ConvertToCylindrical(const avtVector& pt) const
{
  return avtVector(sqrt(pt[0]*pt[0]+pt[1]*pt[1]), atan2(pt[1], pt[0]), pt[2] );
}
