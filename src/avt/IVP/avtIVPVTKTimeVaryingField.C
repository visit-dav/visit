/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
#include <vtkCell.h>
#include <vtkDataSet.h>
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

avtIVPVTKTimeVaryingField::avtIVPVTKTimeVaryingField( vtkVisItInterpolatedVelocityField *velocity1,
                                                      vtkVisItInterpolatedVelocityField *velocity2,
                                                      double t1,
                                                      double t2)
{
    iv1 = velocity1;
    iv1->Register( NULL );
    iv2 = velocity2;
    iv2->Register( NULL );
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
    iv1->Delete();
    iv2->Delete();
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
// ****************************************************************************

avtVec
avtIVPVTKTimeVaryingField::operator()(const double& t, const avtVecRef& x) const
{
    debug5<<"Eval( "<<t<<", "<<x<<") = ";

    // Check for inclusion in this time boundary.
    if (t < time1 || t > time2)
    {
        debug5<<"  **OUT of TIME**\n";
        throw Undefined();
    }

    // Evaluate the field at both timesteps.
    avtVec y1(x.dim()), param(pad(x,t)), y2(x.dim());
    if ( ! iv1->Evaluate(param.values(), y1.values()) ||
         ! iv2->Evaluate(param.values(), y2.values()) )
    {
        debug5<<"  **OUT of BOUNDS**\n";
        throw Undefined();
    }
    
    double prop1 = 1. - (t - time1) / (time2 - time1);
    
    avtVec y(x.dim());
    y = prop1*y1 + (1-prop1)*y2;

    debug5<<"T= "<<t<<" ["<<time1<<" "<<time2<<"]"<<" Y1 = "<<y1<<" Y2= "<<y2<<" y= "<<y<<endl;

    if ( normalized )
    {
        double len = y.length();
        if ( len > 0.0 )
            y /= len;
    }

    return y;
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
// ****************************************************************************

double
avtIVPVTKTimeVaryingField::ComputeVorticity( const double& t, const avtVecRef& x ) const
{
EXCEPTION0(ImproperUseException); // didn't do this.
    avtVec y( x.dim() );
    avtVec param = pad(x,t);
    
    int result = iv1->Evaluate( param.values(), y.values() );
    
    if( !result )
        throw Undefined();

    vtkDataSet *ds = iv1->GetDataSet();
    vtkIdType cellID = iv1->GetLastCell();
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
    double *w = iv1->GetLastWeights();
    double *pcoords = iv1->GetLastPCoords();
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
// ****************************************************************************

bool
avtIVPVTKTimeVaryingField::IsInside( const double& t, const avtVecRef& x ) const
{
    avtVec y(x.dim());
    avtVec param = pad(x,t);

    return (t >= time1 && t <= time2 &&
            iv1->Evaluate(param.values(), y.values()) &&
            iv2->Evaluate(param.values(), y.values()));
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
