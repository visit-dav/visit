/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                                  Cell.h                                   //
// ************************************************************************* //

#ifndef CELL_H
#define CELL_H

#include <math.h>

#include <Frame.h>

//
// No reason to use Silo's centering definitions and force this class to know
// about Silo.
//

typedef enum
{
    NODE_CENTERED,
    ZONE_CENTERED
}  Centering_e;


#define VAR_DIM_MAX 10


// ****************************************************************************
//  Class: Cell
//
//  Purpose:
//      A hexahedral cell and methods defined on it.
//
//  Programmer: Hank Childs
//  Creation:   June 21, 2000 (from structure Cell_t)
//
// ****************************************************************************

class Cell
{
  public:
    inline         Cell();
    
    inline void    GetBounds(float [6]);
    inline bool    IsInCell(const float [3]);
    void           LinearInterpolate(const float [3], float *);
    inline void    SetNodeValue(int, const float *);
    inline void    SetZoneValue(const float *);
    inline void    SetNode(int, float, float, float);
    inline void    SetNDims(int x) { nDims = x; };
    inline int     GetNDims(void)  { return nDims; };

  protected:
    Centering_e    centering;
    int            nDims;
    float          x[8];
    float          y[8];
    float          z[8];
    float          val[8][VAR_DIM_MAX];

    bool           calculatedFaces;
    float          center[3];
    float          faceNormal[6][3];
    float          facePoint[6][3];
    int            faceSide[6];

    float          grad0[VAR_DIM_MAX], grad1[VAR_DIM_MAX], grad2[VAR_DIM_MAX];
    int            oldClosest;

    inline void         CalculateFaces(void);
    int                 CalculateFrameClosestToPoint(Frame &, const float [3]);
    static inline int   PlusSide(const float [3], const float [3], 
                                 const float [3]);
};


// ****************************************************************************
//  Method: Cell constructor
//
//  Programmer: Hank Childs
//  Creation:   June 21, 2000
//
// ****************************************************************************

Cell::Cell()
{
    calculatedFaces = false;
    centering = NODE_CENTERED;
    nDims = 0;
}


// ****************************************************************************
//  Method: Cell::SetNode
//
//  Purpose:
//      Sets what a specific node should be in the cell.
//
//  Arguments:
//      i     The node number (0-7)
//      xf    The x value of node i.
//      yf    The y value of node i.
//      zf    The z value of node i.
//
//  Programmer: Hank Childs
//  Creation:   June 21, 2000
//
// ****************************************************************************

void
Cell::SetNode(int i, float xf, float yf, float zf)
{
    x[i] = xf;
    y[i] = yf;
    z[i] = zf;
    calculatedFaces = false;
    oldClosest = -1;
}


// ****************************************************************************
//  Method: Cell::GetBounds
//
//  Purpose:
//      Gets the bounds of the cell.
//
//  Arguments:
//      bounds      An array to place the bounds into.
//
//  Programmer: Hank Childs
//  Creation:   June 21, 2000
//
// ****************************************************************************

void
Cell::GetBounds(float bounds[6])
{
    bounds[0] = bounds[1] = x[0];
    bounds[2] = bounds[3] = y[0];
    bounds[4] = bounds[5] = z[0];
    for (int i = 1 ; i < 8 ; i++)
    {
        bounds[0] = x[i] < bounds[0] ? x[i] : bounds[0];
        bounds[1] = x[i] > bounds[1] ? x[i] : bounds[1];
        bounds[2] = y[i] < bounds[2] ? y[i] : bounds[2];
        bounds[3] = y[i] > bounds[3] ? y[i] : bounds[3];
        bounds[4] = z[i] < bounds[4] ? z[i] : bounds[4];
        bounds[5] = z[i] > bounds[5] ? z[i] : bounds[5];
    }
}


// ****************************************************************************
//  Method: Cell::IsInCell
//
//  Purpose:
//      Determines if a point is inside the cell.
//
//  Arguments:
//      point     A point to test if it is inside the cell.
//
//  Returns:      True if point is in the cell (or on the boundary), false 
//                otherwise.
//
//  Programmer:   Hank Childs
//  Creation:     June 21, 2000
//
// ****************************************************************************

bool
Cell::IsInCell(const float point[3])
{
    if (! calculatedFaces)
    {
        CalculateFaces();
    }

    for (int face = 0 ; face < 6 ; face++)
    {
        int  pointSide = PlusSide(faceNormal[face], facePoint[face], point);
        if (pointSide != faceSide[face])
        {
            if (pointSide != 0 && faceSide[face] != 0)
            {
                //
                // The point was on a different side of the face than the 
                // center, thus it is not in the cell.
                // 
                return false;
            }
        }
    }

    //
    // The point was on the same side of all the faces as the center, so it
    // is in the cell.
    //
    return true;
}


