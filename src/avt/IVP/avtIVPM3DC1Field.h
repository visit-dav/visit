/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                            avtIVPM3DC1Field.h                             //
// ************************************************************************* //

#ifndef AVT_IVP_M3D_FIELD_H
#define AVT_IVP_M3D_FIELD_H

#include "avtIVPVTKField.h"

#include <vtkVisItInterpolatedVelocityField.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>

#include <ivp_exports.h>


// ****************************************************************************
//  Class:  avtIVPM3DC1Field
//
//  Purpose:
//    A wrapper class to allow the use of vtkDataSets as IVP fields for 
//    streamline integration. Uses vtkInterpolatedVelocityField on top of 
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
  } v_entry;
  
  typedef struct {
    int el0, v, side;
  } d_edge;
  
  typedef struct{
    d_edge o[8];
    int    n;
  } edge;
  
  public:
    avtIVPM3DC1Field( vtkVisItInterpolatedVelocityField* velocity ); 
    avtIVPM3DC1Field( float *elementsPtr, int nelements );

    ~avtIVPM3DC1Field();

    vtkVisItInterpolatedVelocityField* GetBaseField() { return iv; }    

    void findElementNeighbors();
    void register_vert(v_entry *vlist, int *len,
                       double x, double y, int *index);
    void add_edge(edge *list, int *tri, int side, int el, int *nlist);

    int get_tri_coords2D(double *x, double *xout);
    int get_tri_coords2D(double *x, int el, double *xout);

    float interp    (float *var, int el, double *lcoords);
    float interpdR  (float *var, int el, double *lcoords);
    float interpdz  (float *var, int el, double *lcoords);
    float interpdR2 (float *var, int el, double *lcoords);
    float interpdz2 (float *var, int el, double *lcoords);
    float interpdRdz(float *var, int el, double *lcoords);

 protected:
    template< class type >
      type* SetDataPointer( vtkDataSet *ds,
                            const type var,
                            const char* varname,
                            const int ntuples,
                            const int ncomponents );

    // Variables calculated in findElementNeighbors (trigtable,
    // neighbors) or read as part of the mesh (elements).
    float *elements;
    double *trigtable;   /* Geometry of each triangle */
    int    *neighbors;   /* Element neighbor table for efficient searches */

 public:
    // FIX ME - variables on the mesh
    float *psi0, *f0;                  /* Equilibrium field */
    float *psinr, *psini, *fnr, *fni;  /* Complex perturbed field */

    // FIX ME - variable based on attributes (bzero and rzero)
    double F0;                      /* Strength of vacuum toroidal field */
    
    // Variables calculated in findElementNeighbors
    double Rmin, Rmax, zmin, zmax;  /* Mesh bounds */

    // FIX ME - unused variables read from header attributes
    // (xlim, zlim) or explicitly set (psilim).
//  double xlim, zlim, psilim;      /* Information about limiting surface */

    // FIX ME - unused variables read from header attributes (ntime == nframes)
//  int    nframes;

    // FIX ME - variables read from header attributes (linear == linflag,
    // ntor == tmode) or part of the mesh (nelms).
    int linflag, nelms, tmode;

    // FIX ME - variables read from header attributes.
    double bzero, rzero;
};

#endif
