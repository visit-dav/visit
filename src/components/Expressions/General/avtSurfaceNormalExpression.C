// ************************************************************************* //
//                       avtSurfaceNormalExpression.C                        //
// ************************************************************************* //

#include <avtSurfaceNormalExpression.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkVisItPolyDataNormals.h>

#include <ExpressionException.h>

// ****************************************************************************
//  Method: avtSurfaceNormalExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2005
//
// ****************************************************************************

avtSurfaceNormalExpression::avtSurfaceNormalExpression()
{
    isPoint = true;
}


// ****************************************************************************
//  Method: avtSurfaceNormalExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2005
//
// ****************************************************************************

avtSurfaceNormalExpression::~avtSurfaceNormalExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtSurfaceNormalExpression::DeriveVariable
//
//  Purpose:
//      Assigns the zone ID to each variable.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Hank Childs
//  Creation:     September 22, 2005
//
// ****************************************************************************

vtkDataArray *
avtSurfaceNormalExpression::DeriveVariable(vtkDataSet *in_ds)
{
    if (in_ds->GetDataObjectType() != VTK_POLY_DATA)
    {
        EXCEPTION1(ExpressionException, "The Surface normal expression "
                   "can only be calculated on surfaces.  Use the External"
                   "Surface operator to generate the external surface of "
                   "this object.  You must also use the DeferExpression "
                   "operator to defer the evaluation of this expression until "
                   "after the external surface operator.  The external surface"
                   " and defer expression operators are available through "
                   "the plugin manager located under the Options menu");
    }

    vtkPolyData *pd = (vtkPolyData *) in_ds;

    vtkVisItPolyDataNormals *n = vtkVisItPolyDataNormals::New();
    n->SetSplitting(false);
    if (isPoint)
        n->SetNormalTypeToPoint();
    else
        n->SetNormalTypeToCell();
    n->SetInput(pd);
    vtkPolyData *out = n->GetOutput();
    out->Update();

    vtkDataArray *arr = NULL;
    if (isPoint)
        arr = out->GetPointData()->GetNormals();
    else
        arr = out->GetCellData()->GetNormals();
    
    if (arr == NULL)
    {
        EXCEPTION1(ExpressionException, "An internal error occurred where "
                   "the surface normals could not be calculated.  Please "
                   "contact a VisIt developer.");
    }
    arr->Register(NULL);
    n->Delete();

    return arr;
}


