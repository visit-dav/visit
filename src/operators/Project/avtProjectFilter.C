// ************************************************************************* //
//  File: avtProjectFilter.C
// ************************************************************************* //

#include <avtProjectFilter.h>

#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPolyData.h>

#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtProjectFilter constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   September  3, 2004
//
// ****************************************************************************

avtProjectFilter::avtProjectFilter()
{
}


// ****************************************************************************
//  Method: avtProjectFilter destructor
//
//  Programmer: Jeremy Meredith
//  Creation:   September  3, 2004
//
//  Modifications:
//
// ****************************************************************************

avtProjectFilter::~avtProjectFilter()
{
}


// ****************************************************************************
//  Method:  avtProjectFilter::Create
//
//  Programmer: Jeremy Meredith
//  Creation:   September  3, 2004
//
// ****************************************************************************

avtFilter *
avtProjectFilter::Create()
{
    return new avtProjectFilter();
}


// ****************************************************************************
//  Method:      avtProjectFilter::SetAtts
//
//  Purpose:
//      Sets the state of the filter based on the attribute object.
//
//  Arguments:
//      a        The attributes to use.
//
//  Programmer: Jeremy Meredith
//  Creation:   September  3, 2004
//
// ****************************************************************************

void
avtProjectFilter::SetAtts(const AttributeGroup *a)
{
    atts = *(const ProjectAttributes*)a;
}


// ****************************************************************************
//  Method: avtProjectFilter::Equivalent
//
//  Purpose:
//      Returns true if creating a new avtProjectFilter with the given
//      parameters would result in an equivalent avtProjectFilter.
//
//  Programmer: Jeremy Meredith
//  Creation:   September  3, 2004
//
// ****************************************************************************

bool
avtProjectFilter::Equivalent(const AttributeGroup *a)
{
    return (atts == *(ProjectAttributes*)a);
}


// ****************************************************************************
//  Method: avtProjectFilter::ExecuteData
//
//  Purpose:
//      Sends the specified input and output through the Project filter.
//
//  Arguments:
//      in_ds      The input dataset.
//      <unused>   The domain number.
//      <unused>   The label.
//
//  Returns:       The output dataset.
//
//  Programmer: Jeremy Meredith
//  Creation:   September  3, 2004
//
// ****************************************************************************

vtkDataSet *
avtProjectFilter::ExecuteData(vtkDataSet *in_ds, int, std::string)
{
    int  datatype = in_ds->GetDataObjectType();
    switch (datatype)
    {
      case VTK_RECTILINEAR_GRID:
        return ProjectRectilinearGrid((vtkRectilinearGrid*)in_ds);

      case VTK_STRUCTURED_GRID:
      case VTK_UNSTRUCTURED_GRID:
      case VTK_POLY_DATA:
        return ProjectPointSet((vtkPointSet*)in_ds);

      default:
        EXCEPTION0(ImproperUseException);
    }
    
    return NULL;
}


// ****************************************************************************
//  Method:  avtProjectFilter::ProjectPoint
//
//  Purpose:
//    Project a single point in-place.
//
//  Arguments:
//    x,y,z      the point to project
//
//  Programmer:  Jeremy Meredith
//  Creation:    September  3, 2004
//
// ****************************************************************************
void
avtProjectFilter::ProjectPoint(float &x,float &y,float &z)
{
    switch (atts.GetProjectionType())
    {
      case ProjectAttributes::XYCartesian:
        z = 0;
        break;

      case ProjectAttributes::ZRCylindrical:
        {
            float r = sqrt(x*x + y*y);
            x = z;
            y = r;
            z = 0;
        }
        break;
    }
}

// ****************************************************************************
//  Method:  avtProjectFilter::ProjectRectilinearToRectilinear
//
//  Purpose:
//    Project a rectilinear grid in such a way that it remains a
//    rectilinear grid.
//
//  Arguments:
//    in_ds      the rectilinear grid to project
//
//  Programmer:  Jeremy Meredith
//  Creation:    September  6, 2004
//
// ****************************************************************************
vtkDataSet *
avtProjectFilter::ProjectRectilinearToRectilinear(vtkRectilinearGrid *in_ds)
{
    //
    // NOTE: we are only assuming XY Cartesian projection here
    //
    if (atts.GetProjectionType() != ProjectAttributes::XYCartesian)
        EXCEPTION0(ImproperUseException);


    vtkRectilinearGrid *out_ds = in_ds->NewInstance();
    out_ds->ShallowCopy(in_ds);

    vtkDataArray *z_orig = in_ds->GetZCoordinates();
    int nz = z_orig->GetNumberOfTuples();

    // Make a new point array
    vtkDataArray *z_new  = z_orig->NewInstance();
    z_new->SetNumberOfTuples(nz);
    float *zcoords = (float*)z_new->GetVoidPointer(0); // Assume float
    for (int i = 0 ; i < nz ; i++)
    {
        zcoords[i] = 0.0;
    }
    out_ds->SetZCoordinates(z_new);
    z_new->Delete();

    ManageMemory(out_ds);
    out_ds->Delete();
    return out_ds;
}


