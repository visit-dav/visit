/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
//                             avtM3DC1Field.C                               //
// ************************************************************************* //

#include "avtM3DC1Field.h"

#include <DebugStream.h>

#include <vtkCellData.h>
#include <vtkIntArray.h>
#include <vtkFloatArray.h>

#include <avtCallback.h>
#include <InvalidVariableException.h>

#define ELEMENT_SIZE_2D 7
#define SCALAR_SIZE_2D 20

#define ELEMENT_SIZE_3D 9
#define SCALAR_SIZE_3D 80

// ****************************************************************************
//  Method: avtM3DC1Field constructor
//
//  Creationist: Joshua Breslau
//  Creation:   20 November 2009
//
// ****************************************************************************

avtM3DC1Field::avtM3DC1Field( float *elementsPtr,
                              int nelements, int dim, int planes ) 
  : elements( elementsPtr), trigtable(0), neighbors(0),
    element_dimension(dim), nplanes(planes),
    psi0(0), f0(0), psinr(0), psini(0), fnr(0), fni(0),
    I0(0), f(0), psi(0), I(0),
    eqsubtract(0), linflag(0), nelms(nelements), 
    tmode(0), bzero(0), rzero(0), F0(0)
{
  if( element_dimension == 2 )
  {
    element_size = ELEMENT_SIZE_2D;
    scalar_size = SCALAR_SIZE_2D;
  }  
  else //if( element_dimension == 3 )
  {
    element_size = ELEMENT_SIZE_3D;
    scalar_size = SCALAR_SIZE_3D;
  }

  // For the lookup table make the following assumptions:
  // Equally spaced planes,
  // Equal number of elements in each plane,
  // Matching element alignment at each plane.
  
  // As such, construct a table with using elements from one plane.
  tElements = nelms / nplanes;

  findElementNeighbors();
}


// ****************************************************************************
//  Method: avtM3DC1Field destructor
//
//  Creationist: Joshua Breslau
//  Creation:   20 November 2009
//
// ****************************************************************************

avtM3DC1Field::~avtM3DC1Field()
{
  if( neighbors ) free(neighbors);
  if( trigtable ) free(trigtable);

  if( elements ) delete [] elements;

  if( f0 )       delete [] f0;
  if( psi0 )     delete [] psi0;
  if( I0 )      delete [] I0;

  if( fnr )      delete [] fnr;
  if( fni )      delete [] fni;
  if( psinr )    delete [] psinr;
  if( psini )    delete [] psini;

  if( f )       delete [] f;
  if( psi )     delete [] psi;
  if( I )       delete [] I;
}


