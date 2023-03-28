// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtRevolvedVolume.C                             //
// ************************************************************************* //

#include <avtRevolvedVolume.h>

#include <float.h>
#include <vtkCell.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkLine.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkRectilinearGrid.h>

#include <avtCallback.h>

#include <InvalidDimensionsException.h>


// ****************************************************************************
//  Method: avtRevolvedVolume constructor
//
//  Programmer: Hank Childs
//  Creation:   September 8, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Feb  3 11:24:40 PST 2006
//    Initialize revolveAboutX.
//
//    Alister Maguire, Thu Jun 18 10:02:58 PDT 2020
//    Set canApplyToDirectDatabaseQOT to false.
//
//    Alister Maguire, Fri Nov  6 08:53:20 PST 2020
//    Removed direct database QOT disabler as they are now compatible.
//
// ****************************************************************************

avtRevolvedVolume::avtRevolvedVolume()
{
    haveIssuedWarning = false;
    revolveAboutX = true;
}


// ****************************************************************************
//  Method: avtRevolvedVolume::PreExecute
//
//  Purpose:
//      Sets up a data member that ensures that we don't issue multiple
//      warnings.
//
//  Programmer: Hank Childs
//  Creation:   September 8, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Feb  3 11:24:40 PST 2006
//    Initialize revolveAboutX.

// ****************************************************************************

void
avtRevolvedVolume::PreExecute(void)
{
    avtSingleInputExpressionFilter::PreExecute();

    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    if (atts.GetSpatialDimension() > 2)
    {
        EXCEPTION2(InvalidDimensionsException, "Revolved volume",
                                               "2-dimensional");
    }
    haveIssuedWarning = false;
    if (atts.GetMeshCoordType() == AVT_ZR)
        revolveAboutX = false;
    else 
        revolveAboutX = true;
}


// ****************************************************************************
//  Method: avtRevolvedVolume::DeriveVariable
//
//  Purpose:
//      Determines the volume each cell would take if it were revolved around
//      the axis y=0.
//
//  Programmer: Hank Childs
//  Creation:   September 8, 2002
//
// ****************************************************************************

vtkDataArray *
avtRevolvedVolume::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    vtkDataArray *arr = CreateArrayFromMesh(in_ds);
    vtkIdType ncells = in_ds->GetNumberOfCells();
    arr->SetNumberOfTuples(ncells);

    bool haveProcessedAsCurve = false;
    if (in_ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        vtkRectilinearGrid *rg = (vtkRectilinearGrid *) in_ds;
        int dims[3];
        rg->GetDimensions(dims);
        if (dims[1] <= 1 && dims[2] <= 1)
        {
            vtkDataArray *X = rg->GetXCoordinates();
            vtkDataArray *Y = rg->GetPointData()->GetScalars();
            for (vtkIdType i = 0 ; i < ncells ; i++)
            {
                double p0[3],p1[3];
                p0[0] = X->GetTuple1(i);
                p1[0] = X->GetTuple1(i+1);
                p0[1] = Y->GetTuple1(i);
                p1[1] = Y->GetTuple1(i+1);
                p0[2] = 0;
                p1[2] = 0;

                vtkPoints *pts = vtkPoints::New();
                pts->InsertNextPoint(p0);
                pts->InsertNextPoint(p1);

                vtkCell *cell = vtkLine::New();
                cell->Initialize(2,pts);

                double vol = GetZoneVolume(cell);
                arr->SetTuple1(i, vol);
            }
            haveProcessedAsCurve = true;
        }
    }

    if (haveProcessedAsCurve)
        return arr;

    for (vtkIdType i = 0 ; i < ncells ; i++)
    {
        vtkCell *cell = in_ds->GetCell(i);
        double vol = GetZoneVolume(cell);
        arr->SetTuple1(i, vol);
    }

    return arr;
}


// ****************************************************************************
//  Method: avtRevolvedVolume::GetZoneVolume
//
//  Purpose:
//      Revolve the zone around either the y-axis (x = 0), or the 
//      x-axis (y = 0).   Which axis is determined by the mesh coord type.
//      RZ -> about x; ZR -> about y;  XY -> assume RZ, so about x.
//
//      This is done by making two volumes for the top of the zone 
//      and the bottom of the zone and subtracting them.
//
//  Arguments:
//      cell    The input zone.
//
//  Returns:    The volume of the revolved zone.
//
//  Programmer: Hank Childs
//  Creation:   March 29, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Jul 24 13:07:06 PDT 2008
//    Add support for polygons.
//
//    Mark C. Miller, Tue Mar 21 14:20:44 PDT 2023
//    Ensure error message for unsupported types accurately reflects req'mts.
// ****************************************************************************
 
