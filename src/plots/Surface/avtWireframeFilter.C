// ************************************************************************* //
//                            avtWireframeFilter.C                           //
// ************************************************************************* //

#include <avtWireframeFilter.h>

#include <vtkAppendPolyData.h>
#include <vtkDataSet.h>
#include <vtkFieldData.h>
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
//     Hank Childs, Thu Jul 29 17:24:40 PDT 2004
//     Reverse order of inputs to appender to get around VTK funniness.
//
//     Hank Childs, Wed Mar  9 15:53:09 PST 2005
//     Fix memory leak.
//
// ****************************************************************************

vtkDataSet *
avtWireframeFilter::ExecuteData(vtkDataSet *inDS, int, std::string)
{
    // xtract the edges for correct wireframe rendering.
    
    geoFilter->SetInput(inDS);

    //
    // If the input to the geometry filter is poly data, it does not pass
    // the field data through.  So copy that now.
    //
    geoFilter->Update();  // Update now so we can copy over the field data.
    if (inDS->GetDataObjectType() == VTK_POLY_DATA)
        geoFilter->GetOutput()->GetFieldData()
                                           ->ShallowCopy(inDS->GetFieldData());

    edgesFilter->SetInput(geoFilter->GetOutput());

    //
    // Lines must go before polys to avoid VTK bug with indexing cell data.
    //
    appendFilter->SetInputByNumber(0, edgesFilter->GetOutput());
    appendFilter->SetInputByNumber(1, geoFilter->GetOutput());
 
    vtkPolyData *outPolys = vtkPolyData::New();
    appendFilter->SetOutput(outPolys);
    outPolys->Delete();
    appendFilter->Update();

    return outPolys;
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
//    Hank Childs, Fri Mar  4 08:12:25 PST 2005
//    Do not set outputs of filters to NULL, since this will prevent them
//    from re-executing correctly in DLB-mode.
//
//    Hank Childs, Fri Mar 11 07:37:05 PST 2005
//    Fix non-problem size leak introduced with last fix.
//
//    Kathleen Bonnell, Fri May 13 15:39:41 PDT 2005 
//    Fix memory leak.
//
//    Kathleen Bonnell, Tue May 16 13:50:50 PDT 2006 
//    VTK api changes, GetNumberOfInputs now GetTotalNumerOfInputConnections.
//
// ****************************************************************************

void
avtWireframeFilter::ReleaseData(void)
{
    avtStreamer::ReleaseData();

    geoFilter->SetInput(NULL);
    geoFilter->SetLocator(NULL);
    vtkPolyData *p = vtkPolyData::New();
    geoFilter->SetOutput(p);
    p->Delete();

    int nInputs = appendFilter->GetTotalNumberOfInputConnections();
    for (int i = nInputs-1 ; i >= 0 ; i--)
    {
        appendFilter->SetInputByNumber(i, NULL);
    }
    p = vtkPolyData::New();
    appendFilter->SetOutput(p);
    p->Delete();

    edgesFilter->SetInput(NULL);
    edgesFilter->SetLocator(NULL);
    p = vtkPolyData::New();
    edgesFilter->SetOutput(p);
    p->Delete();
}


