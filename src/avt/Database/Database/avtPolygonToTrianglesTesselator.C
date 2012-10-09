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

// ************************************************************************* //
//                   avtPolygonToTrianglesTesselator.C                       //
// ************************************************************************* //
#include <DebugStream.h>
#include <stdlib.h>
#include <tesselator.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <snprintf.h>
#include <vector>
#include <map>

#include <tesselator.h>
#include <avtPolygonToTrianglesTesselator.h>


// ****************************************************************************
// Method: tess_std_free
//
// Purpose:
//   Alloc callback used by the tess2 memory allocator.
//
// Notes:
//
// Programmer: Cyrus Harrison
// Creation:   Tue Oct  9 12:51:23 PDT 2012
//
// Modifications:
//
// ****************************************************************************
void *
tess_std_alloc(void* userData, unsigned int size)
{
    int* allocated = ( int*)userData;
    *allocated += (int)size;
    return malloc(size);
}


// ****************************************************************************
// Method: tess_std_free
//
// Purpose:
//   Free callback used by the tess2 memory allocator.
//
// Notes:
//
// Programmer: Cyrus Harrison
// Creation:   Tue Oct  9 12:51:23 PDT 2012
//
// Modifications:
//
// ****************************************************************************
void
tess_std_free(void* userData, void* ptr)
{
    free(ptr);
}

// ****************************************************************************
// Class: VertexManager
//
// Purpose:
//   This class manages inserts into a vtkPoints object to make sure that the
//   points are unique.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 7 08:58:03 PDT 2007
//
// Modifications:
//
// ****************************************************************************
class VertexManager
{
public:
    VertexManager(vtkPoints *pts)
    {
        points = pts;
    }

    ~VertexManager()
    {
    }

    int GetVertexId(const double *vert)
    {
        char keystr[200];
        SNPRINTF(keystr, 200, "%12.12e,%12.12e,%12.12e", vert[0], vert[1], vert[2]);
        std::string key(keystr);
        int ret;
        std::map<std::string, int>::const_iterator pos =
            vertexNamesToIndex.find(key);
        if(pos != vertexNamesToIndex.end())
        {
            ret = pos->second;
        }
        else
        {
            int index = points->GetNumberOfPoints();
            points->InsertNextPoint((const double *)vert);

            vertexNamesToIndex[key] = index;
            ret = index;
        }
        return ret;
    }

private:
    vtkPoints                  *points; // Does not own this pointer.
    std::map<std::string, int>  vertexNamesToIndex;
};


// ****************************************************************************
// Method: avtPolygonToTrianglesTesselator Constructor
//
//
// Purpose:
//   Creates an instance of avtPolygonToTrianglesTesselator, that interacts
//   with a vtkPoints object.
//
// Notes:
//
// Programmer: Cyrus Harrison
// Creation:   Tue Oct  9 12:51:23 PDT 2012
//
// Modifications:
//
// ****************************************************************************

avtPolygonToTrianglesTesselator::avtPolygonToTrianglesTesselator(vtkPoints *pts)
{
    tessMemAllocated = 0;
    tessMemAllocator = new TESSalloc();
    memset(tessMemAllocator,0,sizeof(tessMemAllocator));
    tessMemAllocator->memalloc      = tess_std_alloc;
    tessMemAllocator->memfree       = tess_std_free;
    tessMemAllocator->userData      = (void*)&tessMemAllocated;
    tessMemAllocator->extraVertices = 5000;
    // TODO, support realloc


    vertexManager = new VertexManager(pts);
    xPoints       = pts;

    verts.clear();

    // default norm vector
    tessNorm[0] = 0.0;
    tessNorm[1] = 0.0;
    tessNorm[2] = 1.0;

    // todo check return on new tess
    tessObj    = tessNewTess(tessMemAllocator);
}

// ****************************************************************************
// Method: avtPolygonToTrianglesTesselator Destructor
//
//
// Purpose:
//   Cleanup for avtPolygonToTrianglesTesselator.
//
//
// Notes:
//
// Programmer: Cyrus Harrison
// Creation:   Tue Oct  9 12:51:23 PDT 2012
//
// Modifications:
//
// ****************************************************************************


avtPolygonToTrianglesTesselator::~avtPolygonToTrianglesTesselator()
{
    tessDeleteTess(tessObj);
    delete tessMemAllocator;
    delete vertexManager;
}


// ****************************************************************************
// Method: avtPolygonToTrianglesTesselator::BeginContour
//
//
// Purpose:
//   Preparse for a new polygon contour.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Oct  9 12:51:23 PDT 2012
//
// Modifications:
//
// ****************************************************************************
void
avtPolygonToTrianglesTesselator::BeginContour()
{
    verts.clear();
}

