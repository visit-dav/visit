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
//                        avtIVPNek5000TimeVaryingField.C                        //
// ************************************************************************* //

#include <avtIVPNek5000TimeVaryingField.h>

#include <DebugStream.h>
#include <avtCallback.h>
#include <InvalidVariableException.h>

#include <vtkUnsignedCharArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkIntArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkGenericCell.h>

#include "ext/findpts_local.h"

#include <iostream>
#include <limits>

//const char* avtIVPNek5000TimeVaryingField::NextTimePrefix = "__nextTimePrefix_";
const char* avtIVPNek5000TimeVaryingField::NextTimePrefix = "__pathlineNextTimeVar__";

// ****************************************************************************
//  Method: avtIVPNek5000TimeVaryingField constructor
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
//  Modifications:
//
//    Hank Childs, Thu Apr  2 16:40:08 PDT 2009
//    Use vtkVisItInterpolatedVelocityField, not vtkInterpolatedVelocityField.
//
//   Christoph Garth, Fri Jul 9, 10:10:22 PDT 2010
//   Incorporate vtkVisItInterpolatedVelocityField in avtIVPNek5000TimeVaryingField.
//
// ****************************************************************************

avtIVPNek5000TimeVaryingField::avtIVPNek5000TimeVaryingField( vtkDataSet* dataset, 
                                                      avtCellLocator* locator,
                                                      double _t0, double _t1 ) 
  : ds(dataset), loc(locator), t0(_t0), t1(_t1), dt(_t1-_t0)
{
    if( ds )
        ds->Register( NULL );

    // if( (velData[0] = ds->GetPointData()->GetVectors()) )
    // {
    //     velCellBased = false;

    //     velData[1] = ds->GetPointData()->GetArray( NextTimePrefix );
    // }
    // else if( (velData[0] = ds->GetCellData()->GetVectors()) )
    // {
    //     velCellBased = true;

    //     velData[1] = ds->GetCellData()->GetArray( NextTimePrefix );
    // }
    // else 
    // {
    //     velData[0] = velData[1] = NULL;
    //     EXCEPTION1( ImproperUseException, 
    //                 "avtIVPNek5000TimeVaryingField: Can't locate vectors to interpolate." );
    // }

    // if( velData[1] == NULL )
    // {
    //     EXCEPTION1( ImproperUseException, 
    //                 "avtIVPNek5000TimeVaryingField: Can't locate second pair of vectors to interpolate." );
    // }

    lastCell = -1;
    lastPos.x = lastPos.y = lastPos.z =
        std::numeric_limits<double>::quiet_NaN();

    std::fill( sclData[0], sclData[0]+256, (vtkDataArray*)NULL );
    std::fill( sclData[1], sclData[1]+256, (vtkDataArray*)NULL );
    std::fill( sclCellBased, sclCellBased+256, false );

    sclDataName.resize( 256 );

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

    // Get the Nektar++ data.
    vtkFieldData *fieldData = ds->GetFieldData();

    const char *fieldname[2] = {"Nek_SpectralElementData",
                                "Nek_SpectralElementData2" };

    // Get the number of point per spectrial elements
    unsigned int iDim = 0;
    unsigned int iBlockSize[3] = {0,0,0};
    unsigned int iNumBlocks = 0;
    unsigned int npts = 1;

  for (int i = 0; i < 2; ++i)
  {
    vtkIntArray *semVTK =
      (vtkIntArray *) fieldData->GetAbstractArray(fieldname[i]);  

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
    nek_pts[i][0] = new double[pts_per_element*num_elements];
    nek_pts[i][1] = new double[pts_per_element*num_elements];
    nek_pts[i][2] = new double[pts_per_element*num_elements];

    // Get the pointer to the points that make up the spetral element.
    int pts_type = pts->GetDataType();

    if( pts_type == VTK_FLOAT ) {
      float *pts_ptr = (float*) pts->GetVoidPointer(0);

      for( unsigned int j=0; j<pts_per_element*num_elements; ++j )
      {
        nek_pts[i][0][j] = pts_ptr[j*3+0];
        nek_pts[i][1][j] = pts_ptr[j*3+1];
        nek_pts[i][2][j] = pts_ptr[j*3+2];
      }
    } else if( pts_type == VTK_DOUBLE ) {
      double *pts_ptr = (double*) pts->GetVoidPointer(0);

      for( unsigned int j=0; j<pts_per_element*num_elements; ++j )
      {
        nek_pts[i][0][j] = pts_ptr[j*3+0];
        nek_pts[i][1][j] = pts_ptr[j*3+1];
        nek_pts[i][2][j] = pts_ptr[j*3+2];
      }
    }
    else {
      EXCEPTION1( InvalidVariableException,
                  "avtIVPNek5000Field - Expected single or double precison floating point vertices." );
    }

    // Move the vectors from the VTK structure into a form that Nek5000
    // uses.
    nek_vec[i][0] = new double[pts_per_element*num_elements];
    nek_vec[i][1] = new double[pts_per_element*num_elements];
    nek_vec[i][2] = new double[pts_per_element*num_elements];

    // Get the pointer to the vectors that make up the spetral element.
    if( vecs->GetNumberOfComponents() != 3 ) {
      EXCEPTION1( InvalidVariableException,
                  "avtIVPNek5000Field - Velocity variable does not contain three components." );
    }

    int vec_type = vecs->GetDataType();

    if( vec_type == VTK_FLOAT ) {
      float *vec_ptr = (float*) vecs->GetVoidPointer(0);

      for( unsigned int j=0; j<pts_per_element*num_elements; ++j )
      {
        nek_vec[i][0][j] = vec_ptr[j*3+0];
        nek_vec[i][1][j] = vec_ptr[j*3+1];
        nek_vec[i][2][j] = vec_ptr[j*3+2];
      }
    } else if( vec_type == VTK_DOUBLE ) {
      double *vec_ptr = (double*) vecs->GetVoidPointer(0);
      
      for( unsigned int j=0; j<pts_per_element*num_elements; ++j )
      {
        nek_vec[i][0][j] = vec_ptr[j*3+0];
        nek_vec[i][1][j] = vec_ptr[j*3+1];
        nek_vec[i][2][j] = vec_ptr[j*3+2];
      }
    }
    else {
      EXCEPTION1( InvalidVariableException,
                  "avtIVPNek5000Field - Expected single or double precison floating point vectors." );
    }

    // Create the internal Nek5000 field
    nek_fld[i] = findpts_local_setup(iDim, nek_pts[i], iBlockSize, num_elements,
                                     iBlockSize2, 0.01, pts_per_element*num_elements,
                                     npts, 1024.0*std::numeric_limits<double>::epsilon());
  }
}