double
avtRevolvedVolume::GetZoneVolume(vtkCell *cell)
{
    int cellType = cell->GetCellType();
    if (cellType != VTK_TRIANGLE && cellType != VTK_QUAD && 
        cellType != VTK_PIXEL && cellType != VTK_POLYGON &&
        cellType != VTK_LINE)
    {
        if (!haveIssuedWarning)
        {
           char msg[1024];
           sprintf(msg, "The revolved_volume expression supports meshes composed of"
                        " only triangles, quads (or \"pixels\"), and polygons or"
                        " curves composed of lines. %d is an unsupported cell type.",
                         cellType);
           avtCallback::IssueWarning(msg);
        }
        haveIssuedWarning = true;
        return 0.;
    }

    double rv = 0.;
    vtkPoints *pts = cell->GetPoints();
    if (cellType == VTK_TRIANGLE)
    {
        double p0[3];
        double p1[3];
        double p2[3];
        pts->GetPoint(0, p0);
        pts->GetPoint(1, p1);
        pts->GetPoint(2, p2);
        double  x[3], y[3];
        x[0] = p0[0];
        x[1] = p1[0];
        x[2] = p2[0];
        y[0] = p0[1];
        y[1] = p1[1];
        y[2] = p2[1];

        rv = GetTriangleVolume(x, y);
    }
    else if (cellType == VTK_QUAD || cellType == VTK_PIXEL || cellType == VTK_LINE)
    {

        double p0[3];
        double p1[3];
        double p2[3];
        double p3[3];
        pts->GetPoint(0, p0);
        pts->GetPoint(1, p1);

        // Bulk of code below written for VTK_QUAD. For PIXEL and LINE, we simply
        // re-organize input points slightly.
        if (cellType == VTK_QUAD)
        {
            pts->GetPoint(2, p2);
            pts->GetPoint(3, p3);
        }
        else if (cellType == VTK_PIXEL)
        {
            // VTK_PIXEL's 2nd and 3rd points are reversed from VTK_QUAD
            pts->GetPoint(3, p2);
            pts->GetPoint(2, p3);
        }
        else // line case
        {

printf("Handling lines\n");

            // Just project first two points onto the axis of revolution.
            p2[0] = p1[0]; p2[1] = p1[1]; p2[2] = p1[2];
            p3[0] = p0[0]; p3[1] = p0[1]; p3[2] = p0[2];
            if (revolveAboutX)
                p2[1] = p3[1] = 0.0;
            else
                p2[0] = p3[0] = 0.0;
        }

printf("p0 = %g, %g, %g\n", p0[0], p0[1], p0[2]);
printf("p1 = %g, %g, %g\n", p1[0], p1[1], p1[2]);
printf("p2 = %g, %g, %g\n", p2[0], p2[1], p2[2]);
printf("p3 = %g, %g, %g\n", p3[0], p3[1], p3[2]);

        double  x[3], y[3];
        double  volume1, volume2;
     
        //
        // Split into two triangles (P0, P1, and P2), and (P0, P2, and P3) and
        // find their volumes.
        //
        x[0] = p0[0];
        y[0] = p0[1];
        x[1] = p1[0];
        y[1] = p1[1];
        x[2] = p2[0];
        y[2] = p2[1];
        volume1 = GetTriangleVolume(x, y);
 
        x[0] = p0[0];
        y[0] = p0[1];
        x[1] = p3[0];
        y[1] = p3[1];
        x[2] = p2[0];
        y[2] = p2[1];
        volume2 = GetTriangleVolume(x, y);
     
printf("rv = %g + %g\n", volume1, volume2);

        rv = (volume1 + volume2);
    }
    else if (cellType == VTK_POLYGON)
    {
        vtkPoints *pts = cell->GetPoints();
        int numPointsForThisCell = cell->GetNumberOfPoints();
        int numTris = numPointsForThisCell-2;
        double pt[3];
        double  x[3], y[3];
        pts->GetPoint(0,pt);
        x[0] = pt[0];
        y[0] = pt[1];
        for (int j = 0 ; j < numTris ; j++)
        {
            pts->GetPoint(j+1,pt);
            x[1] = pt[0];
            y[1] = pt[1];
            pts->GetPoint(j+2,pt);
            x[2] = pt[0];
            y[2] = pt[1];
            rv += GetTriangleVolume(x, y);
        }
    }

    return rv;
}


// ****************************************************************************
//  Function: GetTriangleVolume
// 
//  Purpose:
//    Revolve a triangle around either the x-axis or y-axis.
//
//    Simulates revolution about y-axis by revsering order of x and
//    y coordinates in call to GetTriangleVolume2.
// 
//  Arguments:
//      x       The x-coordinates of the triangle.
//      y       The y-coordinates of the triangle.
// 
//  Returns:    The volume of the revolved triangle.
// 
//  Programmer: Kathleen Bonnell 
//  Creation:   February 3, 2006
//
//  Modifications:
//  
// ****************************************************************************
 
