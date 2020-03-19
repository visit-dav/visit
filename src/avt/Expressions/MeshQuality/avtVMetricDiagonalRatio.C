// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtVMetricDiagonalRatio.C                         //
// ************************************************************************* //

#include "avtVMetricDiagonalRatio.h"

#include <vtkCellType.h>

#include <verdict.h>

// ****************************************************************************
//  Method: avtVMetricDiagonalRatio::Metric
//
//  Purpose:
//      Inspect an element and calculate the diagonal ratio.
//
//  Arguments:
//      coords    The set of xyz points for the cell.
//      numPoints The number of xyz points for the cell.
//      type      The vtk type of the cell.
//
//  Returns: The diagonal ratio of the cell, or defaultValue if not
//           supported.
//
//  Programmer:   Akira Haddox
//  Creation:     June 13, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Oct 17 08:07:53 PDT 2002
//    Update for new verdict interface.
//
//    Sean Ahern, Wed Jun 24 15:17:29 EDT 2009
//    Renamed to be the diagonal ratio, which is what it actual returns.
//
// ****************************************************************************

double avtVMetricDiagonalRatio::Metric (double coords[][3], int type)
{
#ifdef HAVE_VERDICT 
    switch (type)
    {
        case VTK_HEXAHEDRON:
        case VTK_VOXEL:
            return v_hex_diagonal_ratio(8, coords);
    }
#endif
    return -1;
}
