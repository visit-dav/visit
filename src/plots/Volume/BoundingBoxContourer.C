/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                             BoundingBoxContourer.C                        //
// ************************************************************************* //

#include "BoundingBoxContourer.h"

#if defined(_WIN32)
#include <windows.h>
#endif

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
//  Return Arguments:
//    ntriangles   the final number of triangles
//    tr,ts,tt     the texture coordinates of each node
//    vx,vy,vz     the vertex coordinates of each node
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  2, 2003
//
//  Modifications:
//    Jeremy Meredith, Fri Oct 10 16:23:29 PDT 2003
//    Modified the ContourTriangles to return the triangles through arguments.
//
// ****************************************************************************
void
BoundingBoxContourer::ContourTriangles(float value, int   &ntriangles,
                                       float *tr, float *ts, float *tt,
                                       float *vx, float *vy, float *vz)
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

            tr[triangle*3 + node] = c.r;
            ts[triangle*3 + node] = c.s;
            tt[triangle*3 + node] = c.t;

            vx[triangle*3 + node] = c.x;
            vy[triangle*3 + node] = c.y;
            vz[triangle*3 + node] = c.z;
        }
        triangle++;
    }

    ntriangles = triangle;
}