// ****************************************************************************
// Method: avtPolygonToTrianglesTesselator::GetVertexId
//
//
// Purpose:
//   Gets the id of the vertex in the vtkPoints object.
//   Adds the point if it does not already exists in the collection.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Oct  9 12:51:23 PDT 2012
//
// Modifications:
//
// ****************************************************************************
int
avtPolygonToTrianglesTesselator::GetVertexId(double *vals)
{
    return vertexManager->GetVertexId(vals);
}


// ****************************************************************************
// Method: avtPolygonToTrianglesTesselator::AddContourVertex
//
//
// Purpose:
//   Adds the next vertex in the contour.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Oct  9 12:51:23 PDT 2012
//
// Modifications:
//
// ****************************************************************************
void
avtPolygonToTrianglesTesselator::AddContourVertex(double *vals)
{
    // make vertex manager aware to preserve order
    //vertexManager->GetVertexId(vals);
    // TODO optimize!
    verts.push_back(vals[0]);
    verts.push_back(vals[1]);
    verts.push_back(vals[2]);
}

// ****************************************************************************
// Method: avtPolygonToTrianglesTesselator::AddContourVertex
//
//
// Purpose:
//   Adds the next vertex in the contour.
//
// Programmer: Cyrus Harrison
// Creation:   Tue Oct  9 12:51:23 PDT 2012
//
// Modifications:
//
// ****************************************************************************
void
avtPolygonToTrianglesTesselator::AddContourVertex(double x,
                                                  double y,
                                                  double z)
{
    double vals[3];
    vals[0] = x;
    vals[1] = y;
    vals[2] = z;
    AddContourVertex(vals);
}

// // ****************************************************************************
// // Method: avtPolygonToTrianglesTesselator::AddContour
// //
// //
// //
// // Programmer: Cyrus Harrison
// // Creation:   Tue Oct  9 12:51:23 PDT 2012
// //
// // Modifications:
// //
// // ****************************************************************************
// void
// avtPolygonToTrianglesTesselator::AddContour(double *vals, int nverts)
// {
//     tessAddContour(tessObj,3,vals,3*sizeof(double),nverts);
// }

// ****************************************************************************
// Method: avtPolygonToTrianglesTesselator::EndContour
//
//
// Purpose:
//   Completes a contour.
//
// Programmer: Cyrus Harrison
// Creation:   Tue Oct  9 12:51:23 PDT 2012
//
// Modifications:
//
// ****************************************************************************
void avtPolygonToTrianglesTesselator::EndContour()
{
    int nverts = verts.size() / 3;
    tessAddContour(tessObj,3,
                   &verts[0],
                   3*sizeof(double),
                   nverts);
}

// ****************************************************************************
// Method: avtPolygonToTrianglesTesselator::SetNormal
//
// Purpose:
//   Sets the normal vector used during tessellation.
//
// Programmer: Cyrus Harrison
// Creation:   Tue Oct  9 12:51:23 PDT 2012
//
// Modifications:
//
// ****************************************************************************
void
avtPolygonToTrianglesTesselator::SetNormal(double x,
                                           double y,
                                           double z)
{
    tessNorm[0] = x;
    tessNorm[1] = y;
    tessNorm[2] = z;
}

// ****************************************************************************
// Method: avtPolygonToTrianglesTesselator::SetNormal
//
// Purpose:
//   Sets the normal vector used during tessellation.
//
// Programmer: Cyrus Harrison
// Creation:   Tue Oct  9 12:51:23 PDT 2012
//
// Modifications:
//
// ****************************************************************************
void
avtPolygonToTrianglesTesselator::SetNormal(const double *vals)
{
    tessNorm[0] = vals[0];
    tessNorm[1] = vals[1];
    tessNorm[2] = vals[2];
}

// ****************************************************************************
// Method: avtPolygonToTrianglesTesselator::Tessellate
//
// Purpose:
//   Executes tessellation on the current set of contours.
//   Returns the number of triangles created.
//
// Programmer: Cyrus Harrison
// Creation:   Tue Oct  9 12:51:23 PDT 2012
//
// Modifications:
//
// ****************************************************************************
int
avtPolygonToTrianglesTesselator::Tessellate()
{
    // call tess
    if(tessTesselate(tessObj,TESS_WINDING_ODD,TESS_POLYGONS,3,3,tessNorm) != 1)
    {// TODO check return
        /*error*/
        return 0;
    }
    return tessGetElementCount(tessObj);;
}

