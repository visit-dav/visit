 /*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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
//                             avtIVPNIMRODField.C                            //
// ************************************************************************* //

#include "avtIVPNIMRODField.h"

#include <DebugStream.h>

#include <vtkStructuredGrid.h>
#include <vtkIntArray.h>
#include <vtkFloatArray.h>
#include <vtkIdList.h>

#include <math.h>

// ****************************************************************************
//  Method: avtIVPNIMRODField constructor
//
//  Creationist: Allen Sanderson
//  Creation:   20 November 2009
//
// ****************************************************************************

avtIVPNIMRODField::avtIVPNIMRODField( vtkDataSet* dataset, 
                                      avtCellLocator* locator ) : 
    avtIVPVTKField( dataset, locator ),
    grid_fourier_series( 0 ), data_fourier_series( 0 ),
    Drad( 2 ), Dtheta( 2 )
{
  vtkStructuredGrid* grid = vtkStructuredGrid::SafeDownCast( dataset );
  
  // Pick off all of the data stored with the vtk field.
  if( grid->GetDataDimension() == 3 )
  {
    int dims[3];

    grid->GetDimensions( dims );

    Nrad   = dims[0];
    Ntheta = dims[1];
    Nphi   = dims[2];
  }
  else
  {
    debug1 << "Mesh dimension is not 3 " << endl;
    
    return;
  }

  // Dummy variable to the template class
//  float fltVar = 0;

  // The mesh elements.
//  grid_fourier_series =
//    SetDataPointer( ds, fltVar, "hidden/grid_fourier_series", 3 );

  // Vector values from the field.
//  data_fourier_series =
//    SetDataPointer( ds, fltVar, "hidden/data_fourier_series", 3 );

}

// ****************************************************************************
//  Method: avtIVPNIMRODField constructor
//
//  Creationist: Allen Sanderson
//  Creation:   20 November 2009
//
// ****************************************************************************

avtIVPNIMRODField::avtIVPNIMRODField( unsigned int nRad,
                                      unsigned int nTheta,
                                      unsigned int nPhi,
                                      float *gfs,
                                      float *dfs ) 
  : avtIVPVTKField(NULL, NULL),
    grid_fourier_series( gfs ), data_fourier_series( dfs ),
    Nrad( nRad ), Ntheta( nTheta ), Nphi( nPhi ),
    Drad( 2 ), Dtheta( 2 )
{
  lagrange_nodes[0][0] = 0.0;
  lagrange_nodes[1][0] = 0.0; lagrange_nodes[1][1] = 1.0;
  lagrange_nodes[2][0] = 0.0; lagrange_nodes[2][1] = 0.5; lagrange_nodes[2][2] = 0.5;
  lagrange_nodes[3][0] = 0.0; lagrange_nodes[3][1] = 0.276393202250021031; lagrange_nodes[3][2] = 0.723606797749978969; lagrange_nodes[3][3] = 1.0;
  lagrange_nodes[4][0] = 0.0; lagrange_nodes[4][1] = 0.172673164646011429; lagrange_nodes[4][2] = 0.500000000000000000; lagrange_nodes[4][3] = 0.827326835353988571; lagrange_nodes[4][4] = 1.0;
  lagrange_nodes[5][0] = 0.0; lagrange_nodes[5][1] = 0.117472338035267654; lagrange_nodes[5][2] = 0.357384241759677453; lagrange_nodes[5][3] = 0.642615758240322547; lagrange_nodes[5][4] = 0.882527661964732346; lagrange_nodes[5][5] = 1.0;

//       { 0.0 },
//       { 0.0, 1.0 },
//       { 0.0, 0.5, 1.0 },
//       { 0.0, 0.276393202250021031, 0.723606797749978969, 1.0 },
//       { 0.0, 0.172673164646011429, 0.500000000000000000, 0.827326835353988571, 1.0 },
//       { 0.0, 0.117472338035267654, 0.357384241759677453, 0.642615758240322547, 0.882527661964732346, 1.0 },
//     };
}


// ****************************************************************************
//  Method: avtIVPNIMRODField destructor
//
//  Creationist: Allen Sanderson
//  Creation:   20 November 2009
//
// ****************************************************************************

avtIVPNIMRODField::~avtIVPNIMRODField()
{
  if( grid_fourier_series ) free(grid_fourier_series);
  if( data_fourier_series ) free(data_fourier_series);
}


// ****************************************************************************
//  Method: avtIVPNIMRODField destructor
//
//  Creationist: Allen Sanderson
//  Creation:   20 November 2009
//
// ****************************************************************************

template< class type >
type* avtIVPNIMRODField::SetDataPointer( vtkDataSet *ds,
                                        const type var,
                                        const char* varname,
                                        const int ncomponents )
{
  vtkDataArray *array;

  // Because the triangluar mesh is defined by using non unique points
  // and the data is cell centered data VisIt moves it out to the
  // nodes for STREAMLINES thus there are 3 times the number of
  // original values.
  if( ds->GetPointData()->GetArray(varname) )
  {
    array = ds->GetPointData()->GetArray(varname);
  }

  if( array == 0 )
  {
    if (DebugStream::Level1())
        debug1 << "Variable " << varname
               << " does not exist"
               << endl;
    return 0;
  }

  int ntuples = Nrad*Ntheta*Nphi;

  if( ntuples != array->GetNumberOfTuples() ||
      ncomponents != array->GetNumberOfComponents() )
  {
    if (DebugStream::Level1())
        debug1 << "Variable " << varname
               << " size does not equal the number elements and/or components"
               << endl;
    return 0;
  }

  type* newptr = new type[ntuples*ncomponents];

  if( newptr == 0 )
  {
    if (DebugStream::Level1())
        debug1 << "Variable " << varname << " can not allocated" << endl;
    return 0;
  }

  if( array->IsA("vtkIntArray") ) 
  {
    int* ptr = (int*) array->GetVoidPointer(0);

    for( int i=0; i<ntuples; ++i )
      for( int j=0; j<ncomponents; ++j )
        newptr[i*ncomponents+j] = ptr[i*ncomponents+j];

    return newptr;
  }
  else if( array->IsA("vtkFloatArray") ) 
  {
    float* ptr = (float*) array->GetVoidPointer(0);

    for( int i=0; i<ntuples; ++i )
      for( int j=0; j<ncomponents; ++j )
        newptr[i*ncomponents+j] = ptr[i*ncomponents+j];

    return newptr;
  }
  else if( array->IsA("vtkDoubleArray") ) 
  {
    double* ptr = (double*) array->GetVoidPointer(0);

    for( int i=0; i<ntuples; ++i )
      for( int j=0; j<ncomponents; ++j )
        newptr[i*ncomponents+j] = ptr[i*ncomponents+j];

    return newptr;
  }
  else
  {
    if (DebugStream::Level1())
        debug1 << "avtIVPNIMRODField::SetDataPointer "
               << "Variable " << varname
               << " is not of type float - can not safely down cast"
               << endl;
    return 0;
  }
}


// ****************************************************************************
//  Method: avtIVPNIMRODField::operator
//
//  Purpose: Evaluates a point location by consulting a M3D C1 grid.
//      Gets the B field components directly - should not be used for
//      calculating integral curves.
//
//  Programmer: Allen Sanderson
//  Creation:   October 24, 2009
//
//  Modifications:
//
// ****************************************************************************

avtIVPField::Result
avtIVPNIMRODField::operator()( const double &t, const avtVector &p, avtVector &linear_vec ) const
{
  Result result;
  if( (result = avtIVPVTKField::operator()(t, p, linear_vec)) != avtIVPSolverResult::OK )
    return( result );

  vtkIdList *ptIds = vtkIdList::New();

  ds->GetCellPoints( lastCell, ptIds );

  double center[3], pt[3];

  for( unsigned int i=0; i<Nphi; ++ i )
  { 
    ds->GetPoint( i+62*101, center);
    cerr << i << "  "
         << center[0] << "  " << center[1] << "  " << center[2] << "   "
         << sqrt(center[0]*center[0]+center[1]*center[1]) << "  " << endl;
  }

  cerr << lastCell << " ( " << endl;

  unsigned int iRad=Nrad, iTheta=Ntheta, iPhi=Nphi;

  double minRad=+1e10, minTheta=+1e10, minPhi=+1e10;
  double maxRad=-1e10, maxTheta=-1e10, maxPhi=-1e10;

  unsigned int ic=0;


  for( unsigned int i=0; i<ptIds->GetNumberOfIds(); ++i )
  {
    unsigned int tRad, tTheta, tPhi, tmp = ptIds->GetId(i);

    cerr << tmp << " ";

    tRad = tmp / (Ntheta*Nphi);

    tmp = tmp % (Ntheta*Nphi);

    tTheta = tmp / Nphi;

    tPhi = tmp % Nrad;
        
    if( iRad >= tRad )
    {
      iRad = tRad;

      if( iTheta >= tTheta )
      {
        iTheta = tTheta;

        if( iPhi > tPhi )
        {
          iPhi = tPhi;
          
          ic = i;
        }
      }
    }


    ds->GetPoint( ptIds->GetId(i), pt);
    ds->GetPoint( tPhi, center);

      cerr << "  " << tPhi << "  " << tTheta << "  " << tRad << "  "
//       << center[0] << "  " << center[1] << "  " << center[2] << "   ";
         << pt[0] << "  " << pt[1] << "  " << pt[2] << "   ";

    double dx = pt[0] - center[0];
    double dz = pt[2] - center[2];

    double rad  = sqrt( dx*dx + dz*dz );
    double theta = atan2( dz, dx );
    double phi   = atan2( pt[1], pt[0] );

    cerr << phi << " " << theta << " " << rad << " " << endl;

    if( minRad > rad ) minRad = rad;
    if( maxRad < rad ) maxRad = rad;

    if( minTheta > theta ) minTheta = theta;
    if( maxTheta < theta ) maxTheta = theta;

    if( minPhi > phi ) minPhi = phi;
    if( maxPhi < phi ) maxPhi = phi;

  }

  cerr << ") " << iPhi << "  " << iTheta << "  " << iRad << endl;

  cerr << minRad << " " << minTheta << " " << minPhi << " " << endl;
  cerr << maxRad << " " << maxTheta << " " << maxPhi << " " << endl;


  double x[3];
  ds->GetPoint( ptIds->GetId(ic), x);

//  double rad, theta, phi;

  
//  vec3 P;
//  mat3 DRV;

//  interpolate( rad, theta, phi, &P, &DRV );

  return( avtIVPSolverResult::OK );
}

// ****************************************************************************
//  Method: avtIVPNIMRODField::ConvertToCartesian
//
//  Purpose:
//      Converts the coordinates from cylindrical to cartesian coordinates
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

avtVector 
avtIVPNIMRODField::ConvertToCartesian(const avtVector& pt) const
{
  return pt;
}

// ****************************************************************************
//  Method: avtIVPNIMRODField::ConvertToCylindrical
//
//  Purpose:
//      Converts the coordinates from cylindrical to cartesian coordinates
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

avtVector 
avtIVPNIMRODField::ConvertToCylindrical(const avtVector& pt) const
{
  return pt;
}

// -------------------------------------------------------------------------


void avtIVPNIMRODField::lagrange_weights( unsigned int DEG, const double s, 
                                          double* w, double *d ) const
{
    for( int i=0; i<=DEG; ++i )
    {
        double nom = 1.0, den = 1.0, dnom = 0.0;

        for( int j=0; j<=DEG; ++j )
        {
            if( i==j )
                continue;

            den *= (lagrange_nodes[DEG][i]-lagrange_nodes[DEG][j]);
            nom *= (s-lagrange_nodes[DEG][j]);
            
            double dtmp = 1.0;
                    
            for( int k=0; k<=DEG; ++k )
            {
                if( k==i || k==j )
                    continue;
            
                dtmp *= (s-lagrange_nodes[DEG][k]);
            }
            
            dnom += dtmp;
        }

        w[i] = nom/den;
        d[i] = dnom/den;
    }
}

// -------------------------------------------------------------------------

void avtIVPNIMRODField::fourier_weights( unsigned int N, const double t, 
                                         double* w, double* d ) const
{
    // 0th coefficient
    w[0] = 1.0;
    d[0] = 0.0;
     
    for( int n=1, m=N-1; n<N/2; n++, m-- ) 
    {
        double alpha = 2*M_PI*n;
        
        w[n] =  2.0*cos( alpha*t );
        d[n] = -2.0*sin( alpha*t ) * alpha;
        
        w[m] = -2.0*sin( alpha*t );
        d[m] = -2.0*cos( alpha*t ) * alpha;
    }

    // N/2 coefficient
    w[N/2] =  cos( M_PI*t );
    d[N/2] = -sin( M_PI*t ) * M_PI;
}

// -------------------------------------------------------------------------

void avtIVPNIMRODField::interpolate( double rad, double theta, double phi,
                                     vec3* P, mat3* DRV ) const
{
    float *vecs = data_fourier_series;

    // rad, theta, phi come in parametrized on a unit space cube.

    // P is the point in physical space.

    // Transform from the unit space cube 0->1 into cell index space.
    rad   *= (Nrad-1)/Drad;
    theta *= (Ntheta-1)/Dtheta;

    // Get the integer offset values for the cell index space. 
    unsigned int qrad =
      std::max( 0.0, std::min( floor(rad), (double)(Nrad-1)/Drad - 1 ) );
    unsigned int qtheta =
      std::max( 0.0, std::min( floor(theta), (double)(Ntheta-1)/Dtheta - 1 ) );

    // Subtract the integer offset to get the relative cell index values.
    rad -= qrad;
    theta -= qtheta;

    // Index space into original grid.
    qrad *= Drad;
    qtheta *= Dtheta;

    // ---

    double *wtheta = new double[Dtheta+1], *dtheta = new double[Dtheta+1];
    lagrange_weights( Dtheta, theta, wtheta, dtheta );

    double *wrad = new double[Drad+1], *drad = new double[Drad+1];
    lagrange_weights( Drad, rad, wrad, drad );

    double *wphi = new double[Nphi-1], *dphi = new double[Nphi-1];
    fourier_weights( Nphi-1, phi, wphi, dphi );

    // Returned vector value in physical space.
    vec3 p;     // = vec3::Zero();
    vec3 Dp[3]; // = mat3::Zero();

    // vec3 v  = vec3::Zero();
    // mat3 Dv = mat3::Zero();

    for( int i=0; i<=Drad; ++i )
    {
        for( int j=0; j<=Dtheta; ++j )
        {
            for( int k=0; k<=Nphi-1; ++k )
            {
              vec3 data =
                vec3( vecs +
                      3 * (i + qrad + (qtheta+j)*Nrad + k*Nrad*Ntheta) );
                                
                p += wphi[k] * wtheta[j] * wrad[i] * data;
                Dp[0] += wphi[k] * wtheta[j] * drad[i] * data * (Nrad-1)/Drad;
                Dp[1] += wphi[k] * dtheta[j] * wrad[i] * data * (Ntheta-1)/Dtheta;
                Dp[2] += dphi[k] * wtheta[j] * wrad[i] * data;
            }

            // data = vecs + qrad + (qtheta+j)*Nrad + k*Nrad*Ntheta;
            // 
            // for( int i=0; i<=Drad; ++i )
            // {
            //     v += wphi[k] * wtheta[j] * wrad[i] * data[i];
            //     Dv.row(0) += wphi[k] * wtheta[j] * drad[i] * data[i] * (Nrad-1)/Drad;
            //     Dv.row(1) += wphi[k] * dtheta[j] * wrad[i] * data[i] * (Ntheta-1)/Dtheta;
            //     Dv.row(2) += dphi[k] * wtheta[j] * drad[i] * data[i];
            // }
        }
    }
    
    *P   = p;
//    *DRV = Dp;

    delete [] wtheta;
    delete [] dtheta;
    delete [] wrad;
    delete [] drad;
    delete [] wphi;
    delete [] dphi;
}
