// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  avtMeshFilter.C
// ****************************************************************************

#include <avtMeshFilter.h>

#include <vtkAppendPolyData.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDataSetRemoveGhostCells.h>
#include <vtkGeometryFilter.h>
#include <vtkLinesFromOriginalCells.h>
#include <vtkPolyData.h>
#include <vtkRectilinearLinesNoDataFilter.h>
#include <vtkRectilinearGridFacelistFilter.h>

#include <ImproperUseException.h>
#include <DebugStream.h>

#include <string>

using std::string;

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
//      inDR       The input data representation.
//
//  Returns:       The output data representation.
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
//    Only set output topological dimension in UpdateDataObjectInfo.
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
//    Jeremy Meredith, Tue Oct 14 15:09:12 EDT 2008
//    Made various optimizations for regular grids.
//
//    Hank Childs, Thu Jan  8 11:29:28 CST 2009
//    Fix two problem sized memory leaks.
//
//    Eric Brugger, Tue Aug 19 10:55:44 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
//    Garrett Morrison, Wed Apr 25 17:03:10 PDT 2018
//    Added show internal check to draw opaque polys to fix issue if using vtk
//    ospray render passes
//
//    Kathleen Biagas, Thu Mar 14 14:41:12 PDT 2019
//    Remove Line cells from opaquePolys.
//
//    Kathleen Biagas, Wed Apr  3 16:02:16 PDT 2019
//    In place of vtkExtractEdges, use vtkLinesFromOriginalCells, as it has
//    been modified to add line and vertex primitives, and to have a flag
//    requesting use of OriginalCells, or not.
//
//    Kathleen Biagas, Wed Jun 10 13:08:13 PDT 2020
//    Check for 'mesh_points' label, which indicates data is all vertex cells.
//
// ****************************************************************************

avtDataTree_p
avtMeshFilter::ExecuteDataTree(avtDataRepresentation *inDR)
{
    //
    // Get the VTK data set, the domain number, and the label.
    //
    vtkDataSet *inDS = inDR->GetDataVTK();
    int domain = inDR->GetDomain();
    string label = inDR->GetLabel();

    avtDataAttributes &datts = GetInput()->GetInfo().GetAttributes();
    int topoDim = datts.GetTopologicalDimension();

    if (topoDim == 0)
    {
        label = string("mesh_points_") + label;
        return new avtDataTree(inDS, domain, label);
    }
    if (label.compare(0, 12, "mesh_points_") == 0)
    {
        return new avtDataTree(inDS, domain, label);
    }

    vtkLinesFromOriginalCells *lineFilter = vtkLinesFromOriginalCells::New();
    vtkGeometryFilter *geometryFilter = vtkGeometryFilter::New();
    vtkLinesFromOriginalCells *extractEdges = vtkLinesFromOriginalCells::New();
    vtkDataSetRemoveGhostCells *ghostFilter =vtkDataSetRemoveGhostCells::New();
    vtkRectilinearGridFacelistFilter *rectFacesFilter =
        vtkRectilinearGridFacelistFilter::New();

    vtkPolyData *opaquePolys = NULL;
    vtkPolyData *outDS = NULL;
    vtkRectilinearLinesNoDataFilter *rlines = NULL;

    vtkDataSet *revisedInput = NULL;
    vtkDataSet *revisedInput2 = NULL;
    vtkDataSet *revisedInput3 = NULL;

    avtDataValidity &v = GetInput()->GetInfo().GetValidity();
    if (!v.GetUsingAllData() &&
        inDS->GetCellData()->GetArray("avtGhostZones"))
    {
        ghostFilter->SetInputData(inDS);
        ghostFilter->Update();
        revisedInput = ghostFilter->GetOutput();
    }
    else
        revisedInput = inDS;

    //
    // Do not perform opaque if topological dimension of input == 0 or 1
    //
    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() > 1)
    {
        // Create a dataset that can be rendered as a solid surface, using
        // z buffer to shift so surface doesn't override lines of mesh.

        // Note: the vtkRectilinearGridFacelistFilter is able to consolidate
        // the output to one polygon per cube face; much better for
        // rendering
        if (revisedInput->GetDataObjectType() == VTK_RECTILINEAR_GRID)
        {
            rectFacesFilter->SetForceFaceConsolidation(true);
            rectFacesFilter->SetInputData(revisedInput);
            rectFacesFilter->Update();
            opaquePolys = rectFacesFilter->GetOutput();
        }
        else
        {
            geometryFilter->SetInputData(revisedInput);
            geometryFilter->Update();
            opaquePolys = geometryFilter->GetOutput();
        }
        // don't want line cells showing up in our rendering of the surfaces as opaque.
        opaquePolys->SetLines(NULL);
        opaquePolys->SetVerts(NULL);
    }

    if (atts.GetShowInternal() &&
        revisedInput->GetDataObjectType() != VTK_POLY_DATA)
    {
        extractEdges->UseOriginalCellsOff();
        extractEdges->SetInputData(revisedInput);
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
    //  is resolved.  (Note - JSM 2008/10/14 -- fixed for
    //  rectilinear data.)
    //
    if (revisedInput2->GetDataObjectType() != VTK_POLY_DATA &&
        revisedInput2->GetDataObjectType() != VTK_RECTILINEAR_GRID)
    {
        vtkGeometryFilter *geo = vtkGeometryFilter::New();
        geo->SetInputData(revisedInput2);
        geo->Update();
        revisedInput3 = geo->GetOutput()->NewInstance();
        revisedInput3->ShallowCopy(geo->GetOutput());
        geo->Delete();
    }
    else
    {
        revisedInput3 = revisedInput2;
        revisedInput3->Register(NULL);
    }

    if (revisedInput3->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        //
        // Yes, the rectilinear lines filter we're using here
        // can't handle things like: understanding that two neighboring
        // polygons were from the same original cell and not putting
        // a line between them.  However, the cases where we can get
        // into this state essentially imply that we have unstructured
        // or polygonal data, so we should be safe ignoring it.
        //
        rlines = vtkRectilinearLinesNoDataFilter::New();
        rlines->SetInputData(revisedInput3);
        rlines->Update();
        outDS = rlines->GetOutput();
    }
    else if (revisedInput3->GetDataObjectType() == VTK_POLY_DATA)
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
            lineFilter->UseOriginalCellsOn();
            lineFilter->SetInputData((vtkPolyData*)revisedInput3);
            lineFilter->Update();
            outDS = lineFilter->GetOutput();

            // this can occur due to use of 3DCellNums array by the lines
            // filter.  The Lines filter used to call vtkExtractEdges in
            // this case, but that filter loses disconnected lines and
            // vertices, so we will call the lines filter again and turn
            // off the use of OriginalCells.
            if (outDS->GetNumberOfLines() == 0)
            {
                lineFilter->UseOriginalCellsOff();
                lineFilter->SetInputData((vtkPolyData*)revisedInput3);
                lineFilter->Update();
                outDS = lineFilter->GetOutput();
            }
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
        if (opaquePolys != NULL && opaquePolys->GetNumberOfCells() != 0 &&
           (atts.GetOpaqueMode() == MeshAttributes::On || !atts.GetShowInternal()))
        {
            // create an avtDataTree with two ds, one for lines, one for polys,
            // and tack on labels with prefix lines_ and polys_
            // to keep them from being merged together by CompactTreeFilter
            vtkDataSet *outs[2];
            outs[0] = opaquePolys;
            outs[1] = outDS;
            stringVector l;
            l.push_back(string("mesh_polys_") + label);
            l.push_back(string("mesh_lines_") + label);
            rv = new avtDataTree(2, outs, domain, l);
        }
        else
        {
            rv = new avtDataTree(outDS, domain, string("mesh_lines_") + label);
        }
    }

    revisedInput3->Delete();
    lineFilter->Delete();
    geometryFilter->Delete();
    extractEdges->Delete();
    ghostFilter->Delete();
    rectFacesFilter->Delete();

    if(rlines)
        rlines->Delete();
    return rv;
}


