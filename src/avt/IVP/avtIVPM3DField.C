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
//                             avtIVPM3DField.C                              //
// ************************************************************************* //

#include "avtIVPM3DField.h"

#define ELEMENT_SIZE 7
#define SCALAR_SIZE 20

// ****************************************************************************
//  Method: avtIVPM3DField constructor
//
//  Creationist: Allen Sanderson
//  Creation:   20 November 2009
//
// ****************************************************************************

avtIVPM3DField::avtIVPM3DField( vtkVisItInterpolatedVelocityField* velocity ) 
 : avtIVPVTKField(velocity)
{
  findElementNeighbors();
}


// ****************************************************************************
//  Method: avtIVPM3DField destructor
//
//  Creationist: Allen Sanderson
//  Creation:   20 November 2009
//
// ****************************************************************************

avtIVPM3DField::~avtIVPM3DField()
{
  free(neighbors);
  free(trigtable);
}


// ****************************************************************************
//  Method: findElementNeighbors
//
//  Creationist: Allen Sanderson
//  Creation:   20 November 2009
//
// ****************************************************************************
void avtIVPM3DField::findElementNeighbors()
{
  v_entry *vert_list;
  edge    *edge_list;
  double  *ptr, x[3], y[3], co, sn;
  int     el, vert, tri[3], vlen;

  /* Allocate, initialize neighbor table */
  neighbors = (int *)malloc(3 * nelms * sizeof(int));
  if (neighbors == NULL) {
    fputs("Insufficient memory in findElementNeighbors.\n", stderr);
    exit(1);
  }

  for (el=0; el<3*nelms; el++)
    neighbors[el] = -1;

  /* Allocate trig table */
  trigtable = (double *)malloc(2 * nelms * sizeof(double));
  if (trigtable == NULL) {
    fputs("Insufficient memory in findElementNeighbors.\n", stderr);
    exit(1);
  }

  /* Allocate, initialize temporary hash tables */
  vert_list = (v_entry *)malloc(3 * nelms * sizeof(v_entry));
  if (vert_list == NULL) {
    fputs("Insufficient memory in findElementNeighbors.\n", stderr);
    exit(1);
  }
  vlen = 0;
  edge_list = (edge *)malloc(3 * nelms * sizeof(edge));
  if (edge_list == NULL) {
    fputs("Insufficient memory in findElementNeighbors.\n", stderr);
    exit(1);
  }

  for (vert=0; vert<3*nelms; vert++)
    edge_list[vert].n = 0;

  /* Loop over elements, finding vertices, edges, neighbors */

  //For each element, the first 6 values are a, b, c, theta, x, and z.
  //The nodes of the element are located at
  // (x,z),
  // (x+(a+b)*cos(theta),z+(a+b)*sin(theta)),
  // (x+b*cos(theta)-c*sin(theta),z+b*sin(theta)+c*cos(theta)).

  for (el=0; el<nelms; el++) {
    ptr = elements + ELEMENT_SIZE*el;
    co = trigtable[2*el]     = cos(ptr[3]);
    sn = trigtable[2*el + 1] = sin(ptr[3]);

    x[0] = ptr[4];
    y[0] = ptr[5];

    x[1] = x[0] + (ptr[0] + ptr[1])*co;
    y[1] = y[0] + (ptr[0] + ptr[1])*sn;

    x[2] = x[0] + ptr[1]*co - ptr[2]*sn;
    y[2] = y[0] + ptr[1]*sn + ptr[2]*co;

    for (vert=0; vert<3; vert++)
      register_vert(vert_list, &vlen, x[vert], y[vert], tri+vert);

    for (vert=0; vert<3; vert++)
      add_edge(edge_list, tri, vert, el, neighbors);
  } /* end loop el */

  fprintf(stderr, "%d / %d unique vertices\n", vlen, 3*nelms);
  fprintf(stderr, "Neighbors of element 0: %d, %d, %d\n", neighbors[0],
          neighbors[1], neighbors[2]);

  /* Use unique vert list to find mesh bounds */
  Rmin = Rmax = vert_list[0].x;
  zmin = zmax = vert_list[0].y;
  for (vert=1; vert<vlen; vert++) {
    if (Rmin > vert_list[vert].x) Rmin = vert_list[vert].x;
    if (Rmax < vert_list[vert].x) Rmax = vert_list[vert].x;
    if (zmin > vert_list[vert].y) zmin = vert_list[vert].y;
    if (zmax < vert_list[vert].y) zmax = vert_list[vert].y;    
  }

  fprintf(stderr, "R bounds: %lf, %lf\nz bounds: %lf, %lf\n",
          Rmin, Rmax, zmin, zmax);
  
  free(vert_list);
  free(edge_list);
}


