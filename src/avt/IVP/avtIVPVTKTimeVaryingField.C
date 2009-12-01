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
//                        avtIVPVTKTimeVaryingField.C                        //
// ************************************************************************* //

#include <ImproperUseException.h>
#include <avtIVPVTKTimeVaryingField.h>
#include <iostream>
#include <vtkCell.h>
#include <vtkDataSet.h>
#include <vtkCellData.h>
#include <vtkVisItInterpolatedVelocityField.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <DebugStream.h>

// ****************************************************************************
//  Method: avtIVPVTKTimeVaryingField constructor
//
//  Programmer:  Dave Pugmire (on behalf of Hank Childs)
//  Creation:    Tue Feb 24 09:24:49 EST 2009
//
// ****************************************************************************

avtIVPVTKTimeVaryingField::avtIVPVTKTimeVaryingField( vtkVisItInterpolatedVelocityField *velocity,
                                                      double t1,
                                                      double t2)
{
    iv = velocity;
    iv->Register( NULL );
    time1 = t1;
    time2 = t2;
    normalized = false;
}


// ****************************************************************************
//  Method: avtIVPVTKTimeVaryingField destructor
//
//  Programmer:  Dave Pugmire (on behalf of Hank Childs)
//  Creation:    Tue Feb 24 09:24:49 EST 2009
//
// ****************************************************************************