double
avtRevolvedVolume::GetTriangleVolume(double x[3], double y[3])
{
    if (revolveAboutX)
        return GetTriangleVolume2(x, y);
     else
        return GetTriangleVolume2(y, x);
}


// ****************************************************************************
//  Function: GetTriangleVolume2
// 
//  Purpose:
//      Revolve a triangle around the x-axis (y = 0).
// 
//  Arguments:
//      x       The x-coordinates of the triangle.
//      y       The y-coordinates of the triangle.
// 
//  Returns:    The volume of the revolved triangle.
// 
//  Programmer: Hank Childs
//  Creation:   March 29, 2000
//  
//  Modifications:
//    Akira Haddox, Wed Jul  2 12:27:24 PDT 2003
//    Dealt with triangles below or on the x-axis.
// 
//    Kathleen Bonnell, Thu Feb  2 10:53:19 PST 2006 
//    Renamed to GetTriangleVolume2, modifiy comments to more 
//    accurately reflect revolution-about-x, Fixed x-intercept formula
//    for triangles crossing x-axis. Tighten up for-loop for sorting points.
// 
// ****************************************************************************
 
double
avtRevolvedVolume::GetTriangleVolume2(double x[3], double y[3])
{
    double   cone01, cone02, cone12;
    double   slope01, slope02, slope12;
    double   volume;
    double   ls_x[2], ls_y[2];

    //
    // Check to see if we straddle the line y = 0, and break it up into
    // triangles which don't. Keep in mind the points are ordered
    // by x-coordinate.
    // 
    if (y[0] * y[1] < 0 || y[1] * y[2] < 0 || y[0] * y[2] < 0)
    {
        double oppositeX, oppositeY;
        double pt1X, pt2X, pt1Y, pt2Y;

        // Find the one point that is opposite the other two.
        if (y[1] * y[2] >= 0)
        {
            // The point is 0
            oppositeX = x[0];
            oppositeY = y[0];
            pt1X = x[1];
            pt1Y = y[1];
            pt2X = x[2];
            pt2Y = y[2];
        }
        else if(y[0] * y[2] >= 0)
        {
            // The point is 1
            oppositeX = x[1];
            oppositeY = y[1];
            pt1X = x[0];
            pt1Y = y[0];
            pt2X = x[2];
            pt2Y = y[2];
        }
        else
        {
            // The point is 2
            oppositeX = x[2];
            oppositeY = y[2];
            pt1X = x[0];
            pt1Y = y[0];
            pt2X = x[1];
            pt2Y = y[1];
        }
        
        //
        // Now take this information and find the two intersections.
        //
        double xInt1, xInt2;

        // Special cases: infinite slopes
        if (oppositeX == pt1X)
            xInt1 = oppositeX; 
        else
        {
            double nslope = (oppositeY - pt1Y) / (oppositeX - pt1X);
            xInt1 = oppositeX - oppositeY / nslope;
        }
            
        if (oppositeX == pt2X)
            xInt2 = oppositeX;
        else
        {
            double nslope = (oppositeY - pt2Y) / (oppositeX - pt2X);
            xInt2 = oppositeX - oppositeY / nslope;
        }   

        double v1, v2, v3;     
        
        //
        // Find the volume of the single triangle
        //
        x[0] = oppositeX;
        x[1] = xInt1;
        x[2] = xInt2;
        y[0] = oppositeY;
        y[1] = 0;
        y[2] = 0;

        v1 = GetTriangleVolume2(x, y);
        
        //
        // Find the volume of the quad by splitting it into triangles 
        // (the intercepts to pt1, then {pt1, pt2, Intercept2}).
        //

        x[0] = pt1X;
        y[0] = pt1Y;
        v2 = GetTriangleVolume2(x, y);
        
        x[1] = pt2X;
        y[1] = pt2Y;
        v3 = GetTriangleVolume2(x, y);

        return v1 + v2 + v3;
    }
    
    //
    // Sort the points so that they are ordered by x-coordinate.  This will
    // make things much easier later.
    // 
    for (int i = 0 ; i < 2 ; i++)
    {
        for (int j = i + 1 ; j < 3 ; j++)
        {
            if (x[j] < x[i])
            {
                double tmp_x, tmp_y;
                tmp_x = x[j];
                tmp_y = y[j];
                x[j]  = x[i];
                y[j]  = y[i];
                x[i]  = tmp_x;
                y[i]  = tmp_y;
            }
        }
    }
 
    //
    // Revolve each of the line segments.
    //
    ls_x[0] = x[0];
    ls_y[0] = y[0];
    ls_x[1] = x[1];
    ls_y[1] = y[1];
    cone01 = RevolveLineSegment(ls_x, ls_y, &slope01);
 
    ls_x[1] = x[2];
    ls_y[1] = y[2];
    cone02 = RevolveLineSegment(ls_x, ls_y, &slope02);
 
    ls_x[0] = x[1];
    ls_y[0] = y[1];
    cone12 = RevolveLineSegment(ls_x, ls_y, &slope12);

    bool aboveY;
    if (y[0] < 0 || y[1] < 0 || y[2] < 0)
        aboveY = false;
    else
        aboveY = true;
    
    //
    // This is a little tricky and best shown by picture, but if slope01 is
    // greater than slope02, then P0P1 and P1P2 make up the top of the volume
    // and P0P2 make up the bottom.  If not, P0P1 and P1P2 make up the bottom
    // and P0P2 makes up the top.  Remember, the points are sorted by x-coords.
    // Also, if the slopes are equal, we have three collinnear points, but the
    // math actually works out fine, since the volume of the cone from P0P1
    // plus P1P2 equals P0P2.
    //
    // Note that if we're below the line y = 0, then this rule is reversed.
    //
    if (aboveY)
    {
        if (slope01 < slope02)
            volume = cone02 - cone01 - cone12;
        else
            volume = cone12 + cone01 - cone02;
    }
    else
    {
        if (slope01 < slope02)
            volume = cone12 + cone01 - cone02;
        else
            volume = cone02 - cone01 - cone12;

    }
 
    return volume;
}


