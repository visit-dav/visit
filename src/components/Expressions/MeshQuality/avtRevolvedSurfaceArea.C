// ************************************************************************* //
//                           avtRevolvedSurfaceArea.C                        //
// ************************************************************************* //

#include <avtRevolvedSurfaceArea.h>

#include <float.h>
#include <math.h>

#include <vtkCell.h>
#include <vtkDataSet.h>
#include <vtkDataSetRemoveGhostCells.h>
#include <vtkFloatArray.h>
#include <vtkGeometryFilter.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkVisItFeatureEdges.h>

#include <avtCallback.h>

#include <DebugStream.h>
#include <InvalidDimensionsException.h>

static double AreaOfCone(double, double, double);


// ****************************************************************************
//  Method: avtRevolvedSurfaceArea constructor
//
//  Programmer: Hank Childs
//  Creation:   September 8, 2002
//
// ****************************************************************************

avtRevolvedSurfaceArea::avtRevolvedSurfaceArea()
{
    haveIssuedWarning = false;
}


// ****************************************************************************
//  Method: avtRevolvedSurfaceArea::PreExecute
//
//  Purpose:
//      Sets up a data member that ensures that we don't issue multiple
//      warnings.
//
//  Programmer: Hank Childs
//  Creation:   September 8, 2002
//
// ****************************************************************************

void
avtRevolvedSurfaceArea::PreExecute(void)
{
    avtSingleInputExpressionFilter::PreExecute();

    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    if (atts.GetSpatialDimension() != 2)
    {
        EXCEPTION2(InvalidDimensionsException, "Revolved volume",
                                               "2-dimensional");
    }
    haveIssuedWarning = false;
}


// ****************************************************************************
//  Method: avtRevolvedSurfaceArea::DeriveVariable
//
//  Purpose:
//      Determines the volume each cell would take if it were revolved around
//      the axis y=0.
//
//  Programmer: Hank Childs
//  Creation:   September 8, 2002
//
//  Modifications:
//    Jeremy Meredith, Thu Jul 24 13:14:32 PDT 2003
//    Added code to get the boundary edges before calculating the
//    revolved surface area.
//
// ****************************************************************************

vtkDataArray *
avtRevolvedSurfaceArea::DeriveVariable(vtkDataSet *in_ds)
{
    //
    // Create the boundary edges and remove ghost zone edges
    //
    vtkGeometryFilter *geomFilter = vtkGeometryFilter::New();

    vtkVisItFeatureEdges *boundaryFilter = vtkVisItFeatureEdges::New();
    boundaryFilter->BoundaryEdgesOn();
    boundaryFilter->FeatureEdgesOff();
    boundaryFilter->NonManifoldEdgesOff();
    boundaryFilter->ManifoldEdgesOff();
    boundaryFilter->ColoringOff();

    vtkDataSetRemoveGhostCells *gzFilter = vtkDataSetRemoveGhostCells::New();
    gzFilter->SetGhostLevel(1);

    geomFilter->SetInput(in_ds);
    boundaryFilter->SetInput(geomFilter->GetOutput());
    boundaryFilter->GetOutput()->SetUpdateGhostLevel(2);
    boundaryFilter->GetOutput()->Update();

    vtkPolyData *allLines = boundaryFilter->GetOutput();
    allLines->SetSource(NULL);

    gzFilter->SetInput(allLines);
    vtkDataSet *ds_1d_nogz = gzFilter->GetOutput();
    ds_1d_nogz->Update();

    // We need line segment polydata, and should have it by now.
    if (ds_1d_nogz->GetDataObjectType() != VTK_POLY_DATA)
    {
        debug1 << "ERROR:Did not get polydata from ghost zone filter output\n";
        return NULL;
    }
    vtkPolyData *pd_1d_nogz = (vtkPolyData*)ds_1d_nogz;

    vtkFloatArray *arr = vtkFloatArray::New();
    int ncells = pd_1d_nogz->GetNumberOfCells();
    arr->SetNumberOfTuples(ncells);

    for (int i = 0 ; i < ncells ; i++)
    {
        vtkCell *cell = pd_1d_nogz->GetCell(i);
        float area = (float) GetLineArea(cell);
        arr->SetTuple1(i, area);
    }

    geomFilter->Delete();
    gzFilter->Delete();
    boundaryFilter->Delete();

    return arr;
}


