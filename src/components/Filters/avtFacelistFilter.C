// ************************************************************************* //
//                             avtFacelistFilter.C                           //
// ************************************************************************* //

#include <avtFacelistFilter.h>

#include <vector>

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCellTypes.h>
#include <vtkDisjointCubesFacelistFilter.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkRectilinearGridFacelistFilter.h>
#include <vtkStructuredGrid.h>
#include <vtkStructuredGridFacelistFilter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridFacelistFilter.h>
#include <vtkUnsignedIntArray.h>
#include <vtkVisItUtility.h>

#include <avtDataset.h>
#include <avtFacelist.h>
#include <avtMetaData.h>
#include <avtTerminatingSource.h>

#include <BadIndexException.h>
#include <DebugStream.h>


using     std::vector;


// ****************************************************************************
//  Method: avtFacelistFilter constructor
//
//  Programmer: Hank Childs
//  Creation:   October 27, 2000
//
//  Modifications:
//
//    Jeremy Meredith, Thu Mar  1 15:38:20 PST 2001
//    Initialize facelist to null.
//
//    Hank Childs, Wed Apr 11 14:53:57 PDT 2001
//    Added rectilinear facelist filter.
//
//    Hank Childs, Fri Oct 19 08:04:20 PDT 2001
//    Added curvilinear facelist filter.
//
//    Eric Brugger, Wed Jan 23 15:24:07 PST 2002
//    I modified the class to use vtkUnstructuredGridFacelistFilter instead
//    of vtkGeometryFilter for unstructured grids.
//
//    Jeremy Meredith, Tue Jul  9 09:24:40 PDT 2002
//    Added initialization of create3DCellNumbers.
//
//    Hank Childs, Wed Oct 15 19:17:11 PDT 2003
//    Added forceFaceConsolidation.
//
// ****************************************************************************

avtFacelistFilter::avtFacelistFilter()
{
    rf = vtkRectilinearGridFacelistFilter::New();
    sf = vtkStructuredGridFacelistFilter::New();
    uf = vtkUnstructuredGridFacelistFilter::New();
    useFacelists = true;
    create3DCellNumbers = false;
    forceFaceConsolidation = false;
}


// ****************************************************************************
//  Method: avtFacelistFilter destructor
//
//  Programmer: Hank Childs
//  Creation:   October 27, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Apr 11 14:53:57 PDT 2001
//    Added rectilinear facelist filter.
//
//    Hank Childs, Fri Oct 19 08:04:20 PDT 2001
//    Destructed curvilinear facelist filter.
//
//    Eric Brugger, Wed Jan 23 15:24:07 PST 2002
//    I modified the class to use vtkUnstructuredGridFacelistFilter instead
//    of vtkGeometryFilter for unstructured grids.
//
// ****************************************************************************

avtFacelistFilter::~avtFacelistFilter()
{
    if (rf != NULL)
    {
        rf->Delete();
        rf = NULL;
    }
    if (sf != NULL)
    {
        sf->Delete();
        sf = NULL;
    }
    if (uf != NULL)
    {
        uf->Delete();
        uf = NULL;
    }
}

// ****************************************************************************
//  Method:  avtFacelistFilter::SetCreate3DCellNumbers
//
//  Purpose:
//    Tell the facelist filter to create 3D cell number array.
//
//  Arguments:
//    create     true if the filter should create the 3D cell number array
//
//  Programmer:  Jeremy Meredith
//  Creation:    July  9, 2002
//
// ****************************************************************************
void
avtFacelistFilter::SetCreate3DCellNumbers(bool create)
{
    create3DCellNumbers = create;
}


// ****************************************************************************
//  Method:  avtFacelistFilter::SetForceFaceConsolidation
//
//  Purpose:
//      Tells the facelist filter that it can consolidate faces.
//
//  Arguments:
//    create     true if the filter should create the 3D cell number array
//
//  Programmer:  Hank Childs
//  Creation:    October 15, 2003
//
// ****************************************************************************
void
avtFacelistFilter::SetForceFaceConsolidation(bool afc)
{
    forceFaceConsolidation = afc;
    rf->SetForceFaceConsolidation(afc ? 1 : 0);
}


