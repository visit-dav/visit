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
//                        avtIVPVTKTimeVaryingField.C                        //
// ************************************************************************* //

#include <avtIVPVTKTimeVaryingField.h>
#include <iostream>
#include <limits>

#include <vtkUnsignedCharArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkGenericCell.h>
#include <DebugStream.h>

//const char* avtIVPVTKTimeVaryingField::NextTimePrefix = "__nextTimePrefix_";
const char* avtIVPVTKTimeVaryingField::NextTimePrefix = "__pathlineNextTimeVar__";

// ****************************************************************************
//  Method: avtIVPVTKTimeVaryingField constructor
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
//   Incorporate vtkVisItInterpolatedVelocityField in avtIVPVTKTimeVaryingField.
//
// ****************************************************************************

avtIVPVTKTimeVaryingField::avtIVPVTKTimeVaryingField( vtkDataSet* dataset, 
                                                      avtCellLocator* locator,
                                                      double _t0, double _t1 ) 
    : ds(dataset), loc(locator), t0(_t0), t1(_t1)
{
    if( ds )
        ds->Register( NULL );

    if( (velData[0] = ds->GetPointData()->GetVectors()) )
    {
        velCellBased = false;

        velData[1] = 
            ds->GetPointData()
            ->GetArray( NextTimePrefix );
    }
    else if( (velData[0] = ds->GetCellData()->GetVectors()) )
    {
        velCellBased = true;

        velData[1] = 
            ds->GetCellData()
            ->GetArray( NextTimePrefix );
    }
    else 
    {
        velData[0] = velData[1] = NULL;
        EXCEPTION1( ImproperUseException, 
                    "avtIVPVTKTimeVaryingField: Can't locate vectors to interpolate." );
    }

    if( velData[1] == NULL )
    {
        EXCEPTION1( ImproperUseException, 
                    "avtIVPVTKTimeVaryingField: Can't locate second pair of vectors to interpolate." );
    }

    // Cache a raw pointer to the ghost zone flags
    vtkUnsignedCharArray* ghostData = 
        vtkUnsignedCharArray::SafeDownCast( ds->GetCellData()->GetArray( "avtGhostZones" ) );

    ghostPtr = ghostData ? ghostData->GetPointer(0) : NULL;

    lastCell = -1;
    lastPos.x = lastPos.y = lastPos.z =
        std::numeric_limits<double>::quiet_NaN();

    std::fill( sclData[0], sclData[0]+256, (vtkDataArray*)NULL );
    std::fill( sclData[1], sclData[1]+256, (vtkDataArray*)NULL );
    std::fill( sclCellBased, sclCellBased+256, false );

    sclDataName.resize( 256 );
}

// ****************************************************************************
//  Method: avtIVPVTKTimeVaryingField destructor
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
//   Christoph Garth, Fri Jul 9, 10:10:22 PDT 2010
//   Incorporate vtkVisItInterpolatedVelocityField in avtIVPVTKTimeVaryingField.
//
// ****************************************************************************

avtIVPVTKTimeVaryingField::~avtIVPVTKTimeVaryingField()
{
    if( ds )
        ds->Delete();
}

// ****************************************************************************
//  Method: avtIVPVTKTimeVaryingField::HasGhostZones
//
//  Purpose:
//      Determine if this vector field has ghost zones.
//
//  Programmer: Dave Pugmire
//  Creation:   June 8, 2009
//
//  Modifications:
//
//   Christoph Garth, Fri Jul 9, 10:10:22 PDT 2010
//   Incorporate vtkVisItInterpolatedVelocityField in avtIVPVTKTimeVaryingField.
//
// ****************************************************************************

bool
avtIVPVTKTimeVaryingField::HasGhostZones() const
{
    return ghostPtr != NULL; 
}

// ****************************************************************************
//  Method: avtIVPVTKTimeVaryingField::GetExtents
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
//   Incorporate vtkVisItInterpolatedVelocityField in avtIVPVTKTimeVaryingField.
//
// ****************************************************************************

void
avtIVPVTKTimeVaryingField::GetExtents( double extents[6] ) const
{
    ds->GetBounds(extents);
}

// ****************************************************************************
//  Method: avtIVPVTKTimeVaryingField::FindCell
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
// ****************************************************************************

bool
avtIVPVTKTimeVaryingField::FindCell( const double& time, const avtVector& pos ) const
{
    if( pos != lastPos )
    {
        lastPos  = pos;
        
        if( -1 == (lastCell = loc->FindCell( &pos.x, &lastWeights )) )
            return false;

        if( ghostPtr && ghostPtr[lastCell] & 0xfc )
        {
            lastCell = -1;
            return false;
        }
    }       

    if( time < t0 || time > t1 )
        return false;

    return lastCell != -1;
}

// ****************************************************************************
//  Method: avtIVPVTKTimeVaryingField::operator
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
//   Incorporate vtkVisItInterpolatedVelocityField in avtIVPVTKTimeVaryingField.
//
// ****************************************************************************