// ****************************************************************************
//  Method: register_vert
//
//  Creationist: Allen Sanderson
//  Creation:   20 November 2009
//
// ****************************************************************************
void avtIVPM3DField::register_vert(v_entry *vlist, int *len,
                                   double x, double y, int *index)
{
  const double tol=2.5e-13;
  double dx, dy;
  int    vert;

  for (vert=0; vert<(*len); vert++) {
    dx = x - vlist[vert].x;  dy = y - vlist[vert].y;
    if (dx*dx + dy*dy < tol) { /* Found in list! */
      *index = vert;
      return;
    }
  }

  /* Vertex not found -> add to end */
  vlist[*len].x = x;  vlist[*len].y = y;
  *index = *len;
  ++(*len);
}


// ****************************************************************************
//  Method: add_edge
//
//  Creationist: Allen Sanderson
//  Creation:   20 November 2009
//
// ****************************************************************************
void avtIVPM3DField::add_edge(edge *list, int *tri,
                              int side, int el, int *nlist)
{
  int  i, v1, v2, vo;
  edge *ed;

  /* Sort the vertices */
  v1 = tri[side];  v2 = tri[(side+1)%3];
  if (v1 < v2) { ed = list+v1;  vo = v2; }
  else         { ed = list+v2;  vo = v1; }

  /* See if this edge is already present */
  for (i=0; i<ed->n; i++)
    if (ed->o[i].v == vo) {           /* It is! Update the neighbor table. */
      nlist[3*el + side] = ed->o[i].el0;
      nlist[3*ed->o[i].el0 + ed->o[i].side] = el;
      return;
    }

  /* The edge was not present; add it. */
  ed->o[ed->n].v = vo;
  ed->o[ed->n].el0 = el;
  ed->o[ed->n].side = side;
  ed->n++;
}

// ****************************************************************************
//  Method: get_tri_coords2D
//
//  Creationist: Allen Sanderson
//  Creation:   20 November 2009
//
// ****************************************************************************
int avtIVPM3DField::get_tri_coords2D(avtVec &xin, double *xout)
{
  static int el=0;
  double     *tri;
  double     co, sn, rrel, zrel;
  int        last=-1, next, flag0, flag1, flag2;

  for (int count=0; count<nelms; ++count) {

    /* Compute coordinates local to the current element */
    co = trigtable[2*el];
    sn = trigtable[2*el + 1];

    tri = elements + ELEMENT_SIZE*el;

    rrel = xin[0] - (tri[4] + tri[1]*co);
    zrel = xin[2] - (tri[5] + tri[1]*sn);

    xout[0] = rrel*co + zrel*sn;  /* = xi */
    xout[1] = zrel*co - rrel*sn;  /* = eta */

    /* Determine whether point is inside element */
    if ((flag0 = ((*tri + tri[1])*xout[1] < 0.0))) /* "Outside" side 0? */
      if ((next = neighbors[3*el]) >= 0) {
        if (next == last) break;
        last = el;
        el = next;
        continue;
      }

    if ((flag1 = (*tri*xout[1] > tri[2]*(*tri - xout[0])))) /* "Outside" side 1? */
      if ((next = neighbors[3*el + 1]) >= 0) {
        if (next == last) break;
        last = el;
        el = next;
        continue;
      }

    if ((flag2 = (tri[2]*xout[0] < tri[1]*(xout[1] - tri[2])))) /* "Outside" side 2? */
      if ((next = neighbors[3*el + 2]) >= 0) {
        if (next == last) break;
        last = el;
        el = next;
        continue;
      }

    if (flag0 || flag1 || flag2)
      return -1;
    else
      break;
  } /* end loop count */

  /* fprintf(stderr, "Searched %d elements.\n", count); */
  return el;
}

