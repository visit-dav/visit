// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtIVPNIMRODField.h                             //
// ************************************************************************* //

#ifndef AVT_IVP_NIMROD_FIELD_H
#define AVT_IVP_NIMROD_FIELD_H

#include "avtIVPVTKField.h"
#include <avtMatrix.h>

#include <vtkDataSet.h>
#include <vtkPointData.h>

#include <ivp_exports.h>

typedef avtVector vec3;
typedef avtMatrix mat3;

// ****************************************************************************
//  Class:  avtIVPNIMRODField
//
//  Purpose:
//    A wrapper class to allow the use of vtkDataSets as IVP fields for 
//    integral curve integration. Uses vtkInterpolatedVelocityField on top of 
//    the supplied vtkDataSet. 
//
//  Programmer:  Allen Sanderson
//  Creation:    20 Nov 2009
//
// ****************************************************************************

class IVP_API avtIVPNIMRODField: public avtIVPVTKField
{
 protected:
  double lagrange_nodes[6][6];
  
 public:
  avtIVPNIMRODField( vtkDataSet* ds, avtCellLocator* loc ); 
  avtIVPNIMRODField( unsigned int nRad,
                     unsigned int nTheta,
                     unsigned int nPhi,
                     double *grid_fourier_series,
                     double *data_fourier_series );

  ~avtIVPNIMRODField();

  Result    operator()( const double &t, const avtVector &p, avtVector &retV ) const;

  avtVector ConvertToCartesian(const avtVector& pt) const;
  avtVector ConvertToCylindrical(const avtVector& pt) const;

 protected:
  template< class type >
    type* SetDataPointer( vtkDataSet *ds,
                          const type var,
                          const char* varname,
                          const int ncomponents );

  void lagrange_weights( unsigned int DEG, const double s, 
                         double* w = NULL, double *d = NULL ) const;

  void fourier_weights( unsigned int N, const double t, 
                        double* w, double* d = NULL ) const;

  void interpolate( double rad, double theta, double phi,
                    vec3* P, mat3* DRV ) const;

 public: 
  // Variables read as part of the mesh.
  double *grid_fourier_series;
  double *data_fourier_series;

  unsigned int Nrad, Ntheta, Nphi;

  // variables read from header attributes.
  unsigned int Drad;   // = 2;
  unsigned int Dtheta; // = 2;
};

#endif
