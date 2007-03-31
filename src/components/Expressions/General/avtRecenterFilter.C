// ************************************************************************* //
//                              avtRecenterFilter.C                          //
// ************************************************************************* //

#include <avtRecenterFilter.h>

#include <math.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>

#include <DebugStream.h>
#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtRecenterFilter::DeriveVariable
//
//  Purpose:
//      Assigns a recenter number to each point.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Hank Childs
//  Creation:     December 10, 2003
//
// ****************************************************************************

vtkDataArray *
avtRecenterFilter::DeriveVariable(vtkDataSet *in_ds)
{
    if (activeVariable == NULL)
    {
        EXCEPTION1(ExpressionException, "Asked to recenter, but did not "
                   "specify which variable to recenter");
    }

    vtkDataArray *cell_data = in_ds->GetCellData()->GetArray(activeVariable);
    vtkDataArray *pt_data   = in_ds->GetPointData()->GetArray(activeVariable);

    vtkDataArray *rv = NULL;

    if (cell_data != NULL)
    {
        rv = Recenter(in_ds, cell_data, AVT_ZONECENT);
    }
    else if (pt_data != NULL)
    {
        rv = Recenter(in_ds, pt_data, AVT_NODECENT);
    }
    else
    {
        EXCEPTION1(ExpressionException, "Was not able to locate variable to "
                   "recenter.");
    }

    return rv;
}


// ****************************************************************************
//  Method: avtRecenterFilter::IsPointVariable
//
//  Purpose:
//      Switch the centering of the point variable.
//
//  Programmer: Hank Childs
//  Creation:   December 10, 2003
//
// ****************************************************************************

bool
avtRecenterFilter::IsPointVariable(void)
{
    bool oldVar = avtSingleInputExpressionFilter::IsPointVariable();
    return (!oldVar);
}