// ****************************************************************************
//  Method: findElementNeighbors
//
//  Creationist: Joshua Breslau
//  Creation:   20 November 2009
//
// ****************************************************************************
void avtM3DC1Field::findElementNeighbors()
{
  std::vector< vertex > vertexList;
  std::multimap< int, edge > edgeListMap;

  float   *ptr;
  //double  x[3], y[3], co, sn;
  int     el/*, vert, tri[3], vlen*/;

  /* Allocate, initialize neighbor table */
  neighbors = (int *)malloc(3 * tElements * sizeof(int));
  if (neighbors == NULL) {
    EXCEPTION1( InvalidVariableException,
                "M3DC1 findElementNeighbors - Insufficient memory for neighbors" );
  }

  for (el=0; el<3*tElements; el++)
    neighbors[el] = -1;

  /* Allocate trig table */
  trigtable = (double *)malloc(2 * nplanes*tElements * sizeof(double));
  if (trigtable == NULL) {
    EXCEPTION1( InvalidVariableException,
                "M3DC1 findElementNeighbors - Insufficient memory for trigtable" );
  }

  /* Loop over elements, finding vertices, edges, neighbors */

  //For each element, the first 6 values are a, b, c, theta, x, and z.
  //The nodes of the element are located at
  // (x,z),
  // (x+(a+b)*cos(theta),z+(a+b)*sin(theta)),
  // (x+b*cos(theta)-c*sin(theta),z+b*sin(theta)+c*cos(theta)).

  if( element_dimension == 2 )
  {
    for (el=0; el<tElements; el++) {
      ptr = elements + element_size*el;
      trigtable[2*el]     = cos(ptr[3]);
      trigtable[2*el + 1] = sin(ptr[3]);

      // co = trigtable[2*el];
      // sn = trigtable[2*el + 1];

      // x[0] = ptr[4];
      // y[0] = ptr[5];
      
      // x[1] = x[0] + (ptr[0] + ptr[1])*co;
      // y[1] = y[0] + (ptr[0] + ptr[1])*sn;
      
      // x[2] = x[0] + ptr[1]*co - ptr[2]*sn;
      // y[2] = y[0] + ptr[1]*sn + ptr[2]*co;
      
      // for (vert=0; vert<3; vert++)
      //   tri[vert] = register_vert(vertexList, x[vert], y[vert]);
      
      // for (vert=0; vert<3; vert++)
      //   add_edge(edgeListMap, tri, vert, el, neighbors);

    } /* end loop el */
  }
  else //if( element_dimension == 3 )
  {
    for (el=0; el<nplanes*tElements; el++) {
      ptr = elements + element_size*el;
      trigtable[2*el]     = cos(ptr[3]);
      trigtable[2*el + 1] = sin(ptr[3]);
    }
  }
//   fprintf(stderr, "%d / %d unique vertices\n", vlen, 3*tElements);
//   fprintf(stderr, "Neighbors of element 0: %d, %d, %d\n", neighbors[0],
//           neighbors[1], neighbors[2]);

  /* Use unique vert list to find mesh bounds */
//   Rmin = Rmax = vert_list[0].x;
//   zmin = zmax = vert_list[0].y;
//   for (vert=1; vert<vlen; vert++) {
//     if (Rmin > vert_list[vert].x) Rmin = vert_list[vert].x;
//     if (Rmax < vert_list[vert].x) Rmax = vert_list[vert].x;
//     if (zmin > vert_list[vert].y) zmin = vert_list[vert].y;
//     if (zmax < vert_list[vert].y) zmax = vert_list[vert].y;    
//   }

//   fprintf(stderr, "R bounds: %lf, %lf\nz bounds: %lf, %lf\n",
//           Rmin, Rmax, zmin, zmax);
}


// ****************************************************************************
//  Method: register_vert
//
//  Creationist: Joshua Breslau
//  Creation:   20 November 2009
//
// ****************************************************************************
int avtM3DC1Field::register_vert(std::vector< vertex > &vlist,
                                 double x, double y)
{
  const double tol=2.5e-13;

  for( size_t i=0; i<vlist.size(); i++ )
  {
    double dx = x - vlist[i].x;
    double dy = y - vlist[i].y;

    // Are the two points with the tollerance?
    if (dx*dx + dy*dy < tol)
    {
      return (int)i;
    }
  }

  // Vertex not found so add to list.
  vertex vert;

  vert.x = x;
  vert.y = y;

  vlist.push_back( vert );

  return (int)vlist.size() - 1;
}


// ****************************************************************************
//  Method: add_edge
//
//  Creationist: Joshua Breslau
//  Creation:   20 November 2009
//
// ****************************************************************************
void avtM3DC1Field::add_edge(std::multimap< int, edge > &edgeListMap,
                             int *vertexIndexs,
                             int side, int element, int *neighborList)
{
  int v0, v1, key, vertex;

  // Use the smallest vertex index as the key.
  v0 = vertexIndexs[side];
  v1 = vertexIndexs[(side+1)%3];

  // The edge list key is based on the vertex index which is unique.
  if (v0 < v1) { key = v0;  vertex = v1; }
  else         { key = v1;  vertex = v0; }

  // Find all of the edges with that key (i.e. edges that start with
  // the same vertex).
  std::pair<std::multimap<int,edge>::iterator,
            std::multimap<int,edge>::iterator>
    ret = edgeListMap.equal_range(key);

  // For all the edges returned find one with the ssame second vertex
  for (std::multimap<int, edge>::iterator  it=ret.first; it!=ret.second; ++it)
  {
    if( vertex == it->second.vertex )
    {
      // If the edge is present update the neighbor table.
      neighborList[3*element + side] = it->second.element;
      neighborList[3*it->second.element + it->second.side] = element;
        
      return;
    }
  }

  // No with either the first vertex index or no edge with the second
  // vertex index so create a new edge.
  edge newEdge;
  
  newEdge.vertex = vertex;
  newEdge.side = side;
  newEdge.element = element;
  
  edgeListMap.insert( std::pair< int, edge >( key, newEdge ) );
}


