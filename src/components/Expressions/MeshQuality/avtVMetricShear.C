// ************************************************************************* //
//                              avtVMetricShear.C                            //
// ************************************************************************* //

#include "avtVMetricShear.h"

#include <vtkDataSet.h>
#include <vtkFloatArray.h>

#include <verdict.h>

#include <DebugStream.h>

// ****************************************************************************
//  Method: avtVMetricShear::Metric
//
//  Purpose:
//      Inspect an element and calculate the shear.
//
//  Arguments:
//      coords    The set of xyz points for the cell.
//      numPoints The number of xyz points for the cell.
//      type      The vtk type of the cell.
//
//  Returns:      The shear of the cell, or defaultValue if not supported.
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

double avtVMetricShear::Metric (double coords[][3], int type)
{
#ifdef HAVE_VERDICT 
    switch (type)
    {
        case VTK_HEXAHEDRON:
        case VTK_VOXEL:
            return v_hex_shear(8, coords);
        
        case VTK_TETRA:
            return v_tet_shear(4,coords);

        case VTK_QUAD:
            return v_quad_shear(4, coords);

        case VTK_TRIANGLE:
            return v_tri_shear(3, coords);    
    }
#endif
    return -1;
}
