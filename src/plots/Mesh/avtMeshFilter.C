// ************************************************************************* //
//                              avtMeshFilter.C                              //
// ************************************************************************* //

#include <avtMeshFilter.h>

#include <vtkGeometryFilter.h>
#include <vtkLinesFromOriginalCells.h>
#include <vtkUniqueFeatureEdges.h>
#include <vtkPolyData.h>
#include <vtkExtractEdges.h>
#include <vtkDataSetRemoveGhostCells.h>
#include <vtkAppendPolyData.h>

#include <ImproperUseException.h>
#include <DebugStream.h>


// ****************************************************************************
//  Method: avtMeshFilter constructor
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 8, 2001
//
// ****************************************************************************

avtMeshFilter::avtMeshFilter(const MeshAttributes &a)
{
    atts = a;
    lineFilter     = vtkLinesFromOriginalCells::New();
    geometryFilter = vtkGeometryFilter::New();
    featureEdges   = vtkUniqueFeatureEdges::New();
    featureEdges->FeatureEdgesOff();
    featureEdges->ManifoldEdgesOn();
    featureEdges->NonManifoldEdgesOff();
    featureEdges->BoundaryEdgesOff();
}


// ****************************************************************************
//  Method: avtMeshFilter destructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 8, 2001
//
// ****************************************************************************

avtMeshFilter::~avtMeshFilter()
{
    if (lineFilter != NULL)
    {
        lineFilter->Delete();
        lineFilter = NULL;
    }
    if (geometryFilter != NULL)
    {
        geometryFilter->Delete();
        geometryFilter = NULL;
    }
    if (featureEdges != NULL)
    {
        featureEdges->Delete();
        featureEdges = NULL;
    }
}


// ****************************************************************************
//  Method: avtMeshFilter::ExecuteData
//
//  Purpose:
//      Sends the specified input and output through the mesh filter.
//
//  Arguments:
//      inDS       The input dataset.
//      dom        The domain number.
//      lab        The label.
//
//  Returns:       The output dataset.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 8, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Jun 29 10:27:44 PDT 2001
//    Added code to include a surface dataset for opaque mode.
// 
//    Kathleen Bonnell, Mon Aug 20 17:25:32 PDT 2001 
//    Added a ghostZone filter for the case when a subset selection
//    has taken place (and ghostzones are present).  This allows
//    the boundary faces between blocks to still have mesh lines drawn
//    on them when block selection takes place. Also added logic
//    to handle the case where the output dataset(s) have no cells.
// 
//    Kathleen Bonnell, Wed Aug 22 15:22:55 PDT 2001 
//    Instead of returning separate datasets for opaque mode and meshonly
//    mode, append the two into one. Save this filter from requiring 
//    re-execution when opaquemode is the only attribute that changes,
//    by always creating the opaquemode polys if current topo dim is 3. 
//    
//    Hank Childs, Wed Aug 22 15:29:44 PDT 2001
//    If we have a line mesh, do not put them through a line filter.
//
//    Hank Childs, Tue Sep  4 16:14:49 PDT 2001
//    Reflect new interface for avtDataAttributes.
//
//    Hank Childs, Fri Sep  7 10:41:38 PDT 2001
//    Do not set the topological dimension to 2 if it is actually one.
//
//    Kathleen Bonnell, Mon Sep 10 16:21:59 PDT 2001 
//    Enable opaque mode for 2d. 
//
//    Kathleen Bonnell, Wed Sep 19 12:55:57 PDT 2001 
//    Added string argument so that labels will be passed to output. 
//
//    Hank Childs, Fri Apr 12 17:32:07 PDT 2002
//    Fix memory leaks.
//
//    Kathleen Bonnell, Thu May 23 10:50:42 PDT 2002 
//    Only set output topological dimension in RefashionDataObjectInfo. 
//
//    Jeremy Meredith, Tue Jul  9 15:44:14 PDT 2002
//    Made it throw an exception if it does not get polydata.  The Mesh
//    Plot should apply a facelist filter before this one if it is needed.
//
//    Hank Childs, Thu Jul 18 18:07:34 PDT 2002
//    Make sure that the topological dimension of the mesh is really as
//    advertised by seeing if it really has triangle data.
//
// ****************************************************************************

