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
//                             avtIVPVTKField.C                              //
// ************************************************************************* //

#include <avtIVPVTKField.h>
#include <iostream>
#include <vtkCell.h>
#include <vtkDataSet.h>
#include <vtkInterpolatedVelocityField.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <DebugStream.h>


// ****************************************************************************
//  Method: avtIVPVTKField constructor
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

avtIVPVTKField::avtIVPVTKField( vtkInterpolatedVelocityField* velocity ) 
    : iv(velocity)
{
    iv->Register( NULL );
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
    iv->Delete();
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
//    Dave Pugmire, Mon Nov 24 14:39:29 EST 2008
//    Added debug information for printing out evaluations and showing the
//    node values.
//
// ****************************************************************************

avtVec
avtIVPVTKField::operator()(const double& t, const avtVecRef& x) const
{
    avtVec y( x.dim() ), param( pad(x,t));
    
    //debug1<<"Field( "<<x<<" ) = ";
    int result = iv->FunctionValues( param.values(), y.values() );
    
    if( !result )
    {
        //debug1<<"NOT IN DOMAIN\n";
        throw Undefined();
    }
    //debug1<<y<<endl;
    
    // Print out cell info for debugging purposes....
    /*
    vtkIdType cellID = iv->GetLastCellId();
    vtkDataSet *ds = iv->GetLastDataSet();
    vtkCell *cell = ds->GetCell(iv->GetLastCellId());

    debug1<<"Cell: nPts= "<<cell->GetPointIds()->GetNumberOfIds()<<endl;
    vtkDataArray *vectors = ds->GetPointData()->GetVectors();
    int numPts = cell->GetPointIds()->GetNumberOfIds();
    for (int i=0; i < numPts; i++)
    {
        double vec[3], len;
        vectors->GetTuple(cell->PointIds->GetId(i), vec);
        len = sqrt(vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2]);
        debug1<<"    "<<i<<": ["<<vec[0]<<" "<<vec[1]<<" "<<vec[2]<<" ] len= "<<len<<endl;
    }
    debug1<<endl;
    */

    if ( normalized )
    {
        double len = y.length();
        if ( len > 0.0 )
            y /= len;
    }
    return y;
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
// ****************************************************************************

double
avtIVPVTKField::ComputeVorticity( const double& t, const avtVecRef& x ) const
{
    avtVec y( x.dim() );
    avtVec param = pad(x,t);
    
    int result = iv->FunctionValues( param.values(), y.values() );
    
    if( !result )
        throw Undefined();

    vtkDataSet *ds = iv->GetLastDataSet();
    vtkIdType cellID = iv->GetLastCellId();
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
    double pcoords[3], w[100];
    iv->GetLastWeights( w );
    iv->GetLastLocalCoordinates( pcoords );
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
//  Method: avtIVPVTKField::IsInsider
//
//  Purpose:
//      Determines if a point is inside a field.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

bool
avtIVPVTKField::IsInside( const double& t, const avtVecRef& x ) const
{
    avtVec y( x.dim() );
    return iv->FunctionValues( pad( x, t ).values(), y.values() );
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
// ****************************************************************************

unsigned int 
avtIVPVTKField::GetDimension() const
{
    return iv->GetNumberOfFunctions();
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
