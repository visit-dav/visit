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
//                             avtIVPVTKField.C                              //
// ************************************************************************* //

#include <avtIVPVTKField.h>
#include <iostream>
#include <vtkCell.h>
#include <vtkDataSet.h>
#include <vtkVisItInterpolatedVelocityField.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <DebugStream.h>

// ****************************************************************************
//  Method: avtIVPVTKField constructor
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
//  Modifications:
//
//    Hank Childs, Thu Apr  2 16:40:08 PDT 2009
//    Use vtkVisItInterpolatedVelocityField, not vtkInterpolatedVelocityField.
//
// ****************************************************************************

avtIVPVTKField::avtIVPVTKField( vtkVisItInterpolatedVelocityField* velocity ) 
    : iv(velocity)
{
    iv->Register( NULL );
    normalized = false;
}


avtIVPVTKField::avtIVPVTKField() 
    : iv(0)
{
    normalized = false;
}


// ****************************************************************************
//  Method: avtIVPVTKField destructor
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

avtIVPVTKField::~avtIVPVTKField()
{
    if( iv )
      iv->Delete();
}

// ****************************************************************************
//  Method: avtIVPVTKField::HasGhostZones
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
avtIVPVTKField::HasGhostZones() const
{
    return (iv->GetDataSet()->GetCellData()->GetArray("avtGhostZones") != NULL);
}

// ****************************************************************************
//  Method: avtIVPVTKField::GetExtents
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
avtIVPVTKField::GetExtents(double *extents) const
{
    iv->GetDataSet()->GetBounds(extents);
}


// ****************************************************************************
//  Method: avtIVPVTKField::operator
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
// ****************************************************************************

avtVector
avtIVPVTKField::operator()(const double &t, const avtVector &pt) const
{
    double vec[3], param[4] = {pt.x, pt.y, pt.z, t};
    int result = iv->Evaluate(param, vec);
    
    if( !result )
        throw Undefined();
    
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
//  Method: avtIVPVTKField::ComputeVorticity
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
// ****************************************************************************

double
avtIVPVTKField::ComputeVorticity( const double& t, const avtVector &pt ) const
{
    avtVector y = (*this)(t, pt);

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
                 vort[0] * y.x + vort[1] * y.y 
                 + vort[2] * y.z
                 ) / len;
    //cout<<"omega= "<<omega<<endl;

    return omega;
}

// ****************************************************************************
//  Method: avtIVPVTKField::ComputeScalarVariable
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
avtIVPVTKField::ComputeScalarVariable(const double& t,
                                      const avtVector &pt) const
{
    vtkDataSet *ds = iv->GetDataSet();
    vtkCell *cell = ds->GetCell(iv->GetLastCell());
    int numPts = cell->GetNumberOfPoints();
    
    int subId;
    double pcoords[3], dist2, v;
    double *weights = new double[numPts];

    double xvals[3] = {pt.x, pt.y, pt.z};
    cell->EvaluatePosition(xvals, NULL, subId, pcoords, dist2, weights);
    
    double value = 0.0;
    //See if we have node centered data...
    if (ds->GetPointData()->GetScalars() != NULL)
    {
        vtkDataArray *data = ds->GetPointData()->GetScalars();
        for (int i = 0; i < numPts; i++)
        {
            int id = cell->PointIds->GetId(i);
            data->GetTuple(id, &v);
            value += v*weights[i];
        }
    }
    else if (ds->GetCellData()->GetScalars() != NULL)
    {
        vtkDataArray *data = ds->GetCellData()->GetScalars();
        int id = cell->PointIds->GetId(0);
        data->GetTuple(id,&value);
    }

    delete [] weights;
    return value;
}


// ****************************************************************************
//  Method: avtIVPVTKField::IsInside
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
// ****************************************************************************

bool
avtIVPVTKField::IsInside( const double& t, const avtVector &pt ) const
{
    double vec[3], param[4] = {pt.x, pt.y, pt.z, t};
    return iv->Evaluate(param, vec);
}


// ****************************************************************************
//  Method: avtIVPVTKField::GetDimension
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
avtIVPVTKField::GetDimension() const
{
    return 3;
}  

// ****************************************************************************
//  Method: avtIVPVTKField::SetNormalized
//
//  Purpose:
//      Sets field normalization.
//
//  Programmer: Dave Pugmire
//  Creation:   August 6, 2008
//
// ****************************************************************************

void
avtIVPVTKField::SetNormalized( bool v )
{
    normalized = v;
}