// ****************************************************************************
// Method: avtPolygonToTrianglesTesselator::Tessellate
//
// Purpose:
//   Executes tessellation on the current set of contours, and adds the
//   resulitng triangles into the passed vtkPolyData dataset.
//   Returns the number of triangles created.
//
// Programmer: Cyrus Harrison
// Creation:   Tue Oct  9 12:51:23 PDT 2012
//
// Modifications:
//
// ****************************************************************************
int
avtPolygonToTrianglesTesselator::Tessellate(vtkPolyData *pd)
{
    // call tess
    int ntris = Tessellate();
    if(ntris == 0)
    {
        /*error*/
        return ntris;
    }

    const double    *tpts  = tessGetVertices(tessObj);
    const TESSindex *teles = tessGetElements(tessObj);

//         if(pd != NULL)
//     {
//     vtkIdType tri_vert_ids[3];
//     int tri_idx = 0;
//     for(int j=0; j < ntris; ++j)
//     {
//         tri_vert_ids[0] = vertexManager->GetVertexId(&tpts[teles[tri_idx+0]]);
//         tri_vert_ids[1] = vertexManager->GetVertexId(&tpts[teles[tri_idx+1]]);
//         tri_vert_ids[2] = vertexManager->GetVertexId(&tpts[teles[tri_idx+2]]);
//         /*tri_vert_ids[0] = teles[tri_idx+0] + ptIndexOffset;
//         tri_vert_ids[1] = teles[tri_idx+1] + ptIndexOffset;
//         tri_vert_ids[2] = teles[tri_idx+2] + ptIndexOffset;
//         */
//         pd->InsertNextCell(VTK_TRIANGLE, 3, tri_vert_ids);
//         tri_idx +=3;
//     }
//
// //     }
//
//     return ntris;

    // for now simply add all new points along with new tris
    // in the future, use vertex manager to avoid dup points
//     int ptIndexOffset = xPoints->GetNumberOfPoints();
//     cout << "ptIndexOffset " << ptIndexOffset  << endl;
//     int pts_idx   = 0;
//     for(int j=0; j < nverts; ++j)
//     {
//         double *vptr = (double*)&tpts[pts_idx];
//         //vertexManager->GetVertexId(vptr);
//         xPoints->InsertNextPoint(vptr);
//         pts_idx = pts_idx + 3;
//     }
    vtkIdType tri_vert_ids[3];
    int tri_idx = 0;
    for(int j=0; j < ntris; ++j)
    {
        tri_vert_ids[0] = vertexManager->GetVertexId(&tpts[teles[tri_idx+0]*3]);
        tri_vert_ids[1] = vertexManager->GetVertexId(&tpts[teles[tri_idx+1]*3]);
        tri_vert_ids[2] = vertexManager->GetVertexId(&tpts[teles[tri_idx+2]*3]);
        /*tri_vert_ids[0] = teles[tri_idx+0] + ptIndexOffset;
        tri_vert_ids[1] = teles[tri_idx+1] + ptIndexOffset;
        tri_vert_ids[2] = teles[tri_idx+2] + ptIndexOffset;
        */
        pd->InsertNextCell(VTK_TRIANGLE, 3, tri_vert_ids);
        tri_idx +=3;
    }

/*
        for now simply add all new points along with new tris
    in the future, use vertex manager to avoid dup points
    ptIndexOffset = xPoints->GetNumberOfPoints();
    int pts_idx   = 0;
    for(int j=0; j < nverts; ++j)
    {
        double *vptr = (double*)&tpts[pts_idx];
        xPoints->InsertNextPoint(vptr);
        pts_idx = pts_idx + 3;
    }*/

    return ntris;

}

// ****************************************************************************
// Method: avtPolygonToTrianglesTesselator::GetNumberOfTriangles
//
//
// Purpose:
//   Returns the number of triangles generated from the last tessellation call.
//
// Programmer: Cyrus Harrison
// Creation:   Tue Oct  9 12:51:23 PDT 2012
//
// Modifications:
//
// ****************************************************************************
int
avtPolygonToTrianglesTesselator::GetNumberOfTriangles() const
{
    return tessGetElementCount(tessObj);
}

// ****************************************************************************
// Method: avtPolygonToTrianglesTesselator::GetTriangleIndices
//
// Purpose:
//   Returns the point indicies of the selected triangle, genearted from the
//   last tessellation call.
//
// Programmer: Cyrus Harrison
// Creation:   Tue Oct  9 12:51:23 PDT 2012
//
// Modifications:
//
// ****************************************************************************
void
avtPolygonToTrianglesTesselator::GetTriangleIndices(int idx,
                                                    int &a,
                                                    int &b,
                                                    int &c) const
{
    int tri_idx =  3 * idx;

    const double    *tpts  = tessGetVertices(tessObj);
    const TESSindex *teles = tessGetElements(tessObj);

    a = vertexManager->GetVertexId(&tpts[teles[tri_idx+0]*3]);
    b = vertexManager->GetVertexId(&tpts[teles[tri_idx+1]*3]);
    c = vertexManager->GetVertexId(&tpts[teles[tri_idx+2]*3]);
}

