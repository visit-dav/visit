// ************************************************************************* //
//                                    Cell.C                                 //
// ************************************************************************* //

#include <float.h>
#include <iostream.h>

#include <Cell.h>


// ****************************************************************************
//  Method: Cell::LinearInterpolate
//
//  Purpose:
//      Linearly interpolates the value at the specified point based on the
//      value of the points in the cell (node centered) or the value of the
//      cell itself (zone-centered -- no interpolation is performed).
//
//  Arguments:
//      point      The point to interpolate to.
//      iv         The interpolated value at that point.
//
//  Programmer: Hank Childs
//  Creation:   June 21, 2000
//
// ****************************************************************************

void
Cell::LinearInterpolate(const float point[3], float *iv)
{
    if (centering == ZONE_CENTERED)
    {
        for (int i = 0 ; i < nDims ; i++)
        {
            iv[i] = val[0][i];
        }
    }
    else 
    {
        //
        // Each time the Frame is set, it is entirely re-initialized, so make
        // this static to avoid paying the cost of the constructor each time.
        //
        static Frame  f;
        int closestPoint = CalculateFrameClosestToPoint(f, point);

        float  u, v, w;
        f.GetCoordsInFrame(point, u, v, w);

        //
        // We have the coordinates in the new frame <v0, v1, v2, origin>.  Use
        // them to interpolate the value.  The base value is at origin.  
        //
        for (int i = 0 ; i < nDims ; i++)
        {
            iv[i] = val[closestPoint][i] + u*grad0[i] + v*grad1[i] + w*grad2[i];
        }
    }
}


// ****************************************************************************
//  Method: Cell::CalculateFrameClosestToPoint
//
//  Purpose:
//      Determines the new frame.  To prevent striding effects, the frame's
//      origin will be the closest point in the cell.  The basis vectors will
//      be the three edges on the cell that contain the origin.
//
//  Arguments:
//      f       The frame to set.
//      point   The point that will be interpolated to later.
//
//  Returns:    The index of the cell's vertex closest to point.
//
//  Programmer: Hank Childs
//  Creation:   June 23, 2000
//
// ****************************************************************************

int
Cell::CalculateFrameClosestToPoint(Frame &f, const float point[3])
{
    int    i;
    float  closest      = FLT_MAX;
    int    closestIndex = -1;
    for (i = 0 ; i < 8 ; i++)
    {
        float distSquared = (x[i]-point[0])*(x[i]-point[0]) 
                          + (y[i]-point[1])*(y[i]-point[1]) 
                          + (z[i]-point[2])*(z[i]-point[2]);
        if (distSquared < closest)
        {
            closest = distSquared;
            closestIndex = i;
        }
    }

    int  origin = closestIndex;
    if (origin == oldClosest)
    {
        //
        // The frame has already been set for this case.
        //
        return origin;
    }

    int  edgeNeighbors[8][3] = {
                                  { 1, 3, 4 },  // Share edge with 0
                                  { 0, 2, 5 },  // Share edge with 1
                                  { 1, 3, 6 },  // Share edge with 2
                                  { 0, 2, 7 },  // Share edge with 3
                                  { 0, 5, 7 },  // Share edge with 4
                                  { 1, 4, 6 },  // Share edge with 5
                                  { 2, 5, 7 },  // Share edge with 6
                                  { 3, 4, 6 },  // Share edge with 7
                               };

    int  *b = edgeNeighbors[origin];

    //
    // Determine the basis vectors - the direction towards each of the
    // neighbors.
    //
    float basis0[3], basis1[3], basis2[3], or[3];
    basis0[0] = x[b[0]] - x[origin];
    basis0[1] = y[b[0]] - y[origin];
    basis0[2] = z[b[0]] - z[origin];

    basis1[0] = x[b[1]] - x[origin];
    basis1[1] = y[b[1]] - y[origin];
    basis1[2] = z[b[1]] - z[origin];

    basis2[0] = x[b[2]] - x[origin];
    basis2[1] = y[b[2]] - y[origin];
    basis2[2] = z[b[2]] - z[origin];

    //
    // The origin for the frame is just at the point.
    //
    or[0] = x[origin];
    or[1] = y[origin];
    or[2] = z[origin];

    f.SetFrame(basis0, basis1, basis2, or);

    //
    // All of the gradients should be in terms of change in value over change
    // in distance.  The gradients should be in the directions of the basis
    // vectors.
    //
    for (i = 0 ; i < nDims ; i++)
    {
        grad0[i] = val[b[0]][i] - val[origin][i];
        grad1[i] = val[b[1]][i] - val[origin][i];
        grad2[i] = val[b[2]][i] - val[origin][i];
    }

    oldClosest = origin;
    return origin;
}


