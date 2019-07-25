// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtEvalPointExpression.h                       //
// ************************************************************************* //

#include <avtEvalPointExpression.h>

#include <vtkMatrix4x4.h>

#include <avtCustomTransform.h>
#include <avtSourceFromAVTDataset.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtEvalPointExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2006
//
// ****************************************************************************

avtEvalPointExpression::avtEvalPointExpression()
{
}


// ****************************************************************************
//  Method: avtEvalPointExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2006
//
// ****************************************************************************

avtEvalPointExpression::~avtEvalPointExpression()
{
}


// ****************************************************************************
//  Method: avtEvalPointExpression::TransformData
//
//  Purpose:
//      Transforms the data based on the input parameters.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2006
//
// ****************************************************************************

avtDataObject_p
avtEvalPointExpression::TransformData(avtDataObject_p input)
{
    //
    // Gameplan:  For each point (X,Y,Z), the vector to a point (X0, Y0, Z0)
    // is (X0-X, Y0-Y, Z0-Z).  So we want to move 2*(X0-X, Y0-Y, Z0-Z).
    // Then the final point is (2X0-X, 2Y0-Y, 2Z0-Z).  So set up a transform
    // that does this.
    //
    double X = inputParameters[0];
    double Y = inputParameters[1];
    double Z = inputParameters[2];

    vtkMatrix4x4 *mat = vtkMatrix4x4::New();
    mat->SetElement(0, 0, -1);
    mat->SetElement(1, 1, -1);
    mat->SetElement(2, 2, -1);
    mat->SetElement(0, 3, +2.*X);
    mat->SetElement(1, 3, +2.*Y);
    mat->SetElement(2, 3, +2.*Z);

    avtDataset_p ds;
    CopyTo(ds, input);
    avtSourceFromAVTDataset termsrc(ds);

    avtCustomTransform transform;
    transform.SetMatrix(mat);
    transform.SetInput(termsrc.GetOutput());

    avtDataObject_p output = transform.GetOutput();
    output->Update(GetGeneralContract());

    mat->Delete();

    return output;
}


