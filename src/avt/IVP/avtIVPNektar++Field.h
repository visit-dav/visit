// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtIVPNektar++Field.h                          //
// ************************************************************************* //

#ifndef AVT_IVP_NEKTAR_PLUS_PLUS_FIELD_H
#define AVT_IVP_NEKTAR_PLUS_PLUS_FIELD_H

#include "visit-config.h"

#include "avtIVPVTKField.h"

#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkIntArray.h>

#include <ivp_exports.h>

#ifdef HAVE_NEKTAR_PP
#include <MultiRegions/ExpList.h>
#endif

#include <map>
#include <vector> 

struct findpts_local_data;

// ****************************************************************************
//  Class:  avtIVPNektarPPField
//
//  Purpose:
//    A wrapper class to allow the use of vtkDataSets as IVP fields for 
//    integral curve integration. Uses vtkInterpolatedVelocityField on top of 
//    the supplied vtkDataSet. 
//
//  Creationist: Allen Sanderson
//  Creation:    Nov 1, 2014
//
// ****************************************************************************

class IVP_API avtIVPNektarPPField: public avtIVPVTKField
{
 public:
  avtIVPNektarPPField( vtkDataSet* ds, avtCellLocator* loc ); 
  //  avtIVPNektarPPField( float *elementsPtr, int nelements, int dim, int planes );

  ~avtIVPNektarPPField();

  Result operator()( const double &t, const avtVector &v, avtVector &retV ) const;

  avtVector ConvertToCartesian(const avtVector& pt) const;
  avtVector ConvertToCylindrical(const avtVector& pt) const;

 protected:

  // Nektar++ field
  Nektar::MultiRegions::ExpListSharedPtr nektar_field[3];

  // Lookup table for going from VTK elements to Nektar elements
  int *nektar_element_lookup;
};

#endif
