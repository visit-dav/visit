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

#ifndef BOUNDING_BOX_CONTOURER_H
#define BOUNDING_BOX_CONTOURER_H

// ****************************************************************************
//  Class:  BoundingBoxContourer
//
//  Purpose:
//    Creates OpenGL polygon slices through a bounding box.
//    Currently used for view-aligned slice planes of a 3D texturing
//    volume renderer.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  2, 2003
//
//  Modifications:
//    Jeremy Meredith, Fri Oct 10 16:23:29 PDT 2003
//    Modified the ContourTriangles to return the triangles through arguments.
//
// ****************************************************************************
class BoundingBoxContourer
{
  public:
    float x[8];
    float y[8];
    float z[8];
    float v[8];
    float r[8];
    float s[8];
    float t[8];
    void   ContourTriangles(float value, int &ntriangles,
                            float *tr, float *ts, float *tu,
                            float *vx, float *vy, float *vz);

  private:
    struct Coord
    {
        int   edge;     // 1-12 edge index
        int   vertex0;  // 0-7 vertex index
        int   vertex1;  // 0-7 vertex index
        float alpha;    // 0.0 - 1.0 interpolation parameter

        float x,y,z;    // physical x,y,z coordinate
        float r,s,t;    // texture coordinates
    };

    int    CalculateCase(float value);
    Coord  GetEdgeIsoCoord(int edge, float value);
};

#endif
