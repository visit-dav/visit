// ************************************************************************* //
//                          avtCurvatureExpression.C                         //
// ************************************************************************* //

#include <avtCurvatureExpression.h>

#include <vtkCellData.h>
#include <vtkCurvatures.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>

#include <ExpressionException.h>

// ****************************************************************************
//  Method: avtCurvatureExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   May 11, 2006
//
// ****************************************************************************

avtCurvatureExpression::avtCurvatureExpression()
{
    doGauss = false;
}


// ****************************************************************************
//  Method: avtCurvatureExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   May 11, 2006
//
// ****************************************************************************

avtCurvatureExpression::~avtCurvatureExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtCurvatureExpression::DeriveVariable
//
//  Purpose:
//      Calculates the localized compactness at every point.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Hank Childs
//  Creation:     May 11, 2006
//
// ****************************************************************************

vtkDataArray *
avtCurvatureExpression::DeriveVariable(vtkDataSet *in_ds)
{
    if (in_ds->GetDataObjectType() != VTK_POLY_DATA)
    {
        EXCEPTION1(ExpressionException, "The curvature expression "
                   "can only be calculated on surfaces.  VisIt tries to "
                   "evaluate expressions as soon as they are read from a "
                   "database.  For curvature, the expression is typically "
                   "desired after some operators have been applied.  You "
                   "can defer the evaluation of the curvature expression "
                   "using the DeferExpression operator."
                   " The defer expression operator is available through "
                   "the plugin manager located under the Options menu");
    }

    vtkPolyData *pd = (vtkPolyData *) in_ds;

    vtkCurvatures *curvatures = vtkCurvatures::New();
    curvatures->SetCurvatureType((doGauss ? VTK_CURVATURE_GAUSS 
                                          : VTK_CURVATURE_MEAN));
    curvatures->SetInput(pd);
    curvatures->Update();

    vtkPolyData *out = curvatures->GetOutput();
    vtkDataArray *curvature = out->GetPointData()->GetArray(
                               doGauss ? "Gauss_Curvature" : "Mean_Curvature");
    bool shouldDelete = false;

    vtkFloatArray *flt_curvature = vtkFloatArray::New();
    int npts = pd->GetNumberOfPoints();
    flt_curvature->SetNumberOfTuples(npts);
    if (curvature == NULL)
    {
        for (int i = 0 ; i < npts ; i++)
            flt_curvature->SetTuple1(i, 0.);
    }
    else
    {
        for (int i = 0 ; i < npts ; i++)
            flt_curvature->SetTuple1(i, curvature->GetTuple1(i));
    }

    curvatures->Delete();

    // Calling function will clean up memory.
    return flt_curvature;
}