// ****************************************************************************
//  Method: get_tri_coords2D
//
//  Creationist: Joshua Breslau
//  Creation:   20 November 2009
//
// ****************************************************************************
int avtM3DC1Field::get_tri_coords2D(double *xin, int el, double *xout) const
{
  float  *tri;
  double co, sn, rrel, zrel;
  int    index;

  tri = elements + element_size*el;

  /* Compute coordinates local to the current element */
//   if( element_dimension == 2 )
    index = 2 * el;
//  else //if( element_dimension == 3 )
//    index = 2*(el%tElements);

  co = trigtable[index];
  sn = trigtable[index + 1];

  rrel = xin[0] - (tri[4] + tri[1]*co);
  zrel = xin[2] - (tri[5] + tri[1]*sn);
  
  xout[0] = rrel*co + zrel*sn;  /* = xi */
  xout[1] = zrel*co - rrel*sn;  /* = eta */

  if( element_dimension == 3 )  /* = zi */
  {
    float phi = xin[1];

    xout[2] = phi - tri[8]; // tri[8] = phi0

    if( xout[2] < -1.0e8 || tri[7] + 1.0e8 < xout[2] )
    {
      char buf[1024];

      strcpy( buf, "avtM3DC1Field::get_tri_coords2D - Get Triangle Coords 2d an element was specified but the point is outside the phi tollerance");

      /*sprintf( buf, "avtM3DC1Field::get_tri_coords2D - Get Triangle Coords 2d an element was specified but the point is outside the phi tollerance", 
               xin[0], xin[1], xin[2] ); */

      avtCallback::IssueWarning( buf );   
    }
  }

  return el;
}


// ****************************************************************************
//  Method: interpBcomps
//
//  Simultaneously interpolate all three cylindrical components of
//  magnetic field.
//
//  THIS CODE SHOULD NOT BE USED FOR FIELDLINE INTEGRATION WITH 2D
//  ELEMENTS.
//
//  Creationist: Joshua Breslau
//  Creation:   20 November 2009
//
// ****************************************************************************
void avtM3DC1Field::interpBcomps(float *B, double *x,
                                 int element, double *xieta) const
{
  float *B_r   = &(B[0]);
  float *B_z   = &(B[2]);
  float *B_phi = &(B[1]);

  if( element_dimension == 2 )
  {
    // Add in the equalibrium if was subtracted out.
    if( eqsubtract )
    {
      /* n=0 components */
      /* B_R = -1/R dpsi/dz - df'/dR */
      *B_r = -interpdz(psi0, element, xieta) / x[0];
      
      /* B_z = 1/R dpsi/dR - df'/dz */
      *B_z =  interpdR(psi0, element, xieta) / x[0];
      
      /* B_phi = d^2f/dR^2 + 1/R df/dR + d^2f/dz^2 + F0/R^2 */
      *B_phi = (interpdR2(f0, element, xieta) +
                interpdz2(f0, element, xieta) +
                (interpdR(f0, element, xieta) + F0/x[0]) / x[0]);
    }
    else
    {
      *B_r   = 0;
      *B_z   = 0;
      *B_phi = 0;
    }

    // Add in the perturbed parts. n>0 components, if applicable
    if (linflag)
    {
      double co = cos(tmode * x[1]);
      double sn = sin(tmode * x[1]);

      double dfnrdr = interpdR(fnr, element, xieta);
      double dfnidr = interpdR(fni, element, xieta);

      *B_r += (interpdz(psini, element, xieta)*sn -
               interpdz(psinr, element, xieta)*co) / x[0]
        + tmode*(dfnrdr*sn + dfnidr*co);

      *B_z += (interpdR(psinr, element, xieta)*co -
               interpdR(psini, element, xieta)*sn) / x[0]
        + tmode*(interpdz(fnr, element, xieta)*sn +
                 interpdz(fni, element, xieta)*co);

      *B_phi += ( (interpdR2(fnr, element, xieta) +
                   interpdz2(fnr, element, xieta))*co
                - (interpdR2(fni, element, xieta) +
                   interpdz2(fni, element, xieta))*sn +
                  (dfnrdr*co - dfnidr*sn) / x[0] );
    }
  }
  else //if( element_dimension == 3 )
  {
    // B = grad(psi) x grad (phi) + grad_perp d(f)/zi + I grad(phi)

    // Add in the equalibrium if was subtracted out.
    if( eqsubtract )
    {
      *B_r = -interpdz(psi0, element, xieta) / x[0] -
        interpdRdPhi(f0, element, xieta);

      *B_z =  interpdR(psi0, element, xieta) / x[0] -
        interpdzdPhi(f0, element, xieta);

      *B_phi = interp(I0, element, xieta) / x[0];
    }
    else
    {
      *B_r   = 0;
      *B_z   = 0;
      *B_phi = 0;
    }

    // Add in the perturbed parts.
    *B_r +=  -interpdz(psi, element, xieta) / x[0] -
      interpdRdPhi(f, element, xieta);

    *B_z +=   interpdR(psi, element, xieta) / x[0] -
      interpdzdPhi(f, element, xieta);

    *B_phi += interp(I, element, xieta) / x[0];
  }
}


