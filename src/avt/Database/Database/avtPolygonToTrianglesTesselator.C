// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
    memset(tessMemAllocator,0,sizeof(TESSalloc));
    tessMemAllocator->memalloc      = tess_std_alloc;
    tessMemAllocator->memfree       = tess_std_free;
    tessMemAllocator->userData      = (void*)&tessMemAllocated;
    tessMemAllocator->extraVertices = 5000;
    // TODO, support realloc ?

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
//    Kathleen Biagas, Thu Jun 26 13:22:01 MST 2014
//    Uncommented use of vertexManager to preserve order.
//
// ****************************************************************************
void
avtPolygonToTrianglesTesselator::AddContourVertex(double *vals)
{
    // make vertex manager aware to preserve order
    vertexManager->GetVertexId(vals);
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
    int nverts = (int)verts.size() / 3;
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
    {
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

    vtkIdType tri_vert_ids[3];
    int tri_idx = 0;
    for(int j=0; j < ntris; ++j)
    {
        tri_vert_ids[0] = vertexManager->GetVertexId(&tpts[teles[tri_idx+0]*3]);
        tri_vert_ids[1] = vertexManager->GetVertexId(&tpts[teles[tri_idx+1]*3]);
        tri_vert_ids[2] = vertexManager->GetVertexId(&tpts[teles[tri_idx+2]*3]);
        pd->InsertNextCell(VTK_TRIANGLE, 3, tri_vert_ids);
        tri_idx +=3;
    }

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

