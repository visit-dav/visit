// ************************************************************************* //
//                               avtExtractor.C                              //
// ************************************************************************* //

#include <avtExtractor.h>

#include <float.h>
#include <math.h>

#include <avtVolume.h>
#include <avtCellList.h>


const float  avtExtractor::FRUSTUM_MIN_X = -1.;
const float  avtExtractor::FRUSTUM_MAX_X = +1.;
const float  avtExtractor::FRUSTUM_MIN_Y = -1.;
const float  avtExtractor::FRUSTUM_MAX_Y = +1.;
const float  avtExtractor::FRUSTUM_MIN_Z = 0.;
const float  avtExtractor::FRUSTUM_MAX_Z = +1.;


// ****************************************************************************
//  Method: avtExtractor constructor
//
//  Arguments:
//     w     The number of sample points in the x direction (width).
//     h     The number of sample points in the y direction (height).
//     d     The number of sample points in the z direction (depth).
//     vol   The volume to put sample points into.
//     cl    A place to store cells to defer sampling.
//
//  Programmer:  Hank Childs
//  Creation:    December 5, 2000
//
// ****************************************************************************

avtExtractor::avtExtractor(int w, int h, int d, avtVolume *vol,avtCellList *cl)
{
    width  = w;
    height = h;
    depth  = d;
    restrictedMinWidth  = 0;
    restrictedMaxWidth  = width-1;
    restrictedMinHeight = 0;
    restrictedMaxHeight = height-1;
    volume   = vol;
    celllist = cl;
    x_step = (FRUSTUM_MAX_X - FRUSTUM_MIN_X) / (width-1);
    y_step = (FRUSTUM_MAX_Y - FRUSTUM_MIN_Y) / (height-1);
    z_step = (FRUSTUM_MAX_Z - FRUSTUM_MIN_Z) / (depth-1);
    sendCellsMode = false;
    tmpSampleList = new float[depth][AVT_VARIABLE_LIMIT];
}


// ****************************************************************************
//  Method: avtExtractor destructor
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2001
//
// ****************************************************************************

avtExtractor::~avtExtractor()
{
    if (tmpSampleList != NULL)
    {
        delete [] tmpSampleList;
    }
}


// ****************************************************************************
//  Method: avtExtractor::ExtractTriangle
//
//  Purpose:
//      Given a triangle, extract the sample points from it and put them in the
//      avtVolume.
//
//  Arguments:
//      xi        The index of the x-plane this triangle lies on.
//      const_y   The y-coordinates of the vertices of the triangle.
//      const_z   The z-coordinates of the vertices of the triangle.
//      const_v   The value at each vertex of the triangle.
//      nVars     The number of variables in const_v.
//
//  Programmer: Hank Childs
//  Creation:   December 6, 2000
//
//  Modifications:
//
//    Hank Childs, Sat Feb  3 15:59:58 PST 2001
//    Account for slicing methods that now takes x-slices instead of z-slices.
//
//    Hank Childs, Thu Apr  5 14:18:38 PDT 2001
//    Fix degenerate case where triangle is "flat" in z and the side
//    corresponds to a slice of the volume.
//
//    Hank Childs, Tue Nov 13 16:54:47 PST 2001
//    Allow for multiple variables.
//
//    Hank Childs, Mon Nov 19 17:05:18 PST 2001
//    Put in a work-around for numerical errors.
//
//    Hank Childs, Tue Feb 26 11:29:26 PST 2002
//    Pick up another case for numerical errors.
//
//    Hank Childs, Tue Dec 21 11:30:28 PST 2004
//    Change test for 'out of frustum' to accomodate tiling.
//
// ****************************************************************************

