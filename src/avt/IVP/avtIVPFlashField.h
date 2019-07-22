// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtIVPFlashField.h                             //
// ************************************************************************* //

#ifndef AVT_IVP_FLASH_FIELD_H
#define AVT_IVP_FLASH_FIELD_H

#include "avtIVPVTKField.h"

#include <vtkDataSet.h>
#include <vtkPointData.h>

#include <ivp_exports.h>

#include <map>
#include <vector> 


// ****************************************************************************
//  Class:  avtIVPFlashField
//
//  Purpose:
//    A wrapper class to allow the use of vtkDataSets as IVP fields for 
//    integral curve integration. Uses vtkInterpolatedVelocityField on top of 
//    the supplied vtkDataSet. 
//
//  Creationist: Allen Sanderson
//  Creation:   20 June 2011
//
// ****************************************************************************

class IVP_API avtIVPFlashField: public avtIVPVTKField
{
 public:
  avtIVPFlashField( vtkDataSet* ds, avtCellLocator* loc, double fact ); 

  ~avtIVPFlashField();

  Result    operator()( const double &t,
                        const avtVector &p,
                        const avtVector &v,
                              avtVector &retV ) const;

  avtVector ConvertToCartesian(const avtVector& pt) const;
  avtVector ConvertToCylindrical(const avtVector& pt) const;

  double factor;

  // 3D Variables variables on the mesh
  vtkDataArray *B_vtkDataArray;    // Magnetic field conponents
  vtkDataArray *E_vtkDataArray;    // Electric field conponents
};

#endif
