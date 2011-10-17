/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
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
//                            avtGMVFileFormat.C                             //
// ************************************************************************* //

#include <avtGMVFileFormat.h>

#include <string>
#include <set>

#include <vtkCellData.h>
#include <vtkCellTypes.h>
#include <vtkCharArray.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkIdList.h>
#include <vtkLongArray.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatabaseMetaData.h>
#include <avtMaterial.h>

#include <DBOptionsAttributes.h>
#include <Expression.h>

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>
#include <ImproperUseException.h>

#include <gmvPolyhedralSplit.h>

#include <DebugStream.h>

extern "C" {
#include <gmvread.h>
/*We define this to handle errors from GMV's read routines so they don't 
  call exit(0).*/
void gmverror(const char *);
};

// ****************************************************************************
// Method: gmverror
//
// Purpose: 
//   This function gets called from gmv when it wants to exit due to fatal
//   errors with the file. We never want VisIt to exit to I made gmv routines
//   call this function, which will throw an invalid files exception instead
//   of exiting.
//
// Arguments:
//   msg : The reason GMV wants to quit.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 16:51:11 PDT 2010
//
// Modifications:
//   
// ****************************************************************************

void
gmverror(const char *msg)
{
    EXCEPTION1(InvalidFilesException, msg);
}

#ifndef MDSERVER
///////////////////////////////////////////////////////////////////////////////
///
/// ROUTINES TO CONVERT gmv_data INTO VTK OBJECTS
///
///////////////////////////////////////////////////////////////////////////////
#include <gmvMaterialEncoder.h>
#include <gmvPolygonToTriangles.C>

// ****************************************************************************
// Method: print_gmv_data
//
// Purpose: 
//   Print the gmv_data struct.
//
// Arguments:
//   out : The stream to which we want to print.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 27 12:14:43 PDT 2010
//
// Modifications:
//   
// ****************************************************************************

void
print_gmv_data(ostream &out)
{
    out << "keyword=" << gmv_data.keyword << endl;
    out << "datatype=" << gmv_data.datatype<< endl;
    out << "name1=" << gmv_data.name1 << endl;
    out << "num=" << gmv_data.num << endl;
    out << "num2=" << gmv_data.num2 << endl;
    out << "ndoubledata1=" << gmv_data.ndoubledata1 << endl;
    out << "doubledata1=" << (void*)gmv_data.doubledata1 << endl;
    out << "ndoubledata2=" << gmv_data.ndoubledata2 << endl;
    out << "doubledata2=" << (void*)gmv_data.doubledata2 << endl;
    out << "ndoubledata3=" << gmv_data.ndoubledata3 << endl;
    out << "doubledata3=" << (void*)gmv_data.doubledata3 << endl;
    out << "nlongdata1=" << gmv_data.nlongdata1 << endl;
    out << "longdata1=" << (void*)gmv_data.longdata1 << endl;
    out << "nlongdata2=" << gmv_data.nlongdata2 << endl;
    out << "longdata2=" << (void*)gmv_data.longdata2 << endl;

    out << "nchardata1=" << gmv_data.nchardata1 << endl;
    out << "chardata1=" << (void*)gmv_data.chardata1 << endl;
    out << "nchardata2=" << gmv_data.nchardata2 << endl;
    out << "chardata2=" << (void*)gmv_data.chardata2 << endl;
    out << "*******************************************************************" << endl;
}
    
// ****************************************************************************
// Method: gmvCreatePoints
//
// Purpose: 
//   Create a vtkPoints object from NODE data in gmv_data.
//
// Arguments:
//
// Returns:    a vtkPoints containing the nodes.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 27 12:15:13 PDT 2010
//
// Modifications:
//   
// ****************************************************************************

vtkPoints *
gmvCreatePoints()
{
    vtkPoints *pts = vtkPoints::New();
    pts->SetNumberOfPoints(gmv_data.num);
    if (gmv_data.num > 0)
    {
        float *ptr = (float *)pts->GetVoidPointer(0);
        for(int i = 0; i < gmv_data.num; ++i)
        {
            *ptr++ = gmv_data.doubledata1[i];
            *ptr++ = gmv_data.doubledata2[i];
            *ptr++ = gmv_data.doubledata3[i];
        }
    }
    return pts;
}

// ****************************************************************************
// Method: gmvCreateStructuredGrid
//
// Purpose: 
//   Create a vtkStructuredGrid for the nodes in gmv_data.
//
// Arguments:
//
// Returns:    a new vtkStructuredGrid
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 27 12:15:52 PDT 2010
//
// Modifications:
//   
// ****************************************************************************

vtkStructuredGrid *
gmvCreateStructuredGrid()
{
    vtkPoints *pts = gmvCreatePoints();

    vtkStructuredGrid *sgrid = vtkStructuredGrid::New();
    sgrid->SetPoints(pts);
    pts->Delete();

    int dims[3];
    dims[0] = gmv_data.ndoubledata1;
    dims[1] = gmv_data.ndoubledata2;
    dims[2] = gmv_data.ndoubledata3;
    sgrid->SetDimensions(dims);

    return sgrid;
}

// ****************************************************************************
// Method: gmvCreateRectilinearGrid
//
// Purpose: 
//   Create a new vtkRectilinearGrid for the nodes in gmv_data.
//
// Arguments:
//
// Returns:    a new vtkRectilinearGrid.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 27 12:16:33 PDT 2010
//
// Modifications:
//   
// ****************************************************************************

vtkRectilinearGrid *
gmvCreateRectilinearGrid()
{
    vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New();

    int dims[3];
    dims[0] = gmv_data.ndoubledata1;
    dims[1] = gmv_data.ndoubledata2;
    dims[2] = gmv_data.ndoubledata3;
    rgrid->SetDimensions(dims);

    vtkFloatArray *xc = vtkFloatArray::New();
    xc->SetNumberOfTuples(dims[0]);
    for(int i = 0; i < dims[0]; ++i)
        xc->SetTuple1(i, gmv_data.doubledata1[i]);

    vtkFloatArray *yc = vtkFloatArray::New();
    yc->SetNumberOfTuples(dims[1]);
    for(int i = 0; i < dims[1]; ++i)
        yc->SetTuple1(i, gmv_data.doubledata2[i]);

    vtkFloatArray *zc = vtkFloatArray::New();
    zc->SetNumberOfTuples(dims[2]);
    for(int i = 0; i < dims[2]; ++i)
        zc->SetTuple1(i, gmv_data.doubledata3[i]);

    rgrid->SetXCoordinates(xc);
    xc->Delete();
    rgrid->SetYCoordinates(yc);
    yc->Delete();
    rgrid->SetZCoordinates(zc);
    zc->Delete();
    

    return rgrid;
}