// ****************************************************************************
//  Method: avtFacelistFilter::ExecuteData
//
//  Purpose:
//      Calcualtes the external faces of a mesh and returns the result as
//      poly-data.
//
//  Arguments:
//      in_ds      The input dataset.
//      domain     The domain number for in_ds.
//      label      The label.
//
//  Returns:       The output dataset.
//
//  Programmer: Hank Childs
//  Creation:   February 26, 2002
//
//  Modifications:
//
//    Jeremy Meredith, Tue Jul  9 14:00:04 PDT 2002
//    Added a 3DCellNumbers array if requested.
//
//    Hank Childs, Wed Jul 10 22:31:10 PDT 2002
//    Clean up test for when we should free up memory.
//
//    Hank Childs, Sun Aug 18 11:18:20 PDT 2002
//    Add support for meshes that have disjoint elements.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002  
//    Use NewInstance instead of MakeObject, to match new vtk api. 
//
//    Hank Childs, Fri Mar 28 08:21:05 PST 2003
//    Add support for points.
//
//    Kathleen Bonnell, Fri Feb 18 15:08:32 PST 2005 
//    Convert 'point' and 'line' Structured or Rectilinear grids to poly data. 
//
// ****************************************************************************

vtkDataSet *
avtFacelistFilter::ExecuteData(vtkDataSet *in_ds, int domain, std::string)
{
    int tDim = GetInput()->GetInfo().GetAttributes().GetTopologicalDimension();
    int sDim = GetInput()->GetInfo().GetAttributes().GetSpatialDimension();
    int dis_elem = GetInput()->GetInfo().GetValidity().GetDisjointElements();

    vtkDataSet *orig_in_ds = in_ds;
    bool shouldDeleteInDs = false;
    if (tDim == 3 && create3DCellNumbers)
    {
        in_ds = (vtkDataSet *) orig_in_ds->NewInstance();
        shouldDeleteInDs = true;
        in_ds->ShallowCopy(orig_in_ds);

        vtkUnsignedIntArray *cells = vtkUnsignedIntArray::New();
        cells->SetName("avt3DCellNumbers");
        cells->SetNumberOfComponents(1);
        cells->SetNumberOfTuples(in_ds->GetNumberOfCells());
        unsigned int *cellPtr = (unsigned int*)cells->GetVoidPointer(0);
        for (int i = 0; i < in_ds->GetNumberOfCells(); i++)
        {
            cellPtr[i] = i;
        }
        in_ds->GetCellData()->AddArray(cells);
        cells->Delete();
    }

    vtkDataSet *out_ds = NULL;
    switch (tDim)
    {
      // Points and lines
      case 0:
      case 1:
        if (in_ds->GetDataObjectType() != VTK_STRUCTURED_GRID &&
            in_ds->GetDataObjectType() != VTK_RECTILINEAR_GRID)
        {
            out_ds = in_ds; 
        }
        else
        {
            out_ds = ConvertToPolys(in_ds, tDim);
        }
        break;

      // 2D meshes or surfaces
      case 2:
        if (sDim == 3)
        {
            out_ds = in_ds;
        }
        else
        {
            out_ds = Take2DFaces(in_ds);
        }
        break;

      // 3D meshes
      case 3:
        if (dis_elem)
        {
            out_ds = TakeFacesForDisjointElementMesh(in_ds, domain);
        }
        else
            out_ds = Take3DFaces(in_ds, domain);
        break;

      default:
        out_ds = NULL;
        debug1 << "Ran into dataset with bad topological dimension " << tDim
               << endl;
        break;
    }

    if (shouldDeleteInDs)
    {
        in_ds->Delete();
        in_ds = orig_in_ds;
    }

    return out_ds;
}


