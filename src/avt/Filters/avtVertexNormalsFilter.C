// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtVertexNormalsFilter.C                         //
// ************************************************************************* //

#include <avtVertexNormalsFilter.h>

#include <vtkPolyData.h>
#include <vtkStructuredGrid.h>
#include <vtkVisItPolyDataNormals.h>
#include <vtkVisItStructuredGridNormals.h>

#include <avtDataset.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtVertexNormalsFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
//  Modifications:
//
//    Hank Childs, Thu Feb 26 09:55:59 PST 2004
//    Removed pointNormals variable.  Whether or not to do point normals is
//    now decided dynamically.
//
//    Alister Maguire, Mon Apr 27 11:41:06 PDT 2020
//    Added initialization of zonesHaveBeenSplit.
//
// ****************************************************************************

avtVertexNormalsFilter::avtVertexNormalsFilter()
{
    zonesHaveBeenSplit = false;
}


// ****************************************************************************
//  Method: avtVertexNormalsFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtVertexNormalsFilter::~avtVertexNormalsFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtVertexNormalsFilter::ExecuteData
//
//  Purpose:
//      Sends the specified input and output through the VertexNormals filter.
//
//  Arguments:
//      in_dr      The input data representation.
//
//  Returns:       The output data representation.
//
//  Programmer: Hank Childs
//  Creation:   December 31, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Jun 20 13:59:01 PDT 2002
//    Be more careful about what sort of poly data gets sent in (no lines).
//
//    Hank Childs, Tue Aug  6 11:11:50 PDT 2002
//    Do not execute if normals have been deemed inappropriate.
//
//    Hank Childs, Sun Aug 18 11:14:20 PDT 2002
//    Do not find normals for disjoint element meshes.
//
//    Hank Childs, Tue Sep 10 15:17:21 PDT 2002
//    Manage the output's memory without the use of data members.
//
//    Jeremy Meredith, Thu Oct 24 17:40:18 PDT 2002
//    Upped the feature angle to 45 degrees.
//
//    Jeremy Meredith, Mon Jan  6 10:44:59 PST 2003
//    Disabled consistency checking because (1) it has bugs, (2) it is slow,
//    and (3) we are careful enough about cell orderings that we don't need it.
//
//    Jeremy Meredith, Wed Aug 13 18:13:14 PDT 2003
//    Made it use the new VisIt poly data normals filter.  Allowed
//    cell normals as well as just point normals.
//
//    Hank Childs, Thu Feb 26 09:50:37 PST 2004
//    Decide what type of normals (point, cell) to do from inside this filter.
//
//    Jeremy Meredith, Wed Oct 27 15:22:32 PDT 2004
//    Removed check for ensuring that polgons actually existed before doing
//    the normals.  The vtkVisItPolyDataNormals filter accepts them just fine,
//    and we can count on the spatial/topological tests to rule out cases
//    where it will be inefficient to compute normals anyway.  There were
//    cases where some domains had polgons, and others only had lines, but
//    the final append filter removed *all* normals because some domains
//    did not have any.
//
//    Hank Childs, Sat Feb 19 14:58:42 PST 2005
//    Break all memory references.  I didn't think this was necessary, but
//    the data wouldn't delete until I did this.
//
//    Kathleen Bonnell, Tue May 16 09:41:46 PDT 2006
//    Removed call to SetSource(NULL), with new vtk pipeline, it also removes
//    necessary information from the dataset.
//
//    Hank Childs, Thu Dec 28 15:25:50 PST 2006
//    Add support for direct normals calculation of structured grids.
//
//    Eric Brugger, Tue Jul 22 12:19:31 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
//    Kathleen Biagas, Tue Jul 19:16:15:22 MST 2016
//    VTK-8 Port: remove structured-grid specific normals calculation, it
//    fails miserably with VTK-8, though the calculated normals are the same
//    as pre-vtk-8.
//
//    Alister Maguire, Wed Jun 20 10:33:31 PDT 2018
//    Re-integrated structured-grid specific normals calculation after
//    fixing bug regarding VTK-8.
//
//    Kathleen Biagas, Wed Jul 31 11:21:38 PDT 2019
//    Since vtkVisItPolyDataNormals filter doesn't handle triangle strips,
//    and in fact removes them from the output, don't process the input if
//    they are present.
//
//    Alister Maguire, Wed Mar 18 15:16:33 PDT 2020
//    Updated vtkVisItPolyDataNormals filter to handle triangle strips and
//    added set for zonesHaveBeenSplit when appropriate.
//
//    Kathleen Biagas, Thu Jun  11 15:00:10 PDT 2020
//    Don't process poly data containing only lines.
//
// ****************************************************************************

