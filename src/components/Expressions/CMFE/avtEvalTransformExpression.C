// ************************************************************************* //
//                          avtEvalTransformExpression.h                     //
// ************************************************************************* //

#include <avtEvalTransformExpression.h>

#include <vtkMatrix4x4.h>

#include <avtCustomTransform.h>
#include <avtSourceFromAVTDataset.h>


// ****************************************************************************
//  Method: avtEvalTransformExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   January 21, 2006
//
// ****************************************************************************

avtEvalTransformExpression::avtEvalTransformExpression()
{
}


// ****************************************************************************
//  Method: avtEvalTransformExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   January 21, 2006
//
// ****************************************************************************

avtEvalTransformExpression::~avtEvalTransformExpression()
{
}


// ****************************************************************************
//  Method: avtEvalTransformExpression::TransformData
//
//  Purpose:
//      Transforms the data based on the input parameters.
//
//  Programmer: Hank Childs
//  Creation:   January 21, 2006
//
// ****************************************************************************

avtDataObject_p
avtEvalTransformExpression::TransformData(avtDataObject_p input)
{
    vtkMatrix4x4 *mat = vtkMatrix4x4::New();
    mat->SetElement(0, 0, inputParameters[0]);
    mat->SetElement(0, 1, inputParameters[1]);
    mat->SetElement(0, 2, inputParameters[2]);
    mat->SetElement(1, 0, inputParameters[3]);
    mat->SetElement(1, 1, inputParameters[4]);
    mat->SetElement(1, 2, inputParameters[5]);
    mat->SetElement(2, 0, inputParameters[6]);
    mat->SetElement(2, 1, inputParameters[7]);
    mat->SetElement(2, 2, inputParameters[8]);

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