// ****************************************************************************
//  Method: avtFacelistFilter::Take3DFaces
//
//  Purpose:
//      Calculates the external faces of a 3-dimensional mesh.
//
//  Arguments:
//      in_ds      The input dataset.
//      domain     The domain number for in_ds.
//
//  Returns:       The output dataset.
//
//  Programmer: Hank Childs
//  Creation:   October 27, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Apr 11 14:53:57 PDT 2001
//    Add special case for rectilinear facelists.
//
//    Hank Childs, Fri Oct 19 08:04:20 PDT 2001
//    Added special case for curvilinear facelists.
//
//    Eric Brugger, Wed Jan 23 15:24:07 PST 2002
//    I modified the class to use vtkUnstructuredGridFacelistFilter instead
//    of vtkGeometryFilter for unstructured grids.
//
//    Hank Childs, Tue Feb 26 16:27:19 PST 2002
//    Renamed from ExecuteData.  Blew away selective, unnecessary comments.
//
//    Hank Childs, Mon Apr 15 14:28:56 PDT 2002
//    Fix memory leak.
//
//    Hank Childs, Thu Jun 20 13:51:53 PDT 2002
//    Correct misleading typo in debug statement.
//
//    Hank Childs, Thu Jul 18 17:49:38 PDT 2002
//    Store off a reference even if we have poly data input.  This is because
//    it might be deleted out from under us later.
//
//    Hank Childs, Sun Aug  4 18:06:43 PDT 2002
//    Get a little smarter about when we can use avtFacelists.
//
//    Hank Childs, Tue Aug  6 11:09:08 PDT 2002
//    Don't take normals after using a rectilinear facelist.
//
//    Jeremy Meredith, Tue Aug 13 13:14:56 PDT 2002
//    Added a call to enable the unstructured grid facelist filter's two-pass
//    mode if it is needed.
//
//    Hank Childs, Tue Sep 24 11:43:06 PDT 2002
//    Make the facelist object we are dealing with no longer be a ref_ptr.
//
//    Hank Childs, Mon Nov  4 12:52:24 PST 200
//    Removed need for a two-pass mode unstructured grid facelist filter.
//
//    Hank Childs, Tue Jun  3 15:07:11 PDT 2003
//    Account for avtFacelists that are "bad".
//
//    Hank Childs, Wed May  5 16:18:44 PDT 2004
//    Do not prevent normal calculation for rectilinear grids.
//
// ****************************************************************************

vtkDataSet *
avtFacelistFilter::Take3DFaces(vtkDataSet *in_ds, int domain)
{
    vtkPolyData *pd     = vtkPolyData::New();
    vtkDataSet  *out_ds = NULL;
    bool         mustDeReference = false;
    bool         hasCellData = (in_ds->GetCellData()->GetNumberOfArrays() > 0 
                                ? true : false);

    switch (in_ds->GetDataObjectType())
    {
      case VTK_RECTILINEAR_GRID:
        rf->SetInput((vtkRectilinearGrid *) in_ds);
        rf->SetOutput(pd);
        rf->Update();
        out_ds = pd;
        mustDeReference = true;
        break;

      case VTK_STRUCTURED_GRID:
        sf->SetInput((vtkStructuredGrid *) in_ds);
        sf->SetOutput(pd);
        sf->Update();
        out_ds = pd;
        mustDeReference = true;
        break;

      case VTK_UNSTRUCTURED_GRID:
        {
            avtFacelist *fl = NULL;
            avtDataValidity &v = GetInput()->GetInfo().GetValidity();
            if (v.GetUsingAllData() && v.GetZonesPreserved())
            {
                avtMetaData *md = GetMetaData();
                fl = md->GetExternalFacelist(domain);
            }

            //
            // Some facelists from Silo files cannot calculate zonal variables,
            // so disregard the facelist in this case.
            //
            if (fl != NULL && hasCellData && 
                !fl->CanCalculateZonalVariables())
            {
                fl = NULL;
            }

            //
            // Even if the domain does not have facelists, the database should
            // return a facelist with no cells if others have facelists.
            //
            if (fl != NULL)
            {
                debug5 << "Ugrid using facelist from files for domain "
                       << domain << endl;
                TRY
                {
                    fl->CalcFacelist((vtkUnstructuredGrid *) in_ds, pd);
                    out_ds = pd;
                    mustDeReference = true;
                }
                CATCH (BadIndexException)
                {
                    debug1 << "The facelist was invalid.  This often happens "
                           << "with history variables." << endl;
                    debug1 << "The facelist will be calculated by hand."<<endl;
                    fl = NULL;
                }
                ENDTRY
            }

            if (fl == NULL) // Value could have changed since last test.
            {
                debug5 << "Ugrid forced to calculate facelist for domain "
                       << domain << endl;
                uf->SetInput((vtkUnstructuredGrid *) in_ds);
                uf->SetOutput(pd);
                uf->Update();
                out_ds = pd;
                mustDeReference = true;
            }
        }
        break;

      default:
        // We don't know what type this is.  It is probably a mistake that
        // this was called, so minimize the damage by passing this through.
        debug1 << "Unknown meshtype encountered in facelist filter, passing "
               << "input through as output." << endl;
        out_ds = in_ds;
        mustDeReference = false;
        break;
    }

    ManageMemory(out_ds);
    if (mustDeReference)
    {
        out_ds->Delete();
    }
    debug4 << "Facelist filter reduction for domain " << domain 
           << ": input has " << in_ds->GetNumberOfCells() << " cells, out has " 
           << out_ds->GetNumberOfCells() << endl;
    return out_ds;
}