void
avtExtractor::ExtractTriangle(int xi, const float const_y[3],
 const float const_z[3], const float const_v[3][AVT_VARIABLE_LIMIT], int nVars)
{
    //
    // Make our own version of the triangle we can play with.
    //
    float y[3];
    float z[3];
    float v[3][AVT_VARIABLE_LIMIT];
    for (int i = 0 ; i < 3 ; i++)
    {
        y[i] = const_y[i];
        z[i] = const_z[i];
        for (int j = 0 ; j < nVars ; j++)
        {
            v[i][j] = const_v[i][j];
        }
    }

    //
    // Orient the triangle so it is in a layout we know how to deal with.
    // The vertex with the smallest y-value will be placed in position 0 and
    // the biggest in position 2.
    //
    OrientTriangle(z, y, v, nVars);

    //
    // We could be out of the region we are interested in finding sample
    // points for, so lets make an explicit check.
    //
    float smallest_y = YFromIndex(restrictedMinHeight);
    float biggest_y = YFromIndex(restrictedMaxHeight);
    if (y[0] > biggest_y || y[2] < smallest_y)
    {
        return;
    }

    //
    // To make sure that there are no degenerate cases, split the triangle 
    // across the line y=y[1].  Do one triangle (referred to as V2V1) going
    // from the top vertex to the line y=y[1].  Do another (referred to as
    // V0V1) going from the bottom vertex to the same line.
    //
    int   y_top, y_bottom;
    int   yi;

    //
    // Do V2V1.  We can ignore the case if it falls between our samples
    // (y_top < y_bottom) or if the line is exactly on our sampling strip.
    // We can ignore it in the latter case, because we will cover it in V0V1.
    //
    y_top    = SnapYBottom(y[2]);
    y_bottom = SnapYTop(y[1]);
    if (y_bottom <= y_top && y[2] != y[1] && y[1] < biggest_y)
    {
        double z_slope_12 = (z[1] - z[2]) / fabs((y[2] - y[1]));
        double z_slope_02 = (z[0] - z[2]) / fabs((y[2] - y[0]));
        double v_slope_12[AVT_VARIABLE_LIMIT];
        double v_slope_02[AVT_VARIABLE_LIMIT];
        bool numericalErrorCandidate = false;
        if (z_slope_12 > 1e+10 || z_slope_12 < -1e+10 ||
            z_slope_02 > 1e+10 || z_slope_02 < -1e+10)
        {
            numericalErrorCandidate = true;
        }
        if (fabs(y[2]-y[1]) < 1e-5 || fabs(y[1]-y[0]) < 1e-5)
        {
            numericalErrorCandidate = true;
        }
        
        for (int k = 0 ; k < nVars ; k++)
        {
            v_slope_12[k] = (v[1][k] - v[2][k]) / fabs((y[2] - y[1]));
            v_slope_02[k] = (v[0][k] - v[2][k]) / fabs((y[2] - y[0]));
        }
        float cur_v_12[AVT_VARIABLE_LIMIT];
        float cur_v_02[AVT_VARIABLE_LIMIT];
        for (yi = y_top ; yi >= y_bottom ; yi--)
        {
            float curY = YFromIndex(yi);
            if (numericalErrorCandidate)
            {
                if (y[2] < curY && y[1] < curY)
                    continue;
                if (y[2] > curY && y[1] > curY)
                    continue;
            }
            float dist = y[2] - curY;
            float cur_z_12 = z[2] + z_slope_12*dist;
            float cur_z_02 = z[2] + z_slope_02*dist;
            for (int k = 0 ; k < nVars ; k++)
            {
                cur_v_12[k] = v[2][k] + v_slope_12[k]*dist;
                cur_v_02[k] = v[2][k] + v_slope_02[k]*dist;
            }
            ExtractLine(xi, yi, cur_z_02, cur_z_12, cur_v_02, cur_v_12, nVars);
        }
    }

    //
    // Do V0V1.  We can ignore the case if it falls between our samples
    // (y_top < y_bottom) or if the line is exactly on our sampling strip.
    // We can ignore it in the latter case, because we covered it in V1V2.
    //
    y_top    = SnapYBottom(y[1]);
    y_bottom = SnapYTop(y[0]);
    if (y_bottom <= y_top && y[0] != y[1] && y[1] > smallest_y)
    {
        double z_slope_01 = (z[1] - z[0]) / fabs((y[1] - y[0]));
        double z_slope_02 = (z[2] - z[0]) / fabs((y[0] - y[2]));
        double v_slope_01[AVT_VARIABLE_LIMIT];
        double v_slope_02[AVT_VARIABLE_LIMIT];
        bool numericalErrorCandidate = false;
        if (z_slope_01 > 1e+10 || z_slope_01 < -1e+10 ||
            z_slope_02 > 1e+10 || z_slope_02 < -1e+10)
        {
            numericalErrorCandidate = true;
        }
        if (fabs(y[2]-y[1]) < 1e-5 || fabs(y[1]-y[0]) < 1e-5)
        {
            numericalErrorCandidate = true;
        }
        for (int k = 0 ; k < nVars ; k++)
        {
            v_slope_01[k] = (v[1][k] - v[0][k]) / fabs((y[1] - y[0]));
            v_slope_02[k] = (v[2][k] - v[0][k]) / fabs((y[0] - y[2]));
        }
        float cur_v_01[AVT_VARIABLE_LIMIT];
        float cur_v_02[AVT_VARIABLE_LIMIT];
        for (yi = y_bottom ; yi <= y_top ; yi++)
        {
            float curY = YFromIndex(yi);
            if (numericalErrorCandidate)
            {
                if (y[0] < curY && y[1] < curY)
                    continue;
                if (y[0] > curY && y[1] > curY)
                    continue;
            }
            float dist = curY - y[0];
            float cur_z_01 = z[0] + z_slope_01*dist;
            float cur_z_02 = z[0] + z_slope_02*dist;
            for (int k = 0 ; k < nVars ; k++)
            {
                cur_v_01[k] = v[0][k] + v_slope_01[k]*dist;
                cur_v_02[k] = v[0][k] + v_slope_02[k]*dist;
            }
            ExtractLine(xi, yi, cur_z_01, cur_z_02, cur_v_01, cur_v_02, nVars);
        }
    }
}