// ****************************************************************************
//  Method:  avtProjectFilter::ProjectRectilinearToCurvilinear
//
//  Purpose:
//    Converts a rectilinear grid to a curvilinear grid while projecting
//
//  Arguments:
//    in_ds      the input rectilinear grid
//
//  Note: Partially copied from avtTransform::TransformRectilinearToCurvilinear
//        with the exception of the code to transform each point.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September  6, 2004
//
// ****************************************************************************
vtkDataSet *
avtProjectFilter::ProjectRectilinearToCurvilinear(vtkRectilinearGrid *in_ds)
{
    int  dims[3];
    in_ds->GetDimensions(dims);

    int  numPts = dims[0]*dims[1]*dims[2];

    vtkPoints *pts = vtkPoints::New();
    pts->SetNumberOfPoints(numPts);

    vtkDataArray *x = in_ds->GetXCoordinates();
    vtkDataArray *y = in_ds->GetYCoordinates();
    vtkDataArray *z = in_ds->GetZCoordinates();

    int index = 0;
    for (int k = 0 ; k < dims[2] ; k++)
    {
        for (int j = 0 ; j < dims[1] ; j++)
        {
            for (int i = 0 ; i < dims[0] ; i++)
            {
                float pt[3];
                pt[0] = x->GetComponent(i,0);
                pt[1] = y->GetComponent(j,0);
                pt[2] = z->GetComponent(k,0);

                ProjectPoint(pt[0],pt[1],pt[2]);

                pts->SetPoint(index++, pt);
            }
        }
    }

    vtkStructuredGrid *out = vtkStructuredGrid::New();
    out->SetDimensions(dims);
    out->SetPoints(pts);
    pts->Delete();
    out->GetCellData()->ShallowCopy(in_ds->GetCellData());
    out->GetPointData()->ShallowCopy(in_ds->GetPointData());

    //
    // We have said normals are inappropriate, since this will be a
    // 2D data set.  We have not touched the vectors, however.
    // As soon as we figure out the right thing to do with them, we
    // should do it here!
    //


    //
    // We want to reduce the reference count of this dataset so it doesn't get
    // leaked.  But where to store it?  Fortunately, our base class handles
    // this for us.
    //
    ManageMemory(out);
    out->Delete();

    return out;
}

// ****************************************************************************
//  Method:  avtProjectFilter::ProjectRectilinearGrid
//
//  Purpose:
//    Projects a rectilinear grid.  Determines if the rectilinear grid
//    needs to be converted to a curvilinear grid.
//
//  Arguments:
//    in_ds      the rectilinear grid to project
//
//  Programmer:  Jeremy Meredith
//  Creation:    September  6, 2004
//
// ****************************************************************************
vtkDataSet *
avtProjectFilter::ProjectRectilinearGrid(vtkRectilinearGrid *in_ds)
{
    switch (atts.GetProjectionType())
    {
      case ProjectAttributes::XYCartesian:
        return ProjectRectilinearToRectilinear(in_ds);
      case ProjectAttributes::ZRCylindrical:
        return ProjectRectilinearToCurvilinear(in_ds);
    }

    return NULL;
}

// ****************************************************************************
//  Method:  avtProjectFilter::ProjectPointSet
//
//  Purpose:
//    Projects a descendent of vtkPointSet.  This includes
//    curvilinear, unstructured, point meshes, and poly data.
//
//  Arguments:
//    in_ds      the data set to project
//
//  Programmer:  Jeremy Meredith
//  Creation:    September  6, 2004
//
// ****************************************************************************
vtkDataSet *
avtProjectFilter::ProjectPointSet(vtkPointSet *in_ds)
{
    vtkPointSet *out_ds = in_ds->NewInstance();
    out_ds->ShallowCopy(in_ds);

    vtkPoints *old_pts = in_ds->GetPoints();
    int npoints = old_pts->GetNumberOfPoints();

    // Make a new point array
    vtkPoints *new_pts = old_pts->NewInstance();
    new_pts->DeepCopy(old_pts);
    float *points = (float*)new_pts->GetVoidPointer(0); // Assume float
    for (int i = 0 ; i < npoints ; i++)
    {
        
        ProjectPoint(points[i*3+0],
                     points[i*3+1],
                     points[i*3+2]);
    }
    out_ds->SetPoints(new_pts);
    new_pts->Delete();

    ManageMemory(out_ds);
    out_ds->Delete();
    return out_ds;
}

// ****************************************************************************
//  Method:  avtProjectFilter::RefashionDataObjectInfo
//
//  Purpose:
//    Set up the atttributes and validity for the output of the filter.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    September  6, 2004
//
// ****************************************************************************
void
avtProjectFilter::RefashionDataObjectInfo(void)
{
    avtDataAttributes &inAtts      = GetInput()->GetInfo().GetAttributes();
    avtDataAttributes &outAtts     = GetOutput()->GetInfo().GetAttributes();
    avtDataValidity   &outValidity = GetOutput()->GetInfo().GetValidity();
   
    // I suppose the topological dimension shouldn't change...:
    //if (inAtts.GetTopologicalDimension() == 3)
    //    outAtts.SetTopologicalDimension(2);

    if (inAtts.GetSpatialDimension() == 3)
        outAtts.SetSpatialDimension(2);

    outValidity.SetNormalsAreInappropriate(true);
}

