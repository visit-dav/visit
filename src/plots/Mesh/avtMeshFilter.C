// ************************************************************************* //
//                              avtMeshFilter.C                              //
// ************************************************************************* //

#include <avtMeshFilter.h>

#include <vtkAppendPolyData.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDataSetRemoveGhostCells.h>
#include <vtkExtractEdges.h>
#include <vtkGeometryFilter.h>
#include <vtkLinesFromOriginalCells.h>
#include <vtkPolyData.h>

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
//  Modifications:
//    Kathleen Bonnell, Thu Feb  5 10:42:16 PST 2004 
//    Removed featureEdges, hasn't been used in a long time.  
//    Added extractEges. 
//
//    Kathleen Bonnell, Tue Nov  2 10:37:14 PST 2004 
//    Added keepNodeZone. 
//
//    Hank Childs, Thu Mar 10 09:13:03 PST 2005
//    Removed data member filters.
//
// ****************************************************************************

avtMeshFilter::avtMeshFilter(const MeshAttributes &a)
{
    atts = a;
    keepNodeZone = false; 
}


// ****************************************************************************
//  Method: avtMeshFilter destructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 8, 2001
//
//  Modifications:
//    Kathleen Bonnell, Thu Feb  5 10:42:16 PST 2004 
//    Removed featureEdges, hasn't been used in a long time.  
//    Added extractEges. 
//
//    Hank Childs, Thu Mar 10 09:13:03 PST 2005
//    Removed data member filters.
//
// ****************************************************************************

