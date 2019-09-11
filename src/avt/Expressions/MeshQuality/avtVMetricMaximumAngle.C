// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtVMetricMaximumAngle.C                         //
// ************************************************************************* //

#include "avtVMetricMaximumAngle.h"

#include <vtkCellType.h>

#include <verdict.h>

// ****************************************************************************
//  Method: avtVMetricMaximumAngle::Metric
//
//  Purpose:
//      Inspect an element and calculate the MaximumAngle.
//
//  Arguments:
//      coords    The set of xyz points for the cell.
//      numPoints The number of xyz points for the cell.
//      type      The vtk type of the cell.
//
//  Returns:      The MaximumAngle of the cell, or defaultValue if not supported.
//
//  Programmer:   Eric Brugger
//  Creation:     July 31, 2008
//
// ****************************************************************************

double avtVMetricMaximumAngle::Metric (double coords[][3], int type)
{
#ifdef HAVE_VERDICT 
    switch (type)
    {
        case VTK_TRIANGLE:
            return v_tri_maximum_angle(3, coords);
        
        case VTK_QUAD:
            return v_quad_maximum_angle(4, coords);
    }
#endif
    return -1;
}
