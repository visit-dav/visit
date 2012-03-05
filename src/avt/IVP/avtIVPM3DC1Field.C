/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
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
//                             avtIVPM3DC1Field.C                            //
// ************************************************************************* //

#include "avtIVPM3DC1Field.h"

#include <DebugStream.h>

#include <vtkCellData.h>
#include <vtkIntArray.h>
#include <vtkFloatArray.h>

#include <InvalidVariableException.h>

#define ELEMENT_SIZE_2D 7
#define SCALAR_SIZE_2D 20

#define ELEMENT_SIZE_3D 9
#define SCALAR_SIZE_3D 80

// ****************************************************************************
//  Method: avtIVPM3DC1Field constructor
//
//  Creationist: Joshua Breslau
//  Creation:   20 November 2009
//
// ****************************************************************************

avtIVPM3DC1Field::avtIVPM3DC1Field( vtkDataSet* dataset, 
                                    avtCellLocator* locator,
                                    double fact) : 
  avtIVPVTKField( dataset, locator ),
  elements(0), trigtable(0), neighbors(0),
  f0(0), psi0(0), fnr(0), fni(0), psinr(0), psini(0),
  I0(0), f(0), psi(0), I(0),
  eqsubtract(0), linflag(0), tmode(0), bzero(0), rzero(0), F0(0), factor(fact),
  nelms(0), element_dimension(0), nplanes(0), reparameterize(false)
{
  // Pick off all of the data stored with the vtk field.
  // Get the numver of elements for checking the validity of the data.

  // Because the triangluar mesh is defined by using non unique points
  // and the data is cell centered data VisIt moves it out to the
  // nodes for STREAMLINES thus there are 3 times the number of
  // original values.
  if( ds->GetPointData()->GetArray("hidden/elements") )
  {
    nelms =
      ds->GetPointData()->GetArray("hidden/elements")->GetNumberOfTuples() / 3;

    element_size =
      ds->GetPointData()->GetArray("hidden/elements")->GetNumberOfComponents();
  }
  // 2.0 Change data is at the cells for POINCARE
  else
  {
    nelms =
      ds->GetCellData()->GetArray("hidden/elements")->GetNumberOfTuples();

    element_size =
      ds->GetCellData()->GetArray("hidden/elements")->GetNumberOfComponents();
  }

  if( element_size == ELEMENT_SIZE_2D )
  {
    element_dimension = 2;
    scalar_size = SCALAR_SIZE_2D;
  }
  else //if( element_size == ELEMENT_SIZE_3D )
  {
    element_dimension = 3;
    scalar_size = SCALAR_SIZE_3D;
  }

  // Dummy variable to the template class
  int   *intPtr, intVar = 0;
  float *fltPtr, fltVar = 0;
    
  // The mesh elements.
  elements =
    SetDataPointer( ds, fltVar, "hidden/elements", element_size );

  // Equalibrium field
  intPtr = SetDataPointer( ds, intVar, "hidden/header/eqsubtract", 1 );
  eqsubtract = intPtr[0];
  delete [] intPtr;

  if( eqsubtract )
  {
    psi0 = SetDataPointer( ds, fltVar, "hidden/equilibrium/psi", scalar_size );
    f0   = SetDataPointer( ds, fltVar, "hidden/equilibrium/f",   scalar_size );
    
    if( element_size == ELEMENT_SIZE_3D )
      I0 = SetDataPointer( ds, fltVar, "hidden/equilibrium/I", scalar_size );
  }


  if( element_size == ELEMENT_SIZE_2D )
  {
    nplanes = 1;

    intPtr = SetDataPointer( ds, intVar, "hidden/header/linear", 1 );
    linflag = intPtr[0];
    delete [] intPtr;

    intPtr = SetDataPointer( ds, intVar, "hidden/header/ntor", 1 );
    tmode = intPtr[0];
    delete [] intPtr;

    fltPtr = SetDataPointer( ds, fltVar, "hidden/header/bzero", 1 );
    bzero = fltPtr[0];
    delete [] fltPtr;

    fltPtr = SetDataPointer( ds, fltVar, "hidden/header/rzero", 1 );
    rzero = fltPtr[0];
    delete [] fltPtr;

    // Now set some values using the above data.
    F0 = -bzero * rzero;

    // Vector values from the field.
    if( linflag )
    {
      psinr = SetDataPointer( ds, fltVar, "hidden/psi",   scalar_size, factor );
      psini = SetDataPointer( ds, fltVar, "hidden/psi_i", scalar_size, factor );
      fnr   = SetDataPointer( ds, fltVar, "hidden/f",   scalar_size, factor );
      fni   = SetDataPointer( ds, fltVar, "hidden/f_i", scalar_size, factor );
    }
  }
  else //if( element_size == ELEMENT_SIZE_3D )
  {
    // Single values from the header attributes.
    intPtr = SetDataPointer( ds, intVar, "hidden/header/nplanes", 1 );
    nplanes = intPtr[0];
    delete [] intPtr;

    f   = SetDataPointer( ds, fltVar, "hidden/f",   scalar_size, factor );
    psi = SetDataPointer( ds, fltVar, "hidden/psi", scalar_size, factor );
    I   = SetDataPointer( ds, fltVar, "hidden/I"  , scalar_size, factor );
  }

  // As such, construct a table with using elements from one plane.
  tElements = nelms / nplanes;

  findElementNeighbors();
}