// ****************************************************************************
//  Method: avtExtractor::ExtractLine
//
//  Purpose:
//      Takes two endpoints and gets the sample points on that line.
//
//  Arguments:
//      xi      The current x-index.
//      yi      The current y-index.
//      z1      One endpoint of the line.
//      z2      The other endpoint of the line.
//      v1      The value at z1.
//      v2      The value at z2.
//      nVars   The number of variables in v1 and v2.
//
//  Programmer: Hank Childs
//  Creation:   December 6, 2000
//
//  Modifications:
//
//    Hank Childs, Sat Feb  3 16:05:57 PST 2001
//    Modified to have the line go across z instead of x.
//
//    Hank Childs, Thu Apr  5 14:43:59 PDT 2001
//    Added check for degenerate lines.
//
//    Hank Childs, Tue Nov 13 16:54:47 PST 2001
//    Allow for multiple variables.
//
// ****************************************************************************

void
avtExtractor::ExtractLine(int xi, int yi, float z1, float z2, 
                          float v1[AVT_VARIABLE_LIMIT], 
                          float v2[AVT_VARIABLE_LIMIT], int nVars)
{
    //
    // This is easiest if we know z1 < z2.
    //
    if (z1 > z2)
    {
        float t;
        t = z1; z1 = z2; z2 = t;
        for (int j = 0 ; j < nVars ; j++)
        {
            t = v1[j]; v1[j] = v2[j]; v2[j] = t;
        }
    }

    //
    // We could be out of the region we are interested in finding sample
    // points for, so lets make an explicit check.
    //
    if (z1 > FRUSTUM_MAX_Z || z2 < FRUSTUM_MIN_Z)
    {
        return;
    }

    int  frontZ  = SnapZBack(z1);
    int  backZ   = SnapZFront(z2);

    float var_slope[AVT_VARIABLE_LIMIT];
    if (z1 != z2)
    {
        for (int j = 0 ; j < nVars ; j++)
        {
            var_slope[j] = (v2[j] - v1[j]) / (z2 - z1);
        }
    }
    else
    {
        //
        // If the two z-coordinates are equal, we don't need to worry about
        // interpolating, so assuming a zero slope is okay.
        //
        for (int j = 0 ; j < nVars ; j++)
        {
            var_slope[j] = 0.;
        }
    }

    int count = 0;
    for (int zi = frontZ ; zi <= backZ ; zi++)
    {
        float z = ZFromIndex(zi);
        float dist = z - z1;
        for (int j = 0 ; j < nVars ; j++)
        {
            float v = var_slope[j]*dist + v1[j];
            tmpSampleList[count][j] = v;
        }
        count++;
    }

    if (count > 0)
    {
        avtRay *ray = volume->GetRay(xi, yi);
        ray->SetSamples(frontZ, backZ, tmpSampleList);
    }
}


// ****************************************************************************
//  Method: avtExtractor::OrientTriangle
//
//  Purpose:
//      Puts the triangle in a layout that is easy for the ExtractTriangle
//      routine.  This puts the smallest y intercept in position 0 and the
//      biggest in position 2.
//
//  Arguments:
//      x       The x-coordinates for the vertices of a triangle.
//      y       The y-coordinates for the vertices of a triangle.
//      v       The variable defined on the vertices of a triangle.
//      nVars   The number of variables.
//
//  Programmer: Hank Childs
//  Creation:   December 6, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Nov 13 16:54:47 PST 2001
//    Added support for multiple variables.
//
// ****************************************************************************

