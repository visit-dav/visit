// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtCurvatureExpression.C                         //
// ************************************************************************* //

#include <avtCurvatureExpression.h>

#include <vtkCurvatures.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>

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
//  Modifications:
//    Jeremy Meredith, Thu Aug  7 14:33:06 EDT 2008
//    Removed unused var.
//
//    Kathleen Biagas, Wed Apr 4 11:41:11 PDT 2012 
//    Return the array created by vtkCurvatures, rather than converting it
//    to float. 
//
// ****************************************************************************

vtkDataArray *
avtCurvatureExpression::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    if (in_ds->GetDataObjectType() != VTK_POLY_DATA)
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                   "The curvature expression "
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
    curvatures->SetInputData(pd);
    curvatures->Update();

    vtkPolyData *out = curvatures->GetOutput();
    vtkDataArray *curvature = out->GetPointData()->GetArray(
                               doGauss ? "Gauss_Curvature" : "Mean_Curvature");
    curvature->Register(NULL);
    curvatures->Delete();

    // Calling function will clean up memory.
    return curvature;
}