// ****************************************************************************
//  Method: avtIVPM3DC1Field constructor
//
//  Creationist: Joshua Breslau
//  Creation:   20 November 2009
//
// ****************************************************************************

avtIVPM3DC1Field::avtIVPM3DC1Field( float *elementsPtr,
                                    int nelements, int dim, int planes )
  : avtIVPVTKField( 0, 0 ),
    elements( elementsPtr ), neighbors(0),
    f0(0), psi0(0), fnr(0), fni(0), psinr(0), psini(0),
    I0(0), f(0), psi(0), I(0),
    eqsubtract(0), linflag(0), tmode(0), bzero(0), rzero(0), F0(0),
    nelms(nelements), element_dimension(dim), nplanes(planes)
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
//  Method: avtIVPM3DC1Field destructor
//
//  Creationist: Joshua Breslau
//  Creation:   20 November 2009
//
// ****************************************************************************

avtIVPM3DC1Field::~avtIVPM3DC1Field()
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
//  Method: avtIVPM3DC1Field SetDataPointer
//
//  Creationist: Joshua Breslau
//  Creation:   20 November 2009
//
// ****************************************************************************

template< class type >
type* avtIVPM3DC1Field::SetDataPointer( vtkDataSet *ds,
                                        const type var,
                                        const char* varname,
                                        const int component_size,
                                        double factor )
{
  vtkDataArray *array;
  int XX;

  // Because the triangluar mesh is defined by using non unique points
  // and the data is cell centered data VisIt moves it out to the
  // nodes for STREAMLINES thus there are 3 times the number of
  // original values.
  if( ds->GetPointData()->GetArray(varname) )
  {
    array = ds->GetPointData()->GetArray(varname);
    XX = 3;
  }
  // 2.0 Change data is at the cells for POINCARE
  else
  {
    array = ds->GetCellData()->GetArray(varname);
    XX = 1;
  }

  if( array == 0 )
  {
    if (DebugStream::Level1())
        debug1 << "Variable " << varname
               << " does not exist"
               << endl;
    return 0;
  }

  const int ntuples = array->GetNumberOfTuples();
  const int ncomponents = array->GetNumberOfComponents();

  if( ntuples != nelms || ncomponents != component_size )
  {
    if (DebugStream::Level1())
        debug1 << "Variable " << varname
               << " size does not equal the number elements and/or components"
               << endl;
    return 0;
  }

  // 2.0 Change data is no longer at the points but still is at the
  // cells so we should be able to use the pointer directly but for
  // some reason it causes problems.
  // 
  //  return (type*) array->GetVoidPointer(0);

  type* newptr = new type[ntuples*ncomponents];

  if( newptr == 0 )
  {
    if (DebugStream::Level1())
        debug1 << "Variable " << varname << " can not allocated" << endl;
    return 0;
  }

  // Because the triangluar mesh is defined by using non unique points
  // and the data is cell centered data VisIt moves it out to the
  // nodes. So create a new structure that is what is really needed.

  // 2.0 Change data is no longer at the points but still is at the
  // cells so the above is no longer valid.
  if( array->IsA("vtkIntArray") ) 
  {
    int* ptr = (int*) array->GetVoidPointer(0);

    for( int i=0; i<ntuples; ++i )
      for( int j=0; j<ncomponents; ++j )
        newptr[i*ncomponents+j] = (type) (factor * ptr[i*XX*ncomponents+j]);

    return newptr;
  }
  else if( array->IsA("vtkFloatArray") ) 
  {
    float* ptr = (float*) array->GetVoidPointer(0);

    for( int i=0; i<ntuples; ++i )
      for( int j=0; j<ncomponents; ++j )
        newptr[i*ncomponents+j] = (type) (factor * ptr[i*XX*ncomponents+j]);

//     std::cerr << varname << std::endl;
//     for( int j=0; j<ncomponents; ++j )
//       std::cerr << newptr[106795*ncomponents+j]  << "  ";
//     std::cerr << std::endl;
//     std::cerr << std::endl;

    return newptr;
  }
  else if( array->IsA("vtkDoubleArray") ) 
  {
    double* ptr = (double*) array->GetVoidPointer(0);

    for( int i=0; i<ntuples; ++i )
      for( int j=0; j<ncomponents; ++j )
        newptr[i*ncomponents+j] = (type) (factor * ptr[i*XX*ncomponents+j]);

    return newptr;
  }
  else
  {
    if (DebugStream::Level1())
        debug1 << "avtIVPM3DC1Field::SetDataPointer "
               << "Variable " << varname
               << " is not of type float - can not safely down cast"
               << endl;
    if( newptr )
        delete [] newptr;
    return 0;
  }
}


