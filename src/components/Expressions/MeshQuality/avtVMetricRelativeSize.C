// ************************************************************************* //
//                              avtVMetricRelativeSize.C                     //
// ************************************************************************* //

#include "avtVMetricRelativeSize.h"

#include <vtkCellType.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>

#include <verdict.h>

#include <DebugStream.h>

// ****************************************************************************
//  Method: avtVMetricRelativeSize::Metric
//
//  Purpose:
//      Inspect an element and calculate the relative_size.
//
//  Arguments:
//      coords    The set of xyz points for the cell.
//      numPoints The number of xyz points for the cell.
//      type      The vtk type of the cell.
//
//  Returns:      The relative_size of the cell, or defaultValue if not supported.
//
//  Programmer:   Akira Haddox
//  Creation:     June 13, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Oct 17 08:07:53 PDT 2002
//    Update for new verdict interface.
//
// ****************************************************************************

double avtVMetricRelativeSize::Metric (double coords[][3], int type)
{
#ifdef HAVE_VERDICT 
    switch (type)
    {
        case VTK_HEXAHEDRON:
        case VTK_VOXEL:
            return v_hex_relative_size(8, coords);
        
        case VTK_TETRA:
            return v_tet_relative_size(4,coords);

        case VTK_QUAD:
            return v_quad_relative_size(4, coords);
            
        case VTK_TRIANGLE:
            return v_tri_relative_size(3, coords);
    }
#endif
    return -1;
}