// ****************************************************************************
//  Method: interpdR interpolation in dR
//
//  Creationist: Allen Sanderson
//  Creation:   20 November 2009
//
// ****************************************************************************
double avtIVPM3DField::interpdR(double *var, int el, double *lcoords)
{
  double *a = var + SCALAR_SIZE*el;
  double xi = lcoords[0], eta = lcoords[1], xicoef, etacoef;

  xicoef = a[1] + eta*(a[4] + eta*(a[8] + eta*(a[13] + a[18]*eta))) +
    xi*(2.0*(a[3] + eta*(a[7] + eta*(a[12] + a[17]*eta))) +
        xi*(3.0*(a[6] + eta*(a[11] + a[16]*eta)) +
            xi*(4.0*a[10] + xi*5.0*a[15])));

  etacoef = a[2] + xi*(a[4] + xi*(a[7] + a[11]*xi)) +
    eta*(2.0*(a[5] + xi*(a[8] + xi*(a[12] + a[16]*xi))) +
         eta*(3.0*(a[9] + xi*(a[13] + a[17]*xi)) +
              eta*(4.0*(a[14] + a[18]*xi) + eta*5.0*a[19])));

  return xicoef*trigtable[2*el] - etacoef*trigtable[2*el + 1];
}

// ****************************************************************************
//  Method: interpdz interpolation in dz
//
//  Creationist: Allen Sanderson
//  Creation:   20 November 2009
//
// ****************************************************************************
double avtIVPM3DField::interpdz(double *var, int el, double *lcoords)
{
  double *a = var + SCALAR_SIZE*el;
  double xi = lcoords[0], eta = lcoords[1], xicoef, etacoef;

  xicoef = a[1] + eta*(a[4] + eta*(a[8] + eta*(a[13] + a[18]*eta))) +
    xi*(2.0*(a[3] + eta*(a[7] + eta*(a[12] + a[17]*eta))) +
        xi*(3.0*(a[6] + eta*(a[11] + a[16]*eta)) +
            xi*(4.0*a[10] + xi*5.0*a[15])));

  etacoef = a[2] + xi*(a[4] + xi*(a[7] + a[11]*xi)) +
    eta*(2.0*(a[5] + xi*(a[8] + xi*(a[12] + a[16]*xi))) +
         eta*(3.0*(a[9] + xi*(a[13] + a[17]*xi)) +
              eta*(4.0*(a[14] + a[18]*xi) + eta*5.0*a[19])));

  return xicoef*trigtable[2*el + 1] + etacoef*trigtable[2*el];
}


