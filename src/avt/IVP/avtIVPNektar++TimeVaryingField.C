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
//                        avtIVPNektar++TimeVaryingField.C                   //
// ************************************************************************* //

#include <avtIVPNektar++TimeVaryingField.h>

#include <vtkCellData.h>
#include <vtkLongArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkGenericCell.h>
#include <DebugStream.h>

#include <InvalidVariableException.h>

#include <iostream>
#include <limits>

//const char* avtIVPNektarPPTimeVaryingField::NextTimePrefix = "__nextTimePrefix_";
const char* avtIVPNektarPPTimeVaryingField::NextTimePrefix = "__pathlineNextTimeVar__";

// ****************************************************************************
//  Method: avtIVPNektarPPTimeVaryingField constructor
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
//   Incorporate vtkVisItInterpolatedVelocityField in avtIVPNektarPPTimeVaryingField.
//
// ****************************************************************************

avtIVPNektarPPTimeVaryingField::avtIVPNektarPPTimeVaryingField( vtkDataSet* dataset, 
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
    //                 "avtIVPNektarPPTimeVaryingField: Can't locate vectors to interpolate." );
    // }

    // if( velData[1] == NULL )
    // {
    //     EXCEPTION1( ImproperUseException, 
    //                 "avtIVPNektarPPTimeVaryingField: Can't locate second pair of vectors to interpolate." );
    // }

    lastCell = -1;
    lastPos.x = lastPos.y = lastPos.z =
        std::numeric_limits<double>::quiet_NaN();

    std::fill( sclData[0], sclData[0]+256, (vtkDataArray*)NULL );
    std::fill( sclData[1], sclData[1]+256, (vtkDataArray*)NULL );
    std::fill( sclCellBased, sclCellBased+256, false );

    sclDataName.resize( 256 );

    // Get the Nektar++ data.
    vtkFieldData *fieldData = ds->GetFieldData();

    long *fp[2];

    const char *fieldname[2] = {"Nektar++FieldPointers",
                                "Nektar++FieldPointers2" };

    for (int i = 0; i < 2; ++i)
    {
      fp[i] =
        (long *) (fieldData->GetAbstractArray(fieldname[i])->GetVoidPointer(0));

      if( fp[i] )
      {
        // std::cerr << "Get info " << t0 << "  ";

        for (int j = 0; j < 3; ++j)
        {
          if( fp[i][j] )
            nektar_field[i][j] = (Nektar::MultiRegions::ExpListSharedPtr)
              (*((Nektar::MultiRegions::ExpListSharedPtr*) fp[i][j]));

          // std::cerr << (intptr_t) &(nektar_field[i][j]) << "  ";
          // std::cerr << nektar_field[i][j] << "  ";
          // std::cerr << std::endl;
        }

        // std::cerr << std::endl;
      }
      else
      {
        EXCEPTION1( InvalidVariableException,
                    "Uninitialized option: Nektar++FieldPointers. (Please contact visit-developer mailing list to report)" );
      }
    }
}

// ****************************************************************************
//  Method: avtIVPNektarPPTimeVaryingField destructor
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
//   Christoph Garth, Fri Jul 9, 10:10:22 PDT 2010
//   Incorporate vtkVisItInterpolatedVelocityField in avtIVPNektarPPTimeVaryingField.
//
// ****************************************************************************

avtIVPNektarPPTimeVaryingField::~avtIVPNektarPPTimeVaryingField()
{
    if( ds )
        ds->Delete();
}

// ****************************************************************************
//  Method: avtIVPNektarPPTimeVaryingField::GetExtents
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
//   Incorporate vtkVisItInterpolatedVelocityField in avtIVPNektarPPTimeVaryingField.
//
// ****************************************************************************
void
avtIVPNektarPPTimeVaryingField::GetExtents( double extents[6] ) const
{
    ds->GetBounds(extents);
}

// ****************************************************************************
//  Method: avtIVPNektarPPTimeVaryingField::FindCell
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
avtIVPNektarPPTimeVaryingField::FindCell( const double& time, const avtVector& pos ) const
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
//  Method: avtIVPNektarPPTimeVaryingField::operator
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
//   Incorporate vtkVisItInterpolatedVelocityField in avtIVPNektarPPTimeVaryingField.
//
//   Hank Childs, Fri Mar  9 16:50:48 PST 2012
//   Add support for reverse pathlines.
//
// ****************************************************************************

