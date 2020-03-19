// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtVMetricMaxDiagonal.C                         //
// ************************************************************************* //

#include "avtVMetricMaxDiagonal.h"

#include <vtkCellType.h>

#include <verdict.h>

// ****************************************************************************
//  Method: avtVMetricMaxDiagonal::Metric
//
//  Purpose:
//      Inspect an element and calculate the max diagonal length.
//
//  Arguments:
//      coords    The set of xyz points for the cell.
//      numPoints The number of xyz points for the cell.
//      type      The vtk type of the cell.
//
//  Returns: The max diagonal length of the cell, or defaultValue if not
//           supported.
//
//  Programmer:   Sean Ahern
//  Creation:     June 24, 2009
//
//  Modifications:
//
// ****************************************************************************

double avtVMetricMaxDiagonal::Metric (double coords[][3], int type)
{
#ifdef HAVE_VERDICT 
    switch (type)
    {
        case VTK_HEXAHEDRON:
        case VTK_VOXEL:
            return v_hex_max_diagonal(8, coords);
    }
#endif
    return -1;
}