// ****************************************************************************
//  Method: interp basic interpolation
//
//  Creationist: Joshua Breslau
//  Creation:   20 November 2009
//
// ****************************************************************************
float avtM3DC1Field::interp(float *var, int el, double *lcoords) const
{
  float *a = var + scalar_size*el;
  double xi = lcoords[0], eta = lcoords[1];
  double val = 0;

  if( element_dimension == 2 )
  {
    val = a[0] +
      eta*(a[2] + eta*(a[5] + eta*(a[9] + eta*(a[14] + eta*a[19])))) +
      xi*(a[1] + eta*(a[4] + eta*(a[8] + eta*(a[13] + eta*a[18]))) +
          xi*(a[3] + eta*(a[7] + eta*(a[12] + eta*a[17])) +
              xi*(a[6] + eta*(a[11] + eta*a[16]) +
                  xi*(a[10] + xi*a[15]))));
  }
  else //if( element_dimension == 3 )
  {
    double zi = lcoords[2];
    double zi_q = 1;

    for( unsigned int q=0; q<4; ++q )
    {
      val += (a[0] +
              eta*(a[2] + eta*(a[5] + eta*(a[9] + eta*(a[14] + eta*a[19])))) +
              xi*(a[1] + eta*(a[4] + eta*(a[8] + eta*(a[13] + eta*a[18]))) +
                  xi*(a[3] + eta*(a[7] + eta*(a[12] + eta*a[17])) +
                      xi*(a[6] + eta*(a[11] + eta*a[16]) +
                          xi*(a[10] + xi*a[15]))))) * zi_q;

      zi_q *= zi;

      a += scalar_size/4;
    }
  }

  return val;
}


