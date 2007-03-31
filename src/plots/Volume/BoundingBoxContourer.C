// ************************************************************************* //
//                             BoundingBoxContourer.C                        //
// ************************************************************************* //

#include "BoundingBoxContourer.h"

#if defined(_WIN32)
#include <windows.h>
#endif
#include <GL/gl.h>
#include <float.h>

#include <vtkTriangulationTables.h>

// ****************************************************************************
//  Method:  BoundingBoxContourer::GetEdgeIsoCoord
//
//  Purpose:
//   1) Calculate the point of intersection along an edge
//   2) Interpolate that value as an xyz coordinate
//
//  Arguments:
//    edge       the edge index of a hex (0-11)
//    value      the actual value of the isosurface
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  2, 2003
//
// ****************************************************************************
BoundingBoxContourer::Coord
BoundingBoxContourer::GetEdgeIsoCoord(int edge, float value)
{
    Coord c;

    int va = hexVerticesFromEdges[edge][0];
    int vb = hexVerticesFromEdges[edge][1];
    float alpha = (value - this->v[va]) / (this->v[vb] - this->v[va]);
    float ialpha;

    // we know we need an intersection along this edge, so clamp it
    // to be between the two endpoints (0 and 1)
    if (alpha>1) alpha=1;
    if (alpha<0) alpha=0;
    ialpha = 1.0 - alpha;

    // do the linear interpolation
    c.edge    = edge;
    c.alpha   = alpha;
    c.vertex0 = va;
    c.vertex1 = vb;

    c.x = (this->x[va] * ialpha) + (this->x[vb] * alpha);
    c.y = (this->y[va] * ialpha) + (this->y[vb] * alpha);
    c.z = (this->z[va] * ialpha) + (this->z[vb] * alpha);
    c.r = (this->r[va] * ialpha) + (this->r[vb] * alpha);
    c.s = (this->s[va] * ialpha) + (this->s[vb] * alpha);
    c.t = (this->t[va] * ialpha) + (this->t[vb] * alpha);

    return c;
}

// ****************************************************************************
//  Method:  BoundingBoxContourer::CalculateCase
//
//  Purpose:
//    Determine which contouring case a given value will induce.
//
//  Arguments:
//    value      the value to contour at
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  2, 2003
//
// ****************************************************************************
int
BoundingBoxContourer::CalculateCase(float value)
{
    int i;
    int hexcase = 0x00;
    for (i=0; i<8; i++)
    {
        if (this->v[i] > value)
            hexcase |= (1 << i);
    }

    return hexcase;
}

// ****************************************************************************
//  Method:  BoundingBoxContourer::ContourTriangles
//
//  Purpose:
//    Draw the triangles from a contour on the screen using the
//    r/s/t values as 3D texture coordinates.
//
//  Arguments:
//    value      the current contouring value
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  2, 2003
//
// ****************************************************************************
//
//   Contour as individual triangles
//
void
BoundingBoxContourer::ContourTriangles(float value)
{
    int *casepointer;

    int hexcase = CalculateCase(value);
    casepointer = hexTriangulationTable[hexcase];

    int triangle = 0;
    while (*casepointer >= 0)
    {
        for (int node = 0; node < 3; node++)
        {
            int edge = *casepointer++;
            Coord c  = GetEdgeIsoCoord(edge, value);
            glTexCoord3f(c.r,c.s,c.t);
            glVertex3f(c.x,c.y,c.z);
        }
        triangle++;
    }
}