// ****************************************************************************
//  Method: avtMeshFilter::UpdateDataObjectInfo
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
avtMeshFilter::UpdateDataObjectInfo(void)
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
//  Method: avtMeshFilter::ModifyContract
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
//    Kathleen Bonnell, Tue Jul 14 13:42:37 PDT 2009
//    Added test for MayRequireNodes for turning Node numbers on.
//
//    Hank Childs, Thu Aug 26 22:23:26 PDT 2010
//    Calculate the extents of the scaling variable.
//
// ****************************************************************************

avtContract_p
avtMeshFilter::ModifyContract(avtContract_p spec)
{
    avtContract_p rv = new avtContract(spec);

    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() != 0)
    {
        rv->GetDataRequest()->TurnZoneNumbersOn();
    }
    else
    {
        string pointVar = atts.GetPointAtts().GetPointSizeVar();
        avtDataRequest_p dataRequest = spec->GetDataRequest();

        //
        // Find out if we REALLY need to add the secondary variable.
        //
        if (atts.GetPointAtts().GetPointSizeVarEnabled() &&
            pointVar != "default" &&
            pointVar != "\0" &&
            pointVar != dataRequest->GetVariable() &&
            !dataRequest->HasSecondaryVariable(pointVar.c_str()))
        {
            rv->GetDataRequest()->AddSecondaryVariable(pointVar.c_str());
            rv->SetCalculateVariableExtents(pointVar, true);
        }

        if (spec->GetDataRequest()->MayRequireZones() ||
            spec->GetDataRequest()->MayRequireNodes())
        {
            keepNodeZone = true;
            rv->GetDataRequest()->TurnNodeNumbersOn();
        }
        else
        {
            keepNodeZone = false;
        }
    }

    return rv;
}


// ****************************************************************************
//  Method: avtMeshFilter::PostExcecute
//
//  Purpose:
//    Sets the output's label attributes to reflect what is currently
//    present in the tree.
//
//  Programmer: Kathleen Biagas
//  Creation:   May 11, 2016
//
//  Modifications:
//
// ****************************************************************************

void
avtMeshFilter::PostExecute(void)
{
    // Use labels to ensure lines/polys aren't merged back together
    // during CompactTreeFilter
    stringVector treeLabels;
    GetDataTree()->GetAllUniqueLabels(treeLabels);
    GetOutput()->GetInfo().GetAttributes().SetLabels(treeLabels);
}