// ****************************************************************************
//  Method: interp - interpolation for xicoef and etacoef used for dR
//  and dz
//
//  Creationist: Joshua Breslau
//  Creation:   20 November 2009
//
// ****************************************************************************
void avtM3DC1Field::interpdX(float *var, int el, double *lcoords,
                             double &xicoef, double &etacoef) const
{
  float *a = var + scalar_size*el;
  double xi = lcoords[0], eta = lcoords[1];

  if( element_dimension == 2 )
  {
    xicoef = a[1] + eta*(a[4] + eta*(a[8] + eta*(a[13] + a[18]*eta))) +
      xi*(2.0*(a[3] + eta*(a[7] + eta*(a[12] + a[17]*eta))) +
          xi*(3.0*(a[6] + eta*(a[11] + a[16]*eta)) +
              xi*(4.0*a[10] + xi*5.0*a[15])));
    
    etacoef = a[2] + xi*(a[4] + xi*(a[7] + a[11]*xi)) +
      eta*(2.0*(a[5] + xi*(a[8] + xi*(a[12] + a[16]*xi))) +
           eta*(3.0*(a[9] + xi*(a[13] + a[17]*xi)) +
              eta*(4.0*(a[14] + a[18]*xi) + eta*5.0*a[19])));
  }
  else //if( element_dimension == 3 )
  {
    double zi = lcoords[2];
    double zi_q = 1;

    xicoef  = 0;
    etacoef = 0;

    for( unsigned int q=0; q<4; ++q )
    {
      xicoef +=
        (a[1] + eta*(a[4] + eta*(a[8] + eta*(a[13] + a[18]*eta))) +
         xi*(2.0*(a[3] + eta*(a[7] + eta*(a[12] + a[17]*eta))) +
             xi*(3.0*(a[6] + eta*(a[11] + a[16]*eta)) +
                 xi*(4.0*a[10] + xi*5.0*a[15])))) * zi_q;
      
      etacoef +=
        (a[2] + xi*(a[4] + xi*(a[7] + a[11]*xi)) +
         eta*(2.0*(a[5] + xi*(a[8] + xi*(a[12] + a[16]*xi))) +
              eta*(3.0*(a[9] + xi*(a[13] + a[17]*xi)) +
                   eta*(4.0*(a[14] + a[18]*xi) + eta*5.0*a[19])))) * zi_q;
      
      zi_q *= zi; 

      a += scalar_size/4;
    }
  }
}


// ****************************************************************************
//  Method: interpdR interpolation in dR
//
//  Creationist: Joshua Breslau
//  Creation:   20 November 2009
//
// ****************************************************************************
float avtM3DC1Field::interpdR(float *var, int el, double *lcoords) const
{
  double xicoef, etacoef;
  int index;

  if( element_dimension == 2 )
    index = 2 * el;
  else //if( element_dimension == 3 )
    index = 2*(el%tElements);

  interpdX( var, el, lcoords, xicoef, etacoef );

  return xicoef*trigtable[index] - etacoef*trigtable[index + 1];
}

// ****************************************************************************
//  Method: interpdz interpolation in dz
//
//  Creationist: Joshua Breslau
//  Creation:   20 November 2009
//
// ****************************************************************************
float avtM3DC1Field::interpdz(float *var, int el, double *lcoords) const
{
  double xicoef, etacoef;
  int index;

  if( element_dimension == 2 )
    index = 2 * el;
  else //if( element_dimension == 3 )
    index = 2*(el%tElements);

  interpdX( var, el, lcoords, xicoef, etacoef );

  return xicoef*trigtable[index + 1] + etacoef*trigtable[index];
}


// ****************************************************************************
//  Method: interpdPhi interpolation on dPhi
//
//  Creationist: Joshua Breslau
//  Creation:   20 November 2009
//
// ****************************************************************************
float avtM3DC1Field::interpdPhi(float *var, int el, double *lcoords) const
{
  float *a = var + scalar_size*el;
  double xi = lcoords[0], eta = lcoords[1] /*, zi = lcoords[2]*/;
  double val = 0;

  if( element_dimension == 2 )
  {
    val = 0;
  }
  else //if( element_dimension == 3 )
  {
    double zi = lcoords[2];
    double zi_q = 1;

    // For skipping q = 0;
    a += scalar_size/4;

    for( unsigned int q=1; q<4; ++q )
    {
      val += (double) q *
        (a[0] +
         eta*(a[2] + eta*(a[5] + eta*(a[9] + eta*(a[14] + eta*a[19])))) +
         xi*(a[1] + eta*(a[4] + eta*(a[8] + eta*(a[13] + eta*a[18]))) +
             xi*(a[3] + eta*(a[7] + eta*(a[12] + eta*a[17])) +
                 xi*(a[6] + eta*(a[11] + eta*a[16]) +
                     xi*(a[10] + xi*a[15]))))) * zi_q;

      zi_q *= zi;

      a += scalar_size/4;
    }
  }

  return val;
}