// ****************************************************************************
//  Method: avtIVPNek5000TimeVaryingField destructor
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
//   Christoph Garth, Fri Jul 9, 10:10:22 PDT 2010
//   Incorporate vtkVisItInterpolatedVelocityField in avtIVPNek5000TimeVaryingField.
//
// ****************************************************************************

avtIVPNek5000TimeVaryingField::~avtIVPNek5000TimeVaryingField()
{
    if( ds )
        ds->Delete();

    for (int i = 0; i < 2; ++i)
    {
      if( nek_pts[i][0] ) delete [] nek_pts[i][0];
      if( nek_pts[i][1] ) delete [] nek_pts[i][1];
      if( nek_pts[i][2] ) delete [] nek_pts[i][2];
      
      if( nek_vec[i][0] ) delete [] nek_vec[i][0];
      if( nek_vec[i][1] ) delete [] nek_vec[i][1];
      if( nek_vec[i][2] ) delete [] nek_vec[i][2];

      if( nek_fld[i] ) findpts_local_free( nek_fld[i] );
    }
}

// ****************************************************************************
//  Method: avtIVPNek5000TimeVaryingField::GetExtents
//
//  Purpose:
//      Get field bounding box.
//
//  Programmer: Dave Pugmire
//  Creation:   June 8, 2009
//
//  Modifications:
//
//   Christoph Garth, Fri Jul 9, 10:10:22 PDT 2010
//   Incorporate vtkVisItInterpolatedVelocityField in avtIVPNek5000TimeVaryingField.
//
// ****************************************************************************
void
avtIVPNek5000TimeVaryingField::GetExtents( double extents[6] ) const
{
    ds->GetBounds(extents);
}

// ****************************************************************************
//  Method: avtIVPNek5000TimeVaryingField::FindCell
//
//  Purpose:
//      Find the cell containing a given position. Does nothing
//      if this coincides with the position of the last lookup.
//      Returns true if a cell was found, false otherwise.
//
//  Programmer: Christoph Garth
//  Creation:   July 12, 2010
//
//  Modifications:
//
//   David Camp, Tue Feb  1 09:43:32 PST 2011
//   Added a check for time range to the FindCell
//
//   Christoph Garth, Tue Mar 6 16:38:00 PDT 2012
//   Moved ghost data handling into cell locator and changed IsInside()
//   to only consider non-ghost cells.
//
//   Hank Childs, Fri Mar  9 16:50:48 PST 2012
//   Add support for reverse pathlines.
//
//   Dave Pugmire, Wed Jun  5 09:57:41 EDT 2013
//   Fix for pathlines. change from >=, <= to >, < in inclusion for time.
//
// ****************************************************************************