void
avtExtractor::OrientTriangle(float x[3], float y[3],
                             float v[3][AVT_VARIABLE_LIMIT], int nVars)
{
    float t;

    //
    // Put the point with the lowest y-intercept in position 0
    //
    if (y[1] < y[0])
    {
        t = y[1]; y[1] = y[0]; y[0] = t;
        t = x[1]; x[1] = x[0]; x[0] = t;
        for (int j = 0 ; j < nVars ; j++)
        {
            t = v[1][j]; v[1][j] = v[0][j]; v[0][j] = t;
        }
    }
    if (y[2] < y[0])
    {
        t = y[2]; y[2] = y[0]; y[0] = t;
        t = x[2]; x[2] = x[0]; x[0] = t;
        for (int j = 0 ; j < nVars ; j++)
        {
            t = v[2][j]; v[2][j] = v[0][j]; v[0][j] = t;
        }
    }

    //
    // Only need to compare to pos 1, since pos 0 is guaranteed smallest
    //
    if (y[1] > y[2])
    {
        t = y[2]; y[2] = y[1]; y[1] = t;
        t = x[2]; x[2] = x[1]; x[1] = t;
        for (int j = 0 ; j < nVars ; j++)
        {
            t = v[2][j]; v[2][j] = v[1][j]; v[1][j] = t;
        }
    }
 
    //
    // Now do degenerative cases where y's =
    //
    if ( (y[1] == y[2]) && (x[1] > x[2]) )
    {
        t = x[2]; x[2] = x[1]; x[1] = t;
        for (int j = 0 ; j < nVars ; j++)
        {
            t = v[2][j]; v[2][j] = v[1][j]; v[1][j] = t;
        }
    }
    if ( (y[0] == y[1]) && (x[0] > x[1]) )
    {
        t = x[1]; x[1] = x[0]; x[0] = t;
        for (int j = 0 ; j < nVars ; j++)
        {
            t = v[1][j]; v[1][j] = v[0][j]; v[0][j] = t;
        }
    }
}


// ****************************************************************************
//  Method: avtExtractor::SendCellsMode
//
//  Purpose:
//      Tells the extractor whether it can send the cells instead of finding
//      the sample points.
//
//  Arguments:
//      mode     true if we can send cells instead of points, false otherwise.
//
//  Programmer:  Hank Childs
//  Creation:    January 27, 2001
//
// ****************************************************************************

void
avtExtractor::SendCellsMode(bool mode)
{
    sendCellsMode = mode;
}


// ****************************************************************************
//  Method: avtExtractor::ConstructBounds
//
//  Purpose:
//      Determines the bounding box for the current cell and which sample
//      points its extrema corresponds to.
//
//  Arguments:
//      pts      A point list.
//      npts     The number of points in pts.
//
//  Returns:     The number of sample points covered by the cells bounding box.
//
//  Programmer:  Hank Childs
//  Creation:    January 27, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Dec 21 11:30:28 PST 2004
//    Change test for 'out of frustum' to accomodate tiling.
//
// ****************************************************************************

int
avtExtractor::ConstructBounds(const float (*pts)[3], int npts)
{
    float fminx = +FLT_MAX;
    float fmaxx = -FLT_MAX;
    float fminy = +FLT_MAX;
    float fmaxy = -FLT_MAX;
    float fminz = +FLT_MAX;
    float fmaxz = -FLT_MAX;

    for (int i = 0 ; i < npts ; i++)
    {
        if (pts[i][0] < fminx)
        {
            fminx = pts[i][0];
        }
        if (pts[i][0] > fmaxx)
        {
            fmaxx = pts[i][0];
        }
        if (pts[i][1] < fminy)
        {
            fminy = pts[i][1];
        }
        if (pts[i][1] > fmaxy)
        {
            fmaxy = pts[i][1];
        }
        if (pts[i][2] < fminz)
        {
            fminz = pts[i][2];
        }
        if (pts[i][2] > fmaxz)
        {
            fmaxz = pts[i][2];
        }
    }

    //
    // We can get snapped to the frustum if we are outside it, so explicitly
    // check for this.
    //
    float smallest_x = XFromIndex(restrictedMinWidth);
    float biggest_x = XFromIndex(restrictedMaxWidth);
    float smallest_y = YFromIndex(restrictedMinHeight);
    float biggest_y = YFromIndex(restrictedMaxHeight);
    if (fmaxx < smallest_x || fminx > biggest_x ||
        fmaxy < smallest_y || fminy > biggest_y ||
        fmaxz < FRUSTUM_MIN_Z || fminz > FRUSTUM_MAX_Z)
    {
        return 0;
    }

    //
    // We don't want to worry about planes that don't intersect this
    // cell, so when finding the min, round up and when going towards the
    // max, round down.
    //
    minx = SnapXRight(fminx);
    maxx = SnapXLeft(fmaxx);
    miny = SnapYTop(fminy);
    maxy = SnapYBottom(fmaxy);
    minz = SnapZBack(fminz);
    maxz = SnapZFront(fmaxz);

    if (minx > maxx)
        return 0;
    if (miny > maxy)
        return 0;
    if (minz > maxz)
        return 0;

    //
    // Return the number of samples in the bounding box.
    //
    return (maxx - minx + 1)*(maxy - miny + 1)*(maxz - minz + 1);
}