// ****************************************************************************
//  Method: avtIVPM3DC1Field IsInside
//
//  Creationist: Joshua Breslau
//  Creation:   16 April 2010
//
//  The VTK check should work but is not reliable because the mesh
//  lies in the XZ plane and Y may or may not be exactly zero. As such
//  due to round off VTK may say the point is outside the
//  mesh. As such, use the native check instead.
//
//  ****************************************************************************

bool avtIVPM3DC1Field::IsInside(const double& t, const avtVector& x) const
{
  double xin[3];
  double *xieta = new double[element_dimension];

  xin[0] = x[0];
  xin[1] = x[1];
  xin[2] = x[2];

  int el = get_tri_coords2D(xin, xieta);

  delete [] xieta;

  return (bool) ( el >= 0 );
}


// ****************************************************************************
//  Method: findElementNeighbors
//
//  Creationist: Joshua Breslau
//  Creation:   20 November 2009
//
// ****************************************************************************
void avtIVPM3DC1Field::findElementNeighbors()
{
  std::vector< vertex > vertexList;
  std::multimap< int, edge > edgeListMap;

  float   *ptr;
  double  x[3], y[3], co, sn;
  int     el, vert, tri[3], vlen;

  /* Allocate, initialize neighbor table */
  neighbors = (int *) malloc(3 * tElements * sizeof(int));
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

      co = trigtable[2*el];
      sn = trigtable[2*el + 1];

      x[0] = ptr[4];
      y[0] = ptr[5];

      x[1] = x[0] + (ptr[0] + ptr[1])*co;
      y[1] = y[0] + (ptr[0] + ptr[1])*sn;

      x[2] = x[0] + ptr[1]*co - ptr[2]*sn;
      y[2] = y[0] + ptr[1]*sn + ptr[2]*co;

      for (vert=0; vert<3; vert++)
        tri[vert] = register_vert(vertexList, x[vert], y[vert]);
    
      for (vert=0; vert<3; vert++)
        add_edge(edgeListMap, tri, vert, el, neighbors);

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
int avtIVPM3DC1Field::register_vert(std::vector< vertex > &vlist,
                                    double x, double y)
{
  const double tol=2.5e-13;

  for( int i=0; i<vlist.size(); i++ )
  {
    double dx = x - vlist[i].x;
    double dy = y - vlist[i].y;

    // Are the two points with the tollerance?
    if (dx*dx + dy*dy < tol)
    {
      return i;
    }
  }

  // Vertex not found so add to list.
  vertex vert;

  vert.x = x;
  vert.y = y;

  vlist.push_back( vert );

  return vlist.size() - 1;
}


// ****************************************************************************
//  Method: add_edge
//
//  Creationist: Joshua Breslau
//  Creation:   20 November 2009
//
// ****************************************************************************
void
avtIVPM3DC1Field::add_edge(std::multimap< int, edge > &edgeListMap,
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
int avtIVPM3DC1Field::get_tri_coords2D(double *xin, int el, double *xout) const
{
  float  *tri;
  double co, sn, rrel, zrel;
  int    index;

  tri = elements + element_size*el;

  /* Compute coordinates local to the current element */
//   if( element_dimension == 2 )
    index = 2 * el;
//   else //if( element_dimension == 3 )
//     index = 2*(el%tElements);

  co = trigtable[index];
  sn = trigtable[index + 1];

  rrel = xin[0] - (tri[4] + tri[1]*co);
  zrel = xin[2] - (tri[5] + tri[1]*sn);
  
  xout[0] = rrel*co + zrel*sn;  /* = xi */
  xout[1] = zrel*co - rrel*sn;  /* = eta */

  if( element_dimension == 3 )  /* = zi */
  {
    float phi = xin[1];

    while( phi < 0 )
      phi += 2.0*M_PI;

    while( phi >= 2.0*M_PI )
      phi -= 2.0*M_PI;

    xout[2] = phi - tri[8]; // tri[8] = phi0
  }

  return el;
}


// ****************************************************************************
//  Method: get_tri_coords2D
//
//  Creationist: Joshua Breslau
//  Creation:   20 November 2009
//
// ****************************************************************************
int avtIVPM3DC1Field::get_tri_coords2D(double *xin, double *xout) const
{
  static int el=0;  /* Needs to be static so the method is a const. */

  float  *tri;
  double co, sn, rrel, zrel;
  int    last=-1, next, flag0, flag1, flag2;

  if( element_dimension == 2 )
  {
    for (int count=0; count<tElements; ++count)
    {
      tri = elements + element_size*el;
      
      /* Compute coordinates local to the current element */
      co = trigtable[2*el];
      sn = trigtable[2*el + 1];
      
      rrel = xin[0] - (tri[4] + tri[1]*co);
      zrel = xin[2] - (tri[5] + tri[1]*sn);

      xout[0] = rrel*co + zrel*sn;  /* = xi */
      xout[1] = zrel*co - rrel*sn;  /* = eta */

      /* Determine whether point is inside element */
      /* "Outside" side 0? */
      if ((flag0 = ((tri[0] + tri[1])*xout[1] < 0.0)))
      {
        if ((next = neighbors[3*el]) >= 0) {
          if (next != last) // not on the boundary so continue;
          {
            last = el;
            el = next;
            continue;
          }
          else // on the boundary so reset the flag and check the other edges;
            flag0 = 0;
        }
      }

      /* "Outside" side 1? */
      if ((flag1 = (tri[0]*xout[1] > tri[2]*(*tri - xout[0]))))
      {
        if ((next = neighbors[3*el + 1]) >= 0) {
          if (next != last) // not on the boundary so continue;
            {
              last = el;
              el = next;
              continue;
            }
          else // on the boundary so reset the flag and check the other edges;
            flag1 = 0;
        }
      }

      /* "Outside" side 2? */
      if ((flag2 = (tri[2]*xout[0] < tri[1]*(xout[1] - tri[2]))))
      {
        if ((next = neighbors[3*el + 2]) >= 0) {
          if (next != last) // on the boundary so continue;
          {
            last = el;
            el = next;
            continue;
          }
          else // on the boundary so reset the flag and check the other edges;
            flag2 = 0;
        }
      }
      
      if (flag0 || flag1 || flag2)
        return -1;
      else
        break;
      
    } /* end loop count */

// fprintf(stderr, "Searched %d elements.\n", count);

    return el;

#ifdef COMMENT_OUT_ASSUMPTIONS_NOT_TRUE
  if( element_dimension == 2 )
  {
    return el;
  }
  else //if( element_dimension == 3 )
  {
    // The above finds the xi and eta for the phi = 0 plane. Which is
    // the same for any plane. Now find the correct phi plane via a
    // brut force search of each plane.

    // Assumptions:
    // Equal number of elements in each plane,
    // Same ordering of elements in each plane,
    // Matching element alignment at each plane.

    float phi = xin[1];

    while( phi < 0 )
      phi += 2.0*M_PI;

    while( phi >= 2.0*M_PI )
      phi -= 2.0*M_PI;

    for( int i=0; i<nplanes; ++i )
    {
      if( tri[8] <= phi ) // tri[8] = phi0
      {
        xout[2] = phi - tri[8];

        if( xout[2] <= tri[7] ) // tri[7] == depth of the section
        {
          return el + i * tElements;
        }
      }

      // Go to the next plane
      tri += element_size*tElements;
    }

    return -1;
  }
#endif

  }

  // For 3D elements use VisIt's native cell search.
  else //if( element_dimension == 3 )
  {
    avtInterpolationWeights iw[8];

    double xpt[3];

    double phi = xin[1];
    
    while( phi < 0 )
      phi += 2.0*M_PI;
    
    while( phi >= 2.0*M_PI )
      phi -= 2.0*M_PI;

    xpt[0] = xin[0];
    xpt[1] = phi;
    xpt[2] = xin[2];

    el = loc->FindCell( xpt, iw );

    if( el < 0 )
      return -1;

    tri = elements + element_size*el;

    /* Compute coordinates local to the current element */
    co = trigtable[2*el];
    sn = trigtable[2*el + 1];

    rrel = xin[0] - (tri[4] + tri[1]*co);
    zrel = xin[2] - (tri[5] + tri[1]*sn);

    xout[0] = rrel*co + zrel*sn;  /* = xi */
    xout[1] = zrel*co - rrel*sn;  /* = eta */
    xout[2] = phi - tri[8];

    return el;
  }
}


// ****************************************************************************
//  Method: avtIVPM3DC1Field::operator
//
//  Evaluates a point location by consulting a M3D C1 grid.  Gets the
//      B field components directly.
//
//  Programmer: Joshua Breslau
//  Creation:   October 24, 2009
//
//  Modifications:
//
// ****************************************************************************

avtIVPField::Result
avtIVPM3DC1Field::operator()( const double &t, const avtVector &p, avtVector &vec ) const
{
  // NOTE: Assumes the point is in cylindrical coordiantes.
  double pt[3] = { p[0], p[1], p[2] };

  /* Find the element containing the point; get local coords xi,eta */
  double *xieta = new double[element_dimension];
  int    element;

  if ((element = get_tri_coords2D(pt, xieta)) < 0) 
  {
    vec.x = 0.0;
    vec.y = 0.0;
    vec.z = 0.0;
  }
  else 
  {
    float B[3];

    interpBcomps(B, pt, element, xieta);

    // The B value is in cylindrical coordiantes
    vec.x = B[0];
    vec.y = B[1];
    vec.z = B[2];
  }

  delete [] xieta;

  if( reparameterize )
    reparameterizeBcomps( p, vec );

  return( avtIVPSolverResult::OK );
}


// ****************************************************************************
//  Method: avtIVPSolver::reparametrizeBcomps
//
//  Purpose:
//      Reparametrize the B components
//
//  Programmer: Nathan Ferraro
//  Creation:   May 25, 2011
//
// ****************************************************************************
/*
  dphi = change in toroidal angle

  dy = distance traveled in toroidal direction
  dR = distance traveled in radial direction

  By definition : dR/dy = B_R / B_Phi

  In cylindrical coordinates,
  dy = R*dphi    --->    dy/dphi = R

  So: dR/dphi = (dR/dy)*(dy/dphi) = (B_R / B_Phi) * R

  Thus : dR = (B_R / B_phi)*R * dphi

  same for dZ.  Note that the units only work out correctly with the
  factor of R there.

  Below 
*/

void avtIVPM3DC1Field::reparameterizeBcomps( const avtVector &p,
                                             avtVector &v ) const
{
  avtVector pv;

  if( v.y == 0.0 )
  {
    pv.x = pv.z = 0;
  }
  else
  {
    pv.x = v.x / v.y * p.x; // r
    pv.z = v.z / v.y * p.x; // z
  }

  pv.y = 1;                 // phi

  v = pv;
}


// ****************************************************************************
//  Method: avtIVPM3DC1Field::ConvertToCartesian
//
//  Purpose: Converts the coordinates from local cylindrical to
//      cartesian coordinates
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

avtVector 
avtIVPM3DC1Field::ConvertToCartesian(const avtVector& pt) const
{
  return avtVector(pt[0]*cos(pt[1]), pt[0]*sin(pt[1]), pt[2] );
}

// ****************************************************************************
//  Method: avtIVPM3DC1Field::ConvertToCylindrical
//
//  Purpose: Converts the coordinates from local cylindrical to
//      cartesian coordinates
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

avtVector 
avtIVPM3DC1Field::ConvertToCylindrical(const avtVector& pt) const
{
  return pt;
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
void avtIVPM3DC1Field::interpBcomps(float *B, double *x,
                                    int element, double *xieta) const
{
  float *B_r   = &(B[0]);
  float *B_z   = &(B[2]);
  float *B_phi = &(B[1]);

  if( element_dimension == 2 )
  {
    // Add in the equalibrium if it was subtracted out.
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

    // B_R   = -(dpsi/dZ)/R - (d2f/dRdphi)
    // B_Z   =  (dpsi/dR)/R - (d2f/dZdphi)
    // B_Phi =  I/R

    // Add in the equalibrium if it was subtracted out.
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
    *B_r += -interpdz(psi, element, xieta) / x[0] -
      interpdRdPhi(f, element, xieta);
    *B_z +=  interpdR(psi, element, xieta) / x[0] -
      interpdzdPhi(f, element, xieta);
    *B_phi += interp(I, element, xieta) / x[0];

//     std::cerr << element << "  "
//            << x[0] << "  " << x[1] << "  " << x[2] << "    "
//            << x[0]*cos(x[1]) << "  " << x[0]*sin(x[1]) << "  " << x[2]
//            << std::endl;
//     std::cerr << element << "  "
//            << xieta[0] << "  " << xieta[1] << "  " << xieta[2] << "    "
//            << std::endl;

//     std::cerr << std::endl;

//     std::cerr << "-(dpsi/dZ)/R = " << -interpdz(psi, element, xieta) / x[0] << std::endl;
//     std::cerr << "-(dpsi/dR)/R = " <<  interpdR(psi, element, xieta) / x[0] << std::endl;
    
//     std::cerr << "F/R = " << interp(I, element, xieta) / x[0] << std::endl;

//     std::cerr << "-(d2f/dRdphi) = " << -interpdRdPhi(f, element, xieta) << std::endl;
//     std::cerr << "-(d2f/dZdphi) = " << -interpdzdPhi(f, element, xieta) << std::endl;

//     std::cerr << "B = " << *B_r << "  " <<  *B_z << "  " <<  *B_phi <<  std::endl;
  
//     std::cerr << std::endl;
  }
}


// ****************************************************************************
//  Method: interp basic interpolation
//
//  Creationist: Joshua Breslau
//  Creation:   20 November 2009
//
// ****************************************************************************
float avtIVPM3DC1Field::interp(float *var, int el, double *lcoords) const
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
void avtIVPM3DC1Field::interpdX(float *var, int el, double *lcoords,
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
float avtIVPM3DC1Field::interpdR(float *var, int el, double *lcoords) const
{
  double xicoef, etacoef;
  int index;

//   if( element_dimension == 2 )
    index = 2 * el;
//   else //if( element_dimension == 3 )
//     index = 2*(el%tElements);

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
float avtIVPM3DC1Field::interpdz(float *var, int el, double *lcoords) const
{
  double xicoef, etacoef;
  int index;

//   if( element_dimension == 2 )
    index = 2 * el;
//   else //if( element_dimension == 3 )
//     index = 2*(el%tElements);

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
float avtIVPM3DC1Field::interpdPhi(float *var, int el, double *lcoords) const
{
  float *a = var + scalar_size*el;
  double xi = lcoords[0], eta = lcoords[1];
  double val;

  if( element_dimension == 2 )
  {
    val = 0;
  }
  else //if( element_dimension == 3 )
  {
    double zi = lcoords[2];
    double zi_q = 1;

    val = 0;

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
void avtIVPM3DC1Field::interpdX2(float *var, int el, double *lcoords,
                                 double &xixicoef, double &etaetacoef,
                                 double &xietacoef ) const
{
  float *a = var + scalar_size*el;
  double xi = lcoords[0], eta = lcoords[1];
  int index;

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
float avtIVPM3DC1Field::interpdR2(float *var, int el, double *lcoords) const
{
  double xixicoef, etaetacoef, xietacoef;
  int index;

//   if( element_dimension == 2 )
    index = 2 * el;
//   else //if( element_dimension == 3 )
//     index = 2*(el%tElements);

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
float avtIVPM3DC1Field::interpdz2(float *var, int el, double *lcoords) const
{
  double xixicoef, etaetacoef, xietacoef;
  int index;

//   if( element_dimension == 2 )
    index = 2 * el;
//   else //if( element_dimension == 3 )
//     index = 2*(el%tElements);

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
float avtIVPM3DC1Field::interpdRdz(float *var, int el, double *lcoords) const
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
//     index = 2*(el%tElements);
    index = 2 * el;

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
void avtIVPM3DC1Field::interpdXdPhi(float *var, int el, double *lcoords,
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
//  Method: interpdRdPhi interpolation in dRdPhi
//
//  Creationist: Joshua Breslau
//  Creation:   20 November 2009
//
// ****************************************************************************
float avtIVPM3DC1Field::interpdRdPhi(float *var, int el, double *lcoords) const
{
  double xicoef, etacoef;
  int index;

//   if( element_dimension == 2 )
    index = 2 * el;
//   else //if( element_dimension == 3 )
//     index = 2*(el%tElements);

  interpdXdPhi(var, el, lcoords, xicoef, etacoef);

  return xicoef*trigtable[index] - etacoef*trigtable[index + 1];
}


// ****************************************************************************
//  Method: interpdzdPhi interpolation in dzdPhi
//
//  Creationist: Joshua Breslau
//  Creation:   20 November 2009
//
// ****************************************************************************
float avtIVPM3DC1Field::interpdzdPhi(float *var, int el, double *lcoords) const
{
  double xicoef, etacoef;
  int index;

//   if( element_dimension == 2 )
    index = 2 * el;
//   else //if( element_dimension == 3 )
//     index = 2*(el%tElements);

  interpdXdPhi(var, el, lcoords, xicoef, etacoef);

  return xicoef*trigtable[index + 1] + etacoef*trigtable[index];
}
