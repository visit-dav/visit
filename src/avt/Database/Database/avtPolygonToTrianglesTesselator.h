/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

#ifndef AVT_POLYGON_TO_TRIANGLES_TESSELATOR_H
#define AVT_POLYGON_TO_TRIANGLES_TESSELATOR_H

#include <vector>
#include "database_exports.h"


// ****************************************************************************
//  Class:  avtPolygonToTrianglesTesselator
//
//  Purpose: Uses tess2 to create triangles from polygon contours.
//
//  Programmer:  Cyrus Harrison
//  Creation:    Tue Oct  9 12:51:23 PDT 2012
//
//  Modifications:
//
// ****************************************************************************

class VertexManager;
class vtkPoints;
class vtkPolyData;
struct TESSalloc;
struct TESStesselator;

//
// Note on Cyrus' spelling quandry: tesselator vs tessellator
// tess2 uses "tesselator" - for "Tessellate",  two 'l's
// is correct, but I not sure about "tesselator" - so I stuck with one 'l'
// for class names.
//


class DATABASE_API avtPolygonToTrianglesTesselator
{
  public:
             avtPolygonToTrianglesTesselator(vtkPoints *);
    virtual ~avtPolygonToTrianglesTesselator();

    // set normal use for tessellation
    void    SetNormal(double x, double y, double z);
    void    SetNormal(const double *vals);

    // define a contour
    void    BeginContour();
    void    AddContourVertex(double *vals);
    void    AddContourVertex(double x, double y, double z);
    void    EndContour();

    // access to vertex point ids
    int     GetVertexId(double *vals);

    // exec tessellation
    int     Tessellate();

    // this variant adds the generated tris to vtkPolyData
    int     Tessellate(vtkPolyData *pd);

    // these provide access results of last Tessellate call
    int     GetNumberOfTriangles() const;
    void    GetTriangleIndices(int i, int &a, int &b, int &c) const;

  private:
    // normal vector used for tessellation
    double               tessNorm[3];

    // holds verts for current contour
    std::vector<double>  verts;

    // bookkeeping for tess2 memory allocator
    int                  tessMemAllocated;

    // tess2 objs
    TESSalloc           *tessMemAllocator;
    TESStesselator      *tessObj;

    // used to take care of dup verts
    VertexManager       *vertexManager;

    // ref to external vtkPoints object
    vtkPoints           *xPoints;
};



#endif