// ****************************************************************************
//  Method: avtFacelistFilter::Take2DFaces
//
//  Purpose:
//      Takes "2D" faces -- this means we should pull out one side only of a
//      mesh.
//
//  Arguments:
//      in_ds      The input dataset.
//
//  Returns:       The output dataset.
//
//  Programmer: Hank Childs
//  Creation:   February 26, 2002
//
//  Modifications:
//
//    Hank Childs, Mon Apr 15 14:28:56 PDT 2002
//    Fix memory leak.
//
//    Hank Childs, Wed Oct  2 17:00:01 PDT 2002
//    Re-wrote routine by hand.  Approx. 15x faster for structured grids.
//
//    Hank Childs, Wed Oct 15 21:09:26 PDT 2003
//    Use the rectilinear grid facelist filter since it can do face
//    consolidation.
//
// ****************************************************************************

vtkDataSet *
avtFacelistFilter::Take2DFaces(vtkDataSet *in_ds)
{
    int dstype = in_ds->GetDataObjectType();

    //
    // Our goal is to convert this to poly data...
    //
    if (dstype == VTK_POLY_DATA)
    {
        return in_ds;
    }

    //
    // Set up all of the things that won't change -- the points and the point
    // and cell data.
    //
    vtkPoints *pts = vtkVisItUtility::GetPoints(in_ds);
    vtkPolyData *out_ds = vtkPolyData::New();
    out_ds->SetPoints(pts);
    pts->Delete();
    out_ds->GetPointData()->PassData(in_ds->GetPointData());
    out_ds->GetCellData()->PassData(in_ds->GetCellData());
    out_ds->GetFieldData()->PassData(in_ds->GetFieldData());

    //
    // Now create the cells.  Structured ones are easy.
    //
    if (dstype == VTK_RECTILINEAR_GRID)
    {
        rf->SetInput((vtkRectilinearGrid *) in_ds);
        rf->SetOutput(out_ds);
        out_ds->Update();
        rf->SetOutput(NULL);
        out_ds->SetSource(NULL);
    }
    else if (dstype == VTK_STRUCTURED_GRID)
    {
        int dims[3];
        ((vtkStructuredGrid *) in_ds)->GetDimensions(dims);
        int nx = dims[0]-1;
        int ny = dims[1]-1;
        int ncells = nx*ny;
        out_ds->Allocate(ncells);
/* * We should be able to do this, but there is a VTK bug.
        vtkIdType *ptr = out_ds->GetPolys()->WritePointer(ncells, 5*ncells);
        for (int j = 0 ; j < ny ; j++)
        {
            for (int i = 0 ; i < nx ; i++)
            {
                *ptr++ = 4;
                *ptr++ = j*(nx+1) + i;
                *ptr++ = j*(nx+1) + i+1;
                *ptr++ = (j+1)*(nx+1) + i+1;
                *ptr++ = (j+1)*(nx+1) + i;
            }
        }
 */
        vtkIdType quad[4];
        for (int j = 0 ; j < ny ; j++)
        {
            for (int i = 0 ; i < nx ; i++)
            {
                quad[0] = j*(nx+1) + i;
                quad[1] = j*(nx+1) + i+1;
                quad[2] = (j+1)*(nx+1) + i+1;
                quad[3] = (j+1)*(nx+1) + i;
                out_ds->InsertNextCell(VTK_QUAD, 4, quad);
            }
        }
    }
    else if (dstype == VTK_UNSTRUCTURED_GRID)
    {
        vtkUnstructuredGrid *ug = (vtkUnstructuredGrid *) in_ds;
        int ncells = ug->GetNumberOfCells();
        out_ds->Allocate(ncells);
        vtkIdList *idlist = vtkIdList::New();
        for (int i = 0 ; i < ncells ; i++)
        {
            ug->GetCellPoints(i, idlist);
            out_ds->InsertNextCell(ug->GetCellType(i), idlist);
        }
        idlist->Delete();
    }

    ManageMemory(out_ds);
    out_ds->Delete();

    return out_ds;
}