// ****************************************************************************
// Method: gmvAddRegularCell
//
// Purpose: 
//   Add a new cell to a vtkUnstructuredGrid, using the cell in gmv_data.
//
// Arguments:
//   ugrid : The unstructured grid to which we're adding a cell.
//
// Returns:    True if the cell was added; false otherwise.
//
// Note:       The node ordering has not yet been tested for the majority of
//             the cell types. We'll likely have some tangled cells for now.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 27 12:17:13 PDT 2010
//
// Modifications:
//   
// ****************************************************************************
#define GMV_DEBUG_PRINT
bool
gmvAddRegularCell(vtkUnstructuredGrid *ugrid)
{
    struct gmv2vtk_t
    {
        const char *cellname;
        int         cellnamelen;
        int         vtktype;
        int        *reorder;
    };
    static int hex_reorder[] = {4,5,6,7,0,1,2,3};
    static int prism_reorder[] = {3,4,5,0,1,2};
    static int pyramid_reorder[] = {1,2,3,4,0};
    static gmv2vtk_t gmv2vtk[] = {
        {"tri",      3, VTK_TRIANGLE,             NULL},
        {"quad",     4, VTK_QUAD,                 NULL},
        {"tet",      3, VTK_TETRA,                NULL},
        {"hex",      3, VTK_HEXAHEDRON,           hex_reorder},
        {"prism",    5, VTK_WEDGE,                prism_reorder},
        {"pyramid",  7, VTK_PYRAMID,              pyramid_reorder},
        {"line",     4, VTK_LINE,                 NULL},
        {"phex8",    5, VTK_HEXAHEDRON,           NULL},
        {"phex20",   6, VTK_QUADRATIC_HEXAHEDRON, NULL},
        {"ppyrmd5",  7, VTK_PYRAMID,              NULL},
        {"ppyrmd13", 8, VTK_QUADRATIC_PYRAMID,    NULL},
        {"pprism6",  7, VTK_WEDGE,                NULL},
        {"pprism15", 8, VTK_QUADRATIC_WEDGE,      NULL},
        {"ptet4",    5, VTK_TETRA,                NULL},
        {"ptet10",   6, VTK_QUADRATIC_TETRA,      NULL},
        {"6tri",     4, VTK_QUADRATIC_TRIANGLE,   NULL},
        {"8quad",    5, VTK_QUADRATIC_QUAD,       NULL},
        {"3line",    5, VTK_QUADRATIC_EDGE,       NULL}
/*                {"phex27",   6, VTK_, NULL}*/
    };
    vtkIdType ids[27];
    bool addedCell = false;
    for(int t = 0; t < sizeof(gmv2vtk)/sizeof(gmv2vtk_t); ++t)
    {
        if(strncmp(gmv_data.name1, gmv2vtk[t].cellname, gmv2vtk[t].cellnamelen) == 0)
        {
#ifdef GMV_DEBUG_PRINT
            debug5 << "Adding VTK cell: ";
#endif
            if(gmv2vtk[t].reorder != NULL)
            {
                for(int i = 0; i < gmv_data.nlongdata1; ++i)
                {
                    ids[i] = ((int)gmv_data.longdata1[gmv2vtk[t].reorder[i]])-1;
#ifdef GMV_DEBUG_PRINT
                    debug5 << ids[i] << " ";
#endif
                }
            }
            else
            {
                for(int i = 0; i < gmv_data.nlongdata1; ++i)
                {
                    ids[i] = ((int)gmv_data.longdata1[i])-1;
#ifdef GMV_DEBUG_PRINT
                    debug5 << ids[i] << " ";
#endif
                }
            }
#ifdef GMV_DEBUG_PRINT
            debug5 << endl;
#endif
            ugrid->InsertNextCell(gmv2vtk[t].vtktype, gmv_data.nlongdata1, ids);
            addedCell = true;
            break;
        }
    }

    if(!addedCell)
    {
        debug5 << "Cell " << gmv_data.name1 << " was not added to the mesh" << endl;
    }

    return addedCell;
}

// ****************************************************************************
// Method: gmvAddGeneralCell
//
// Purpose: 
//   Add a polyhedral cell to the unstructured mesh using the cell in gmv_data
//
// Arguments:
//   ugrid           : The mesh to which we're adding cells.
//   points          : The points used for ugrid.
//   polyhedralSplit : The object used to keep track of polyhedral cell splits
//   newnodes        : A vector to which we append new nodes for the ph cell centers.
//   phCenter        : The id of the new ph cell center node.
//
// Returns:    The number of cells created as a result of adding this ph cell.
//
// Note:       Ideally, we'll use vtkPolyhedron when it comes out. For now,
//             we split the cell into zoo elements.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 27 12:19:28 PDT 2010
//
// Modifications:
//   
// ****************************************************************************