// ****************************************************************************
//  Method: Cell::CalculateFaces
//
//  Purpose:
//      Calculates the center of the cell and some information about which
//      side of the faces the center is on.
//
//  Programmer: Hank Childs
//  Creation:   June 21, 2000
//
// ****************************************************************************

void
Cell::CalculateFaces(void)
{
    //
    // Calculate the center.  We are assuming the cell is convex by placing the
    // center in its middle.
    //
    center[0] = center[1] = center[2] = 0.;
    for (int i = 0 ; i < 8 ; i++)
    {
        center[0] += x[i] / 8;
        center[1] += y[i] / 8;
        center[2] += z[i] / 8;
    }

    //
    // Calculate the planes that each of the 6 faces lie on.  
    //
    int  planes[6][3] = { 
                            { 0, 1, 2 },   // Bottom
                            { 4, 5, 6 },   // Top
                            { 0, 1, 4 },   // Front
                            { 2, 3, 6 },   // Back
                            { 0, 3, 4 },   // Left
                            { 1, 2, 5 },   // Right
                        };

    for (int face = 0 ; face < 6 ; face++)
    {
        //
        // facePoint should just be a point on the face.  Just use the first
        // one (ie planes[...][0]).
        //
        facePoint[face][0] = x[planes[face][0]];
        facePoint[face][1] = y[planes[face][0]];
        facePoint[face][2] = z[planes[face][0]];

        //
        // We have three points (P0, P1, P2).  The cross product of P1-P0 and
        // P2-P0 will be a normal vector to the plane.
        //
        float P1P0[3];
        P1P0[0] = x[planes[face][1]] - x[planes[face][0]];
        P1P0[1] = y[planes[face][1]] - y[planes[face][0]];
        P1P0[2] = z[planes[face][1]] - z[planes[face][0]];
        float P2P0[3];
        P2P0[0] = x[planes[face][2]] - x[planes[face][0]];
        P2P0[1] = y[planes[face][2]] - y[planes[face][0]];
        P2P0[2] = z[planes[face][2]] - z[planes[face][0]];
        faceNormal[face][0] = P1P0[1]*P2P0[2] - P1P0[2]*P2P0[1];
        faceNormal[face][1] = P1P0[2]*P2P0[0] - P1P0[0]*P2P0[2];
        faceNormal[face][2] = P1P0[0]*P2P0[1] - P1P0[1]*P2P0[0];

        //
        // Use the center to determine what side of the face the cell is on.
        //
        faceSide[face] = PlusSide(faceNormal[face], facePoint[face], center);
    }
 
    calculatedFaces = true;
}


// ****************************************************************************
//  Method: Cell::PlusSide
//
//  Purpose:
//      Determines if the point given is on the plus side of the plane 
//      specified.
//
//  Arguments:
//      normal      The normal of the plane.
//      on_plane    A point on the plane.
//      point       The point to determine if it is on the plus side.
//
//  Returns:     0 if point is on the plane, 1 if it is on the plus side and
//               -1 otherwise.
//
//  Note:        This is a static function.
//
//  Programmer:  Hank Childs
//  Creation:    June 21, 2000
//
// ****************************************************************************

int
Cell::PlusSide(const float normal[3], const float on_plane[3], 
               const float point[3])
{
    //
    // Construct the vector from the plane to point.
    //
    float  v_point[3];
    v_point[0] = point[0] - on_plane[0];
    v_point[1] = point[1] - on_plane[1];
    v_point[2] = point[2] - on_plane[2];

    float  dot = normal[0]*v_point[0] + normal[1]*v_point[1] + 
                 normal[2]*v_point[2];

    float epsilon = 0.0001;
    if (fabs(dot) < epsilon)
    {
        return 0;
    }
    return (dot > 0. ? 1 : -1);
}


// ****************************************************************************
//  Method: Cell::SetNodeValue
//
//  Purpose:
//      Sets the value of a node in the cell.
//
//  Arguments:
//      nI      The index of the node this value corresponds to.
//      v       The value of the node.
//
//  Programmer: Hank Childs
//  Creation:   June 21, 2000
//
// ****************************************************************************

void
Cell::SetNodeValue(int nI, const float *v)
{
    centering = NODE_CENTERED;
    for (int i = 0 ; i < nDims ; i++)
    {
        val[nI][i] = v[i];
    }
}


// ****************************************************************************
//  Method: Cell::SetZoneValue
//
//  Purpose:
//      Sets the value for the cell.
//
//  Arguments:
//      v     The value of the cell.
//
//  Programmer: Hank Childs
//  Creation:   June 21, 2000
//
// ****************************************************************************

void
Cell::SetZoneValue(const float *v)
{
    centering = ZONE_CENTERED;
    for (int i = 0 ; i < nDims ; i++)
    {
        val[0][i] = v[i];
    }
}


#endif


