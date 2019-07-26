// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtPlane.h>

// ****************************************************************************
//  Function: PlaneIntersectsCube
//
//  Purpose:
//      Determines if a plane intersects a cube.
//
//  Arguments:
//      plane   The equation of a plane as (A,B,C,D).
//      bounds  The bounds of a cube as (minx,maxx,miny,maxy,minz,maxz).
//
//  Returns:    True if the plane intersects the cube, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   August 5, 2002
//
// ****************************************************************************

bool
PlaneIntersectsCube(double plane[4], double bounds[6])
{
    bool has_low_point  = false;
    bool has_high_point = false;
    for (int i = 0 ; i < 8 ; i++)
    {
        double x = (i&1 ? bounds[1] : bounds[0]);
        double y = (i&2 ? bounds[3] : bounds[2]);
        double z = (i&4 ? bounds[5] : bounds[4]);
        double val = plane[3] - plane[0]*x - plane[1]*y - plane[2]*z;

        if (val == 0.)  // If we are on the plane, intersect
            return true;

        if (val < 0)
            has_low_point = true;
        else
            has_high_point = true;

        if (has_low_point && has_high_point)
            return true;
    }

    return false;
}
