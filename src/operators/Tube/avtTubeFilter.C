// ************************************************************************* //
//  File: avtTubeFilter.C
// ************************************************************************* //

#include <avtTubeFilter.h>

#include <vtkCleanPolyData.h>
#include <vtkConnectedTubeFilter.h>
#include <vtkPolyData.h>
#include <vtkTubeFilter.h>

#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtTubeFilter constructor
//
//  Programmer: childs<generated>
//  Creation:   August28,2001
//
// ****************************************************************************

avtTubeFilter::avtTubeFilter()
{
}


// ****************************************************************************
//  Method: avtTubeFilter destructor
//
//  Programmer: childs<generated>
//  Creation:   August28,2001
//
//  Modifications:
//
// ****************************************************************************

avtTubeFilter::~avtTubeFilter()
{
}


// ****************************************************************************
//  Method:  avtTubeFilter::Create
//
//  Programmer:  childs<generated>
//  Creation:    August28,2001
//
// ****************************************************************************

avtFilter *
avtTubeFilter::Create()
{
    return new avtTubeFilter();
}


// ****************************************************************************
//  Method:      avtTubeFilter::SetAtts
//
//  Purpose:
//      Sets the state of the filter based on the attribute object.
//
//  Arguments:
//      a        The attributes to use.
//
//  Programmer:  childs<generated>
//  Creation:    August28,2001
//
// ****************************************************************************

void
avtTubeFilter::SetAtts(const AttributeGroup *a)
{
    atts = *(const TubeAttributes*)a;
}


// ****************************************************************************
//  Method: avtTubeFilter::Equivalent
//
//  Purpose:
//      Returns true if creating a new avtTubeFilter with the given
//      parameters would result in an equivalent avtTubeFilter.
//
//  Programmer: childs<generated>
//  Creation:   August28,2001
//
// ****************************************************************************

bool
avtTubeFilter::Equivalent(const AttributeGroup *a)
{
    return (atts == *(TubeAttributes*)a);
}


// ****************************************************************************
//  Method: avtTubeFilter::ExecuteData
//
//  Purpose:
//      Sends the specified input and output through the Tube filter.
//
//  Arguments:
//      in_ds      The input dataset.
//      <unused>   The domain number.
//      <unused>   The label.
//
//  Returns:       The output dataset.
//
//  Programmer: childs<generated>
//  Creation:   August28,2001
//
//  Modifications:
//
//    Hank Childs, Tue Sep 10 18:29:14 PDT 2002
//    Clean up leaks.
//
//    Hank Childs, Wed Oct 30 11:16:49 PST 2002
//    Added new options.  Also cleaned up poly data.
//
//    Jeremy Meredith, Fri Nov  1 19:07:40 PST 2002
//    Made it use the connected tube filter (our own creation) whenever
//    possible; it welds adjacent segments and creates good normals.
//
// ****************************************************************************

vtkDataSet *
avtTubeFilter::ExecuteData(vtkDataSet *in_ds, int, std::string)
{
    if (in_ds->GetDataObjectType() != VTK_POLY_DATA)
    {
        EXCEPTION0(ImproperUseException);
    }

    vtkTubeFilter          *vtktube = vtkTubeFilter::New();
    vtkCleanPolyData       *cpd     = vtkCleanPolyData::New();
    vtkConnectedTubeFilter *tube    = vtkConnectedTubeFilter::New();
    vtkDataSet *output;

    cpd->SetInput((vtkPolyData *)in_ds);
    tube->SetInput((vtkPolyData*)in_ds);
    if (tube->BuildConnectivityArrays())
    {
        tube->SetRadius(atts.GetWidth());
        tube->CreateNormalsOn();
        tube->SetNumberOfSides(atts.GetFineness());
        tube->SetCapping(atts.GetCapping() ? 1 : 0);
        output = tube->GetOutput();
    }
    else
    {
        // The vtk tube filter cries like a little baby if the points are not
        // exactly like it expects.  Use the vtkCleanPolyData filter.
        vtktube->SetInput(cpd->GetOutput());
        vtktube->SetVaryRadius(VTK_VARY_RADIUS_OFF);
        vtktube->SetRadius(atts.GetWidth());
        vtktube->SetUseDefaultNormal(1);
        vtktube->SetDefaultNormal(0.001, 0.001, 0.001);
        vtktube->SetNumberOfSides(atts.GetFineness());
        vtktube->SetCapping(atts.GetCapping() ? 1 : 0);
        output = vtktube->GetOutput();
    }

    output->Update();
    ManageMemory(output);
    vtktube->Delete();
    tube->Delete();
    cpd->Delete();

    return output;
}


// ****************************************************************************
//  Method: avtTubeFilter::RefashionDataObjectInfo
//
//  Purpose:
//      Changes attributes of output.
//
//  Programmer: Jeremy Meredith
//  Creation:   November  1, 2002
//
//  Modifications:
//
// ****************************************************************************

void
avtTubeFilter::RefashionDataObjectInfo(void)
{
    avtDataValidity   &outValidity = GetOutput()->GetInfo().GetValidity();
   
    outValidity.InvalidateZones();
    outValidity.SetNormalsAreInappropriate(true);
}