int
gmvAddGeneralCell(vtkUnstructuredGrid *ugrid, vtkPoints *points, 
    gmvPolyhedralSplit *polyhedralSplit, 
    floatVector &newnodes, int &phCenter)
{
    int splitCount = 0;

    // Iterate over the faces and get a list of unique points
    std::set<int> uniquePointIds;
    for(int i = 0; i < gmv_data.nlongdata2; ++i)
        uniquePointIds.insert(gmv_data.longdata2[i]-1);

    // Come up with a center point and store it.
    double pt[3] = {0.,0.,0.}, center[3] = {0.,0.,0.};
#ifdef GMV_DEBUG_PRINT
    debug5 << "PH cell nodes: " << endl;
#endif
    polyhedralSplit->AppendPolyhedralNode(uniquePointIds.size());
    for(std::set<int>::const_iterator it = uniquePointIds.begin();
        it != uniquePointIds.end(); ++it)
    {
        polyhedralSplit->AppendPolyhedralNode(*it);
        points->GetPoint(*it, pt);
#ifdef GMV_DEBUG_PRINT
        debug5 << "    " << *it << ": " << pt[0] << ", " << pt[1] << ", " << pt[2] << endl;
#endif
        center[0] += pt[0];
        center[1] += pt[1];
        center[2] += pt[2];
    }
    double m = 1. / double(uniquePointIds.size());
    center[0] *= m;
    center[1] *= m;
    center[2] *= m; 
    newnodes.push_back(center[0]);
    newnodes.push_back(center[1]);
    newnodes.push_back(center[2]);
#ifdef GMV_DEBUG_PRINT
    debug5 << "center=" << center[0] << ", " << center[1] << ", " << center[2]
           << " as new node " << phCenter << endl;
    debug5 << "nfaces=" << gmv_data.nlongdata1 << endl;
#endif
    // Now, iterate over the faces, adding solid cells for them
    vtkIdType verts[5];
    const long *nodes = gmv_data.longdata2;
    for(int i = 0; i < gmv_data.nlongdata1; ++i)
    {
        int nPointsInFace = gmv_data.longdata1[i];
#ifdef GMV_DEBUG_PRINT
        debug5 << "face " << i << " has " << nPointsInFace << " nodes." << endl;
#endif
        if(nPointsInFace == 3)
        {
            // Add a tet
            verts[0] = nodes[2]-1;
            verts[1] = nodes[1]-1;
            verts[2] = nodes[0]-1;
            verts[3] = phCenter;
            ugrid->InsertNextCell(VTK_TETRA, 4, verts);
#ifdef GMV_DEBUG_PRINT
            debug5 << "    adding tet: " << verts[0] << ", " << verts[1] << ", " << verts[2] << ", " << verts[3] << endl;
#endif
            splitCount++;
        }
        else if(nPointsInFace == 4)
        {
            // Add a pyramid
            verts[0] = nodes[3]-1;
            verts[1] = nodes[2]-1;
            verts[2] = nodes[1]-1;
            verts[3] = nodes[0]-1;
            verts[4] = phCenter;
            ugrid->InsertNextCell(VTK_PYRAMID, 5, verts);
#ifdef GMV_DEBUG_PRINT
            debug5 << "    adding pyr: " << verts[0] << ", " << verts[1] << ", " << verts[2] << ", " << verts[3] << ", " << verts[4] << endl;
#endif
            splitCount++;
        }
        else if(nPointsInFace > 4)
        {
            // Find the face center so we can determine a proxy for a normal.
            double fc[3] = {0., 0., 0.};
            for(int j = 0; j < nPointsInFace; ++j)
            {
                points->GetPoint(nodes[j]-1, pt);
                fc[0] += pt[0];
                fc[1] += pt[1];
                fc[2] += pt[2];
            }
            fc[0] /= double(nPointsInFace);
            fc[1] /= double(nPointsInFace);
            fc[2] /= double(nPointsInFace);
            double n[3] = {0.,0.,0.};
            n[0] = center[0] - fc[0];
            n[1] = center[1] - fc[1];
            n[2] = center[2] - fc[2];

            // Tesselate the shape into triangles and add tets. We create
            // a tessellator each time so we can add the face's points to
            // it. This should cause the points to be in the same order as
            // they are in the face.
            vtkPoints *localPts = vtkPoints::New();
            localPts->Allocate(nPointsInFace);
            int *local2Global = new int[nPointsInFace];
            VertexManager           uniqueVerts(localPts);
            gmvPolygonToTriangles   tess(&uniqueVerts);
            tess.SetNormal(n);
            tess.BeginPolygon();
            tess.BeginContour();
            for(int j = 0; j < nPointsInFace; ++j)
            {
                local2Global[j] = nodes[j]-1;
                tess.AddVertex(points->GetPoint(local2Global[j]));
            }
            tess.EndContour();
            tess.EndPolygon();

            for(int t = 0; t < tess.GetNumTriangles(); ++t)
            {
                int a,b,c;
                tess.GetTriangle(t, a, b, c);
                verts[0] = local2Global[a];
                verts[1] = local2Global[b];
                verts[2] = local2Global[c];
                verts[3] = phCenter;
                ugrid->InsertNextCell(VTK_TETRA, 4, verts);
#ifdef GMV_DEBUG_PRINT
                debug5 << "    adding tet: " << verts[0] << ", " << verts[1] << ", " << verts[2] << ", " << verts[3] << endl;
#endif
                splitCount++;
            }

            localPts->Delete();
            delete [] local2Global;
        }
        
        nodes += nPointsInFace;
    }

    return splitCount;
}

// ****************************************************************************
// Method: gmvAddVFace2D
//
// Purpose: 
//   Add VFace2D cells to the mesh.
//
// Arguments:
//   ugrid : The mesh to which we're adding cells.
//
// Returns:    Number of splits.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue May 31 14:38:38 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

int
gmvAddVFace(vtkUnstructuredGrid *ugrid, const std::vector<int> &nodes)
{
    const char *mName = "gmvAddVFace: ";
    int nsplits = 1;

    int nPoints = nodes.size()-1;
    vtkIdType verts[10];
    if(nPoints == 3)
    {
        // Add a triangle
        verts[0] = nodes[0]-1;
        verts[1] = nodes[1]-1;
        verts[2] = nodes[2]-1;
        debug5 << mName << "Adding triangle ("
               << verts[0] << ", " << verts[1] << ", " << verts[2]
               << ")" << endl;
        ugrid->InsertNextCell(VTK_TRIANGLE, 3, verts);
    }
    else if(nPoints == 4)
    {
        // Add a quad
        verts[0] = nodes[0]-1;
        verts[1] = nodes[1]-1;
        verts[2] = nodes[2]-1;
        verts[3] = nodes[3]-1;
        debug5 << mName << "Adding quad ("
               << verts[0] << ", " << verts[1] << ", " << verts[2] << ", " << verts[3]
               << ")" << endl;
        ugrid->InsertNextCell(VTK_QUAD, 4, verts);
    }
    else if(nPoints <= 10)
    {
        debug4 << mName << "Break " << nPoints << "-gon into triangles" << endl;
        int nTri = nPoints - 2;
        for(int t = 0; t < nTri; ++t)
        {
            // Add triangles
            verts[0] = nodes[0]-1;
            verts[1] = nodes[t+1]-1;
            verts[2] = nodes[t+2]-1;
            debug5 << "    triangle ("
                   << verts[0] << ", " << verts[1] << ", " << verts[2]
                   << ")" << endl;
            ugrid->InsertNextCell(VTK_TRIANGLE, 3, verts);
        }

        nsplits = nTri;
    }
    else
    {
        debug5 << "gmvAddVFace2D: an invalid number of points in the vface2d" << endl;
    }

    return nsplits;
}

// ****************************************************************************
// Method: gmvCreateUnstructuredGrid
//
// Purpose: 
//   Create a new vtkUnstructuredGrid object using the nodes and cells in 
//   gmv_data.
//
// Arguments:
//   polyhedralSplit : The object we use to track splits to polyhedral cells.
//   topoDim         : Return the topological dimension.
//
// Returns:    
//
// Note:      This function calls gmvread_data to get the cells needed to 
//            construct the mesh. 
//
//  This function doesn't take into account the case where the gmv zone 
//  can't be mapped into a VTK zone. The mesh will be fine but the mapping
//  needed to handle gmv->vtk zone variables will be incomplete.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 27 12:23:38 PDT 2010
//
// Modifications:
//   Brad Whitlock, Tue May 31 16:44:17 PDT 2011
//   I added initial vfaces support for 2D.
//
// ****************************************************************************