avtDataTree_p
avtMeshFilter::ExecuteDataTree(vtkDataSet *inDS, int dom, string lab)
{
    vtkPolyData *outDS = vtkPolyData::New();
    vtkPolyData *opaquePolys = vtkPolyData::New();

    vtkDataSet *revisedInput = NULL; 

    avtDataValidity &v = GetInput()->GetInfo().GetValidity();
    if (!v.GetUsingAllData() && 
        inDS->GetCellData()->GetArray("vtkGhostLevels"))
    {
        vtkDataSetRemoveGhostCells *ghostFilter = 
             vtkDataSetRemoveGhostCells::New();
        ghostFilter->SetInput(inDS);
        ghostFilter->SetGhostLevel(1);
        ghostFilter->Update();
        revisedInput = ghostFilter->GetOutput();
        revisedInput->Register(NULL);
        ghostFilter->Delete();
    }
    else
    {
        revisedInput = inDS;
        revisedInput->Register(NULL);
    }

    if (atts.GetOutlineOnlyFlag())
    {
     // we need to do some decimation (?) here
     // use atts.GetErrorTolerance();
    }

    //
    // Do not perform opaque if topological dimension of input ==  1 
    // 
    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() != 1) 
    {
        // Create a dataset that can be rendered as a solid surface, using
        // z buffer to shift so surface doesn't override lines of mesh.
        geometryFilter->SetInput(revisedInput);
        geometryFilter->SetOutput(opaquePolys);
        geometryFilter->Update();
    }

    if (revisedInput->GetDataObjectType() == VTK_POLY_DATA)
    {
        avtDataAttributes &datts = GetInput()->GetInfo().GetAttributes();
        int topoDim = datts.GetTopologicalDimension();

        //
        // Make extra sure that we really have surfaces.
        //
        vtkPolyData *pd = (vtkPolyData *) revisedInput;
        if (pd->GetPolys()->GetNumberOfCells() == 0 &&
            pd->GetStrips()->GetNumberOfCells() == 0)
        {
            topoDim = 1;
        }
 
        if (topoDim == 2)
        {
            lineFilter->SetInput((vtkPolyData*)revisedInput);
            lineFilter->SetOutput(outDS);
            lineFilter->Update();
        }
        else
        {
            outDS->Delete();
            outDS = (vtkPolyData*)revisedInput;
            outDS->Register(NULL); // We will remove this later.
            debug5 << "MeshFilter not making a line mesh go through the line "
                   << "filter." << endl;
        }
    }
    else
    {
        EXCEPTION1(ImproperUseException,
                   "avtMeshFilter expects PolyData but didn't get it -- "
                   "was the facelist filter not applied by the avtMeshPlot?");
                   
    }

    //
    //  Don't want this information tagging along any more.
    //
    outDS->GetCellData()->RemoveArray("avtOriginalCellNumbers");

    avtDataTree_p rv = NULL;
    if (outDS->GetNumberOfCells() == 0)
    {
        //
        // We have an empty mesh.
        //
    }
    else
    {
        //
        //  Tack on the opaque poly's to the outDS (which is only lines
        //  at this point. 
        //
        if (opaquePolys->GetNumberOfCells() != 0)
        {
            vtkAppendPolyData *append = vtkAppendPolyData::New();
            vtkPolyData *outPoly = vtkPolyData::New();
            append->AddInput(outDS);
            append->AddInput(opaquePolys);
            append->SetOutput(outPoly);
            append->Update();
            rv = new avtDataTree(outPoly, dom, lab);
            append->Delete();
            outPoly->Delete();
        }
        else  
        {
            rv = new avtDataTree(outDS, dom, lab);
        }
    }
    outDS->Delete();
    opaquePolys->Delete();
    revisedInput->Delete();
    return rv;
}


// ****************************************************************************
//  Method: avtMeshFilter::RefashionDataObjectInfo
//
//  Purpose:
//    Indicates that topological dimension of the output is 1.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 8, 2001 
//
//  Modifications:
//
//    Hank Childs, Thu Jul 26 20:14:29 PDT 2001
//    Smarter logic for output dimension.
//
//    Hank Childs, Tue Sep  4 16:14:49 PDT 2001
//    Reflect new interface for avtDataAttributes.
//
//    Kathleen Bonnell, Thu May 23 10:50:42 PDT 2002 
//    Set output topological dimension to 1, to allow plot to be shifted
//    towards camera. 
//
//    Hank Childs, Wed Oct  9 16:13:32 PDT 2002
//    Do not calculate normals after the mesh plot.
//
// ****************************************************************************

void
avtMeshFilter::RefashionDataObjectInfo(void)
{
    //
    // Technically, the topological dimension should be 2 if drawing
    // in opaque mode, and 1 otherwise.  However, setting the
    // topological dimension to 2 will not allow the mesh to be
    // shifted towards the camera (in ShiftPlot operation),
    // and thus other plots will possibly obscure the mesh lines, making
    // them appear less-than solid.
    //
    GetOutput()->GetInfo().GetAttributes().SetTopologicalDimension(1);
    GetOutput()->GetInfo().GetValidity().InvalidateZones();
    GetOutput()->GetInfo().GetValidity().SetNormalsAreInappropriate(true);
}


// ****************************************************************************
//  Method: avtMeshFilter::PerformRestriction
//
//  Purpose:
//    Turn on Zone numbers flag so that the database does not make
//    any assumptions. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 25, 2002
//
// ****************************************************************************
 
avtPipelineSpecification_p
avtMeshFilter::PerformRestriction(avtPipelineSpecification_p spec)
{
    avtPipelineSpecification_p rv = new avtPipelineSpecification(spec);
    rv->GetDataSpecification()->TurnZoneNumbersOn();
    return rv;
}


// ****************************************************************************
//  Method: avtMeshFilter::ReleaseData
//
//  Purpose:
//      Release the problem sized data associated with this filter.
//
//  Programmer: Hank Childs
//  Creation:   September 10, 2002
//
// ****************************************************************************

void
avtMeshFilter::ReleaseData(void)
{
    avtDataTreeStreamer::ReleaseData();

    lineFilter->SetInput(NULL);
    lineFilter->SetOutput(NULL);
    lineFilter->SetLocator(NULL);
    geometryFilter->SetInput(NULL);
    geometryFilter->SetOutput(NULL);
    featureEdges->SetInput(NULL);
    featureEdges->SetOutput(NULL);
}


