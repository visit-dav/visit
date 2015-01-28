/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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
//                             avtIVPNek5000Field.C                          //
// ************************************************************************* //

#include "visit-config.h"

#include "avtIVPNek5000Field.h"

#include <limits>

#include <DebugStream.h>
#include <avtCallback.h>

#include <vtkCellData.h>
#include <vtkIntArray.h>
#include <vtkUnstructuredGrid.h>

#include <InvalidVariableException.h>

#include "ext/findpts_local.h"

// ****************************************************************************
//  Method: avtIVPNek5000Field constructor
//
//  Creationist: Allen Sanderson
//  Creation:    May 1, 2013
//
// ****************************************************************************

avtIVPNek5000Field::avtIVPNek5000Field( vtkDataSet* dataset, 
                                        avtCellLocator* locator) :
  avtIVPVTKField( dataset, locator )
{
  vtkFieldData *fieldData = dataset->GetFieldData();

  // Pick off all of the data stored with the vtk field.
  vtkUnstructuredGrid *ugrid = (vtkUnstructuredGrid*) dataset;

  vtkPoints *pts = ugrid->GetPoints();
  if (pts == NULL) {
    EXCEPTION1( InvalidVariableException,
                "avtIVPNek5000Field - Can not find vertices." );
  }

  vtkDataArray *vecs = ugrid->GetPointData()->GetVectors();
  if (vecs == NULL) {
    EXCEPTION1( InvalidVariableException,
                "avtIVPNek5000Field - Can not find velocity variable." );
  }

  // Get the number of point per spectrial elements
  unsigned int iDim = 0;
  unsigned int iBlockSize[3] = {0,0,0};
  unsigned int iNumBlocks = 0;
  unsigned int npts = 1;

  vtkIntArray *semVTK =
    (vtkIntArray *) fieldData->GetAbstractArray("Nek_SpectralElementData");  

  if( semVTK )
  {
    iBlockSize[0] = semVTK->GetValue(0);
    iBlockSize[1] = semVTK->GetValue(1);
    iBlockSize[2] = semVTK->GetValue(2);
    iNumBlocks    = semVTK->GetValue(3);

    if( iBlockSize[2] > 1 )
      iDim = 3;
    else
      iDim = 2;
  }
  else 
  {
    EXCEPTION1( InvalidVariableException,
                "Uninitialized option. (Please contact visit-developer mailing list to report)" );
  }

  unsigned int iBlockSize2[3] = { 2*iBlockSize[0],
                                  2*iBlockSize[1],
                                  2*iBlockSize[2] };

  unsigned int pts_per_element = iBlockSize[0] * iBlockSize[1];
  if (iDim == 3)
    pts_per_element *= iBlockSize[2];
 
  // Get the numver of elements for checking the validity of the data.
  unsigned int num_elements = pts->GetNumberOfPoints() / pts_per_element;

  if( num_elements != iNumBlocks )
  {
    std::string str("The number of elements available for advection does not "
                    "match the number blocks in the dataset. As such, curves "
                    "may not be advected across element boundaries. "
                    "This can occur when the Nek5000 file contains boundary or "
                    "extents meta data. In the 'Advanced' tab select "
                    "'Parallelization' to be 'Parallelize over domains'.");

    avtCallback::IssueWarning(str.c_str());
  }

  unsigned int hexes_per_element = (iBlockSize[0]-1)*(iBlockSize[1]-1);
  if (iDim == 3)
    hexes_per_element *= (iBlockSize[2]-1);

  // Move the vertices from the VTK structure into a form that Nek5000
  // uses.
  nek_pts[0] = new double[pts_per_element*num_elements];
  nek_pts[1] = new double[pts_per_element*num_elements];
  nek_pts[2] = new double[pts_per_element*num_elements];

  // Get the pointer to the points that make up the spetral element.
  int pts_type = pts->GetDataType();

  if( pts_type == VTK_FLOAT ) {
    float *pts_ptr = (float*) pts->GetVoidPointer(0);

    for( unsigned int i=0; i<pts_per_element*num_elements; ++i )
    {
      nek_pts[0][i] = pts_ptr[i*3+0];
      nek_pts[1][i] = pts_ptr[i*3+1];
      nek_pts[2][i] = pts_ptr[i*3+2];
    }
  } else if( pts_type == VTK_DOUBLE ) {
    double *pts_ptr = (double*) pts->GetVoidPointer(0);

    for( unsigned int i=0; i<pts_per_element*num_elements; ++i )
    {
      nek_pts[0][i] = pts_ptr[i*3+0];
      nek_pts[1][i] = pts_ptr[i*3+1];
      nek_pts[2][i] = pts_ptr[i*3+2];
    }
  }
  else {
    EXCEPTION1( InvalidVariableException,
                "avtIVPNek5000Field - Expected single or double precison floating point vertices." );
  }

  // Move the vectors from the VTK structure into a form that Nek5000
  // uses.
  nek_vec[0] = new double[pts_per_element*num_elements];
  nek_vec[1] = new double[pts_per_element*num_elements];
  nek_vec[2] = new double[pts_per_element*num_elements];

  // Get the pointer to the vectors that make up the spetral element.
  if( vecs->GetNumberOfComponents() != 3 ) {
    EXCEPTION1( InvalidVariableException,
                "avtIVPNek5000Field - Velocity variable does not contain three components." );
  }

  int vec_type = vecs->GetDataType();

  if( vec_type == VTK_FLOAT ) {
    float *vec_ptr = (float*) vecs->GetVoidPointer(0);

    for( unsigned int i=0; i<pts_per_element*num_elements; ++i )
    {
      nek_vec[0][i] = vec_ptr[i*3+0];
      nek_vec[1][i] = vec_ptr[i*3+1];
      nek_vec[2][i] = vec_ptr[i*3+2];
    }
  } else if( vec_type == VTK_DOUBLE ) {
    double *vec_ptr = (double*) vecs->GetVoidPointer(0);

    for( unsigned int i=0; i<pts_per_element*num_elements; ++i )
    {
      nek_vec[0][i] = vec_ptr[i*3+0];
      nek_vec[1][i] = vec_ptr[i*3+1];
      nek_vec[2][i] = vec_ptr[i*3+2];
    }
  }
  else {
    EXCEPTION1( InvalidVariableException,
                "avtIVPNek5000Field - Expected single or double precison floating point vectors." );
  }

  // Create the internal Nek5000 field
  nek_fld = findpts_local_setup(iDim, nek_pts, iBlockSize, num_elements,
                                iBlockSize2, 0.01, pts_per_element*num_elements,
                                npts, 1024.0*std::numeric_limits<double>::epsilon());
}