vtkUnstructuredGrid *
gmvCreateUnstructuredGrid(gmvPolyhedralSplit *polyhedralSplit, int &topoDim)
{
    const char *mName = "gmvCreateUnstructuredGrid: ";

    // Let's create our own points.
    vtkPoints *pts = gmvCreatePoints();

    // Create a ugrid
    debug4 << mName << "Allocating initial cells " << (gmv_data.num * 8) << endl;
    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    ugrid->Allocate(gmv_data.num * 8);

    // Iterate over all of the cells and add cells.
    int vtkCellId = 0;
    int normalCells = 0;
    floatVector newNodes;
    int phCells = 0;
    int phNode = pts->GetNumberOfPoints();
    bool foundVFace2D = false;
    while(gmv_data.datatype != ENDKEYWORD)
    {
        bool addedCell = false;
        if(gmv_data.datatype == GENERAL)
        {
            int nsplits = gmvAddGeneralCell(ugrid, pts, polyhedralSplit, newNodes, phNode);
            phNode++;
            polyhedralSplit->AppendCellSplits(vtkCellId, nsplits);
            phCells++;
            addedCell = true;
        }
        else if(gmv_data.datatype == REGULAR)
        {
            addedCell = gmvAddRegularCell(ugrid);
            normalCells++;
        }
        else if(gmv_data.datatype == VFACE2D)
        {
            // Make a note that we had vface2d cells. We ignore the vface2d definitions
            // currently because it does not look like they appear in multiple cells.
            foundVFace2D = true;

            // Indicate the mesh is topologically 2D.
            topoDim = 2;
        }
        else if(gmv_data.datatype == VFACE3D)
        {
            debug5 << "VFACE3D not supported" << endl;
        }
#if 0
        else
        {
            pts->Delete();
            ugrid->Delete();
            // For now...
            debug5 << "Unsupported cell type: " << gmv_data.datatype << endl;
            EXCEPTION1(ImproperUseException, "Unsupported cell type");
        }
#endif

        gmvread_data();

//#ifndef MDSERVER
//        print_gmv_data(DebugStream::Stream5());
//#endif

        if(addedCell)
            vtkCellId++;
    }

    if(foundVFace2D)
    {
        // Read the vfaces now
        std::map<int, std::vector<int> > cellVertices;
        std::map<int, std::vector<int> >::iterator it;
        do
        {
            gmvread_data();
//#ifndef MDSERVER
//            print_gmv_data(DebugStream::Stream5());
//#endif
            if(gmv_data.longdata1 != 0 && gmv_data.longdata2 != 0)
            {
                int cellid = (int)gmv_data.longdata2[3];

                int A = (int)gmv_data.longdata1[0];
                int B = (int)gmv_data.longdata1[1];

                if((it = cellVertices.find(cellid)) == cellVertices.end())
                {
                    std::vector<int> pts;
                    pts.push_back(A);
                    pts.push_back(B);
                    cellVertices[cellid] = pts;
                }
                else
                {
                    if(it->second[it->second.size()-1] != A)
                        it->second.push_back(A);
                    it->second.push_back(B);
                }
            }
        } while(gmv_data.datatype != ENDKEYWORD);

        // Add cells for the vfaces that we just read.
        for(it = cellVertices.begin(); it != cellVertices.end(); ++it)
        {
            int nsplits = gmvAddVFace(ugrid, it->second);
            if(nsplits > 1)
            {
                polyhedralSplit->AppendCellSplits(vtkCellId, nsplits);
                phCells++;
            }
            else
            {
                ++normalCells;
            }

            ++vtkCellId;
        }
    }

    // Tack the new polyhedron nodes onto the end of the points.
    if(!newNodes.empty())
    {
        int nNewPoints = newNodes.size()/3;
        debug4 << mName << "Adding extra points for polyhedral cells" << endl;
        vtkPoints *newPoints = vtkPoints::New();
        newPoints->SetNumberOfPoints(pts->GetNumberOfPoints() + nNewPoints);

        float *fptr = (float *)newPoints->GetVoidPointer(0);
        memcpy(fptr, pts->GetVoidPointer(0), 
               pts->GetNumberOfPoints() * sizeof(float) * 3);
        memcpy(fptr + 3 * pts->GetNumberOfPoints(), &newNodes[0], 
               newNodes.size() * sizeof(float));

        pts->Delete();
        pts = newPoints;
    }

    // Give the points to the ugrid.
    ugrid->SetPoints(pts);
    pts->Delete();

    debug4 << mName << "Total input cells: " << (normalCells + phCells) << endl;
    debug4 << mName << "Total output cells: " << ugrid->GetNumberOfCells() << endl;
    debug4 << mName << "Total output points: " << ugrid->GetPoints()->GetNumberOfPoints() << endl;

    // If we had any polyhedral cells then append the original zones array.
    if(phCells > 0)
    {
        vtkDataArray *oz = polyhedralSplit->CreateOriginalCells(0, normalCells);
        ugrid->GetCellData()->AddArray(oz);
    }

    return ugrid;
}

// ****************************************************************************
// Method: gmvCreatePointMesh
//
// Purpose: 
//   Create a point mesh from data in gmv_data.
//
// Arguments:
//
// Returns:    a new vtkUnstructuredGrid
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 27 12:27:44 PDT 2010
//
// Modifications:
//   
// ****************************************************************************

vtkUnstructuredGrid *
gmvCreatePointMesh()
{
    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    vtkPoints *pts = gmvCreatePoints();
    ugrid->SetPoints(pts);
    pts->Delete();

    vtkIdType nnodes = pts->GetNumberOfPoints();
    ugrid->Allocate(nnodes);
    vtkIdType onevertex;
    for(int i = 0; i < nnodes; ++i)
    {
        onevertex = i;
        ugrid->InsertNextCell(VTK_VERTEX, 1, &onevertex);
    }

    return ugrid;
}

// ****************************************************************************
// Method: gmvCreateVariable
//
// Purpose: 
//   Create new vtkDataArray using data from gmv_data.
//
// Arguments:
//   varname : The name to use for the data array.
//
// Returns:    A new vtkDataArray containing the data.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 27 12:29:25 PDT 2010
//
// Modifications:
//   
// ****************************************************************************