avtIVPField::Result
avtIVPNek5000TimeVaryingField::FindCell( const double& time, const avtVector& pos ) const
{
    bool inside[2] = {true, true};

    if( pos != lastPos )
    {
        lastPos  = pos;
        
        lastCell = loc->FindCell(&pos.x, &lastWeights, false);
        inside[0] = (lastCell != -1);
    }       
    
    if (t0 < t1)
    {
        if( time < t0 || t1 < time )
            inside[1] = false;
    }
    else
    {
        // backwards integration
        if( time < t1 || t0 < time )
            inside[1] = false;
    }

    if (inside[0] && inside[1])
        return OK;
    else if (!inside[0] && !inside[1])
        return OUTSIDE_BOTH;
    else if (!inside[0])
        return OUTSIDE_SPATIAL;
    else
        return OUTSIDE_TEMPORAL;
}

// ****************************************************************************
//  Method: avtIVPNek5000TimeVaryingField::operator
//
//  Purpose:
//      Evaluates a point location by consulting a VTK grid.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
//  Modifications:
//
//    Hank Childs, Thu Apr  2 16:40:08 PDT 2009
//    Use vtkVisItInterpolatedVelocityField, not vtkInterpolatedVelocityField.
//
//    Dave Pugmire, Tue Dec  1 11:50:18 EST 2009
//    Switch from avtVec to avtVector.
//
//   Christoph Garth, Fri Jul 9, 10:10:22 PDT 2010
//   Incorporate vtkVisItInterpolatedVelocityField in avtIVPNek5000TimeVaryingField.
//
//   Hank Childs, Fri Mar  9 16:50:48 PST 2012
//   Add support for reverse pathlines.
//
// ****************************************************************************

avtIVPField::Result
avtIVPNek5000TimeVaryingField::operator()( const double &t,
                                       const avtVector &p,
                                       avtVector &vel ) const
{
    double p0 = (t1-t)/dt;
    double p1 = (t-t0)/dt;

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

    // Now loop through each coordinate and do the appropriate
    // interpolation.
    for( unsigned int j=0; j<3; ++j )
    {
      double v[2];

      for( unsigned int i=0; i<2; ++i )
      {
        // Translate world space coordinates into the spectral parameteric
        // coordinate space.
        findpts_local( code_base,  code_stride,
                       el_base,    el_stride,
                       r_base,     r_stride,
                       dist2_base, dist2_stride,
                       x_base,     x_stride,
                       npts, nek_fld[i] );
    
        findpts_local_eval( &v[i],   out_stride,
                            el_base, el_stride,
                            r_base,  r_stride,
                            npts, nek_vec[i][j], nek_fld[i] );
      }
      
      vel[j] = p1 * v[1] + p0 * v[0];
    }


    // Result res = FindCell(t, p);
    // if (res != OK)
    //     return res;

    // double v0[3], v1[3];

    // double p0 = (t1-t)/dt;
    // double p1 = (t-t0)/dt;

    // if (velCellBased)
    // {
    //     velData[0]->GetTuple( lastCell, v0 );
    //     velData[1]->GetTuple( lastCell, v1 );

    //     vel.x = p1 * v1[0] + p0 * v0[0];
    //     vel.y = p1 * v1[1] + p0 * v0[1];
    //     vel.z = p1 * v1[2] + p0 * v0[2];
    // }
    // else
    // {
    //     for( avtInterpolationWeights::const_iterator wi=lastWeights.begin();
    //          wi!=lastWeights.end(); ++wi )
    //     {
    //         velData[0]->GetTuple( wi->i, v0 );
    //         velData[1]->GetTuple( wi->i, v1 );

    //         v0[0] = p1 * v1[0] + p0 * v0[0];
    //         v0[1] = p1 * v1[1] + p0 * v0[1];
    //         v0[2] = p1 * v1[2] + p0 * v0[2];

    //         vel.x += wi->w * v0[0];
    //         vel.y += wi->w * v0[1];
    //         vel.z += wi->w * v0[2];
    //     }
    // }

    return OK;
}

// ****************************************************************************
//  Method: avtIVPNek5000TimeVaryingField::ConvertToCartesian
//
//  Purpose: Does nothing because the original coordinate system is
//      unknown.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

