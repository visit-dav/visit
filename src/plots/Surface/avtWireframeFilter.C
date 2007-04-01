// ************************************************************************* //
//                            avtWireframeFilter.C                           //
// ************************************************************************* //

#include <avtWireframeFilter.h>

#include <vtkAppendPolyData.h>
#include <vtkDataSet.h>
#include <vtkGeometryFilter.h>
#include <vtkPolyData.h>
#include <vtkUniqueFeatureEdges.h>

#include <avtDatasetExaminer.h>
#include <avtExtents.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtWireframeFilter constructor
//
//  Arguments:
//      a       The attributed group with which to set the atts. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 24, 2004
//
//  Modifications:
//
// ****************************************************************************

avtWireframeFilter::avtWireframeFilter(const AttributeGroup *a)
{
    atts = *(SurfaceAttributes*)a;
    geoFilter    = vtkGeometryFilter::New();
    appendFilter = vtkAppendPolyData::New();
    edgesFilter  = vtkUniqueFeatureEdges::New();

    edgesFilter->ManifoldEdgesOn();
    edgesFilter->NonManifoldEdgesOff();
    edgesFilter->FeatureEdgesOff();
    edgesFilter->BoundaryEdgesOff();

    appendFilter->UserManagedInputsOn();
    appendFilter->SetNumberOfInputs(2);
}


// ****************************************************************************
//  Method: avtWireframeFilter destructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 24, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtWireframeFilter::~avtWireframeFilter()
{
    geoFilter->Delete();
    geoFilter = NULL;

    appendFilter->Delete();
    appendFilter = NULL;

    edgesFilter->Delete();
    edgesFilter = NULL;
}


// ****************************************************************************
//  Method:  avtWireframeFilter::Create
//
//  Purpose:
//    Call the constructor.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    May 24, 2004
//
//  Modifications:
//
// ****************************************************************************

avtFilter *
avtWireframeFilter::Create(const AttributeGroup *atts)
{
    return new avtWireframeFilter(atts);
}


// ****************************************************************************
//  Method: avtWireframeFilter::Equivalent
//
//  Purpose:
//      Returns true if creating a new avtWireframeFilter with the given
//      parameters would result in an equivalent avtWireframeFilter.
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 24, 2004 
//
//  Modifications:
//
// ****************************************************************************

bool
avtWireframeFilter::Equivalent(const AttributeGroup *a)
{
    return (atts == *(SurfaceAttributes*)a);
}


// ****************************************************************************
//  Method: avtWireframeFilter::ExecuteData
//
//  Purpose:
//      Maps this input 2d dataset to a 3d dataset by setting z coordinates
//      to a scaled version of the point/cell data.
//
//  Arguments:
//      inDS      The input dataset.
//      <unused>  The domain number.
//      <unused>  The label as a string.
//
//  Returns:      The output dataset. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 24, 2004 
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
avtWireframeFilter::ExecuteData(vtkDataSet *inDS, int, std::string)
{
    // xtract the edges for correct wireframe rendering.
    
    geoFilter->SetInput(inDS);
    edgesFilter->SetInput(geoFilter->GetOutput());

    appendFilter->SetInputByNumber(0, geoFilter->GetOutput());
    appendFilter->SetInputByNumber(1, edgesFilter->GetOutput());
 
    vtkPolyData *outPolys = vtkPolyData::New();
    appendFilter->SetOutput(outPolys);
    appendFilter->Update();

    return (vtkDataSet*) outPolys;
}


// ****************************************************************************
//  Method: avtWireframeFilter::ReleaseData
//
//  Purpose:
//      Releases all problem size data associated with this filter.
//
//  Programmer: Kathleen Bonnell
//  Creation:   May 24, 2004 
//
//  Modifications:
//
// ****************************************************************************

void
avtWireframeFilter::ReleaseData(void)
{
    avtStreamer::ReleaseData();

    geoFilter->SetInput(NULL);
    geoFilter->SetOutput(NULL);
    geoFilter->SetLocator(NULL);
    int nInputs = appendFilter->GetNumberOfInputs();
    for (int i = nInputs-1 ; i >= 0 ; i--)
    {
        appendFilter->SetInputByNumber(i, NULL);
    }
    appendFilter->SetOutput(NULL);
    edgesFilter->SetInput(NULL);
    edgesFilter->SetOutput(NULL);
    edgesFilter->SetLocator(NULL);
}


// ****************************************************************************
//  Method: avtWireframeFilter::PostExecute
//
//  Purpose:
//    Send accurate Spatial Extents to output. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   May 24, 2004 
//
//  Modifications:
//
// ****************************************************************************

void
avtWireframeFilter::PostExecute(void)
{
    avtDataAttributes& outAtts = GetOutput()->GetInfo().GetAttributes();
            
    // get the outputs's spatial extents
    double se[6];
    avtDataset_p output = GetTypedOutput();
    avtDatasetExaminer::GetSpatialExtents(output, se);

    // over-write spatial extents
    outAtts.GetTrueSpatialExtents()->Clear();
    outAtts.GetCumulativeTrueSpatialExtents()->Set(se);
}