vtkDataArray *
gmvCreateVariable(const char *varname)
{
    const char *mName = "gmvCreateVariable: ";
    vtkDataArray *retval = 0;
    if(gmv_data.ndoubledata1 > 0 && gmv_data.ndoubledata2 > 0 && gmv_data.ndoubledata3 > 0)
    {
        debug1 << mName << "Create vector " << varname << " with "
               << gmv_data.ndoubledata1 << " tuples" << endl;

        vtkDoubleArray *arr = vtkDoubleArray::New();
        arr->SetNumberOfComponents(3);
        arr->SetNumberOfTuples(gmv_data.ndoubledata1);
        double *dptr = (double *)arr->GetVoidPointer(0);
        for(int i = 0; i < gmv_data.ndoubledata1; ++i)
        {
            *dptr++ = gmv_data.doubledata1[i];
            *dptr++ = gmv_data.doubledata2[i];
            *dptr++ = gmv_data.doubledata3[i];
        }
        retval = arr;
    }
    // Scalar case
    else if(gmv_data.ndoubledata1 > 0)
    {
        debug1 << mName << "Create double scalar " << varname << " with "
               << gmv_data.ndoubledata1 << " tuples" << endl;

        vtkDoubleArray *arr = vtkDoubleArray::New();
        arr->SetNumberOfTuples(gmv_data.ndoubledata1);
        memcpy(arr->GetVoidPointer(0), gmv_data.doubledata1, 
               sizeof(double) * gmv_data.ndoubledata1);
        retval = arr;
    }
    else if(gmv_data.nlongdata1 > 0)
    {
        debug1 << mName << "Create long scalar " << varname << " with "
               << gmv_data.nlongdata1 << " tuples" << endl;

        vtkLongArray *arr = vtkLongArray::New();
        arr->SetNumberOfTuples(gmv_data.nlongdata1);
        memcpy(arr->GetVoidPointer(0), gmv_data.longdata1, 
               sizeof(long) * gmv_data.nlongdata1);
        retval = arr;
    }
    else if(gmv_data.nchardata1 > 0)
    {
        debug1 << mName << "Create char scalar " << varname << " with "
               << gmv_data.nchardata1 << " tuples" << endl;

        vtkCharArray *arr = vtkCharArray::New();
        arr->SetNumberOfTuples(gmv_data.nchardata1);
        memcpy(arr->GetVoidPointer(0), gmv_data.chardata1, 
               sizeof(char) * gmv_data.nchardata1);
        retval = arr;
    }

    if(retval != 0)
        retval->SetName(varname);

    return retval;
}
#endif
///////////////////////////////////////////////////////////////////////////////


// ****************************************************************************
//  Method: avtGMVFileFormat constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 22 14:42:45 PST 2010
//
//  Modifcations:
//
// ****************************************************************************

avtGMVFileFormat::avtGMVFileFormat(const char *filename)
    : avtSTMDFileFormat(&filename, 1)
{
    // Open the file to make sure it's the right format.
    if(gmvread_open_fromfileskip(const_cast<char*>(filename)) > 0)
    {
        EXCEPTION1(InvalidFilesException, filename);
    }
    gmvread_close();
    fileOpen = false;
    dataRead = false;
    probTime = 0.;
    probCycle = 0;
}

// ****************************************************************************
// Method: avtGMVFileFormat destructor
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 27 12:33:23 PDT 2010
//
// Modifications:
//   
// ****************************************************************************

avtGMVFileFormat::~avtGMVFileFormat()
{
    FreeUpResources();
}

// ****************************************************************************
//  Method: avtGMVFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 22 14:42:45 PST 2010
//
// ****************************************************************************

void
avtGMVFileFormat::FreeUpResources(void)
{
    if(fileOpen)
    {
        gmvread_close();
        fileOpen = false;
    }

    md = avtDatabaseMetaData();
    probTime = 0.;
    probCycle = 0;
    for(std::map<std::string,MeshData>::iterator pos = meshes.begin(); pos != meshes.end(); ++pos)
    {
        if(pos->second.dataset != 0)
            pos->second.dataset->Delete();
        if(pos->second.material != 0)
            pos->second.material->Delete();
        if(pos->second.polyhedralSplit != 0)
            delete pos->second.polyhedralSplit;
    }
    meshes.clear();
    dataRead = false;
}

// ****************************************************************************
// Method: avtGMVFileFormat::GetMeshName
//
// Purpose: 
//   Create a new mesh name.
//
// Arguments:
//   initial : The initial mesh name.
//
// Returns:  The new mesh name.  
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 27 12:28:21 PDT 2010
//
// Modifications:
//   
// ****************************************************************************

std::string
avtGMVFileFormat::GetMeshName(const std::string &initial) const
{
    std::string mesh(initial);
    if(meshes.find(initial) == meshes.end())
         return mesh;

    int index = 1;
    char tmp[10];
    do
    {
        SNPRINTF(tmp, 10, "%d", index++);
        mesh = initial + std::string(tmp);
    } while(meshes.find(mesh) != meshes.end());

    return mesh;
}

// ****************************************************************************
// Method: removets
//
// Purpose: 
//   Remove trailing spaces in a string.
//
// Arguments:
//   s : The string for which we want to remove trailing spaces.
//
// Returns:    A massaged std::string
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 27 12:34:19 PDT 2010
//
// Modifications:
//   
// ****************************************************************************

static std::string
removets(const char *s)
{
    char tmp[33];
    memset(tmp, 0, 33 * sizeof(char));
    strcpy(tmp, s);
    for(int i = 31; i >= 0 && (tmp[i] == ' ' || tmp[i] == '\0'); i--)
        tmp[i] = '\0';
    return std::string(tmp);
}

// ****************************************************************************
// Method: avtGMVFileFormat::ReadData
//
// Purpose: 
//   Read the data from the GMV file, creating VTK objects and metadata for 
//   VisIt.
//
// Arguments:
//
// Returns:    True if the file can be read.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 27 12:35:02 PDT 2010
//
// Modifications:
//   
// ****************************************************************************