// ****************************************************************************
//  Method: avtFacelistFilter::TakeFacesForDisjointElementMesh
//
//  Purpose:
//      Calculates the external faces for meshes that are made up of disjoint
//      elements.
//
//  Arguments:
//      in_ds   The input dataset.
//      dom     The domain index of the dataset.
//
//  Returns:    The external faces.
//
//  Programmer: Hank Childs
//  Creation:   August 18, 2002
//
// ****************************************************************************

vtkDataSet *
avtFacelistFilter::TakeFacesForDisjointElementMesh(vtkDataSet *in_ds, int dom)
{
    //
    // Make sure we are dealing with an unstructured grid.
    //
    int type = in_ds->GetDataObjectType();
    if (type != VTK_UNSTRUCTURED_GRID)
    {
        debug5 << "Cannot work with disjoint mesh because it is not "
               << "unstructured." << endl;
        return Take3DFaces(in_ds, dom);
    }

    //
    // Make sure that we only have hexahedrons.
    //
    vtkUnstructuredGrid *ugrid = (vtkUnstructuredGrid *) in_ds;
    vtkCellTypes *types = vtkCellTypes::New();
    ugrid->GetCellTypes(types);
    int ntypes = types->GetNumberOfTypes();
    int cell_type  = types->GetCellType(0);
    types->Delete();
    if (ntypes != 1 || cell_type != VTK_HEXAHEDRON)
    {
        debug5 << "Cannot work with disjoint mesh because it is not made up "
               << "solely of hexahedrons." << endl;
        return Take3DFaces(in_ds, dom);
    }

    //
    // Use the disjoint cubes facelist filter to find the external faces.
    //
    vtkDisjointCubesFacelistFilter *dcff =
                                         vtkDisjointCubesFacelistFilter::New();
    dcff->SetInput(ugrid);
    dcff->Update();

    vtkDataSet *output = dcff->GetOutput();
    ManageMemory(output);
    dcff->Delete();
    return output;
}


// ****************************************************************************
//  Method: avtFacelistFilter::ConvertToPolys
//
//  Purpose:
//      Converts the incoming dataset to polydata.
//
//  Arguments:
//      in_ds      The input dataset.
//
//  Returns:       The output dataset.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 18, 2005
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
avtFacelistFilter::ConvertToPolys(vtkDataSet *in_ds, int tDim)
{
    int dstype = in_ds->GetDataObjectType();

    if (dstype != VTK_STRUCTURED_GRID && dstype != VTK_RECTILINEAR_GRID)
    {
        return in_ds;
    }

    //
    // Set up all of the things that won't change -- the points and the point
    // and cell data.
    //
    vtkPoints *pts = vtkVisItUtility::GetPoints(in_ds);
    vtkPolyData *out_ds = vtkPolyData::New();
    out_ds->SetPoints(pts);
    pts->Delete();
    out_ds->GetPointData()->PassData(in_ds->GetPointData());
    out_ds->GetCellData()->PassData(in_ds->GetCellData());
    out_ds->GetFieldData()->PassData(in_ds->GetFieldData());

    int nPts = pts->GetNumberOfPoints();
    vtkCellArray *cells = vtkCellArray::New();
    cells->Allocate(nPts*(tDim+1));
    vtkIdType ids[2] = {0, 0}; 
    int i;
    if (tDim == 0)
    {
        for (i = 0; i < nPts; i++)
        {
            ids[0] = i;
            cells->InsertNextCell(1, ids);
        }
        out_ds->SetVerts(cells);
    }
    else
    {
        for (i = 0; i < nPts-1; i++)
        {
            ids[0] = i;
            ids[1] = i+1;
            cells->InsertNextCell(2, ids);
        }
        out_ds->SetLines(cells);
    }
    cells->Delete();

    ManageMemory(out_ds);
    out_ds->Delete();

    return out_ds;
}