// ****************************************************************************
//  Method: interp interpolation for xixicoef, etaetacoef, and
//  xietacoef used for dR2 and dz2
//
//  Creationist: Joshua Breslau
//  Creation:   20 November 2009
//
// ****************************************************************************
void avtM3DC1Field::interpdX2(float *var, int el, double *lcoords,
                                 double &xixicoef, double &etaetacoef,
                                 double &xietacoef ) const
{
  float *a = var + scalar_size*el;
  double xi = lcoords[0], eta = lcoords[1];

  if( element_dimension == 2 )
  {
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
  }
  else //if( element_dimension == 3 )
  {
    double zi = lcoords[2];
    double zi_q = 1;

    xixicoef   = 0;
    etaetacoef = 0;
    xietacoef  = 0;

    for( unsigned int q=0; q<4; ++q )
    {
      xixicoef +=
        (2.0*(a[3] + eta*(a[7] + eta*(a[12] + eta*a[17]))) +
         xi*(6.0*(a[6] + eta*(a[11] + eta*a[16])) +
             xi*(12.0*a[10] + xi*20.0*a[15]))) * zi_q;

      etaetacoef +=
        (2.0*(a[5] + xi*(a[8] + xi*(a[12] + xi*a[16]))) +
         eta*(6.0*(a[9] + xi*(a[13] + xi*a[17])) +
              eta*(12.0*(a[14] + xi*a[18]) + 20.0*eta*a[19]))) * zi_q;

      xietacoef +=
        (2.0*a[4] +
         eta*(4.0*a[8] + xi*(8.0*a[12] + 12.0*xi*a[16]) +
              eta*(6.0*a[13] + 12.0*xi*a[17] + 8.0*eta*a[18])) +
         xi*(4.0*a[7] + 6.0*xi*a[11])) * zi_q;

      zi_q *= zi; 

      a += scalar_size/4;
    }
  }
}


// ****************************************************************************
//  Method: interpdR2 interpolation in dR2
//
//  Creationist: Joshua Breslau
//  Creation:   20 November 2009
//
// ****************************************************************************
float avtM3DC1Field::interpdR2(float *var, int el, double *lcoords) const
{
  double xixicoef, etaetacoef, xietacoef;
  int index;

  if( element_dimension == 2 )
    index = 2 * el;
  else //if( element_dimension == 3 )
    index = 2*(el%tElements);

  interpdX2(var, el, lcoords, xixicoef, etaetacoef, xietacoef);

  double co=trigtable[index], sn=trigtable[index + 1];

  return (xixicoef*co - xietacoef*sn)*co + etaetacoef*sn*sn;
}


// ****************************************************************************
//  Method: interpdz2 interpolation in dz2
//
//  Creationist: Joshua Breslau
//  Creation:   20 November 2009
//
// ****************************************************************************
float avtM3DC1Field::interpdz2(float *var, int el, double *lcoords) const
{
  double xixicoef, etaetacoef, xietacoef;
  int index;

  if( element_dimension == 2 )
    index = 2 * el;
  else //if( element_dimension == 3 )
    index = 2*(el%tElements);

  interpdX2(var, el, lcoords, xixicoef, etaetacoef, xietacoef);

  double co=trigtable[index], sn=trigtable[index + 1];

  return (xixicoef*sn + xietacoef*co)*sn + etaetacoef*co*co;
}