// ****************************************************************************
//  Method: avtIVPNek5000Field destructor
//
//  Creationist: Allen Sanderson
//  Creation:    May 1, 2013
//
// ****************************************************************************

avtIVPNek5000Field::~avtIVPNek5000Field()
{
  if( nek_pts[0] ) delete [] nek_pts[0];
  if( nek_pts[1] ) delete [] nek_pts[1];
  if( nek_pts[2] ) delete [] nek_pts[2];
  
  if( nek_vec[0] ) delete [] nek_vec[0];
  if( nek_vec[1] ) delete [] nek_vec[1];
  if( nek_vec[2] ) delete [] nek_vec[2];

  if( nek_fld ) findpts_local_free( nek_fld );
}


// ****************************************************************************
//  Method: avtIVPNek5000Field::operator
//
//  Evaluates a point location by consulting a Nek 5000 grid.
//
//  Programmer: Allen Sanderson
//  Creation:   May 1, 2013
//
//  Modifications:
//
// ****************************************************************************

avtIVPField::Result
avtIVPNek5000Field::operator()( const double &t,
                                const avtVector &p,
                                avtVector &vec ) const
{
  // These are return values.
  unsigned long code_base[1], el_base[1];
  double r_base[3], dist2_base[1];

  // Stuff the one point into the array for findpts.
  double xpt[3] = { p[0], p[1], p[2] };
  double * const x_base[3] = { &(xpt[0]), &(xpt[1]), &(xpt[2]) };

  // Strides for all of the return data.
  unsigned int x_stride[3] = { 3*sizeof( double ),
                               3*sizeof( double ),
                               3*sizeof( double ) };

  unsigned int  code_stride = sizeof( unsigned long );
  unsigned int    el_stride = sizeof( unsigned long );
  unsigned int    r_stride = 3 * sizeof( double );
  unsigned int dist2_stride = sizeof( double );
  unsigned int out_stride = sizeof( double );

  unsigned long npts = 1;

  // Translate world space coordinates into the spectral parameteric
  // coordinate space.
  findpts_local( code_base,  code_stride,
                 el_base,    el_stride,
                 r_base,     r_stride,
                 dist2_base, dist2_stride,
                 x_base,     x_stride,
                 npts, nek_fld );

  // Now loop through each coordinate and do the appropriate
  // interpolation.
  for( unsigned int i=0; i<3; ++i )
  {
    double tmp;

    findpts_local_eval( &tmp,    out_stride,
                        el_base, el_stride,
                        r_base,  r_stride,
                        npts, nek_vec[i], nek_fld );

    vec[i] = tmp;
  }

  return OK;
    // static int el = 0; // element

    // // Locate the cell that surrounds the point.
    // avtInterpolationWeights iw[8];

    // double xpt[3];

    // xpt[0] = p[0];
    // xpt[1] = p[1];
    // xpt[2] = p[2];

    // el = loc->FindCell( xpt, iw, false );

    // if( el < 0 )
    //   return OUTSIDE_SPATIAL;

    // // The above element is based on the linear mesh not the spectral
    // // element mesh so find the element in the spectral mesh.

    // // Note this is integer arthimetic. 
    // el /= hexes_per_element;

    //return avtIVPVTKField::operator()( t, p, vec );
}

// ****************************************************************************
//  Method: avtIVPNek5000Field::ConvertToCartesian
//
//  Purpose: Converts the coordinates from local cylindrical to
//      cartesian coordinates
//
//  Programmer: Allen Sanderson
//  Creation:   May 1, 2013
//
// ****************************************************************************

avtVector 
avtIVPNek5000Field::ConvertToCartesian(const avtVector& pt) const
{
  return pt;
}

// ****************************************************************************
//  Method: avtIVPNek5000Field::ConvertToCylindrical
//
//  Purpose: Converts the coordinates from local cylindrical to
//      cartesian coordinates
//
//  Programmer: Allen Sanderson
//  Creation:   May 1, 2013
//
// ****************************************************************************

avtVector 
avtIVPNek5000Field::ConvertToCylindrical(const avtVector& pt) const
{
  return avtVector(sqrt(pt[0]*pt[0]+pt[1]*pt[1]), atan2(pt[1],pt[0]), pt[2] );
}