avtDataRepresentation *
avtVertexNormalsFilter::ExecuteData(avtDataRepresentation *in_dr)
{
    //
    // Get the VTK data set.
    //
    vtkDataSet *in_ds = in_dr->GetDataVTK();

    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();

    if (atts.GetSpatialDimension() != 3 || atts.GetTopologicalDimension() != 2)
    {
        return in_dr;
    }

    if (GetInput()->GetInfo().GetValidity().NormalsAreInappropriate())
    {
        return in_dr;
    }

    if (GetInput()->GetInfo().GetValidity().GetDisjointElements() == true)
    {
        return in_dr;
    }

    if (in_ds->GetDataObjectType() == VTK_POLY_DATA)
    {
        vtkPolyData *pd = (vtkPolyData *)in_ds;
        if (pd->GetNumberOfLines() == pd->GetNumberOfCells())
        {
            // don't really want normals for lines.
            return in_dr;
        }
        bool pointNormals = true;
        if (atts.ValidActiveVariable())
        {
            avtCentering cent = atts.GetCentering();
            if (cent == AVT_ZONECENT)
                pointNormals = false;
        }
        vtkVisItPolyDataNormals *normals = vtkVisItPolyDataNormals::New();
        normals->SetInputData(pd);
        normals->SetFeatureAngle(45.);
        if (pointNormals)
            normals->SetNormalTypeToPoint();
        else
            normals->SetNormalTypeToCell();
        normals->SetSplitting(true);
        normals->Update();

        vtkPolyData *out_ds = normals->GetOutput();

        avtDataRepresentation *out_dr = new avtDataRepresentation(out_ds,
            in_dr->GetDomain(), in_dr->GetLabel());

        if (normals->GetStripsHaveBeenDecomposed())
        {
            zonesHaveBeenSplit = true;
        }

        normals->Delete();

        return out_dr;
    }
    else if (in_ds->GetDataObjectType() == VTK_STRUCTURED_GRID)
    {
        vtkStructuredGrid *sgrid = (vtkStructuredGrid *)in_ds;

        bool pointNormals = true;
        if (atts.ValidActiveVariable())
        {
            avtCentering cent = atts.GetCentering();
            if (cent == AVT_ZONECENT)
                pointNormals = false;
        }
        vtkVisItStructuredGridNormals *normals =
                                            vtkVisItStructuredGridNormals::New();
        normals->SetInputData(sgrid);
        if (pointNormals)
            normals->SetNormalTypeToPoint();
        else
            normals->SetNormalTypeToCell();

        normals->Update();

        vtkStructuredGrid *out_ds = normals->GetOutput();

        avtDataRepresentation *out_dr = new avtDataRepresentation(out_ds,
            in_dr->GetDomain(), in_dr->GetLabel());

        normals->Delete();

        return out_dr;
    }

    // Don't know what to do with other grid types.
    debug1 << "Sent unsupported grid type into normals filter" << endl;
    return in_dr;
}


// ****************************************************************************
//  Method:  avtVertexNormalsFilter::FilterUnderstandsTransformedRectMesh
//
//  Purpose:
//    If this filter returns true, this means that it correctly deals
//    with rectilinear grids having an implied transform set in the
//    data attributes.  It can do this conditionally if desired.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 15, 2007
//
// ****************************************************************************

bool
avtVertexNormalsFilter::FilterUnderstandsTransformedRectMesh()
{
    // Creating normals for a transformed rectilinar mesh is okay;
    // the normals will be transformed when the mesh is.
    return true;
}


// ****************************************************************************
//  Method:  avtVertexNormalsFilter::UpdateDataObjectInfo
//
//  Purpose:
//    Update the data object information.
//
//  Programmer:  Alister Maguire
//  Creation:    April 27, 2020
//
// ****************************************************************************

void
avtVertexNormalsFilter::UpdateDataObjectInfo(void)
{
    //
    // We need to inform VisIt if the PolyDataNormalsFilter has split
    // cells.
    //
    if (zonesHaveBeenSplit)
    {
        GetOutput()->GetInfo().GetValidity().InvalidateZones();
        GetOutput()->GetInfo().GetValidity().ZonesSplit();
    }
}
