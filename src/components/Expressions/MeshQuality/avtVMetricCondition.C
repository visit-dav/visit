// ************************************************************************* //
//                              avtVMetricCondition.C                        //
// ************************************************************************* //

#include "avtVMetricCondition.h"

#include <vtkDataSet.h>
#include <vtkFloatArray.h>

#include <verdict.h>

#include <DebugStream.h>

// ****************************************************************************
//  Method: avtVMetricCondition::Metric
//
//  Purpose:
//      Inspect an element and calculate the condition.
//     
//
//  Arguments:
//      coords    The set of xyz points for the cell.
//      numPoints The number of xyz points for the cell.
//      type      The vtk type of the cell.
//
//  Returns:      The condition of the cell, or defaultValue if not supported.
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

double avtVMetricCondition::Metric (double coords[][3], int type)
{
#ifdef HAVE_VERDICT
    switch (type)
    {
        case VTK_HEXAHEDRON:
        case VTK_VOXEL:
            return v_hex_condition(8, coords);
        
        case VTK_TETRA:
            return v_tet_condition(4,coords);

        case VTK_QUAD:
            return v_quad_condition(4, coords);

        case VTK_TRIANGLE:
            return v_tri_condition(3, coords);
    }
#endif
    return 0;
}
