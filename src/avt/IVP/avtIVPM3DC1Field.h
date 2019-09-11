// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtIVPM3DC1Field.h                             //
// ************************************************************************* //

#ifndef AVT_IVP_M3D_FIELD_H
#define AVT_IVP_M3D_FIELD_H

#include "avtIVPVTKField.h"

#include <vtkDataSet.h>
#include <vtkPointData.h>

#include <ivp_exports.h>

#include <map>
#include <vector> 


// ****************************************************************************
//  Class:  avtIVPM3DC1Field
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

class IVP_API avtIVPM3DC1Field: public avtIVPVTKField
{
 protected:

/* Local typedefs */
  typedef struct {
    double x,y;
  } vertex;
  
  typedef struct {
    int element, vertex, side;
  } edge;
  
 public:
  avtIVPM3DC1Field( vtkDataSet* ds, avtCellLocator* loc, double fact ); 
  avtIVPM3DC1Field( double *elementsPtr, int nelements, int dim, int planes );

  ~avtIVPM3DC1Field();

  virtual Result IsInside(const double& t, const avtVector &x) const;

  void findElementNeighbors();
  int register_vert(std::vector< vertex > &vlist, double x, double y);
  
  void add_edge(std::multimap< int, edge > &edgeMaplist,
                int *vertexIndexs, int side, int element, int *neighborList);
  
  int get_tri_coords2D(double *x, double *xout) const;

  Result    operator()( const double &t, const avtVector &v, avtVector &retV ) const;

  avtVector ConvertToCartesian(const avtVector& pt) const;
  avtVector ConvertToCylindrical(const avtVector& pt) const;

  void interpBcomps(double *B, double *x, int element, double *xieta) const;

  void reparameterizeBcomps( const avtVector &p, avtVector &v ) const;

  double interp(double *var, int el, double *lcoords) const;

  void interpdX(double *var, int el, double *lcoords,
               double &xicoef, double &etacoef) const;
  void interpdX2(double *var, int el, double *lcoords,
                 double &xixicoef, double &etaetacoef,
                 double &xietacoef ) const;
  void interpdXdPhi(double *var, int el, double *lcoords,
                    double &xicoef, double &etacoef) const;

  double interpdR  (double *var, int el, double *lcoords) const;
  double interpdz  (double *var, int el, double *lcoords) const;
  double interpdPhi(double *var, int el, double *lcoords) const;

  double interpdR2 (double *var, int el, double *lcoords) const;
  double interpdz2 (double *var, int el, double *lcoords) const;

  double interpdRdz  (double *var, int el, double *lcoords) const;
  double interpdRdPhi(double *var, int el, double *lcoords) const;
  double interpdzdPhi(double *var, int el, double *lcoords) const;

 protected:
  template< class type >
    type* SetDataPointer( vtkDataSet *ds,
                          const type var,
                          const char* varname,
                          const int component_size,
                          double factor = 1.0 );
  
  // Variables calculated in findElementNeighbors (trigtable,
  // neighbors) or read as part of the mesh (elements).
  double *elements;
  double *trigtable;   /* Geometry of each triangle */
  int    *neighbors;   /* Element neighbor table for efficient searches */
  
  int tElements;       /* Number elements in a plane */

  int element_dimension;
  int nplanes;
  int element_size;
  int scalar_size;

 public:

  bool reparameterize;
  double factor;

  // 2D Variables variables on the mesh
  double *psi0, *f0;                  /* Equilibrium B field conponents */
  double *psinr, *psini, *fnr, *fni;  /* Complex perturbed field */
  
  // 3D Variables variables on the mesh
  double *I0;                         /* Equilibrium B field conponents */
  double *f, *psi, *I;                /* Perturbed field */

  // Variables calculated in findElementNeighbors
//double Rmin, Rmax, zmin, zmax;  /* Mesh bounds */

  // unused variables read from header attributes
  // (xlim, zlim) or explicitly set (psilim).
//double xlim, zlim, psilim;      /* Information about limiting surface */

  // unused variables read from header attributes (ntime == nframes)
//int    nframes;

  // variables read from header attributes (linear == linflag,
  // ntor == tmode) or part of the mesh (nelms).
  int eqsubtract, linflag, nelms, tmode;
  
  // variables read from header attributes.
  double bzero, rzero;

  // variable based on attributes (bzero and rzero)
  double F0;                      /* Strength of vacuum toroidal field */  
};

#endif
