// ************************************************************************* //
//                            avtEvalPlaneExpression.h                       //
// ************************************************************************* //

#include <avtEvalPlaneExpression.h>

#include <vtkMatrix4x4.h>

#include <avtCustomTransform.h>
#include <avtSourceFromAVTDataset.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtEvalPlaneExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   January 21, 2006
//
// ****************************************************************************

avtEvalPlaneExpression::avtEvalPlaneExpression()
{
}


// ****************************************************************************
//  Method: avtEvalPlaneExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   January 21, 2006
//
// ****************************************************************************

avtEvalPlaneExpression::~avtEvalPlaneExpression()
{
}


// ****************************************************************************
//  Method: avtEvalPlaneExpression::TransformData
//
//  Purpose:
//      Transforms the data based on the input parameters.
//
//  Programmer: Hank Childs
//  Creation:   January 21, 2006
//
// ****************************************************************************

avtDataObject_p
avtEvalPlaneExpression::TransformData(avtDataObject_p input)
{
    //
    // Gameplan:  For each point (X,Y,Z), the distance to the plane is:
    // DIST = (A*X + B*Y + C*Z + D) / sqrt(A^2 + B^2 + C^2).
    // So the reflection would be a translation along (A,B,C) by a distance
    // of 2*DIST.  So set up a matrix that reflects this.
    //
    float A = inputParameters[0];
    float B = inputParameters[1];
    float C = inputParameters[2];

    //
    // Start off by normalizing the plane.
    //
    float mag = sqrt(A*A + B*B + C*C);
    if (mag == 0.)
    {
        EXCEPTION1(ExpressionException, "The plane has a degenerate normal.");
    }
    A /= mag;
    B /= mag;
    C /= mag;

    float Ox = inputParameters[3];
    float Oy = inputParameters[4];
    float Oz = inputParameters[5];
    float D = -(A*Ox + B*Oy + C*Oz);

    vtkMatrix4x4 *mat = vtkMatrix4x4::New();
    mat->SetElement(0, 0, -2*A*A + 1);
    mat->SetElement(0, 1, -2*A*B);
    mat->SetElement(0, 2, -2*A*C);
    mat->SetElement(0, 3, -2*A*D);
    mat->SetElement(1, 0, -2*B*A);
    mat->SetElement(1, 1, -2*B*B + 1);
    mat->SetElement(1, 2, -2*B*C);
    mat->SetElement(1, 3, -2*B*D);
    mat->SetElement(2, 0, -2*C*A);
    mat->SetElement(2, 1, -2*C*B);
    mat->SetElement(2, 2, -2*C*C + 1);
    mat->SetElement(2, 3, -2*C*D);

    avtDataset_p ds;
    CopyTo(ds, input);
    avtSourceFromAVTDataset termsrc(ds);

    avtCustomTransform transform;
    transform.SetMatrix(mat);
    transform.SetInput(termsrc.GetOutput());

    avtDataObject_p output = transform.GetOutput();
    output->Update(GetGeneralPipelineSpecification());

    mat->Delete();

    return output;
}


