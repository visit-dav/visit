// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtIVPNek5000Field.h                             //
// ************************************************************************* //

#ifndef AVT_IVP_NEK5000_FIELD_H
#define AVT_IVP_NEK5000_FIELD_H

#include "visit-config.h"

#include "avtIVPVTKField.h"

#include <vtkDataSet.h>
#include <vtkPointData.h>

#include <ivp_exports.h>

#include <map>
#include <vector> 

struct findpts_local_data;

// ****************************************************************************
//  Class:  avtIVPNek5000Field
//
//  Purpose:
//    A wrapper class to allow the use of vtkDataSets as IVP fields for 
//    integral curve integration. Uses vtkInterpolatedVelocityField on top of 
//    the supplied vtkDataSet. 
//
//  Creationist: Allen Sanderson
//  Creation:    May 1, 2013
//
// ****************************************************************************

class IVP_API avtIVPNek5000Field: public avtIVPVTKField
{
 public:
  avtIVPNek5000Field( vtkDataSet* ds, avtCellLocator* loc ); 
  avtIVPNek5000Field( float *elementsPtr, int nelements, int dim, int planes );

  ~avtIVPNek5000Field();

  Result operator()( const double &t, const avtVector &v, avtVector &retV ) const;

  avtVector ConvertToCartesian(const avtVector& pt) const;
  avtVector ConvertToCylindrical(const avtVector& pt) const;

 protected:

  // Storage for the Nek5000 mesh and the vector.
  double *nek_pts[3], *nek_vec[3];

  // Variables needed for Nek5000 find points
  mutable struct findpts_local_data *nek_fld;
};

#endif