// ****************************************************************************
//  Method: avtRevolvedSurfaceArea::GetLineArea
//
//  Purpose:
//      Revolve the zone around the line y = 0.  This is done by making
//      two cones -- one that intersects the line y = 0 and one that is
//      the requested line segment, but extended to the line y = 0.  Then
//      we can difference them and get the surface area for just this cell.
//
//  Arguments:
//      line    The input line.
//
//  Returns:    The surface area of the revolved line segment.
//
//  Programmer: Hank Childs
//  Creation:   March 18, 2003
//
// ****************************************************************************
 
double
avtRevolvedSurfaceArea::GetLineArea(vtkCell *cell)
{
    int cellType = cell->GetCellType();
    if (cellType != VTK_LINE)
    {
        if (!haveIssuedWarning)
        {
           char msg[1024];
           sprintf(msg, "The revolved surface area is only supported for "
                        " line segments.  %d is an invalid cell type.",
                         cellType);
           avtCallback::IssueWarning(msg);
        }
        haveIssuedWarning = true;
        return 0.;
    }

    vtkPoints *pts = cell->GetPoints();
    float p0[3];
    float p1[3];
    pts->GetPoint(0, p0);
    pts->GetPoint(1, p1);
    double  x[2], y[2];
    x[0] = p0[0];
    x[1] = p1[0];
    y[0] = p0[1];
    y[1] = p1[1];

    //
    // It's easier to think about this if we know x[0] < x[1]
    //
    if (x[1] < x[0])
    {
        double tmp;
        tmp = x[0]; x[0] = x[1]; x[1] = tmp;
        tmp = y[0]; y[0] = y[1]; y[1] = tmp;
    }

#if defined(_WIN32) && !defined(M_PI)
#define M_PI 3.141592653589793
#endif

    //
    // Make sure we are not dealing with a vertical line.
    //
    if (x[0] == x[1])
    {
        // Degenerate case -- this is a flat doughnut.
        if (y[1] < y[0])
        {
            double tmp;
            tmp = y[0]; y[0] = y[1]; y[1] = tmp;
        }
        double outer_circle = M_PI*y[1]*y[1];
        double inner_circle = M_PI*y[0]*y[0];
        return outer_circle - inner_circle;
    }

    double slope = (y[1] - y[0]) / (x[1] - x[0]);

    //
    // Make sure we are not dealing with a horizontal line.
    //
    if (slope == 0.)
    {
        // Degenerate case -- this is a cylinder.
        double height = x[1] - x[0];
        double radius = y[0];
        return 2*M_PI*radius*height;
    }

    //
    // Calculate the distance to the line y=0.
    //
    bool Pt0IsCloser = (fabs(y[0]) < fabs(y[1]));

    double rv = 0.;
    if (Pt0IsCloser)
    {
        double axis_intersection = (slope*x[1] - y[1]) / slope;
        double area_of_small_cone =
                    AreaOfCone(axis_intersection, x[0], y[0]);
        double area_of_big_cone =
                    AreaOfCone(axis_intersection, x[1], y[1]);
        rv = area_of_big_cone - area_of_small_cone;
    }
    else
    {
        double axis_intersection = (slope*x[1] - y[1]) / slope;
        double area_of_small_cone =
                    AreaOfCone(axis_intersection, x[1], y[1]);
        double area_of_big_cone =
                    AreaOfCone(axis_intersection, x[0], y[0]);
        rv = area_of_big_cone - area_of_small_cone;
    }

    return rv;
}


// ****************************************************************************
//  Function: AreaOfCone
// 
//  Purpose:
//      Returns the surface area obtained when revolving a line segment
//      around the y-axis.
// 
//  Arguments:
//      x0      The x-coordinate of the first point.  The y-coordinate for this
//              point is assumed to be 0.;
//      x1      The x-coordinate of the second point.
//      y1      The y-coordinate of the second point.
// 
//  Returns:    The surface area of the revolved line segment.
// 
//  Programmer: Hank Childs
//  Creation:   March 18, 2003
// 
// ****************************************************************************
 
static double
AreaOfCone(double x0, double x1, double y1)
{
    //
    // The formula for the surface area of a cone is:
    // A = pi*r*p, where p = sqrt(r*r + h*h)
    //
    // A useful website for this is: 
    //  http://www.rdrop.com/~half/Creations/Puzzles/ConeArea/
    //
    double r = y1 - 0.;
    double h = (x1 - x0);
    double p = sqrt(r*r + h*h);
    return M_PI*r*p;
}


