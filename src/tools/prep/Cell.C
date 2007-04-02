/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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
//                                    Cell.C                                 //
// ************************************************************************* //

#include <float.h>
#include <visitstream.h>

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


