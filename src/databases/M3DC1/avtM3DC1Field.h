// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtM3DC1Field.h                              //
// ************************************************************************* //

#ifndef AVT_M3DC1_FIELD_H
#define AVT_M3DC1_FIELD_H

#include <vtkDataSet.h>
#include <vtkPointData.h>

#include <avtVector.h>

#include <map>
#include <vector> 

// ****************************************************************************
//  Class:  avtM3DC1Field
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

class avtM3DC1Field
{
 protected:

/* Local typedefs */
  typedef struct {
    double x,y;
  } vertex;
  
  typedef struct {
    int vertex, side, element;
  } edge;

  public:
    avtM3DC1Field( float *elementsPtr, int nelements, int dim, int planes );

    ~avtM3DC1Field();

    void findElementNeighbors();
    int register_vert(std::vector< vertex > &vlist,
                      double x, double y);

    void add_edge(std::multimap< int, edge > &edgeMaplist,
                  int *vertexIndexs, int side, int element, int *neighborList);

    int get_tri_coords2D(double *x, int el, double *xout) const;

    void interpBcomps(float *B, double *x, int element, double *xieta) const;

    float interp    (float *var, int el, double *lcoords) const;

    void interpdX(float *var, int el, double *lcoords,
                  double &xicoef, double &etacoef) const;
    void interpdX2(float *var, int el, double *lcoords,
                   double &xixicoef, double &etaetacoef,
                   double &xietacoef ) const;
    void interpdXdPhi(float *var, int el, double *lcoords,
                      double &xicoef, double &etacoef) const;

    float interpdR  (float *var, int el, double *lcoords) const;
    float interpdz  (float *var, int el, double *lcoords) const;
    float interpdPhi(float *var, int el, double *lcoords) const;

    float interpdR2 (float *var, int el, double *lcoords) const;
    float interpdz2 (float *var, int el, double *lcoords) const;

    float interpdRdz  (float *var, int el, double *lcoords) const;
    float interpdRdPhi(float *var, int el, double *lcoords) const;
    float interpdzdPhi(float *var, int el, double *lcoords) const;

 protected:

    // Variables calculated in findElementNeighbors (trigtable,
    // neighbors) or read as part of the mesh (elements).
    float *elements;
    double *trigtable;   /* Geometry of each triangle */
    int    *neighbors;   /* Element neighbor table for efficient searches */

    int tElements;       /* Number elements in a plane */

    int element_dimension;
    int nplanes;
    int element_size;
    int scalar_size;

 public:

  // 2D Variables variables on the mesh
  float *psi0, *f0;                  /* Equilibrium B field conponents */
  float *psinr, *psini, *fnr, *fni;  /* Complex perturbed field */
  
  // 3D Variables variables on the mesh
  float *I0;                         /* Equilibrium B field conponents */
  float *f, *psi, *I;                /* Perturbed field */

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
