// ************************************************************************* //
//                              avtTruecolorFilter.C                              //
// ************************************************************************* //

#include <avtTruecolorFilter.h>
#include <vtkDataSet.h>
#include <vtkCellData.h>
#include <vtkUnsignedCharArray.h>



// ****************************************************************************
//  Method: avtTruecolorFilter constructor
//
//  Programmer: Chris Wojtan
//  Creation:   Monday, June 15 2004
//
// ****************************************************************************

avtTruecolorFilter::avtTruecolorFilter()
{
}


// ****************************************************************************
//  Method: avtTruecolorFilter destructor
//
//  Programmer: Chris Wojtan
//  Creation:   Monday, June 15 2004
//
// ****************************************************************************

avtTruecolorFilter::~avtTruecolorFilter()
{
}


// ****************************************************************************
//  Method: avtTruecolorFilter::ExecuteData
//
//  Purpose:
//      Does the actual VTK code to modify the dataset.
//
//  Arguments:
//      inDS      The input dataset.
//      <unused>  The domain number.
//      <unused>  The label.
//
//  Returns:      The output dataset.
//
//  Programmer: Chris Wojtan
//  Creation:   Monday, June 15 2004
//
// ****************************************************************************

vtkDataSet *
avtTruecolorFilter::ExecuteData(vtkDataSet *inDS, int, std::string)
{
    vtkDataSet *outDS = (vtkDataSet *) inDS->NewInstance();
    outDS->ShallowCopy(inDS);

    // convert RGB vectors into a RGB unsigned char array
    // and use these data as colors
    vtkDataArray *vecdata = inDS->GetCellData()->GetVectors();
    vtkUnsignedCharArray *color_array = vtkUnsignedCharArray::New();
    color_array->DeepCopy(vecdata);

    // Add scalar array to the output dataset and make it active
    color_array->SetName("ucharColors");
    outDS->GetCellData()->AddArray(color_array);
    outDS->GetCellData()->SetActiveScalars("ucharColors");

    color_array->Delete();

    return outDS;
}


// ****************************************************************************
//  Method: avtTruecolorFilter::RefashionDataObjectInfo
//
//  Purpose:
//      Allows the filter to change its output's data object information, which
//      is a description of the data object.
//
//  Programmer: Chris Wojtan
//  Creation:   Monday, June 15 2004
//
// ****************************************************************************

void
avtTruecolorFilter::RefashionDataObjectInfo(void)
{
}