// ****************************************************************************
//  Method: interpdR2 interpolation in dR2
//
//  Creationist: Allen Sanderson
//  Creation:   20 November 2009
//
// ****************************************************************************
double avtIVPM3DField::interpdR2(double *var, int el, double *lcoords)
{
  double *a = var + SCALAR_SIZE*el;
  double co=trigtable[2*el], sn=trigtable[2*el + 1];
  double xi = lcoords[0], eta = lcoords[1], xixicoef, etaetacoef, xietacoef;

  xixicoef = 2.0*(a[3] + eta*(a[7] + eta*(a[12] + eta*a[17]))) +
    xi*(6.0*(a[6] + eta*(a[11] + eta*a[16])) +
        xi*(12.0*a[10] + xi*20.0*a[15]));

  etaetacoef = 2.0*(a[5] + xi*(a[8] + xi*(a[12] + xi*a[16]))) +
    eta*(6.0*(a[9] + xi*(a[13] + xi*a[17])) +
         eta*(12.0*(a[14] + xi*a[18]) + 20.0*eta*a[19]));

  xietacoef = 2.0*a[4] +
    eta*(4.0*a[8] + xi*(8.0*a[12] + 12.0*xi*a[16]) +
         eta*(6.0*a[13] + 12.0*xi*a[17] + 8.0*eta*a[18])) +
    xi*(4.0*a[7] + 6.0*xi*a[11]);

  return (xixicoef*co - xietacoef*sn)*co + etaetacoef*sn*sn;
}


// ****************************************************************************
//  Method: interpdz2 interpolation in dz2
//
//  Creationist: Allen Sanderson
//  Creation:   20 November 2009
//
// ****************************************************************************
double avtIVPM3DField::interpdz2(double *var, int el, double *lcoords)
{
  double *a = var + SCALAR_SIZE*el;
  double co=trigtable[2*el], sn=trigtable[2*el + 1];
  double xi = lcoords[0], eta = lcoords[1], xixicoef, etaetacoef, xietacoef;

  xixicoef = 2.0*(a[3] + eta*(a[7] + eta*(a[12] + eta*a[17]))) +
    xi*(6.0*(a[6] + eta*(a[11] + eta*a[16])) +
        xi*(12.0*a[10] + xi*20.0*a[15]));

  etaetacoef = 2.0*(a[5] + xi*(a[8] + xi*(a[12] + xi*a[16]))) +
    eta*(6.0*(a[9] + xi*(a[13] + xi*a[17])) +
         eta*(12.0*(a[14] + xi*a[18]) + 20.0*eta*a[19]));

  xietacoef = 2.0*a[4] +
    eta*(4.0*a[8] + xi*(8.0*a[12] + 12.0*xi*a[16]) +
         eta*(6.0*a[13] + 12.0*xi*a[17] + 8.0*eta*a[18])) +
    xi*(4.0*a[7] + 6.0*xi*a[11]);

  return (xixicoef*sn + xietacoef*co)*sn + etaetacoef*co*co;
}


// ****************************************************************************
//  Method: interpdR interpolation in dR
//
//  Creationist: Allen Sanderson
//  Creation:   20 November 2009
//
// ****************************************************************************
double avtIVPM3DField::interpdRdz(double *var, int el, double *lcoords)
{
  double *a = var + SCALAR_SIZE*el;
  double co=trigtable[2*el], sn=trigtable[2*el + 1];
  double xi = lcoords[0], eta = lcoords[1], xixicoef, etaetacoef, xietacoef;

  xixicoef = 2.0*(a[3] + eta*(a[7] + eta*(a[12] + eta*a[17]))) +
    xi*(6.0*(a[6] + eta*(a[11] + eta*a[16])) +
        xi*(12.0*a[10] + xi*20.0*a[15]));

  etaetacoef = 2.0*(a[5] + xi*(a[8] + xi*(a[12] + xi*a[16]))) +
    eta*(6.0*(a[9] + xi*(a[13] + xi*a[17])) +
         eta*(12.0*(a[14] + xi*a[18]) + 20.0*eta*a[19]));

  xietacoef = a[4] +
    eta*(2.0*a[8] + xi*(4.0*a[12] + 6.0*xi*a[16]) +
         eta*(3.0*a[13] + 6.0*xi*a[17] + 4.0*eta*a[18])) +
    xi*(2.0*a[7] + 3.0*xi*a[11]);

  return (xixicoef - etaetacoef)*co*sn + xietacoef*(co*co - sn*sn);
}