avtIVPVTKTimeVaryingField::~avtIVPVTKTimeVaryingField()
{
    iv->Delete();
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
// ****************************************************************************

bool
avtIVPVTKTimeVaryingField::HasGhostZones() const
{
    return (iv->GetDataSet()->GetCellData()->GetArray("avtGhostZones") != NULL);
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
// ****************************************************************************

void
avtIVPVTKTimeVaryingField::GetExtents(double *extents) const
{
    iv->GetDataSet()->GetBounds(extents);
}



// ****************************************************************************
//  Method: avtIVPVTKTimeVaryingField::operator
//
//  Purpose:
//      Evaluates a point location by consulting a VTK grid.
//
//  Programmer:  Dave Pugmire (on behalf of Hank Childs)
//  Creation:    Tue Feb 24 09:24:49 EST 2009
//
//  Modifications:
//
//    Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//    Check time bounds and throw execption.
//
//    Hank Childs, Thu Apr  2 16:36:50 PDT 2009
//    Update to use vtkVisItInterpolatedVelocityField, not 
//    vtkInterpolatedVelocityField.
//
//    Hank Childs, Tue Apr  7 08:52:59 CDT 2009
//    Use a single vtkVisItInterpolatedVelocityField, which saves on
//    computation.
//
//    Hank Childs, Fri Apr 10 23:31:22 CDT 2009
//    Put if statements in front of debug's.  The generation of strings to
//    output to debug was doubling the total integration time.
//
//    Mark C. Miller, Wed Apr 22 13:48:13 PDT 2009
//    Changed interface to DebugStream to obtain current debug level.
//
//    Dave Pugmire, Tue Dec  1 11:50:18 EST 2009
//    Switch from avtVec to avtVector.
//
// ****************************************************************************

avtVector
avtIVPVTKTimeVaryingField::operator()(const double &t, const avtVector &pt) const
{
    // Check for inclusion in this time boundary.
    if (t < time1 || t > time2)
    {
        throw Undefined();
    }

    // Evaluate the field at both timesteps.
    double vec[3], param[4] = {pt.x, pt.y, pt.z, t};
    if ( ! iv->Evaluate(param, vec, t))
    {
        throw Undefined();
    }
    
    avtVector v(vec[0], vec[1], vec[2]);

    if ( normalized )
    {
        double len = v.length();
        if ( len > 0.0 )
            v /= len;
    }

    return v;
}


// ****************************************************************************
//  Method: avtIVPVTKTimeVaryingField::ComputeVorticity
//
//  Purpose:
//      Computes the vorticity at a point.
//
//  Programmer:  Dave Pugmire (on behalf of Hank Childs)
//  Creation:    Tue Feb 24 09:24:49 EST 2009
//
//  Modifications:
//
//    Hank Childs, Thu Apr  2 16:36:50 PDT 2009
//    Update to use vtkVisItInterpolatedVelocityField, not 
//    vtkInterpolatedVelocityField.
//
//    Dave Pugmire, Tue Dec  1 11:50:18 EST 2009
//    Switch from avtVec to avtVector.
//
// ****************************************************************************

double
avtIVPVTKTimeVaryingField::ComputeVorticity( const double &t, const avtVector &pt ) const
{
EXCEPTION0(ImproperUseException); // didn't do this.
/*
    avtVec y( x.dim() );
    avtVec param = pad(x,t);
    
    int result = iv->Evaluate( param.values(), y.values() );
    
    if( !result )
        throw Undefined();

    vtkDataSet *ds = iv->GetDataSet();
    vtkIdType cellID = iv->GetLastCell();
    vtkCell *cell = ds->GetCell( cellID );
    
    vtkDoubleArray *cellVectors;
    cellVectors = vtkDoubleArray::New();
    cellVectors->SetNumberOfComponents(3);
    cellVectors->Allocate(3*VTK_CELL_SIZE);
        
    vtkPointData *pd = ds->GetPointData();
    vtkDataArray *inVectors = pd->GetVectors();

    double derivs[9];
    inVectors->GetTuples( cell->PointIds, cellVectors );

    double *cellVel = cellVectors->GetPointer(0);
    double *w = iv->GetLastWeights();
    double *pcoords = iv->GetLastPCoords();
    cell->Derivatives( 0, pcoords, cellVel, 3, derivs);
    //cout<<"pcoords= "<<pcoords[0]<<" "<<pcoords[1]<<" "<<pcoords[2]<<endl;

    cellVectors->Delete();
    double vort[3];
    vort[0] = derivs[7] - derivs[5];
    vort[1] = derivs[2] - derivs[6];
    vort[2] = derivs[3] - derivs[1];
    //cout<<"id= "<<cellID<<" p= "<<x<<" ";
    //cout<<"y= "<<y<<" ";
    //cout<<" vort["<<vort[0]<<" "<<vort[1]<<" "<<vort[2]<<"] ";
    double omega = 0.0;
    
    double len = y.length();
    if ( len > 0.0 )
        omega = (
                  vort[0] * y.values()[0] + vort[1] * y.values()[1] 
                  + vort[2] * y.values()[2]
                ) / len;
    //cout<<"omega= "<<omega<<endl;

    return omega;
*/
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
// ****************************************************************************

double
avtIVPVTKTimeVaryingField::ComputeScalarVariable(const double &t,
                                                 const avtVector &pt) const
{
    EXCEPTION0(ImproperUseException); // didn't do this.
}

// ****************************************************************************
//  Method: avtIVPVTKTimeVaryingField::IsInside
//
//  Purpose:
//      Determines if a point is inside a field.
//
//  Programmer:  Dave Pugmire (on behalf of Hank Childs)
//  Creation:    Tue Feb 24 09:24:49 EST 2009
//
//  Modifications:
//
//    Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//    Check time bounds.
//
//    Hank Childs, Fri Apr 10 23:31:22 CDT 2009
//    Make sure to pass time along to the VTK module, as it now uses it.
//
//    Dave Pugmire, Tue Dec  1 11:50:18 EST 2009
//    Switch from avtVec to avtVector.
//
// ****************************************************************************

bool
avtIVPVTKTimeVaryingField::IsInside( const double &t, const avtVector &pt ) const
{
    double vec[3], param[4] = {pt.x, pt.y, pt.z, t};

    return (t >= time1 && t <= time2 && iv->Evaluate(param, vec, t));
}


// ****************************************************************************
//  Method: avtIVPVTKTimeVaryingField::GetDimension
//
//  Purpose:
//      Gets the dimension.
//
//  Programmer:  Dave Pugmire (on behalf of Hank Childs)
//  Creation:    Tue Feb 24 09:24:49 EST 2009
//
//  Modifications:
//
//    Hank Childs, Thu Apr  2 16:36:50 PDT 2009
//    Update to use vtkVisItInterpolatedVelocityField, not 
//    vtkInterpolatedVelocityField.
//    (Hard code "3", since that is what the old implementation was doing
//    anyways...)
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
//  Programmer:  Dave Pugmire (on behalf of Hank Childs)
//  Creation:    Tue Feb 24 09:24:49 EST 2009
//
// ****************************************************************************

void
avtIVPVTKTimeVaryingField::SetNormalized( bool v )
{
    normalized = v;
}

// ****************************************************************************
//  Method: avtIVPVTKTimeVaryingField::GetValidTimeRange
//
//  Purpose:
//      Return the time boundaries.
//
//  Programmer:  Dave Pugmire
//  Creation:    Tue Mar 10 2009
//
// ****************************************************************************

bool
avtIVPVTKTimeVaryingField::GetValidTimeRange(double range[]) const
{
    range[0] = time1;
    range[1] = time2;
    return true;
}