avtVector 
avtIVPNek5000TimeVaryingField::ConvertToCartesian(const avtVector& pt) const
{
  return pt;
}

// ****************************************************************************
//  Method: avtIVPNek5000TimeVaryingField::ConvertToCylindrical
//
//  Purpose: Does nothing because the original coordinate system is
//      unknown.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

avtVector 
avtIVPNek5000TimeVaryingField::ConvertToCylindrical(const avtVector& pt) const
{
  return pt;
}

// ****************************************************************************
//  Method: avtIVPNek5000TimeVaryingField::ComputeVorticity
//
//  Purpose:
//      Computes the vorticity at a point.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
//  Modifications:
//
//    Hank Childs, Tue Aug 19 16:59:16 PDT 2008
//    Increase the size of the "w" (weights) variable to prevent stack 
//    overwrites.
//
//    Hank Childs, Thu Apr  2 16:40:08 PDT 2009
//    Use vtkVisItInterpolatedVelocityField, not vtkInterpolatedVelocityField.
//
//    Dave Pugmire, Tue Dec  1 11:50:18 EST 2009
//    Switch from avtVec to avtVector.
//
//    Christoph Garth, Fri Jul 9, 10:10:22 PDT 2010
//    Incorporate vtkVisItInterpolatedVelocityField in avtIVPNek5000TimeVaryingField, and
//    use vtkGenericCell for thread safety.
//
// ****************************************************************************

double
avtIVPNek5000TimeVaryingField::ComputeVorticity( const double& t, const avtVector &pt ) const
{
#if 0
    if( velCellBased )
        return 0.0;

    avtVector y;
    this->operator()( t, pt, y );

    double ylen = y.length();

    if( ylen == 0.0 )
        return 0.0;

    double derivs[9], *vel = new double[3*lastWeights.size()];

    for( unsigned int i=0; i<lastWeights.size(); ++i )
        velData->GetTuple( lastWeights[i].i, vel + 3*i );
         
    vtkGenericCell *cell = vtkGenericCell::New();
    ds->GetCell( lastCell, cell );

    {
        double tmp1[2], pcoord[3], dist2, w[1024];
        int subid;

        cell->EvaluatePosition( (double*)&pt.x, tmp1, subid, pcoord, dist2, w );
        cell->Derivatives( 0, pcoord, vel, 3, derivs );
    }

    delete[] vel;
    cell->Delete();

    avtVector vort( derivs[7]-derivs[5],
                    derivs[2]-derivs[6],
                    derivs[3]-derivs[1] );

    return (vort * y) / ylen;
#else
    return 0.0;
#endif
}

// ****************************************************************************
//  Method: avtIVPNek5000TimeVaryingField::ComputeScalarVariable
//
//  Purpose:
//      Computes the variable value at a point.
//
//  Programmer: Dave Pugmire
//  Creation:   June 5, 2009
//
//  Modifications:
//
//    Dave Pugmire, Tue Dec  1 11:50:18 EST 2009
//    Switch from avtVec to avtVector.
//
//   Dave Pugmire, Tue Dec 29 14:37:53 EST 2009
//   Generalize the compute scalar variable.
//
//   Christoph Garth, Fri Jul 9, 10:10:22 PDT 2010
//   Incorporate vtkVisItInterpolatedVelocityField in avtIVPNek5000TimeVaryingField.
//
// ****************************************************************************

double
avtIVPNek5000TimeVaryingField::ComputeScalarVariable(unsigned char index,
                                                 const double& t,
                                                 const avtVector &pt) const
{
    vtkDataArray* data0 = sclData[0][index];
    vtkDataArray* data1 = sclData[1][index];

// TODO. Not all of the scalar fields have the data from the second data set.
// Do we want to limit our selfs, that you must have both data values?
// Currently the color only has the one value.
    //if( data0 == NULL || data1 == NULL )
    if( data0 == NULL )
        return 0.0;

    if (FindCell(t, pt) != OK)
        return 0.0;

    double result = 0.0, tmp0, tmp1;

    if( !sclCellBased[index] )
    {
        for( avtInterpolationWeights::const_iterator wi=lastWeights.begin();
             wi!=lastWeights.end(); ++wi )
        {
            data0->GetTuple( wi->i, &tmp0 );
// TODO. Look if we want this code.
            if( data1 )
            {
                data1->GetTuple( wi->i, &tmp1 );

                result += wi->w * ((t-t0)/(t1-t0)*tmp1 + (t1-t)/(t1-t0)*tmp0);
            }
            else
            {
                result += wi->w * tmp0;
            }
        }
    }
    else
    {
        data0->GetTuple( lastCell, &tmp0 );
// TODO. Look if we want this code.
        if( data1 )
        {
            data1->GetTuple( lastCell, &tmp1 );

            result = (t-t0)/(t1-t0)*tmp1 + (t1-t)/(t1-t0)*tmp0;
        }
        else
        {
           result = tmp0;
        }
    }

    return result;
}