// ****************************************************************************
//  Method: interpdRdz interpolation in dRdz
//
//  Creationist: Joshua Breslau
//  Creation:   20 November 2009
//
// ****************************************************************************
float avtM3DC1Field::interpdRdz(float *var, int el, double *lcoords) const
{
  float *a = var + scalar_size*el;
  double xi = lcoords[0], eta = lcoords[1];
  double xixicoef, etaetacoef, xietacoef;
  int index;

  if( element_dimension == 2 )
  {
    index = 2 * el;

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

  }
  else //if( element_dimension == 3 )
  {
    index = 2*(el%tElements);

    double zi = lcoords[2];
    double zi_q = 1;

    xixicoef   = 0;
    etaetacoef = 0;
    xietacoef  = 0;

    for( unsigned int q=0; q<4; ++q )
    {
      xixicoef += 2.0*(a[3] + eta*(a[7] + eta*(a[12] + eta*a[17]))) +
        xi*(6.0*(a[6] + eta*(a[11] + eta*a[16])) +
            xi*(12.0*a[10] + xi*20.0*a[15])) * zi_q;

      etaetacoef += 2.0*(a[5] + xi*(a[8] + xi*(a[12] + xi*a[16]))) +
        eta*(6.0*(a[9] + xi*(a[13] + xi*a[17])) +
             eta*(12.0*(a[14] + xi*a[18]) + 20.0*eta*a[19])) * zi_q;

      xietacoef += a[4] +
        eta*(2.0*a[8] + xi*(4.0*a[12] + 6.0*xi*a[16]) +
             eta*(3.0*a[13] + 6.0*xi*a[17] + 4.0*eta*a[18])) +
        xi*(2.0*a[7] + 3.0*xi*a[11]) * zi_q;

      zi_q *= zi; 

      a += scalar_size/4;
    }
  }

  double co=trigtable[index], sn=trigtable[index + 1];

  return (xixicoef - etaetacoef)*co*sn + xietacoef*(co*co - sn*sn);
}


// ****************************************************************************
//  Method: interpdR interpolation in dXdPhi
//
//  Creationist: Joshua Breslau
//  Creation:   20 November 2009
//
// ****************************************************************************
void avtM3DC1Field::interpdXdPhi(float *var, int el, double *lcoords,
                                    double &xicoef, double &etacoef) const
{
  float *a = var + scalar_size*el;
  double xi = lcoords[0], eta = lcoords[1];

  if( element_dimension == 2 )
  {
    xicoef  = 0;
    etacoef = 0;
  }
  else //if( element_dimension == 3 )
  {
    xicoef  = 0;
    etacoef = 0;

    double zi = lcoords[2];
    double zi_q = 1;

    // For skipping q = 0;
    a += scalar_size/4;

    for( unsigned int q=1; q<4; ++q )
    {
      xicoef += (double) q *
        (a[1] + eta*(a[4] + eta*(a[8] + eta*(a[13] + a[18]*eta))) +
         xi*(2.0*(a[3] + eta*(a[7] + eta*(a[12] + a[17]*eta))) +
             xi*(3.0*(a[6] + eta*(a[11] + a[16]*eta)) +
                 xi*(4.0*a[10] + xi*5.0*a[15])))) * zi_q;
      
      etacoef += (double) q * 
        (a[2] + xi*(a[4] + xi*(a[7] + a[11]*xi)) +
         eta*(2.0*(a[5] + xi*(a[8] + xi*(a[12] + a[16]*xi))) +
              eta*(3.0*(a[9] + xi*(a[13] + a[17]*xi)) +
                   eta*(4.0*(a[14] + a[18]*xi) + eta*5.0*a[19])))) * zi_q;

      zi_q *= zi;

      a += scalar_size/4;
    }
  }
}


// ****************************************************************************
//  Method: interpdR interpolation in dRdPhi
//
//  Creationist: Joshua Breslau
//  Creation:   20 November 2009
//
// ****************************************************************************
float avtM3DC1Field::interpdRdPhi(float *var, int el, double *lcoords) const
{
  double xicoef, etacoef;
  int index;

  if( element_dimension == 2 )
    index = 2 * el;
  else //if( element_dimension == 3 )
    index = 2*(el%tElements);

  interpdXdPhi(var, el, lcoords, xicoef, etacoef);

  return xicoef*trigtable[index] - etacoef*trigtable[index + 1];
}


// ****************************************************************************
//  Method: interpdz interpolation in dzdPhi
//
//  Creationist: Joshua Breslau
//  Creation:   20 November 2009
//
// ****************************************************************************
float avtM3DC1Field::interpdzdPhi(float *var, int el, double *lcoords) const
{
  double xicoef, etacoef;
  int index;

  if( element_dimension == 2 )
    index = 2 * el;
  else //if( element_dimension == 3 )
    index = 2*(el%tElements);

  interpdXdPhi(var, el, lcoords, xicoef, etacoef);

  return xicoef*trigtable[index + 1] + etacoef*trigtable[index];
}