void
avtGMVFileFormat::ReadData()
{
    if(!dataRead)
    {
//        int err = gmvread_open_fromfileskip(filenames[0]);
        fileOpen = false;
        int err = gmvread_open(filenames[0]);
        debug5 << "gmvread_open(" << filenames[0] << ") returned " << err << endl;
        fileOpen = err >= 0;

        if(fileOpen)
        {
            std::string meshname("mesh");
            MeshData meshdata;
            std::map<std::string, MeshData>::iterator pos;

            // Iterate over the file
            bool keepGoing = true;
            while(keepGoing)
            {
                gmvread_data();

//#ifndef MDSERVER
//                print_gmv_data(DebugStream::Stream5());
//#endif

                switch (gmv_data.keyword)
                {
                case NODES:
                    {
                    meshname = GetMeshName("mesh");

                    meshdata.dataset = NULL;
                    meshdata.polyhedralSplit = NULL;
                    meshdata.material = NULL;
                    meshdata.materialCentering = CELL;

                    avtMeshType mt = AVT_UNKNOWN_MESH;
                    int topological_dimension = 3;
                    if(gmv_data.datatype == UNSTRUCT)
                    {
#ifndef MDSERVER
                        TRY
                        {
                            meshdata.polyhedralSplit = new gmvPolyhedralSplit;
                            meshdata.dataset = gmvCreateUnstructuredGrid(meshdata.polyhedralSplit,
                                                                         topological_dimension);
                        }
                        CATCH(ImproperUseException)
                        {
                            delete meshdata.polyhedralSplit;
                            RETHROW;
                        }
                        ENDTRY
#endif
                        mt = AVT_UNSTRUCTURED_MESH;
                    }
                    else if(gmv_data.datatype == LOGICALLY_STRUCT)
                    {
#ifndef MDSERVER
                        meshdata.dataset = gmvCreateStructuredGrid();
#endif
                        mt = AVT_CURVILINEAR_MESH;
                    }
                    else if(gmv_data.datatype == STRUCT)
                    {
#ifndef MDSERVER
                        meshdata.dataset = gmvCreateRectilinearGrid();
#endif
                        mt = AVT_RECTILINEAR_MESH;
                    }
                    else if(gmv_data.datatype == AMR)
                    {
                        mt = AVT_AMR_MESH;
                    }
                    else if(gmv_data.datatype == FROMFILE)
                    {
                        EXCEPTION1(InvalidFilesException,"NODE from file");
                    }

                    // Save the mesh dataset.
                    meshes[meshname] = meshdata;
                    pos = meshes.find(meshname);

                    // Add this mesh to the metadata.
                    int nblocks = 1;
                    int block_origin = 0;
                    int spatial_dimension = 3;
                    double *extents = NULL;
                    AddMeshToMetaData(&md, meshname, mt, extents, nblocks, 
                                      block_origin, spatial_dimension, 
                                      topological_dimension);
                    }
                    break;
                case CELLS:
                    // We handle cells for unstructured meshes by calling gmvread_mesh.
                    break;
                case MATERIAL:
                    {
                    if(pos == meshes.end())
                    {
                        EXCEPTION1(InvalidFilesException,"Material without mesh");
                    }
                    // Store the material data with the mesh so we can deal with it later.
#ifndef MDSERVER  
                    vtkDataArray *mats = gmvCreateVariable("material");
                    // Expand out the data if we needed to split cells or add nodes.
                    if(mats != NULL && pos->second.polyhedralSplit != NULL)
                    {
                        vtkDataArray *expanded = pos->second.polyhedralSplit->
                            ExpandDataArray(mats, gmv_data.datatype == CELL, false);
                        mats->Delete();
                        mats = expanded;
                    }
                    pos->second.material = mats;
                    pos->second.materialCentering = gmv_data.datatype;

                    // If there were ph cells then expand the material array for zonal.
                    // For nodal, figure out the dominant material for the cell and use
                    // that for the new ph nodes.
#endif
                    // Create material metadata.
                    stringVector names;
                    for(int i = 0; i < gmv_data.num; ++i)
                        names.push_back(removets(gmv_data.chardata1 + i * 33));
                    std::string matname(GetMeshName("material"));
                    avtMaterialMetaData *mmd = new avtMaterialMetaData(matname, 
                        meshname, names.size(), names);
                    md.Add(mmd);
                    }
                    break;
                case VELOCITY:
                    {
                        if(pos == meshes.end())
                        {
                            EXCEPTION1(InvalidFilesException,"Variable without mesh");
                        }
                        avtCentering c = AVT_UNKNOWN_CENT;
                        bool valid = true;
#ifndef MDSERVER
                        vtkDataArray *arr = gmvCreateVariable("velocity");
                        if(pos->second.polyhedralSplit != NULL)
                        {
                            vtkDataArray *expanded = pos->second.polyhedralSplit->
                                ExpandDataArray(arr, gmv_data.datatype == CELL);
                            arr->Delete();
                            arr = expanded;
                        } 
#endif
                        if(gmv_data.datatype == NODE)
                        {
#ifndef MDSERVER
                            pos->second.dataset->GetPointData()->AddArray(arr);
#endif
                            c = AVT_NODECENT;
                        }
                        else if(gmv_data.datatype == CELL)
                        {
#ifndef MDSERVER
                            pos->second.dataset->GetCellData()->AddArray(arr);
#endif
                            c = AVT_ZONECENT;
                        }
                        else
                        {
#ifndef MDSERVER
                            arr->Delete(); arr = 0;
#endif
                            valid = false;
                            debug1 << "Unsupported variable centering" << endl;
                        }
                        avtVectorMetaData *vmd = new avtVectorMetaData;
                        vmd->name = "velocity";
                        vmd->meshName = meshname;
                        vmd->centering = c;
                        vmd->validVariable = valid;
                        md.Add(vmd);
                    }
                    break;
                case VARIABLE:
                    if(gmv_data.datatype != ENDKEYWORD)
                    {
                        std::string name(removets(gmv_data.name1));
                        if(pos == meshes.end())
                        {
                            EXCEPTION1(InvalidFilesException,"Variable without mesh");
                        }
#ifndef MDSERVER
                        vtkDataArray *arr = gmvCreateVariable(name.c_str());
                        // Expand out the data if we needed to split cells or add nodes.
                        if(pos->second.polyhedralSplit != NULL)
                        {
                            vtkDataArray *expanded = pos->second.polyhedralSplit->
                                ExpandDataArray(arr, gmv_data.datatype == CELL);
                            arr->Delete();
                            arr = expanded;
                        }
#endif
                        bool valid = true;
                        avtCentering c = AVT_UNKNOWN_CENT;
                        if(gmv_data.datatype == NODE)
                        {
#ifndef MDSERVER
                            pos->second.dataset->GetPointData()->AddArray(arr);
#endif
                            c = AVT_NODECENT;
                        }
                        else if(gmv_data.datatype == CELL)
                        {
#ifndef MDSERVER
                            pos->second.dataset->GetCellData()->AddArray(arr);
#endif
                            c = AVT_ZONECENT;
                        }
                        else
                        {
                            valid = false;
                            debug1 << "Unsupported variable centering" << endl;
                        }
                        avtScalarMetaData *smd = new avtScalarMetaData;
                        smd->name = name;
                        smd->meshName = meshname;
                        smd->centering = c;
                        smd->validVariable = valid;
                        md.Add(smd);
                    }
                    break;
                case FLAGS:
                    if(gmv_data.datatype != ENDKEYWORD)
                    {
                        std::string name(removets(gmv_data.name1));
                        if(pos == meshes.end())
                        {
                            EXCEPTION1(InvalidFilesException,"Variable without mesh");
                        }
#ifndef MDSERVER
                        vtkDataArray *arr = gmvCreateVariable(name.c_str());
                        // Expand out the data if we needed to split cells or add nodes.
                        if(pos->second.polyhedralSplit != NULL)
                        {
                            vtkDataArray *expanded = pos->second.polyhedralSplit->
                                ExpandDataArray(arr, gmv_data.datatype == CELL);
                            arr->Delete();
                            arr = expanded;
                        }
#endif
                        bool valid = true;
                        avtCentering c = AVT_UNKNOWN_CENT;
                        if(gmv_data.datatype == NODE)
                        {
#ifndef MDSERVER
                            pos->second.dataset->GetPointData()->AddArray(arr);
#endif
                            c = AVT_NODECENT;
                        }
                        else if(gmv_data.datatype == CELL)
                        {
#ifndef MDSERVER
                            pos->second.dataset->GetCellData()->AddArray(arr);
#endif
                            c = AVT_ZONECENT;
                        }
                        else
                        {
#ifndef MDSERVER
                            arr->Delete(); arr = 0;
#endif
                            valid = false;
                            debug1 << "Unsupported variable centering" << endl;
                        }
                        avtScalarMetaData *smd = new avtScalarMetaData;
                        smd->name = name;
                        smd->meshName = meshname;
                        smd->centering = c;
                        smd->validVariable = valid;
                        smd->SetEnumerationType(avtScalarMetaData::ByValue);
                        for(int i = 0; i < gmv_data.num2; ++i)
                            smd->AddEnumNameValue(removets(gmv_data.chardata1 + (i * 33)), i+1);
                        md.Add(smd);
                    }
                    break;
                case POLYGONS:
// There can be lots of polygons.
//                    debug1 << "POLYGONS are not handled yet" << endl;
                    break;
                case TRACERS:
                    if(gmv_data.datatype == XYZ)
                    {
                    meshname = GetMeshName("tracers");

#ifndef MDSERVER
                    meshdata.dataset = gmvCreatePointMesh();
#else
                    meshdata.dataset = NULL;
#endif
                    meshdata.material = NULL;
                    meshdata.materialCentering = CELL;
                    meshdata.polyhedralSplit = NULL;
                    meshes[meshname] = meshdata;
                    pos = meshes.find(meshname);

                    // Add this mesh to the metadata.
                    avtMeshType mt = AVT_POINT_MESH;
                    int nblocks = 1;
                    int block_origin = 0;
                    int spatial_dimension = 3;
                    int topological_dimension = 0;
                    double *extents = NULL;
                    AddMeshToMetaData(&md, meshname, mt, extents, nblocks, 
                                      block_origin, spatial_dimension, 
                                      topological_dimension);
                    }
                    else if(gmv_data.datatype == TRACERDATA)
                    {
                        std::string name(removets(gmv_data.name1));
                        if(pos == meshes.end())
                        {
                            EXCEPTION1(InvalidFilesException,"Variable without mesh");
                        }
#ifndef MDSERVER
                        vtkDataArray *arr = gmvCreateVariable(name.c_str());
                        pos->second.dataset->GetPointData()->AddArray(arr);
#endif
                        avtScalarMetaData *smd = new avtScalarMetaData;
                        smd->name = name;
                        smd->meshName = meshname;
                        smd->centering = AVT_NODECENT;
                        smd->validVariable = true;
                        md.Add(smd);
                    }
                    break;
                case PROBTIME:
                    probTime = gmv_data.doubledata1[0];
                    break;
                case CYCLENO:
                    probCycle = gmv_data.num;
                    break;
                case NODEIDS:
                    debug1 << "NODEIDS: not handled yet." << endl;
                    break;
                case CELLIDS:
                    debug1 << "CELLIDS: not handled yet." << endl;
                    break;
                case SURFACE:
                    debug1 << "SURFACE: not handled yet." << endl;
                    break;
                case SURFMATS:
                    debug1 << "SURFMATS: not handled yet." << endl;
                    break;
                case SURFVEL:
                    debug1 << "SURFVEL: not handled yet." << endl;
                    break;
                case SURFVARS:
                    debug1 << "SURFVARS: not handled yet." << endl;
                    break;
                case SURFFLAG:
                    debug1 << "SURFFLAG: not handled yet." << endl;
                    break;
                case UNITS:
                    debug1 << "UNITS: not handled yet." << endl;
                    break;
                case VINFO:
                    debug1 << "VINFO: not handled yet." << endl;
                    break;
                case TRACEIDS:
                    debug1 << "TRACEIDS: not handled yet." << endl;
                    break;
                case GROUPS:
                    debug1 << "GROUPS: not handled yet." << endl;
                    break;
                case FACEIDS:
                    debug1 << "FACEIDS: not handled yet." << endl;
                    break;
                case SURFIDS:
                    debug1 << "SURFIDS: not handled yet." << endl;
                    break;
                case CELLPES:
                    debug1 << "CELLPES: not handled yet." << endl;
                    break;
                case SUBVARS:
                    debug1 << "SUBVARS: not handled yet." << endl;
                    break;
                case GHOSTS:
                    debug1 << "GHOSTS: not handled yet." << endl;
                    break;
                case VECTORS:
                    debug1 << "VECTORS: not handled yet." << endl;
                    break;
                case CODENAME:
                    md.SetDatabaseComment((md.GetDatabaseComment() + "CODENAME: ") + gmv_data.name1);
                    break;
                case CODEVER:
                    md.SetDatabaseComment((md.GetDatabaseComment() + "\nCODEVER: ") + gmv_data.name1);
                    break;
                case SIMDATE:
                    md.SetDatabaseComment((md.GetDatabaseComment() + "\nSIMDATE: ") + gmv_data.name1);
                    break;
                case GMVEND:
                    keepGoing = false;
                    break;
                }
            }

            gmvread_close();
            fileOpen = false;
            dataRead = true;
        }
    }
}