// ****************************************************************************
//  Method: avtExtractor::ContributeSmallCell
//
//  Purpose:
//      Finds the closest sample point to a small cell and submits its variable
//      values to that sample point (where an arbitrator may or may not accept
//      them as the "best").
//
//  Arguments:
//      pts     The points.
//      vals    The variable values.
//      npts    The number of points.
//
//  Programmer: Hank Childs
//  Creation:   January 23, 2002
//
//  Modifications:
//
//    Hank Childs, Tue Dec 21 11:30:28 PST 2004
//    Change test for 'out of frustum' to accomodate tiling.
//
// ****************************************************************************

void
avtExtractor::ContributeSmallCell(const float (*pts)[3],
                             const float (*vals)[AVT_VARIABLE_LIMIT], int npts)
{
    //
    // Note that this assumes the cell is very small, that it does not span
    // multiple sample points.
    //
    // The calling functions don't actually check for this -- they just see if
    // it intersects any of the sample points.  These two measures typically
    // coincide, but they don't when we have cells with high aspect ratios.
    //
    // In practice, this doesn't come up very much and is only noticable in
    // rare cases when we are resampling onto a very small rectilinear grid
    // (often for the preview method -- hardware accelerated).
    //
    for (int i = 0 ; i < npts ; i++)
    {
        float smallest_x = XFromIndex(restrictedMinWidth);
        float biggest_x = XFromIndex(restrictedMaxWidth);
        float smallest_y = YFromIndex(restrictedMinHeight);
        float biggest_y = YFromIndex(restrictedMaxHeight);
        if (pts[i][0] < smallest_x || pts[i][0] > biggest_x ||
            pts[i][1] < smallest_y || pts[i][1] > biggest_y ||
            pts[i][2] < FRUSTUM_MIN_Z || pts[i][2] > FRUSTUM_MAX_Z)
        {
            continue;
        }

        //
        // Identify the closest sample.  It doesn't really matter which of the
        // eight surrounding samples we choose as long as we have a policy.
        //
        int x = SnapXRight(pts[i][0]);
        int y = SnapYTop(pts[i][1]);
        int z = SnapZBack(pts[i][2]);

        avtRay *ray = volume->GetRay(x, y);
        ray->SetSample(z, vals[i]);
    }
}


// ****************************************************************************
//  Method: avtExtractor::Restrict
//
//  Purpose:
//      Restricts the area that the volume will operate on to a portion of
//      the original volume.  This is only used for parallel settings.  This
//      is done because some cells may have some sample points on this
//      processor and _many_ that are not and restricting the area of
//      interest allows us to play some games with the frustum to make sure
//      they don't get calculated.
//
//  Arguments:
//      minw    The new minimum width.
//      maxw    The new maximum width.
//      minh    The new minimum height.
//      maxh    The new maximum height.
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2001
//
// ****************************************************************************

void
avtExtractor::Restrict(int minw, int maxw, int minh, int maxh)
{
    restrictedMinWidth  = minw;
    restrictedMaxWidth  = maxw;
    restrictedMinHeight = minh;
    restrictedMaxHeight = maxh;
}


// ****************************************************************************
//  Method: avtExtractor::IndexToTriangulationTable
//
//  Purpose:
//      Determines what vertices are above and below the plane and returns
//      the correct index to the triangulation table.
//
//  Arguments:
//      pts     The points for the cell.
//      npts    The number of points in pts.
//      x       The x-plane (1*X + 0*Y + 0*Z = x) to intersect the plane by.
//
//  Returns:    The index into the triangulation table.
//
//  Programmer: Hank Childs
//  Creation:   December 6, 2000
//
//  Modifications:
//
//    Hank Childs, Sat Feb  3 15:53:44 PST 2001
//    Pushed down to base class and sliced by x instead of z.
//
// ****************************************************************************

int
avtExtractor::IndexToTriangulationTable(const float (*pts)[3],int npts,float x)
{
    int triIndex = 0;
    for (int i = npts-1 ; i >= 0 ; i--)
    {
        //
        // Make this greater than or equal to, so we catch cases were the plane
        // coincides with one of the faces.
        //
        triIndex <<= 1;
        if (pts[i][0] >= x)
        {
            triIndex |= 1;
        }
    }

    return triIndex;
}