// ****************************************************************************
//  Method: avtIVPNek5000TimeVaryingField::SetScalarVariable
//
//  Purpose:
//      Set up a scalar variable for query through ComputeScalarVariable().
//
//  Programmer: Christoph Garth
//  Creation:   July 13, 2010
//
// ****************************************************************************

void
avtIVPNek5000TimeVaryingField::SetScalarVariable(unsigned char index, const std::string& name)
{
    vtkDataArray* data[2] = { NULL, NULL };
    bool cellBased;

    if( (data[0] = ds->GetPointData()->GetScalars( name.c_str() )) )
    {
        cellBased = false;
        data[1] = 
            ds->GetPointData()->GetArray( (NextTimePrefix+name).c_str() );
    }
    else if( (data[0] = ds->GetCellData()->GetScalars( name.c_str() )) )
    {
        cellBased = true;
        data[1] = 
            ds->GetCellData()->GetArray( (NextTimePrefix+name).c_str() );
    }
    else
        EXCEPTION1( ImproperUseException, 
                    "avtIVPNek5000TimeVaryingField: Can't locate scalar \"" + name + 
                    "\" to interpolate." );

/* TODO. not sure we want all data to have a second value
    if( data[1] == NULL )
        EXCEPTION1( ImproperUseException, 
                    "avtIVPNek5000TimeVaryingField: Can't locate next scalar \"" + name + 
                    "\" to interpolate." );
*/

    if( data[0]->GetNumberOfComponents() != 1 || 
        (data[1] && data[1]->GetNumberOfComponents() != 1) )
        EXCEPTION1( ImproperUseException,
                    "avtIVPNek5000TimeVaryingField: Given variable \"" + name +
                    "\" is not scalar." );
        
    sclDataName[index] = name;
    sclData[0][index] = data[0];
    sclData[1][index] = data[1];
    sclCellBased[index] = cellBased;
}

// ****************************************************************************
//  Method: avtIVPNek5000TimeVaryingField::IsInside
//
//  Purpose:
//      Determines if a point is inside a field.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
//  Modifications:
//
//    Hank Childs, Thu Apr  2 16:40:08 PDT 2009
//    Use vtkVisItInterpolatedVelocityField, not vtkInterpolatedVelocityField.
//
//    Dave Pugmire, Tue Dec  1 11:50:18 EST 2009
//    Switch from avtVec to avtVector.
//
//    Christoph Garth, Fri Jul 9, 10:10:22 PDT 2010
//    Incorporate vtkVisItInterpolatedVelocityField in avtIVPNek5000TimeVaryingField.
//
// ****************************************************************************

avtIVPField::Result
avtIVPNek5000TimeVaryingField::IsInside( const double& t, const avtVector &pt ) const
{
    return FindCell( t, pt );
}

// ****************************************************************************
//  Method: avtIVPNek5000TimeVaryingField::GetDimension
//
//  Purpose:
//      Gets the dimension.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
//  Modifications:
//
//    Hank Childs, Thu Apr  2 16:40:08 PDT 2009
//    Use vtkVisItInterpolatedVelocityField, not vtkInterpolatedVelocityField.
//    (The old method was just returning 0 anyways...)
//
// ****************************************************************************

unsigned int 
avtIVPNek5000TimeVaryingField::GetDimension() const
{
    return 3;
}  

// ****************************************************************************
//  Method: avtIVPNek5000TimeVaryingField::GetTimeRange
//
//  Purpose:
//      Returns temporal extent of the field.
//
//  Programmer: Dave Pugmire
//  Creation:   August 6, 2008
//
// ****************************************************************************

void
avtIVPNek5000TimeVaryingField::GetTimeRange( double range[2] ) const
{
    if (t0 < t1)
    {
        range[0] = t0;
        range[1] = t1;
    }
    else
    {
        // we have a field for reverse pathlines.
        // we need to put the times in ascending order to keep the logic
        // in avtIntegralCurve consistent.
        range[0] = t1;
        range[1] = t0;
    }
}