// ****************************************************************************
// Method: avtGMVFileFormat::ActivateTimestep
//
// Purpose: 
//   Called on the new timestep to read data if it needs to be read.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 27 12:42:19 PDT 2010
//
// Modifications:
//   
// ****************************************************************************

void
avtGMVFileFormat::ActivateTimestep()
{
    ReadData();
}

// ****************************************************************************
// Method: avtGMVFileFormat::GetCycle
//
// Purpose: 
//   Return the problem cycle.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 27 12:39:53 PDT 2010
//
// Modifications:
//   
// ****************************************************************************

int
avtGMVFileFormat::GetCycle()
{
    return probCycle;
}

// ****************************************************************************
// Method: avtGMVFileFormat::GetTime
//
// Purpose: 
//   Return the problem time.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 27 12:40:15 PDT 2010
//
// Modifications:
//   
// ****************************************************************************

double
avtGMVFileFormat::GetTime()
{
    return probTime;
}

// ****************************************************************************
//  Method: avtGMVFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 22 14:42:45 PST 2010
//
// ****************************************************************************

void
avtGMVFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *m)
{
    // Selectively copy stuff from md into m.

    m->SetDatabaseComment(md.GetDatabaseComment());

    for(int i = 0; i < md.GetNumMeshes(); ++i)
        m->AddMeshes(md.GetMeshes(i));

    for(int i = 0; i < md.GetNumMaterials(); ++i)
        m->AddMaterials(md.GetMaterials(i));

    for(int i = 0; i < md.GetNumScalars(); ++i)
        m->AddScalars(md.GetScalars(i));

    for(int i = 0; i < md.GetNumVectors(); ++i)
        m->AddVectors(md.GetVectors(i));
}

