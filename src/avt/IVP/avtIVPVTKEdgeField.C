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
//                             avtIVPVTKEdgeField.C                              //
// ************************************************************************* //

#include <avtIVPVTKField.h>
#include <avtIVPVTKEdgeField.h>
#include <ImproperUseException.h>
#include <DebugStream.h>

#include <iostream>
#include <limits>
#include <cmath>
#include <sstream>

#include <vtkUnsignedCharArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkGenericCell.h>
#include <vtkCell.h>

// ****************************************************************************
//  Method: avtIVPVTKEdgeField constructor
//
//  Programmer: Alexander Pletzer
//  Creation:   Sun Dec  1 19:32:05 MST 2013
//
//  Modifications:
//
// ****************************************************************************

avtIVPVTKEdgeField::avtIVPVTKEdgeField( vtkDataSet* dataset, 
                                        avtCellLocator* locator) 
: avtIVPVTKField(dataset, locator)
{
    // Make cell locator for rectilinear edge vector fields
    this->locRE = dynamic_cast<avtCellLocatorRectEdge*>(locator);
}

// ****************************************************************************
//  Method: avtIVPVTKEdgeField destructor
//
//  Programmer: Alexander Pletzer
//  Creation:   Sun Dec  1 19:32:05 MST 2013
//
//  Modifications:
//
// ****************************************************************************

avtIVPVTKEdgeField::~avtIVPVTKEdgeField()
{
}

// ****************************************************************************
//  Method: avtIVPVTKField::operator
//
//  Purpose:
//      Evaluates a point location by consulting a VTK grid.
//
//  Programmer: Alexander Pletzer
//  Creation:   Sun Dec  1 19:32:05 MST 2013
//
//  Modifications:
//
// ****************************************************************************

avtIVPField::Result
avtIVPVTKEdgeField::operator()( const double &t, const avtVector &pt, avtVector &retV ) const
{
    avtVector result = operator()(t, pt);
    retV.x = result.x;
    retV.y = result.y;
    retV.z = result.z;
    
    return OK;
}

avtVector
avtIVPVTKEdgeField::operator()( const double &t, const avtVector &p ) const
{
    avtVector zeros(0, 0, 0);
    avtVector vel(0, 0, 0);
    if ( FindValue( velData, vel, t, p ) != OK ) 
    {
        return zeros;
    }
    return vel;
}
// ****************************************************************************
//  Method: avtIVPVTKEdgeField::FindValue
//
//  Purpose:
//      Evaluates a point after consulting a VTK grid.
//
//  Programmer: Alexander Pletzer
//  Creation:   Sun Dec  1 19:32:05 MST 2013
//
// ****************************************************************************

bool
avtIVPVTKEdgeField::FindValue(vtkDataArray *vectorData, avtVector &vel, 
                              double t, const avtVector &p) const
{

    vel.x = vel.y = vel.z = 0.0;

    // nodal field components, may contain some offset
    for ( size_t j = 0; j < 3; ++j )
    {
        
        // reset the direction of the vector field
        this->locRE->SetDirection(j);

        // re-compute the weights for each direction of the vector field
        this->lastPos = p;
        this->lastCell = this->locRE->FindCell(&p.x, &this->lastWeights, false);
        if ( this->lastCell == -1 ) 
        {
            return OUTSIDE_SPATIAL;
        }

        // interpolate
        double tmp[3];
            
        for( avtInterpolationWeights::const_iterator wi=lastWeights.begin();
             wi != lastWeights.end(); ++wi )
        {
            vectorData->GetTuple( wi->i, tmp );
            vel[j] += wi->w * tmp[j];
        }
    }

    return OK;
}