avtIVPField::Result
avtIVPNektarPPTimeVaryingField::operator()( const double &t,
                                       const avtVector &p,
                                       avtVector &vel ) const
{
    Result res = FindCell(t, p);
    if (res != OK)
        return res;

    double v0[3], v1[3];

    double p0 = (t1-t)/dt;
    double p1 = (t-t0)/dt;

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

    double vec[2][3];

    // Get the Nektar++ element id at this point. Assume the cell
    // boundaries are liner and not curves thus the nektar element is
    // the vtk element.
    int nt_el = lastCell, nt_numElements = nektar_field[0][0]->GetExpSize();

    // for( int i=0, j=1; j<nt_numElements; ++i, ++j)
    // {
    //   if( nektar_element_lookup[i] <= el && el < nektar_element_lookup[i] )
    //   {
    //    nt_el = i;
    //    break;
    //   }
    // }

    // Set up the point in the Nektar++ format.
    Nektar::Array<OneD, NekDouble> coords(3);
    coords[0] = p[0];
    coords[1] = p[1];
    coords[2] = p[2];
    
    // Loop through each velocity component and do the appropriate
    // interpolation at the given point.
    for (int i = 0; i < 2; ++i)
    {
      for (int j = 0; j < 3; ++j)
      {
        if( nektar_field[i][j] )
        {
          Nektar::Array<Nektar::OneD, Nektar::NekDouble> physVals =
            nektar_field[i][j]->GetPhys() +
            nektar_field[i][j]->GetPhys_Offset(nt_el);
          
          vec[i][j] =
            nektar_field[i][j]->GetExp(nt_el)->PhysEvaluate(coords, physVals);
        }
        else
        {
          vec[i][j] = 0;
        }
      }
    }

    vel.x = p1 * vec[1][0] + p0 * vec[0][0];
    vel.y = p1 * vec[1][1] + p0 * vec[0][1];
    vel.z = p1 * vec[1][2] + p0 * vec[0][2];

    return OK;
}

// ****************************************************************************
//  Method: avtIVPNektarPPTimeVaryingField::ConvertToCartesian
//
//  Purpose: Does nothing because the original coordinate system is
//      unknown.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

avtVector 
avtIVPNektarPPTimeVaryingField::ConvertToCartesian(const avtVector& pt) const
{
  return pt;
}

// ****************************************************************************
//  Method: avtIVPNektarPPTimeVaryingField::ConvertToCylindrical
//
//  Purpose: Does nothing because the original coordinate system is
//      unknown.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

avtVector 
avtIVPNektarPPTimeVaryingField::ConvertToCylindrical(const avtVector& pt) const
{
  return pt;
}

// ****************************************************************************
//  Method: avtIVPNektarPPTimeVaryingField::ComputeVorticity
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
//    Incorporate vtkVisItInterpolatedVelocityField in avtIVPNektarPPTimeVaryingField, and
//    use vtkGenericCell for thread safety.
//
// ****************************************************************************

double
avtIVPNektarPPTimeVaryingField::ComputeVorticity( const double& t, const avtVector &pt ) const
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
//  Method: avtIVPNektarPPTimeVaryingField::ComputeScalarVariable
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
//   Incorporate vtkVisItInterpolatedVelocityField in avtIVPNektarPPTimeVaryingField.
//
// ****************************************************************************

double
avtIVPNektarPPTimeVaryingField::ComputeScalarVariable(unsigned char index,
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
//  Method: avtIVPNektarPPTimeVaryingField::SetScalarVariable
//
//  Purpose:
//      Set up a scalar variable for query through ComputeScalarVariable().
//
//  Programmer: Christoph Garth
//  Creation:   July 13, 2010
//
// ****************************************************************************

void
avtIVPNektarPPTimeVaryingField::SetScalarVariable(unsigned char index, const std::string& name)
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
                    "avtIVPNektarPPTimeVaryingField: Can't locate scalar \"" + name + 
                    "\" to interpolate." );

/* TODO. not sure we want all data to have a second value
    if( data[1] == NULL )
        EXCEPTION1( ImproperUseException, 
                    "avtIVPNektarPPTimeVaryingField: Can't locate next scalar \"" + name + 
                    "\" to interpolate." );
*/

    if( data[0]->GetNumberOfComponents() != 1 || 
        (data[1] && data[1]->GetNumberOfComponents() != 1) )
        EXCEPTION1( ImproperUseException,
                    "avtIVPNektarPPTimeVaryingField: Given variable \"" + name +
                    "\" is not scalar." );
        
    sclDataName[index] = name;
    sclData[0][index] = data[0];
    sclData[1][index] = data[1];
    sclCellBased[index] = cellBased;
}

// ****************************************************************************
//  Method: avtIVPNektarPPTimeVaryingField::IsInside
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
//    Incorporate vtkVisItInterpolatedVelocityField in avtIVPNektarPPTimeVaryingField.
//
// ****************************************************************************

avtIVPField::Result
avtIVPNektarPPTimeVaryingField::IsInside( const double& t, const avtVector &pt ) const
{
    return FindCell( t, pt );
}

// ****************************************************************************
//  Method: avtIVPNektarPPTimeVaryingField::GetDimension
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
avtIVPNektarPPTimeVaryingField::GetDimension() const
{
    return 3;
}  

// ****************************************************************************
//  Method: avtIVPNektarPPTimeVaryingField::GetTimeRange
//
//  Purpose:
//      Returns temporal extent of the field.
//
//  Programmer: Dave Pugmire
//  Creation:   August 6, 2008
//
// ****************************************************************************

void
avtIVPNektarPPTimeVaryingField::GetTimeRange( double range[2] ) const
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