// ****************************************************************************
//  Method: avtGMVFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      domain      The index of the domain.  If there are NDomains, this
//                  value is guaranteed to be between 0 and NDomains-1,
//                  regardless of block origin.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 22 14:42:45 PST 2010
//
// ****************************************************************************

vtkDataSet *
avtGMVFileFormat::GetMesh(int domain, const char *meshname)
{
    vtkDataSet *dataset = NULL;
#ifndef MDSERVER
    std::map<std::string,MeshData>::iterator pos = meshes.find(meshname);
    if(pos != meshes.end())
    {
        dataset = pos->second.dataset;
        if(dataset != NULL)
            dataset->Register(NULL);
    }
#endif
    return dataset;
}


// ****************************************************************************
//  Method: avtGMVFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 22 14:42:45 PST 2010
//
// ****************************************************************************

vtkDataArray *
avtGMVFileFormat::GetVar(int domain, const char *varname)
{
    vtkDataArray *arr = NULL;
#ifndef MDSERVER
    std::string meshname = md.MeshForVar(varname);
    std::map<std::string,MeshData>::iterator pos = meshes.find(meshname);
    if(pos != meshes.end())
    {
        vtkDataSet *dataset = pos->second.dataset;
        if(dataset != NULL)
        {
            arr = dataset->GetPointData()->GetArray(varname);
            if(arr == NULL)
                arr = dataset->GetCellData()->GetArray(varname);
        }
    }

    if(arr != NULL)
        arr->Register(NULL);
#endif

    return arr;
}


// ****************************************************************************
//  Method: avtGMVFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 22 14:42:45 PST 2010
//
// ****************************************************************************

vtkDataArray *
avtGMVFileFormat::GetVectorVar(int domain, const char *varname)
{
    return GetVar(domain, varname);
}

// ****************************************************************************
// Method: avtGMVFileFormat::GetAuxiliaryData
//
// Purpose: 
//   Return auxiliary data.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 15:08:52 PDT 2010
//
// Modifications:
//   
// ****************************************************************************

void *
avtGMVFileFormat::GetAuxiliaryData(const char *var, int domain,
    const char *type, void *args, DestructorFunction &df)
{
    void *retval = NULL;

#ifndef MDSERVER
    if(strcmp(type, AUXILIARY_DATA_MATERIAL) == 0)
    {
        const avtMaterialMetaData *mmd = md.GetMaterial(var);
        if(mmd != NULL)
        {
            std::map<std::string,MeshData>::const_iterator pos = 
                meshes.find(mmd->meshName);
            vtkDataArray *material = pos->second.material;
            if(pos != meshes.end() && material != 0)
            {
                gmvMaterialEncoder M;
                for(int i = 0; i < mmd->materialNames.size(); ++i)
                    M.AddMaterial(mmd->materialNames[i]);
                M.AllocClean(pos->second.dataset->GetNumberOfCells());

                if(pos->second.materialCentering == CELL)
                {
                    debug4 << "Create cell-centered material data" << endl;
                    for(int cellid = 0; cellid < material->GetNumberOfTuples(); ++cellid)
                    {
                        int matno = (int)material->GetTuple1(cellid);
                        M.AddClean(cellid, matno);
                    }
                }
                else // NODE
                {
                    debug4 << "Create cell-centered material data from node-"
                              "centered material data." << endl;

                    // We have node-based material data. Let's assume that means 
                    // it is potentially mixed. We'll examine the nodes from each
                    // cell and see if the cell is mixed or clean.
                    vtkIdList *idlist = vtkIdList::New();
                    vtkDataSet *ds = pos->second.dataset;
                    intVector matnos; 
                    floatVector matvf;
                    for(vtkIdType cellid = 0; cellid < ds->GetNumberOfCells(); ++cellid)
                    {
                        // Determine the number of times each material is used in the cell.
                        std::map<int,int> counts;
                        ds->GetCellPoints(cellid, idlist);
                        int npts = idlist->GetNumberOfIds();
                        for(int j = 0; j < npts; ++j)
                        {
                            int ptid = (int)idlist->GetId(j);
                            int pointMat = (int)material->GetTuple1(ptid);
                            if(counts.find(pointMat) == counts.end())
                                counts[pointMat] = 1;
                            else
                                counts[pointMat]++;
                        }

                        if(counts.size() == 1)
                        {
                            std::map<int,int>::const_iterator it = counts.begin();
                            int matno = it->first;
                            M.AddClean(cellid, matno);
                        }
                        else
                        {
                            matnos.clear();
                            matvf.clear();
                            for(std::map<int,int>::const_iterator it = counts.begin();
                                it != counts.end(); ++it)
                            {
                                matnos.push_back(it->first);
                                // Just weight points equally to make a volume fraction.
                                matvf.push_back(float(it->second) / float(npts));
                            }
                            M.AddMixed(cellid, &matnos[0], &matvf[0], matnos.size());
                        }                        
                    }
                    idlist->Delete();
                }
                df = avtMaterial::Destruct;
                int dims[1];
                int ndims = 1;
                dims[0] = pos->second.dataset->GetNumberOfCells();
                retval = (void *)M.CreateMaterial(dims, ndims);
            }
        }
    }
#endif

    return retval;
}