avtMeshFilter::~avtMeshFilter()
{
    ;
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
//    Kathleen Bonnell, Thu Feb  5 10:42:16 PST 2004 
//    Use extractEdges, if user wants to see internal zones and input
//    is not already polydata. 
//
//    Jeremy Meredith, Tue May  4 12:34:25 PDT 2004
//    Add support for unglyphed point meshes (i.e. topological dimension
//    is still 0 after going through the glyph filter).
//
//    Hank Childs, Fri Aug 27 15:24:09 PDT 2004
//    Renamed ghost data array.
//
//    Kathleen Bonnell, Tue Nov  2 10:37:14 PST 2004 
//    No need to process this data if topological dimension is 0 (point mesh). 
//
//    Kathleen Bonnell, Tue May 16 09:41:46 PDT 2006 
//    Removed call to SetSource(NULL), with new vtk pipeline, it also removes
//    necessary information from the dataset. 
//
// ****************************************************************************

avtDataTree_p
avtMeshFilter::ExecuteDataTree(vtkDataSet *inDS, int dom, string lab)
{
    avtDataAttributes &datts = GetInput()->GetInfo().GetAttributes();
    int topoDim = datts.GetTopologicalDimension();

    if (topoDim == 0) 
        return new avtDataTree(inDS, dom, lab);

    vtkLinesFromOriginalCells *lineFilter = vtkLinesFromOriginalCells::New();
    vtkGeometryFilter *geometryFilter = vtkGeometryFilter::New();
    vtkExtractEdges *extractEdges = vtkExtractEdges::New();
    vtkDataSetRemoveGhostCells *ghostFilter =vtkDataSetRemoveGhostCells::New();

    vtkPolyData *opaquePolys = NULL;
    vtkPolyData *outDS = NULL;

    vtkDataSet *revisedInput = NULL; 
    vtkDataSet *revisedInput2 = NULL; 
    vtkDataSet *revisedInput3 = NULL; 

    avtDataValidity &v = GetInput()->GetInfo().GetValidity();
    if (!v.GetUsingAllData() && 
        inDS->GetCellData()->GetArray("avtGhostZones"))
    {
        ghostFilter->SetInput(inDS);
        ghostFilter->Update();
        revisedInput = ghostFilter->GetOutput();
    }
    else
        revisedInput = inDS;

    if (atts.GetOutlineOnlyFlag())
    {
     // we need to do some decimation (?) here
     // use atts.GetErrorTolerance();
    }

    //
    // Do not perform opaque if topological dimension of input == 0 or 1 
    // 
    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() > 1) 
    {
        // Create a dataset that can be rendered as a solid surface, using
        // z buffer to shift so surface doesn't override lines of mesh.
        geometryFilter->SetInput(revisedInput);
        geometryFilter->Update();
        opaquePolys = geometryFilter->GetOutput();
    }

    if (atts.GetShowInternal() && 
        revisedInput->GetDataObjectType() != VTK_POLY_DATA)
    {
        extractEdges->SetInput(revisedInput);
        extractEdges->Update();
        revisedInput2 = extractEdges->GetOutput();
    }
    else
    {
        revisedInput2 = inDS;
    }

    //
    //  HACK, the correct way is to have a different filter
    //  than vtLinesFromOriginalCells to generate the mesh lines
    //  for Non polyData input.  Remove this hack until '6068
    //  is resolved.
    //
    if (revisedInput2->GetDataObjectType() != VTK_POLY_DATA)
    {
        vtkGeometryFilter *geo = vtkGeometryFilter::New();
        geo->SetInput(revisedInput2);
        geo->Update();
        revisedInput3 = geo->GetOutput()->NewInstance();
        revisedInput3->ShallowCopy(geo->GetOutput());
        revisedInput3->Register(NULL);
        geo->Delete();
    }
    else
    {
        revisedInput3 = revisedInput2;
        revisedInput3->Register(NULL);
    }

    if (revisedInput3->GetDataObjectType() == VTK_POLY_DATA) 
    {
        //
        // Make extra sure that we really have surfaces.
        //
        vtkPolyData *pd = (vtkPolyData *) revisedInput3;
        if (pd->GetPolys()->GetNumberOfCells() == 0 &&
            pd->GetStrips()->GetNumberOfCells() == 0 &&
            pd->GetVerts()->GetNumberOfCells() == 0)
        {
            topoDim = 1;
        }

        if (topoDim == 2)
        {
            lineFilter->SetInput((vtkPolyData*)revisedInput3);
            lineFilter->Update();
            outDS = lineFilter->GetOutput();
        }
        else
        {
            outDS = (vtkPolyData*)revisedInput3;
            debug5 << "MeshFilter not making a line mesh go through the line "
                   << "filter." << endl;
        }
    }

    if (outDS == NULL)
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
        if (opaquePolys != NULL && opaquePolys->GetNumberOfCells() != 0)
        {
            vtkAppendPolyData *append = vtkAppendPolyData::New();
            append->AddInput(outDS);
            append->AddInput(opaquePolys);
            append->Update();
            vtkPolyData *outPoly = vtkPolyData::New();
            outPoly->ShallowCopy(append->GetOutput());
            rv = new avtDataTree(outPoly, dom, lab);
            append->Delete();
        }
        else  
        {
            rv = new avtDataTree(outDS, dom, lab);
        }
    }

    revisedInput3->Delete();
    lineFilter->Delete();
    geometryFilter->Delete();
    extractEdges->Delete();
    ghostFilter->Delete();

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
//    Kathleen Bonnell, Tue Nov  2 10:41:33 PST 2004 
//    Set KeepNodeZoneArrays, and don't set TopologicalDimension to 1 if 
//    original topo dim was 0. 
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
    GetOutput()->GetInfo().GetAttributes().SetKeepNodeZoneArrays(keepNodeZone);
    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() != 0)
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
//  Modifications:
//    Kathleen Bonnell, Tue Nov  2 10:37:14 PST 2004
//    Handle point meshes differently.
//
//    Jeremy Meredith, Thu Aug 18 10:55:35 PDT 2005
//    Don't turn on zone numbers for point meshes.  It doesn't make sense, and
//    it can wind up doing strange things later (e.g. with pick -- see '6550).
//
// ****************************************************************************
 
avtPipelineSpecification_p
avtMeshFilter::PerformRestriction(avtPipelineSpecification_p spec)
{
    avtPipelineSpecification_p rv = new avtPipelineSpecification(spec);

    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() != 0)
    {
        rv->GetDataSpecification()->TurnZoneNumbersOn();
    }
    else
    {
        string pointVar = atts.GetPointSizeVar();
        avtDataSpecification_p dspec = spec->GetDataSpecification();

        //
        // Find out if we REALLY need to add the secondary variable.
        //
        if (atts.GetPointSizeVarEnabled() && 
            pointVar != "default" &&
            pointVar != "\0" &&
            pointVar != dspec->GetVariable() &&
            !dspec->HasSecondaryVariable(pointVar.c_str()))
        {
            rv->GetDataSpecification()->AddSecondaryVariable(pointVar.c_str());
        }

        avtDataAttributes &data = GetInput()->GetInfo().GetAttributes();
        if (spec->GetDataSpecification()->MayRequireZones())
        {
            keepNodeZone = true;
            rv->GetDataSpecification()->TurnNodeNumbersOn();
        }
        else
        {
            keepNodeZone = false;
        }
    }

    return rv;
}