avtIVPField::Result
avtIVPVTKTimeVaryingField::operator()( const double &t, const avtVector &p, avtVector &vel ) const
{
    if( !FindCell( t, p ) )
        return( avtIVPSolverResult::OUTSIDE_DOMAIN );
    if( t < t0 || t > t1 )
        return( avtIVPSolverResult::OUTSIDE_TIME_FRAME );

    if( velCellBased )
    {
        double v0[3], v1[3];

        velData[0]->GetTuple( lastCell, v0 );
        velData[1]->GetTuple( lastCell, v1 );

        vel.x = (t-t0)/(t1-t0) * v1[0] + (t1-t)/(t1-t0) * v0[0];
        vel.y = (t-t0)/(t1-t0) * v1[1] + (t1-t)/(t1-t0) * v0[1];
        vel.z = (t-t0)/(t1-t0) * v1[2] + (t1-t)/(t1-t0) * v0[2];
    }
    else
    {
        double v0[3], v1[3];

        for( avtInterpolationWeights::const_iterator wi=lastWeights.begin();
             wi!=lastWeights.end(); ++wi )
        {
            velData[0]->GetTuple( wi->i, v0 );
            velData[1]->GetTuple( wi->i, v1 );

            v0[0] = (t-t0)/(t1-t0) * v1[0] + (t1-t)/(t1-t0) * v0[0];
            v0[1] = (t-t0)/(t1-t0) * v1[1] + (t1-t)/(t1-t0) * v0[1];
            v0[2] = (t-t0)/(t1-t0) * v1[2] + (t1-t)/(t1-t0) * v0[2];

            vel.x += wi->w * v0[0];
            vel.y += wi->w * v0[1];
            vel.z += wi->w * v0[2];
        }
    }

    return( avtIVPSolverResult::OK );
}

// ****************************************************************************
//  Method: avtIVPVTKTimeVaryingField::ConvertToCartesian
//
//  Purpose: Does nothing because the original coordinate system is
//      unknown.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

avtVector 
avtIVPVTKTimeVaryingField::ConvertToCartesian(const avtVector& pt) const
{
  return pt;
}

// ****************************************************************************
//  Method: avtIVPVTKTimeVaryingField::ConvertToCylindrical
//
//  Purpose: Does nothing because the original coordinate system is
//      unknown.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

avtVector 
avtIVPVTKTimeVaryingField::ConvertToCylindrical(const avtVector& pt) const
{
  return pt;
}

// ****************************************************************************
//  Method: avtIVPVTKTimeVaryingField::ComputeVorticity
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
//    Incorporate vtkVisItInterpolatedVelocityField in avtIVPVTKTimeVaryingField, and
//    use vtkGenericCell for thread safety.
//
// ****************************************************************************

double
avtIVPVTKTimeVaryingField::ComputeVorticity( const double& t, const avtVector &pt ) const
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
//  Method: avtIVPVTKTimeVaryingField::ComputeScalarVariable
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
//   Incorporate vtkVisItInterpolatedVelocityField in avtIVPVTKTimeVaryingField.
//
// ****************************************************************************

double
avtIVPVTKTimeVaryingField::ComputeScalarVariable(unsigned char index,
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

    if( !FindCell( t, pt ) )
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
//  Method: avtIVPVTKTimeVaryingField::SetScalarVariable
//
//  Purpose:
//      Set up a scalar variable for query through ComputeScalarVariable().
//
//  Programmer: Christoph Garth
//  Creation:   July 13, 2010
//
// ****************************************************************************

void
avtIVPVTKTimeVaryingField::SetScalarVariable(unsigned char index, const std::string& name)
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
                    "avtIVPVTKTimeVaryingField: Can't locate scalar \"" + name + 
                    "\" to interpolate." );

/* TODO. not sure we want all data to have a second value
    if( data[1] == NULL )
        EXCEPTION1( ImproperUseException, 
                    "avtIVPVTKTimeVaryingField: Can't locate next scalar \"" + name + 
                    "\" to interpolate." );
*/

    if( data[0]->GetNumberOfComponents() != 1 || 
        (data[1] && data[1]->GetNumberOfComponents() != 1) )
        EXCEPTION1( ImproperUseException,
                    "avtIVPVTKTimeVaryingField: Given variable \"" + name +
                    "\" is not scalar." );
        
    sclDataName[index] = name;
    sclData[0][index] = data[0];
    sclData[1][index] = data[1];
    sclCellBased[index] = cellBased;
}

// ****************************************************************************
//  Method: avtIVPVTKTimeVaryingField::IsInside
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
//    Incorporate vtkVisItInterpolatedVelocityField in avtIVPVTKTimeVaryingField.
//
// ****************************************************************************

bool
avtIVPVTKTimeVaryingField::IsInside( const double& t, const avtVector &pt ) const
{
    return FindCell( t, pt );
}

// ****************************************************************************
//  Method: avtIVPVTKTimeVaryingField::GetDimension
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
avtIVPVTKTimeVaryingField::GetDimension() const
{
    return 3;
}  

// ****************************************************************************
//  Method: avtIVPVTKTimeVaryingField::SetNormalized
//
//  Purpose:
//      Sets field normalization.
//
//  Programmer: Dave Pugmire
//  Creation:   August 6, 2008
//
// ****************************************************************************

void
avtIVPVTKTimeVaryingField::SetNormalized( bool v )
{
}

// ****************************************************************************
//  Method: avtIVPVTKTimeVaryingField::GetTimeRange
//
//  Purpose:
//      Returns temporal extent of the field.
//
//  Programmer: Dave Pugmire
//  Creation:   August 6, 2008
//
// ****************************************************************************

void
avtIVPVTKTimeVaryingField::GetTimeRange( double range[2] ) const
{
    range[0] = t0;
    range[1] = t1;
}