// ****************************************************************************
//  Method: avtRevolvedVolume::RevolveLineSegment
// 
//  Purpose:
//      Calculate the volume of the cone created by revolving a line segment
//      around the x-axis (y = 0).
// 
//  Arguments:
//      x       The x-coordinates of the line segment.
//      y       The y-coordinates of the line segment.
//      slope   The slope of the line segments (Output Arg).
// 
//  Returns:    The volume of the (cropped) cone.
// 
//  Programmer: Hank Childs
//  Creation:   March 29, 2000
//  
//  Modifications:
//    Kathleen Bonnell, Thu Feb  2 11:26:23 PST 2006
//    Renamed some vars for clarity.
//
// ****************************************************************************
 
double
avtRevolvedVolume::RevolveLineSegment(double x[2], double y[2], double *slope)
{
    double   m, b;
    double   x_intercept;
    double   radius, height;
    double   coneAll, coneCropped;
 
    //
    // Sort out degenerate cases, slope = 0, infinity
    //
    if (x[0] == x[1])
    {
        /* Vertical line */
        if (y[0] > y[1])
        {
            *slope = -FLT_MAX;
        }
        else
        {
            *slope = FLT_MAX;
        }
        return 0.;
    }
    if (y[0] == y[1])
    {
#if defined(_WIN32) && !defined(M_PI)
#define M_PI 3.14159265358979323846
#endif

        // 0-slope line, return cylinder volume, not cone 
        double  radius = y[0];
        double  height = x[1] - x[0];
        double  volume = M_PI * radius * radius * height;
        *slope = 0.;
        return (volume);
    }

    //
    // Calculate where the line segment will hit the line y = 0.
    // Note we have already taken care of degenerate cases.
    //
    m = (y[1] - y[0]) / (x[1] - x[0]);
    b = y[0] - m*x[0];
    x_intercept = -b / m;
 
    //
    // We are now going to calculate the cone that contains our volume and
    // the tip of the cone that needs to be cropped off to give the volume.
    // Note that we have been very careful to construct right circular cones,
    // which have volume PI*r^2*h/3.
    //
    if (m < 0)
    {
        //
        //         . <= P0
        //
        //                . <= P1
        //                      x_intercept
        //                          ||
        //                          \/
        // x-axis  -----------------.--------------------
        //
        radius = y[0];
        height = x_intercept - x[0];
        coneAll = M_PI * radius * radius * height / 3;
 
        radius = y[1];
        height = x_intercept - x[1];
        coneCropped = M_PI * radius * radius * height / 3;
    }
    else  // m > 0 
    {
        //
        //                           . <= P1
        //
        //                       . <= P0
        //         x_intercept
        //             ||
        //             \/
        // x-axis  ----.---------------------------------
        //
        radius = y[1];
        height = x[1] - x_intercept;
        coneAll = M_PI * radius * radius * height / 3;
 
        radius = y[0];
        height = x[0] - x_intercept;
        coneCropped = M_PI * radius * radius * height / 3;
    }
 
    *slope = m;
    return (coneAll - coneCropped);
}

