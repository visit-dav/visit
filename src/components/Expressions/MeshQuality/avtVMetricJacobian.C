// ************************************************************************* //
//                              avtVMetricJacobian.C                         //
// ************************************************************************* //

#include "avtVMetricJacobian.h"

#include <vtkDataSet.h>
#include <vtkFloatArray.h>

#include <verdict.h>

#include <DebugStream.h>

// ****************************************************************************
//  Method: avtVMetricJacobian::Metric
//
//  Purpose:
//      Inspect an element and calculate the jacobian.
//
//  Arguments:
//      coords    The set of xyz points for the cell.
//      numPoints The number of xyz points for the cell.
//      type      The vtk type of the cell.
//
//  Returns:      The jacobian of the cell, or defaultValue if not supported.
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

double avtVMetricJacobian::Metric (double coords[][3], int type)
{
#ifdef HAVE_VERDICT 
    switch (type)
    {
        case VTK_HEXAHEDRON:
        case VTK_VOXEL:
            return v_hex_jacobian(8, coords);
        
        case VTK_TETRA:
            return v_tet_jacobian(4,coords);

        case VTK_QUAD:
            return v_quad_jacobian(4, coords);
    }
#endif
    return 0;
}
