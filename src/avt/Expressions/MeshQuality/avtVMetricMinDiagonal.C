// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtVMetricMinDiagonal.C                         //
// ************************************************************************* //

#include "avtVMetricMinDiagonal.h"

#include <vtkCellType.h>

#include <verdict.h>

// ****************************************************************************
//  Method: avtVMetricMinDiagonal::Metric
//
//  Purpose:
//      Inspect an element and calculate the min diagonal length.
//
//  Arguments:
//      coords    The set of xyz points for the cell.
//      numPoints The number of xyz points for the cell.
//      type      The vtk type of the cell.
//
//  Returns: The min diagonal length of the cell, or defaultValue if not
//           supported.
//
//  Programmer:   Sean Ahern
//  Creation:     June 24, 2009
//
//  Modifications:
//
// ****************************************************************************

double avtVMetricMinDiagonal::Metric (double coords[][3], int type)
{
#ifdef HAVE_VERDICT 
    switch (type)
    {
        case VTK_HEXAHEDRON:
        case VTK_VOXEL:
            return v_hex_min_diagonal(8, coords);
    }
#endif
    return -1;
}
