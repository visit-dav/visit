// ************************************************************************* //
//                           avtRevolvedVolume.C                             //
// ************************************************************************* //

#include <avtRevolvedVolume.h>

#include <float.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>

#include <avtCallback.h>

#include <InvalidDimensionsException.h>


// ****************************************************************************
//  Method: avtRevolvedVolume constructor
//
//  Programmer: Hank Childs
//  Creation:   September 8, 2002
//
// ****************************************************************************

avtRevolvedVolume::avtRevolvedVolume()
{
    haveIssuedWarning = false;
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
// ****************************************************************************

void
avtRevolvedVolume::PreExecute(void)
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
avtRevolvedVolume::DeriveVariable(vtkDataSet *in_ds)
{
    vtkFloatArray *arr = vtkFloatArray::New();
    int ncells = in_ds->GetNumberOfCells();
    arr->SetNumberOfTuples(ncells);

    for (int i = 0 ; i < ncells ; i++)
    {
        vtkCell *cell = in_ds->GetCell(i);
        float vol = (float) GetZoneVolume(cell);
        arr->SetTuple(i, &vol);
    }

    return arr;
}


// ****************************************************************************
//  Method: avtRevolvedVolume::GetZoneVolume
//
//  Purpose:
//      Revolve the zone around the axis x = 0.  This is done by making
//      two volumes for the top of the zone and the bottom of the zone and
//      subtracting them.
//
//  Arguments:
//      cell    The input zone.
//
//  Returns:    The volume of the revolved zone.
//
//  Programmer: Hank Childs
//  Creation:   March 29, 2000
//
// ****************************************************************************
 
double
avtRevolvedVolume::GetZoneVolume(vtkCell *cell)
{
    int cellType = cell->GetCellType();
    if (cellType != VTK_TRIANGLE && cellType != VTK_QUAD && 
        cellType != VTK_PIXEL)
    {
        if (!haveIssuedWarning)
        {
           char msg[1024];
           sprintf(msg, "The revolved volume is only support for triangles and"
                        " quadrilaterals.  %d is an invalid cell type.",
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
        float p0[3];
        float p1[3];
        float p2[3];
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
    else if (cellType == VTK_QUAD)
    {
        float p0[3];
        float p1[3];
        float p2[3];
        float p3[3];
        pts->GetPoint(0, p0);
        pts->GetPoint(1, p1);
        pts->GetPoint(2, p2);
        pts->GetPoint(3, p3);

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
     
        rv = (volume1 + volume2);
    }
    else if (cellType == VTK_PIXEL)
    {
        float p0[3];
        float p1[3];
        float p2[3];
        float p3[3];
        pts->GetPoint(0, p0);
        pts->GetPoint(1, p1);
        pts->GetPoint(2, p2);
        pts->GetPoint(3, p3);

        double  x[3], y[3];
        double  volume1, volume2;
     
        //
        // Split into two triangles (P0, P1, and P3), and (P0, P2, and P3) and
        // find their volumes.
        //
        x[0] = p0[0];
        y[0] = p0[1];
        x[1] = p3[0];
        y[1] = p3[1];
        x[2] = p1[0];
        y[2] = p1[1];
        volume1 = GetTriangleVolume(x, y);
 
        x[0] = p0[0];
        y[0] = p0[1];
        x[1] = p3[0];
        y[1] = p3[1];
        x[2] = p2[0];
        y[2] = p2[1];
        volume2 = GetTriangleVolume(x, y);
     
        rv = (volume1 + volume2);
    }

    return rv;
}


// ****************************************************************************
//  Function: GetTriangleVolume
// 
//  Purpose:
//      Revolve a triangle around the y-axis.
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
//      Akira Haddox, Wed Jul  2 12:27:24 PDT 2003
//      Dealt with triangles below or on the y-axis.
// 
// ****************************************************************************
 
double
avtRevolvedVolume::GetTriangleVolume(double x[3], double y[3])
{
    int     i, j;
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
            xInt1 = oppositeX + oppositeY / nslope;
        }
            
        if (oppositeX == pt2X)
            xInt2 = oppositeX;
        else
        {
            double nslope = (oppositeY - pt2Y) / (oppositeX - pt2X);
            xInt2 = oppositeX + oppositeY / nslope;
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

        v1 = GetTriangleVolume(x, y);
        
        //
        // Find the volume of the quad by splitting it into triangles 
        // (the intercepts to pt1, then {pt1, pt2, Intercept2}).
        //

        x[0] = pt1X;
        y[0] = pt1Y;
        v2 = GetTriangleVolume(x, y);
        
        x[1] = pt2X;
        y[1] = pt2Y;
        v3 = GetTriangleVolume(x, y);

        return v1 + v2 + v3;
    }
    
    //
    // Sort the points so that they are ordered by x-coordinate.  This will
    // make things much easier later.
    // 
    for (i = 0 ; i < 3 ; i++)
    {
        for (j = i ; j < 3 ; j++)
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
//      around the y-axis.
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
// ****************************************************************************
 
double
avtRevolvedVolume::RevolveLineSegment(double x[2], double y[2], double *slope)
{
    double   m, b;
    double   x_at_y_axis;
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
    // Calculate where the line segment will hit the line x = 0.
    // Note we have already taken care of degenerate cases.
    //
    m = (y[1] - y[0]) / (x[1] - x[0]);
    b = y[0] - m*x[0];
    x_at_y_axis = -b / m;
 
    //
    // We are now going to calculate the cone that contains are volume and
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
        //                      x_at_y_axis
        //                          ||
        //                          \/
        // y-axis  -----------------.--------------------
        //
        radius = y[0];
        height = x_at_y_axis - x[0];
        coneAll = M_PI * radius * radius * height / 3;
 
        radius = y[1];
        height = x_at_y_axis - x[1];
        coneCropped = M_PI * radius * radius * height / 3;
    }
    else  // m > 0 
    {
        //
        //                           . <= P1
        //
        //                       . <= P0
        //         x_at_y_axis
        //             ||
        //             \/
        // y-axis  ----.---------------------------------
        //
        radius = y[1];
        height = x[1] - x_at_y_axis;
        coneAll = M_PI * radius * radius * height / 3;
 
        radius = y[0];
        height = x[0] - x_at_y_axis;
        coneCropped = M_PI * radius * radius * height / 3;
    }
 
    *slope = m;
    return (coneAll - coneCropped);
}


