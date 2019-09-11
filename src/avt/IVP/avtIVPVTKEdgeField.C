// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