// ****************************************************************************
//  Method: avtFacelistFilter::InitializeFilter
//
//  Purpose:
//      Initializes the facelist filter after the input has changed by getting
//      the new facelist.
//
//  Programmer: Hank Childs
//  Creation:   October 27, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Jun  5 11:44:45 PDT 2001
//    Re-wrote function to only guide whether we should try to use facelists
//    later on.  Blew away previous comments.
//
//    Hank Childs, Sat Feb 19 14:55:03 PST 2005
//    Do not assume we have a valid input.
//
// ****************************************************************************

void
avtFacelistFilter::InitializeFilter(void)
{
    if (*(GetInput()) == NULL)
        return;
    if (GetInput()->GetInfo().GetValidity().GetZonesPreserved())
    {
        useFacelists = true;
    }
    else
    {
        useFacelists = false;
    }
}


// ****************************************************************************
//  Method: avtFacelistFilter::RefashionDataObjectInfo
//
//  Purpose:
//      Copies the mutable metadata that is associated with individual 
//      datasets.  This is done by avtDatasetToDatasetFilter, but is redefined
//      here to indicate that the zones are invalidated after this operation.
//
//  Programmer: Hank Childs
//  Creation:   January 5, 2001
//
//  Modifications:
//
//    Hank Childs, Sun Mar 25 11:44:12 PST 2001
//    Account for new interface with data object information.  Also set
//    topological dimension of output to be 2.
//
//    Hank Childs, Sat Apr  7 18:35:40 PDT 2001
//    Copy over data extents if they are valid.
//
//    Hank Childs, Mon Jun  4 19:41:59 PDT 2001
//    Renamed routine from CopyDatasetMetaData.
//
//    Hank Childs, Tue Sep  4 15:12:40 PDT 2001
//    Removed fancy parallel logic.
//
//    Hank Childs, Mon Feb 25 14:07:55 PST 2002
//    If we didn't apply the facelist filter, don't refashion the output's
//    info.
//
//    Hank Childs, Sat Feb 19 14:55:03 PST 2005
//    Do not assume we have a valid input.
//
// ****************************************************************************

void
avtFacelistFilter::RefashionDataObjectInfo(void)
{
    if (*(GetInput()) == NULL)
        return;
    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() == 3)
    {
        avtDataObject_p output = GetOutput();
        output->GetInfo().GetValidity().InvalidateZones();
        output->GetInfo().GetAttributes().SetTopologicalDimension(2);
    }
}


// ****************************************************************************
//  Method: avtFacelistFilter::ReleaseData
//
//  Purpose:
//      Releases any problem sized data being stored in the filter.
//
//  Programmer: Hank Childs
//  Creation:   September 10, 2002
//
//  Modifications:
//
//    Hank Childs, Fri Mar  4 08:12:25 PST 2005
//    Do not set outputs of filters to NULL, since this will prevent them
//    from re-executing correctly in DLB-mode.
//
// ****************************************************************************

void
avtFacelistFilter::ReleaseData(void)
{
    avtStreamer::ReleaseData();

    rf->SetInput(NULL);
    rf->SetOutput(vtkPolyData::New());
    sf->SetInput(NULL);
    sf->SetOutput(vtkPolyData::New());
    uf->SetInput(NULL);
    uf->SetOutput(vtkPolyData::New());
}


// ****************************************************************************
//  Method: avtFacelistFilter::PerformRestriction
//
//  Purpose:
//      Tell the database that we will need ghost nodes.
//
//  Programmer: Hank Childs
//  Creation:   August 11, 2004
//
// ****************************************************************************

avtPipelineSpecification_p
avtFacelistFilter::PerformRestriction(avtPipelineSpecification_p in_spec)
{
    //
    // Only declare that we want ghost nodes if someone downstream hasn't said
    // that they want ghost zones and also if we are dealing with 3D data.
    //
    avtPipelineSpecification_p spec = new avtPipelineSpecification(in_spec);
    if (spec->GetDataSpecification()->GetDesiredGhostDataType() !=
                                                               GHOST_ZONE_DATA
       && GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() == 3)
        spec->GetDataSpecification()->SetDesiredGhostDataType(GHOST_NODE_DATA);

    return spec;
}


