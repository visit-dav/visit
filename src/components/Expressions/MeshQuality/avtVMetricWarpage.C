// ************************************************************************* //
//                              avtVMetricWarpage.C                          //
// ************************************************************************* //

#include "avtVMetricWarpage.h"

#include <vtkCellType.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>

#include <verdict.h>

#include <DebugStream.h>

// ****************************************************************************
//  Method: avtVMetricWarpage::Metric
//
//  Purpose:
//      Inspect an element and calculate the Warpage.
//
//  Arguments:
//      coords    The set of xyz points for the cell.
//      numPoints The number of xyz points for the cell.
//      type      The vtk type of the cell.
//
//  Returns:      The Warpage of the cell, or defaultValue if not supported.
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

double avtVMetricWarpage::Metric (double coords[][3], int type)
{
#ifdef HAVE_VERDICT 
    if (type == VTK_QUAD)
        return v_quad_warpage(4, coords);
#endif
    return -1;
}
