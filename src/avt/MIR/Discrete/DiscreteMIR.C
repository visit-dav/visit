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

#include "DiscreteMIR.h"

#include <map>

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <TimingsManager.h>

#include <avtMaterial.h>
#include <avtMixedVariable.h>
#include <vtkCell.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkDataSetFromVolume.h>
#include <vtkIdTypeArray.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkTriangulationTables.h>
#include <vtkObjectFactory.h>

#include <VisItArray.h>

// Programmer: John C. Anderson, Fri Oct 31 12:53:10 2008
#define RANDOM ((double) rand() / (double) RAND_MAX)


// WARNING by John C. Anderson: The following arrays are probably
// somewhere else in VisIt/VTK, but for now I've just copied them from
// my own code.
int DiscreteMIR::Cell::m_incidentCell4[4][2] =
{
    {  0, -1 },
    {  1,  0 },
    {  0,  1 },
    { -1,  0 }
};

int DiscreteMIR::Cell::m_incidentCell6[6][3] =
{
    {  0, -1,  0 },
    {  1,  0,  0 },
    {  0,  1,  0 },
    { -1,  0,  0 },
    {  0,  0, -1 },
    {  0,  0,  1 }
};

int DiscreteMIR::Cell::m_incidentCell8[8][2] =
{
    { -1, -1 },
    {  0, -1 },
    {  1, -1 },
    {  1,  0 },
    {  1,  1 },
    {  0,  1 },
    { -1,  1 },
    { -1,  0 }
};

int DiscreteMIR::Cell::m_incidentCell26[26][3] =
{
    { -1, -1, -1 },
    {  0, -1, -1 },
    {  1, -1, -1 },
    { -1,  0, -1 },
    {  0,  0, -1 },
    {  1,  0, -1 },
    { -1,  1, -1 },
    {  0,  1, -1 },
    {  1,  1, -1 },

    { -1, -1,  0 },
    {  0, -1,  0 },
    {  1, -1,  0 },
    { -1,  0,  0 },
    //{  0,  0,  0 },
    {  1,  0,  0 },
    { -1,  1,  0 },
    {  0,  1,  0 },
    {  1,  1,  0 },


    { -1, -1,  1 },
    {  0, -1,  1 },
    {  1, -1,  1 },
    { -1,  0,  1 },
    {  0,  0,  1 },
    {  1,  0,  1 },
    { -1,  1,  1 },
    {  0,  1,  1 },
    {  1,  1,  1 }
};

int DiscreteMIR::Cell::m_incidentNode4[4][2] =
{
    {  0,  0 },
    {  1,  0 },
    {  1,  1 },
    {  0,  1 }
};

int DiscreteMIR::Cell::m_incidentNode8[8][3] =
{
    {  0,  0,  0 },
    {  1,  0,  0 },
    {  1,  1,  0 },
    {  0,  1,  0 },
    {  0,  0,  1 },
    {  1,  0,  1 },
    {  1,  1,  1 },
    {  0,  1,  1 },
};

int DiscreteMIR::Node::m_incidentCell4[4][2] =
{
    {  0, -1 },
    {  0,  0 },
    { -1,  0 },
    { -1, -1 }
};

int DiscreteMIR::Node::m_incidentCell8[8][3] =
{
    {  0, -1, -1 },
    {  0,  0, -1 },
    { -1,  0, -1 },
    { -1, -1, -1 },
    {  0, -1,  0 },
    {  0,  0,  0 },
    { -1,  0,  0 },
    { -1, -1,  0 }
};

int DiscreteMIR::Node::m_neighborNode[4][2] =
{
    {  0, -1 },
    {  1,  0 },
    {  0,  1 },
    { -1,  0 }
};
// END WARNING by John C. Anderson.

// ****************************************************************************
//  Constructor:  DiscreteMIR::DiscreteMIR
//
//  Programmer:  John C. Anderson
//  Creation:    October 17, 2008
//
//  Modifications:
//
// ****************************************************************************
DiscreteMIR::DiscreteMIR()
{
    singleMat = -1;
    dimension = -1;
    mesh = NULL;
    outPts = NULL;

    DX = DY = DZ = 5;
    NX = NY = NZ = 1;

    xspacing = NULL;
    yspacing = NULL;
    zspacing = NULL;

    m_labels = NULL;
    m_neighborhood = NULL;

    m_temperature = 0.25;

    dimensions[0] = dimensions[1] = dimensions[2] = -1;
}

// ****************************************************************************
//  Destructor:  DiscreteMIR::~DiscreteMIR
//
//  Programmer:  John C. Anderson
//  Creation:    October 17, 2008
//
//  Modifications:
//
//    Hank Childs, Fri Nov 27 07:08:22 CET 2009
//    Fix compilation error of some linux boxes.
//
// ****************************************************************************
DiscreteMIR::~DiscreteMIR()
{
    if(m_labels)
    {
        int nCells = mesh->GetNumberOfCells();
        for(int i = 0; i < nCells; ++i)
        {
            if((m_labels[i]) != NULL)
                free(m_labels[i]);
        }
        free(m_labels);
    }

    if(m_neighborhood)
        free(m_neighborhood);
}

// ****************************************************************************
//  Method:  DiscreteMIR::Reconstruct3DMesh
//
//  Purpose:
//    Main method for interface reconstruction in 3D.
//
//  Arguments:
//    mesh_orig       the mesh
//    mat_orig        the material
//
//  Programmer:  John C. Anderson
//  Creation:    October 17, 2008
//
// ****************************************************************************
bool
DiscreteMIR::Reconstruct3DMesh(vtkDataSet *mesh_orig, avtMaterial *mat_orig)
{
    return ReconstructMesh(mesh_orig, mat_orig, 3);
}



// ****************************************************************************
//  Method:  DiscreteMIR::Reconstruct2DMesh
//
//  Purpose:
//    Main method for interface reconstruction in 2D.
//
//  Arguments:
//    mesh_orig       the mesh
//    mat_orig        the material
//
//  Programmer:  John C. Anderson
//  Creation:    October 17, 2008
//
// ****************************************************************************
bool
DiscreteMIR::Reconstruct2DMesh(vtkDataSet *mesh_orig, avtMaterial *mat_orig)
{
    return ReconstructMesh(mesh_orig, mat_orig, 2);
}

// ****************************************************************************
//  Method:  DiscreteMIR::ReconstructMesh
//
//  Purpose:
//    Main method for interface reconstruction in any dimension.
//
//  Arguments:
//    mesh_orig       the mesh
//    mat_orig        the material
//
//  Programmer:  John C. Anderson
//  Creation:    October 17, 2008
//
//  Modifications:
//
//    Hank Childs, Fri Nov 27 07:08:22 CET 2009
//    Replace assert with exception.
//
// ****************************************************************************
bool
DiscreteMIR::ReconstructMesh(vtkDataSet *mesh_orig, avtMaterial *mat_orig, int dim)
{
    // Check that Reconstruct hasn't already been called.
    if(0)
    {
        EXCEPTION1(ImproperUseException,
                   "Reconstruct has already been called!");
    }

    // Check that we have a vtkRectilinearGrid.
    if (mesh_orig->GetDataObjectType() != VTK_RECTILINEAR_GRID)
    {
        EXCEPTION1(ImproperUseException,
                   "DiscreteMIR only works upon rectilinear grids.");
    }

    // Store the mesh for use later.
    mesh = mesh_orig;
    mesh->Register(NULL);
    nOrigMaterials = mat_orig->GetNMaterials();

    // see if we can perform a clean-zone-only algorithm
    if (options.cleanZonesOnly ||
        mat_orig->GetMixlen() <= 0 ||
        mat_orig->GetMixMat() == NULL)
    {
        noMixedZones = true;
        bool status = ReconstructCleanMesh(mesh, mat_orig);
        return status;
    }

    // start a timer
    int timerHandle = visitTimer->StartTimer();

    // Set the dimensionality
    dimension = dim;

    // Set the connectivity
    MIRConnectivity conn;
    conn.SetUpConnectivity(mesh);

    // Pack the material
    avtMaterial *mat = mat_orig->CreatePackedMaterial();

    mapMatToUsedMat = mat_orig->GetMapMatToUsedMat();
    mapUsedMatToMat = mat_orig->GetMapUsedMatToMat();

    // Get some attributes
    noMixedZones = false;
    nMaterials   = mat->GetNMaterials();

    int nCells  = mesh->GetNumberOfCells();
    int nPoints = mesh->GetNumberOfPoints();

    // extract coordinate arrays
    SetUpCoords();

    // Do the discrete optimization.
    int timerHandle2 = visitTimer->StartTimer();

    // Matlist pointer.
    const int *matlist = mat->GetMatlist();

    // Extract the grid dimensions.
    vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *) mesh;
    rgrid->GetDimensions(dimensions);

    // I want dimensions[...] to be the number of zones, not nodes.
    --dimensions[0];
    --dimensions[1];
    if(dimension == 3)
        --dimensions[2];

    // Seed the random number generator.
    srand((int) time(NULL));

    // Intialize the label pointer array.
    m_labels = (unsigned char **) calloc(dimensions[0] * dimensions[1] * dimensions[2],
                                         sizeof(unsigned char *));
    if (m_labels == NULL)
        EXCEPTION1(ImproperUseException, "Memory allocation problem");

    if(dimension == 2)
    {
        DZ = 1;
        NZ = 0;
    }

    m_size[0] = DX * dimensions[0];
    m_size[1] = DY * dimensions[1];
    m_size[2] = DZ * dimensions[2];

    // Initialize the cell neighborhoods.
    m_neighborhood = (float *) calloc((2 * NX + 1) * (2 * NY + 1) * (2 * NZ + 1), sizeof(float));
    float *n = m_neighborhood;
    for(int z = -NZ; z <= NZ; ++z)
        for(int y = -NY; y <= NY; ++y)
            for(int x = -NX; x <= NX; ++x)
            {
                if(z == 0 && y == 0 && x == 0)
                    *n++ = 0;
                else
                    *n++ = 1.0 / sqrtf(x * x + y * y + z * z);
            }

    // Initialize the label array:
    //   - negative values indicate pure cells material;
    //   - positive values are pointers to discretized blocks.
    for(int k = 0; k < dimensions[2]; ++k)
        for(int j = 0; j < dimensions[1]; ++j)
            for(int i = 0; i < dimensions[0]; ++i)
            {
                Cell cell(i, j, k);
                int cellid = id(cell);

                if(matlist[cellid] >= 0)
                {
                    m_labels[cellid] = (unsigned char *) -matlist[cellid];
                    continue;
                }

                // Add the cell to the list of mixed cells.
                m_mixedCells.push_back(cell);

                // Initialize the label array for this cell.
                m_labels[cellid] =
                    (unsigned char *) calloc(DX * DY * DZ, sizeof(unsigned char));
                if (m_labels[cellid] == NULL)
                    EXCEPTION1(ImproperUseException, "Memory allocation problem");
            }

    // Initialize the discretized blocks.
    int target[nMaterials];
    int count[nMaterials];
    for(std::vector< Cell >::iterator iter = m_mixedCells.begin();
        iter != m_mixedCells.end();
        ++iter)
    {
        Cell cell(*iter);
        int cellid = id(cell);

        std::vector< float > vfs;
        mat->GetVolFracsForZone(cellid, vfs);

        int total = 0;
        for(int m = 0; m < nMaterials; ++m)
        {
            target[m] =
                DX * DY * DZ *
                vfs[m];

            total += target[m];
            count[m] = 0;
        }

        // Make sure all volume is accounted for.
        for(int m = 0; m < nMaterials; ++m)
        {
            if(target[m])
            {
                target[m] += DX * DY * DZ - total;
                break;
            }
        }
        total = DX * DY * DZ;

        unsigned char
            *tmp = m_labels[cellid],
            *end = tmp + (DX * DY * DZ);

        while(tmp != end)
        {
            double r = total * RANDOM;

            unsigned int l = 0;
            int cdf = target[0] - count[0];
            while(r > cdf)
            {
                ++l;
                cdf += target[l] - count[l];
            }

            count[l]++;
            total--;

            *tmp++ = l;
        }
    }

    // Perform some optimization of the labeling...
    optimize();

    visitTimer->StopTimer(timerHandle2, "DiscreteMIR: Finished optimization");
    visitTimer->StopTimer(timerHandle, "Full MIR reconstruction");
    visitTimer->DumpTimings();

    // Some pure zones will be next to mixed zones; we'll process
    // these "cleanish" zones later.
    std::vector< Cell > cleanish;

    // Go through the non-mixed cells and divide them into clean and
    // cleanish.  Cleanish cells are non-mixed cells incident to mixed
    // cells.  They'll need some special handling to connect with
    // mixed cells.
    int *conn_ptr = conn.connectivity;
    zonesList.reserve(nCells);
    for(int k = 0; k < dimensions[2]; ++k)
        for(int j = 0; j < dimensions[1]; ++j)
            for(int i = 0; i < dimensions[0]; ++i)
            {
                Cell cell(i, j, k);
                int cellid = id(cell);

                int nIds = *conn_ptr;
                const int *ids = conn_ptr+1;

                conn_ptr += nIds+1;

                if(matlist[cellid] < 0)
                    continue;
                else
                {
                    for(int n = 0; n < ((dimension == 2) ? 4 : 26); ++n)
                    {
                        Cell celln =
                            (dimension == 2) ?
                            cell.incidentCell4(n) :
                            cell.incidentCell26(n);
                        if(matlist[id(celln)] < 0)
                        {
                            cleanish.push_back(cell);
                            goto NEXT_CELL;
                        }
                    }
                }

#if 1
                ReconstructedZone zone;
                zone.origzone   = cellid;
                zone.mat        = matlist[cellid];
                zone.celltype   = conn.celltype[cellid];
                zone.nnodes     = nIds;
                zone.startindex = indexList.size();
                zone.mix_index  = -1;

                for(int n = 0; n < nIds; n++)
                    indexList.push_back(ids[n]);

                zonesList.push_back(zone);
#endif

                // We go here when we want to skip cleanish cells.
                NEXT_CELL:
                ;
            }

    // We don't want to duplicate inserted nodes...  Populate this map
    // with the absolute discretized node coordinate.  This might be
    // very slow -- but for now it should work.
    std::map< Node, int > discretized_inserted_nodes;

    // Start inserting new nodes.
    for(std::vector< Cell >::iterator iter = m_mixedCells.begin();
        iter != m_mixedCells.end();
        ++iter)
    {
        Cell cell(*iter);
        int cellid = id(cell);

        int basex = DX * cell.m_i,
            basey = DY * cell.m_j,
            basez = DZ * cell.m_k;

        for(int z = 0; z <= DZ; ++z)
            for(int y = 0; y <= DY; ++y)
                for(int x = 0; x <= DX; ++x)
                {
                    Node node(basex + x, basey + y, basez + z);

                    // Don't insert cell corner points or duplicates
                    // -- corner points should be references to
                    // original mesh nodes.
                    if((x == 0 || x == DX) &&
                       (y == 0 || y == DY) &&
                       (z == 0 || z == DZ))
                    {
                        discretized_inserted_nodes[node] =
                            id(Node(cell.m_i + (x ? 1 : 0),
                                    cell.m_j + (y ? 1 : 0),
                                    cell.m_k + (z ? 1 : 0)));
                        continue;
                    }
                    else
                    {
                        if(discretized_inserted_nodes.count(node))
                            continue;
                        else
                            discretized_inserted_nodes[node] = origNPoints + coordsList.size();
                    }

                    float dx = (float) x / DX;
                    float dy = (float) y / DY;
                    float dz = (float) z / DZ;

                    ReconstructedCoord coord;
                    coord.x = (1 - dx) * xspacing[cell.m_i] + dx * xspacing[cell.m_i + 1];
                    coord.y = (1 - dy) * yspacing[cell.m_j] + dy * yspacing[cell.m_j + 1];
                    if(dimension == 3)
                        coord.z = (1 - dz) * zspacing[cell.m_k] + dz * zspacing[cell.m_k + 1];
                    else
                        coord.z = 0;

                    if(dimension == 2)
                    {
                        coord.weight[0] = (1 - dx) * (1 - dy);
                        coord.weight[1] = (    dx) * (1 - dy);
                        coord.weight[2] = (    dx) * (    dy);
                        coord.weight[3] = (1 - dx) * (    dy);
                    }
                    else
                    {
                        coord.weight[0] = (1 - dx) * (1 - dy) * (1 - dz);
                        coord.weight[1] = (    dx) * (1 - dy) * (1 - dz);
                        coord.weight[2] = (    dx) * (    dy) * (1 - dz);
                        coord.weight[3] = (1 - dx) * (    dy) * (1 - dz);
                        coord.weight[4] = (1 - dx) * (1 - dy) * (    dz);
                        coord.weight[5] = (    dx) * (1 - dy) * (    dz);
                        coord.weight[6] = (    dx) * (    dy) * (    dz);
                        coord.weight[7] = (1 - dx) * (    dy) * (    dz);
                    }

                    coord.origzone = cellid;

                    coordsList.push_back(coord);
                }
    }

#if 1
    // Create discretized zones in mixed cells.
    for(std::vector< Cell >::iterator iter = m_mixedCells.begin();
        iter != m_mixedCells.end();
        ++iter)
    {
        Cell cell(*iter);
        int cellid = id(cell);

        int basex = DX * cell.m_i,
            basey = DY * cell.m_j,
            basez = DZ * cell.m_k;

        for(int z = 0; z < DZ; ++z)
            for(int y = 0; y < DY; ++y)
                for(int x = 0; x < DX; ++x)
                {
                    ReconstructedZone zone;
                    zone.origzone   = cellid;
                    zone.startindex = indexList.size();
                    zone.mix_index  = -1; // TODO: What is this values supposed to be?
                    zone.mat        = m_labels[cellid][DX * DY * z + DX * y + x];
                    zone.celltype   = (dimension == 3) ? VTK_HEXAHEDRON : VTK_QUAD;
                    zone.nnodes     = (dimension == 3) ? 8 : 4;

                    int corner_positions[8][3] = {
                        { 0, 0, 0 },
                        { 1, 0, 0 },
                        { 1, 1, 0 },
                        { 0, 1, 0 },
                        { 0, 0, 1 },
                        { 1, 0, 1 },
                        { 1, 1, 1 },
                        { 0, 1, 1 }
                    };

                    for(int i = 0; i < ((dimension == 2) ? 4 : 8); ++i)
                    {
                        Node node(basex + x + corner_positions[i][0],
                                  basey + y + corner_positions[i][1],
                                  basez + z + corner_positions[i][2]);
                        indexList.push_back(discretized_inserted_nodes[node]);
                    }

                    zonesList.push_back(zone);
                }
    }
#endif

    // Handle clean zones neighboring mixed zones -- "cleanish" zones.
    // To understand this code start with the 2D case.
    //
    // In 2D, I insert a new centroid node within the cleanish cell.
    // Then I make an ordered "ring" of the nodes (discretized and
    // original) around the cell.  Then, I make triangles to
    // successive pairs of ring vetices with the centroid.
    //
    // TODO: Describe the 3D process...
    if(dimension == 2)
    {
        for(std::vector< Cell >::iterator iter = cleanish.begin();
            iter != cleanish.end();
            ++iter)
        {
            Cell cell(*iter);
            int cellid = id(cell);

            // Insert zone (i.e., face) centroid.
            ReconstructedCoord coord;
            float dx = 0.5;
            float dy = 0.5;
            coord.x = (1 - dx) * xspacing[cell.m_i] + dx * xspacing[cell.m_i + 1];
            coord.y = (1 - dy) * yspacing[cell.m_j] + dy * yspacing[cell.m_j + 1];
            coord.z = 0;
            coord.weight[0] = (1 - dx) * (1 - dy);
            coord.weight[1] = (    dx) * (1 - dy);
            coord.weight[2] = (    dx) * (    dy);
            coord.weight[3] = (1 - dx) * (    dy);
            coord.origzone = cellid;
            coordsList.push_back(coord);

            // Make a list of border vertices (i.e., outside of
            // triangle fan around centroid).
            std::vector< int > ring;
            for(int n = 0; n < 4; ++n)
            {
                Cell celln = cell.incidentCell4(n);
                int cellidn = id(celln);

                // First node of the edge across which we're looking.
                Node edge_begin = cell.incidentNode4(n);
                Node n1 = edge_begin;
                n1.m_i *= DX;
                n1.m_j *= DY;

                // If it is mixed, then we have a discretized
                // boundary.  Add in the nodes along the boundary.
                if(isValid(celln) && matlist[cellidn] < 0)
                {
                    Node n2 = cell.incidentNode4((n + 1) % 4);
                    n2.m_i *= DX;
                    n2.m_j *= DY;

                    while(n1.m_i != n2.m_i || n1.m_j != n2.m_j)
                    {
                        ring.push_back(discretized_inserted_nodes[n1]);

                        if(n1.m_i < n2.m_i)
                            n1.m_i += 1;
                        else if(n1.m_i > n2.m_i)
                            n1.m_i -= 1;
                        else if(n1.m_j < n2.m_j)
                            n1.m_j += 1;
                        else if(n1.m_j > n2.m_j)
                            n1.m_j -= 1;
                    }

                }
                else
                    ring.push_back(id(edge_begin));
            }

            for(int t = 0; t < ring.size(); ++t)
            {
                ReconstructedZone zone;
                zone.origzone   = cellid;
                zone.startindex = indexList.size();
                zone.mix_index  = -1; // TODO: What is this values supposed to be?
                zone.mat        = matlist[cellid];
                zone.celltype   = VTK_TRIANGLE;
                zone.nnodes     = 3;

                indexList.push_back(origNPoints + coordsList.size() - 1);
                indexList.push_back(ring[t]);
                indexList.push_back(ring[(t + 1) % ring.size()]);

                zonesList.push_back(zone);
            }
        }
    }
    // 3D cleanish cells.
    else
    {
        // Store the nodes that we insert upon faces; these should not
        // be duplicated.  Like the discretized_inserted_nodes map,
        // this is probably a slow, lousy way to avoid dulication.
        // This structure maps two cells to the handle of the inserted
        // node on the face between the cells.
        //
        // The inserted face node is associated with the "smaller"
        // cell (i.e., coordinate-wise smallest).
        std::map< std::pair< int, int >, int > cleanish_face_centroids;

        for(std::vector< Cell >::iterator iter = cleanish.begin();
            iter != cleanish.end();
            ++iter)
        {
            Cell cell(*iter);
            int cellid = id(cell);

            int basex = DX * cell.m_i,
                basey = DY * cell.m_j,
                basez = DZ * cell.m_k;

            // Insert zone centroid.
            ReconstructedCoord coord;
            float dx = 0.5;
            float dy = 0.5;
            float dz = 0.5;
            coord.x = (1 - dx) * xspacing[cell.m_i] + dx * xspacing[cell.m_i + 1];
            coord.y = (1 - dy) * yspacing[cell.m_j] + dy * yspacing[cell.m_j + 1];
            coord.z = (1 - dz) * zspacing[cell.m_k] + dz * zspacing[cell.m_k + 1];
            coord.weight[0] = (1 - dx) * (1 - dy) * (1 - dz);
            coord.weight[1] = (    dx) * (1 - dy) * (1 - dz);
            coord.weight[2] = (    dx) * (    dy) * (1 - dz);
            coord.weight[3] = (1 - dx) * (    dy) * (1 - dz);
            coord.weight[4] = (1 - dx) * (1 - dy) * (    dz);
            coord.weight[5] = (    dx) * (1 - dy) * (    dz);
            coord.weight[6] = (    dx) * (    dy) * (    dz);
            coord.weight[7] = (1 - dx) * (    dy) * (    dz);
            coord.origzone = cellid;

            int centroid = origNPoints + coordsList.size();

            coordsList.push_back(coord);

            // Marked face list, stores:
            //   * 0: no information,
            //   * 1: discretized,
            //   * 2: in ring.
            int face_marks[6] = { 0, 0, 0, 0, 0, 0 };
            
            // List of points on the hexahedron.
            int point_list[8][3] = {
                {     cell.m_i,     cell.m_j,     cell.m_k },
                { cell.m_i + 1,     cell.m_j,     cell.m_k },
                {     cell.m_i, cell.m_j + 1,     cell.m_k },
                { cell.m_i + 1, cell.m_j + 1,     cell.m_k },
                {     cell.m_i,     cell.m_j, cell.m_k + 1 },
                { cell.m_i + 1,     cell.m_j, cell.m_k + 1 },
                {     cell.m_i, cell.m_j + 1, cell.m_k + 1 },
                { cell.m_i + 1, cell.m_j + 1, cell.m_k + 1 },
            };

            // Face list 1, stores:
            //   * 4 points of each face;
            //   * axis not to increment.
            int face_list[6][5] = {
                { 0, 1, 5, 4,    1 },
                { 1, 3, 7, 5,    0 },
                { 2, 6, 7, 3,    1 },
                { 0, 4, 6, 2,    0 },
                { 0, 2, 3, 1,    2 },
                { 4, 5, 7, 6,    2 }
            };

            // Special list of 12 edges, stores:
            //   * 1-3) local coordinates of edge node,
            //   * 4) increment axis,
            //   * 5) first associated face,
            //   * 6) second associated face.
            //
            // I had to make a labeled, paper cube to get these
            // correct...
            int edge_list[12][6] = {
                { 0, 0, 0,    0,    0, 4 },
                { 0, 0, 1,    0,    0, 5 },
                { 0, 1, 0,    0,    2, 4 },
                { 0, 1, 1,    0,    2, 5 },

                { 0, 0, 0,    1,    3, 4 },
                { 0, 0, 1,    1,    3, 5 },
                { 1, 0, 0,    1,    1, 4 },
                { 1, 0, 1,    1,    1, 5 },

                { 0, 0, 0,    2,    0, 3 },
                { 0, 1, 0,    2,    2, 3 },
                { 1, 0, 0,    2,    0, 1 },
                { 1, 1, 0,    2,    1, 2 }
            };

            // Neighboring cell ids.
            int nids[6] = { -1, -1, -1, -1, -1, -1 };

            // Go over each cell face.  If the face is discretized
            // (i.e., neighbor was discretized), then make some
            // pyramids from the centoid to the discretized nodes.
            //
            // TODO by John C. Anderson: This is really kludgy, there
            // is bound to be a nicer way to enumerate the pyramids to
            // a discretized face.
            for(int n = 0; n < 6; ++n)
            {
                Cell celln = cell.incidentCell6(n);

                if(!isValid(celln))
                    continue;

                nids[n] = id(celln);

                if(matlist[nids[n]] >= 0)
                    continue;

                face_marks[n] = 1;

#if 1
                int fx = DX * point_list[face_list[n][0]][0];
                int fy = DY * point_list[face_list[n][0]][1];
                int fz = DZ * point_list[face_list[n][0]][2];

                for(int b = 0; b < ((face_list[n][4] == 2) ? DY : DZ); ++b)
                    for(int a = 0; a < ((face_list[n][4] == 0) ? DY : DX); ++a)
                    {
                        ReconstructedZone zone;
                        zone.origzone   = cellid;
                        zone.startindex = indexList.size();
                        zone.mix_index  = -1; // TODO: What is this values supposed to be?
                        zone.mat        = matlist[cellid];
                        zone.celltype   = VTK_PYRAMID;
                        zone.nnodes     = 5;
                        
                        // The pyramid bases need to be oriented
                        // inward -- I might come back to this and
                        // unify it a bit later.
                        switch(n) {
                          case 0:
                            indexList.push_back(discretized_inserted_nodes[Node(fx + a,     fy, fz + b)]);
                            indexList.push_back(discretized_inserted_nodes[Node(fx + a + 1, fy, fz + b)]);
                            indexList.push_back(discretized_inserted_nodes[Node(fx + a + 1, fy, fz + b + 1)]);
                            indexList.push_back(discretized_inserted_nodes[Node(fx + a,     fy, fz + b + 1)]);
                            break;

                          case 1:
                            indexList.push_back(discretized_inserted_nodes[Node(fx, fy + a,     fz + b)]);
                            indexList.push_back(discretized_inserted_nodes[Node(fx, fy + a + 1, fz + b)]);
                            indexList.push_back(discretized_inserted_nodes[Node(fx, fy + a + 1, fz + b + 1)]);
                            indexList.push_back(discretized_inserted_nodes[Node(fx, fy + a,     fz + b + 1)]);
                            break;
                            
                          case 2:
                            indexList.push_back(discretized_inserted_nodes[Node(fx + a,     fy, fz + b)]);
                            indexList.push_back(discretized_inserted_nodes[Node(fx + a,     fy, fz + b + 1)]);
                            indexList.push_back(discretized_inserted_nodes[Node(fx + a + 1, fy, fz + b + 1)]);
                            indexList.push_back(discretized_inserted_nodes[Node(fx + a + 1, fy, fz + b)]);
                            break;
                            
                          case 3:
                            indexList.push_back(discretized_inserted_nodes[Node(fx, fy + a,     fz + b)]);
                            indexList.push_back(discretized_inserted_nodes[Node(fx, fy + a,     fz + b + 1)]);
                            indexList.push_back(discretized_inserted_nodes[Node(fx, fy + a + 1, fz + b + 1)]);
                            indexList.push_back(discretized_inserted_nodes[Node(fx, fy + a + 1, fz + b)]);
                            break;
                            
                          case 4:
                            indexList.push_back(discretized_inserted_nodes[Node(fx + a,     fy + b,     fz)]);
                            indexList.push_back(discretized_inserted_nodes[Node(fx + a,     fy + b + 1, fz)]);
                            indexList.push_back(discretized_inserted_nodes[Node(fx + a + 1, fy + b + 1, fz)]);
                            indexList.push_back(discretized_inserted_nodes[Node(fx + a + 1, fy + b,     fz)]);
                            break;

                          case 5:
                            indexList.push_back(discretized_inserted_nodes[Node(fx + a,     fy + b,     fz)]);
                            indexList.push_back(discretized_inserted_nodes[Node(fx + a + 1, fy + b,     fz)]);
                            indexList.push_back(discretized_inserted_nodes[Node(fx + a + 1, fy + b + 1, fz)]);
                            indexList.push_back(discretized_inserted_nodes[Node(fx + a,     fy + b + 1, fz)]);
                            break;

                          default:
                            EXCEPTION1(ImproperUseException, "Unexpected case");
                        }

                        indexList.push_back(centroid);                 
                        
                        zonesList.push_back(zone);
                    }
#endif
                }

            // Go over each edge.  Make a list of small tets and big
            // tets that should be incident to this edge.
            std::vector< int > small_tets, big_tets;
            for(int e = 0; e < 12; ++e)
            {
                int xyz_base[3] = { cell.m_i + edge_list[e][0],
                                    cell.m_j + edge_list[e][1],
                                    cell.m_k + edge_list[e][2] };

                int xyz_disc[3] = { DX * xyz_base[0],
                                    DY * xyz_base[1],
                                    DZ * xyz_base[2] };

                xyz_disc[edge_list[e][3]]++;

                // If this edge is not discretized then add big tets.
                if(!discretized_inserted_nodes.count(Node(xyz_disc[0], xyz_disc[1], xyz_disc[2])))
                {
                    // TODO: Describe...
                    for(int f = 4; f < 6; ++f)
                        if(face_marks[edge_list[e][f]] != 1)
                        {
                            big_tets.push_back(edge_list[e][f]);
                            big_tets.push_back(id(Node(xyz_base[0], xyz_base[1], xyz_base[2])));
                            ++xyz_base[edge_list[e][3]];
                            big_tets.push_back(id(Node(xyz_base[0], xyz_base[1], xyz_base[2])));
                            --xyz_base[edge_list[e][3]];
                        }
                }
                // Otherwise, add in small tets and mark incident
                // faces as needing a centroid.
                else
                {
                    // Make sure to decrement the discrete marker back
                    // to its original value.
                    int ahead = --xyz_disc[edge_list[e][3]];
                    int behind = ahead;

                    int max =
                        xyz_disc[edge_list[e][3]] +
                        ((edge_list[e][3] == 0) ? DX :
                         ((edge_list[3][3] == 1) ? DY : DZ));

                    while(ahead < max)
                    {
                        ++ahead;

                        // TODO: Describe...
                        for(int f = 4; f < 6; ++f)
                            if(face_marks[edge_list[e][f]] != 1)
                            {
                                face_marks[edge_list[e][f]] = 2;

                                small_tets.push_back(edge_list[e][f]);
                                xyz_disc[edge_list[e][3]] = behind;                         
                                small_tets.push_back(discretized_inserted_nodes[Node(xyz_disc[0], xyz_disc[1], xyz_disc[2])]);
                                xyz_disc[edge_list[e][3]] = ahead;
                                small_tets.push_back(discretized_inserted_nodes[Node(xyz_disc[0], xyz_disc[1], xyz_disc[2])]);
                            }

                        ++behind;
                    }
                }
            }

            // Face centroid handles.
            int centroid_handles[6] = { 0, 0, 0, 0, 0, 0 };

            // Face centroid coordinates.
            float centroid_coords[6][3] = {
                { 0.5,   0, 0.5 },
                {   1, 0.5, 0.5 },
                { 0.5,   1, 0.5 },
                {   0, 0.5, 0.5 },
                { 0.5, 0.5,   0 },
                { 0.5, 0.5,   1 }
            };

            // Insert required face centroid nodes.
            for(int n = 0; n < 6; ++n)
            {
                // Make sure this face needs a centroid.
                if(face_marks[n] != 2)
                    continue;

                std::pair< int, int > k;
                if(nids[n] >= 0)
                {
                    k = std::make_pair(cellid, nids[n]);
                    if(nids[n] < cellid)
                        std::swap(k.first, k.second);

                    if(cleanish_face_centroids.count(k))
                    {
                        centroid_handles[n] = cleanish_face_centroids[k];
                        continue;
                    }
                    else
                    {
                        centroid_handles[n] = origNPoints + coordsList.size();
                        cleanish_face_centroids[k] = centroid_handles[n];
                    }
                }
                else
                    centroid_handles[n] = origNPoints + coordsList.size();

                // Insert face centroid.
                ReconstructedCoord coord;
                float dx = centroid_coords[n][0];
                float dy = centroid_coords[n][1];
                float dz = centroid_coords[n][2];
                coord.x = (1 - dx) * xspacing[cell.m_i] + dx * xspacing[cell.m_i + 1];
                coord.y = (1 - dy) * yspacing[cell.m_j] + dy * yspacing[cell.m_j + 1];
                coord.z = (1 - dz) * zspacing[cell.m_k] + dz * zspacing[cell.m_k + 1];
                coord.weight[0] = (1 - dx) * (1 - dy) * (1 - dz);
                coord.weight[1] = (    dx) * (1 - dy) * (1 - dz);
                coord.weight[2] = (    dx) * (    dy) * (1 - dz);
                coord.weight[3] = (1 - dx) * (    dy) * (1 - dz);
                coord.weight[4] = (1 - dx) * (1 - dy) * (    dz);
                coord.weight[5] = (    dx) * (1 - dy) * (    dz);
                coord.weight[6] = (    dx) * (    dy) * (    dz);
                coord.weight[7] = (1 - dx) * (    dy) * (    dz);
                
                coord.origzone = (nids[n] >= 0) ? k.first : cellid;             

                coordsList.push_back(coord);
            }

#if 1
            // Make each of the small tets.
            for(int st = 0; st < small_tets.size(); st += 3)
            {
                // Make sure that the centroid handle exists.
                if (centroid_handles[small_tets[st]] == 0)
                    EXCEPTION1(ImproperUseException, "Memory allocation problem");

                ReconstructedZone zone;
                zone.origzone   = cellid;
                zone.startindex = indexList.size();
                zone.mix_index  = -1; // TODO: What is this values supposed to be?
                zone.mat        = matlist[cellid];
                zone.celltype   = VTK_TETRA;
                zone.nnodes     = 4;

                indexList.push_back(centroid);
                indexList.push_back(centroid_handles[small_tets[st]]);
                indexList.push_back(small_tets[st + 1]);
                indexList.push_back(small_tets[st + 2]);

                zonesList.push_back(zone);
            }
#endif

#if 1
            // Maybe make the big tets if the corresponding face has a
            // centroid point.  If there is no centroid point, then
            // we'll skip it and handle it in the next section.
            for(int bt = 0; bt < big_tets.size(); bt += 3)
            {
                // Skip this tet if the corresponding face's centroid
                // doesn't exist.
                if(face_marks[big_tets[bt]] != 2)
                    continue;
                
                // Make sure that the centroid handle exists.
                if (centroid_handles[big_tets[bt]] == 0)
                    EXCEPTION1(ImproperUseException, "Memory allocation problem");

                ReconstructedZone zone;
                zone.origzone   = cellid;
                zone.startindex = indexList.size();
                zone.mix_index  = -1; // TODO: What is this values supposed to be?
                zone.mat        = matlist[cellid];
                zone.celltype   = VTK_TETRA;
                zone.nnodes     = 4;

                indexList.push_back(centroid);
                indexList.push_back(centroid_handles[big_tets[bt]]);
                indexList.push_back(big_tets[bt + 1]);
                indexList.push_back(big_tets[bt + 2]);

                zonesList.push_back(zone);
            }      
#endif
 
#if 1
            // Make big pyramids from the centroid to faces that
            // haven't been handled so far...
            for(int n = 0; n < 6; ++n)
            {
                // Skip the face if we've done something with it.
                if(face_marks[n] != 0)
                    continue;

                ReconstructedZone zone;
                zone.origzone   = cellid;
                zone.startindex = indexList.size();
                zone.mix_index  = -1; // TODO: What is this values supposed to be?
                zone.mat        = matlist[cellid];
                zone.celltype   = VTK_PYRAMID;
                zone.nnodes     = 5;

                for(int p = 0; p < 4; ++p)
                {
                    int *pt = &point_list[face_list[n][p]][0];
                    indexList.push_back(id(Node(pt[0], pt[1], pt[2])));
                }

                indexList.push_back(centroid);

                zonesList.push_back(zone);
            }
#endif

        }
    }

    delete mat;

    return true;
}


// ****************************************************************************
//  Method:  DiscreteMIR::GetDataset
//
//  Purpose:
//    TODO John: Fill this in when you understand what this function does.
//
//  Arguments:
//    mats        the materials to select
//    ds          the original dataset
//    mixvars     the mixed variables
//    doMats      flag to write out the material numbers as a new zonal array
//
//  Programmer:  John C. Anderson
//  Creation:    October 17, 2008
//
//  Modifications:
//
// ****************************************************************************
vtkDataSet *
DiscreteMIR::GetDataset(std::vector<int> mats, vtkDataSet *ds,
                        std::vector<avtMixedVariable*> mixvars, bool doMats,
                        avtMaterial *mat)
{
    int timerHandle = visitTimer->StartTimer();
    bool doAllMats = mats.empty();

    //
    // Start off by determining which materials we should reconstruct and
    // which we should leave out.
    //
    bool *matFlag = new bool[nMaterials+1];
    if (!doAllMats)
    {
        for (int i = 0; i < nMaterials+1; i++)
            matFlag[i] = false;
        for (size_t i = 0; i < mats.size(); i++)
        {
            int origmatno = mats[i];
            if (origmatno == nOrigMaterials)
            {
                matFlag[nMaterials] = true;
            }
            else
            {
                int usedmatno = mapMatToUsedMat[origmatno];
                if (usedmatno != -1)
                    matFlag[usedmatno] = true;
            }
        }
    }
    else
    {
        for (int i = 0; i < nMaterials+1; i++)
            matFlag[i] = true;
    }

    //
    // If this domain was truly clean, we can re-use the input mesh
    //
    if (singleMat >= 0)
    {
        if (matFlag[singleMat])
        {
            // Since if we make a copy, we get corrupt ->Links
            // for an unstructured grid, we're just going to add
            // avtSubsets to the input mesh and
            vtkDataSet *outmesh = ds;

            // (this was the other option: ----->>>
            //vtkDataSet *outmesh = ds->NewInstance();
            //outmesh->ShallowCopy(ds);
            //if (outmesh->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
            //    ((vtkUnstructuredGrid*)outmesh)->BuildLinks();
            // <<----- )

            if (doMats)
            {
                //
                // Add the avtSubsets array
                //
                int ncells = outmesh->GetNumberOfCells();
                vtkIntArray *outmat = vtkIntArray::New();
                outmat->SetName("avtSubsets");
                outmat->SetNumberOfTuples(ncells);
                int *buff = outmat->GetPointer(0);
                for (int i=0; i<ncells; i++)
                    buff[i] = singleMat;
                outmesh->GetCellData()->AddArray(outmat);
                outmat->Delete();
            }
            outmesh->Register(NULL);
            visitTimer->StopTimer(timerHandle,
                                  "MIR: Getting *completely* clean dataset");
            visitTimer->DumpTimings();
            return outmesh;
        }
        else
        {
            visitTimer->StopTimer(timerHandle,
                                  "MIR: Getting *empty* clean dataset");
            visitTimer->DumpTimings();
            return NULL;
        }
    }

    //
    // Now count up the total number of cells we will have.
    //
    int ntotalcells = zonesList.size();
    int ncells = 0;
    int *cellList = new int[ntotalcells];
    int totalsize = 0;
    for (int c = 0; c < ntotalcells; c++)
    {
        int matno = zonesList[c].mat;
        if (matno >= 0)
        {
            if (matFlag[matno])
            {
                cellList[ncells++] = c;
                totalsize += zonesList[c].nnodes;
            }
        }
        else if (!mat)
        {
            if (matFlag[nMaterials])
            {
                cellList[ncells++] = c;
                totalsize += zonesList[c].nnodes;
            }
        }
        else
        {
            bool match = false;
            int  mixIndex = -matno - 1;
            while (mixIndex >= 0 && !match)
            {
                matno = mat->GetMixMat()[mixIndex];

                if (matFlag[matno])
                    match = true;

                mixIndex = mat->GetMixNext()[mixIndex] - 1;
            }
            if (match)
            {
                cellList[ncells++] = c;
                totalsize += zonesList[c].nnodes;
            }
        }
    }

    //
    // Instantiate the output dataset (-> VTK magic).
    //
    vtkUnstructuredGrid   *rv  = vtkUnstructuredGrid::New();
    rv->GetFieldData()->PassData(ds->GetFieldData());

    //
    // Set up the coordinate array.
    //
    int newNPoints = coordsList.size();
    int npoints = origNPoints + newNPoints;
    if (outPts == NULL)
    {
        outPts = vtkPoints::New();
        outPts->SetNumberOfPoints(npoints);
        float *pts_buff = (float *) outPts->GetVoidPointer(0);
        int outIndex = 0;
        for (int i=0; i<origNPoints; i++)
        {
            pts_buff[outIndex++] = origXCoords[i];
            pts_buff[outIndex++] = origYCoords[i];
            pts_buff[outIndex++] = origZCoords[i];
        }
        for (int i=0; i<newNPoints; i++)
        {
            pts_buff[outIndex++] = coordsList[i].x;
            pts_buff[outIndex++] = coordsList[i].y;
            pts_buff[outIndex++] = coordsList[i].z;
        }
    }
    rv->SetPoints(outPts);

    //
    // Now insert the connectivity array.
    //
    vtkIdTypeArray *nlist = vtkIdTypeArray::New();
    nlist->SetNumberOfValues(totalsize + ncells);
    vtkIdType *nl = nlist->GetPointer(0);

    vtkUnsignedCharArray *cellTypes = vtkUnsignedCharArray::New();
    cellTypes->SetNumberOfValues(ncells);
    unsigned char *ct = cellTypes->GetPointer(0);

    vtkIdTypeArray *cellLocations = vtkIdTypeArray::New();
    cellLocations->SetNumberOfValues(ncells);
    int *cl = cellLocations->GetPointer(0);

    int offset = 0;
    for (int i=0; i<ncells; i++)
    {
        int c = cellList[i];

        *ct++ = zonesList[c].celltype;

        const int nnodes = zonesList[c].nnodes;
        *nl++ = nnodes;
        const int *indices = &indexList[zonesList[c].startindex];
        for (int j=0; j<nnodes; j++)
            *nl++ = indices[j];

        *cl++ = offset;
        offset += nnodes+1;
    }

    vtkCellArray *cells = vtkCellArray::New();
    cells->SetCells(ncells, nlist);
    nlist->Delete();

    rv->SetCells(cellTypes, cellLocations, cells);
    cellTypes->Delete();
    cellLocations->Delete();
    cells->Delete();

    //
    // Copy over all node-centered data.
    //
    vtkPointData *outpd = rv->GetPointData();
    vtkPointData *inpd  = ds->GetPointData();
    if (inpd->GetNumberOfArrays() > 0)
    {
        outpd->CopyAllocate(inpd, npoints);
        int outIndex = 0;
        //
        // For each point, copy over the original point data if the
        // reconstructed coordinate corresponds to a point in the dataset.
        // If the point is new, interpolate the values from the zone it
        // comes from.
        //
        for (int i=0; i<origNPoints; i++)
        {
            outpd->CopyData(inpd, i, outIndex++);
        }

        int newPtStartingIndex = outIndex;
        for (int i=0; i<newNPoints; i++)
        {
            ReconstructedCoord &coord = coordsList[i];
            vtkCell *cell = mesh->GetCell(coord.origzone);
            outpd->InterpolatePoint(inpd, outIndex++, cell->GetPointIds(),
                                    coord.weight);
        }

        //
        // The original node numbers should *not* be interpolated!
        // Throw a -1 in there for new nodes as a flag.
        //
        vtkDataArray *origNodes = outpd->GetArray("avtOriginalNodeNumbers");
        if (origNodes)
        {
            int  multiplier = origNodes->GetNumberOfComponents();
            int  offset     = (multiplier == 2) ? 1 : 0;
            int *origNodeArray = (int*)origNodes->GetVoidPointer(0);
            for (int i=0; i<newNPoints; i++)
            {
                origNodeArray[(newPtStartingIndex+i)*multiplier + offset] = -1;
            }
        }
    }
    //
    // Copy over all the cell-centered data.  The logic gets awfully confusing
    // when using the VTK convenience methods *and* we have mixed variables,
    // so just do as normal and we will copy over the mixed values later.
    //
    vtkCellData *outcd = rv->GetCellData();
    vtkCellData *incd  = ds->GetCellData();
    if (incd->GetNumberOfArrays() > 0)
    {
        outcd->CopyAllocate(incd, ncells);
        for (int i=0; i<ncells; i++)
        {
            int c = cellList[i];
            int origzone = zonesList[c].origzone;
            outcd->CopyData(incd, origzone, i);
        }
    }

    //
    // Now go and write over the mixed part of the mixed variables.  The non-
    // mixed part was already copied over in the last operation.
    //
    for (size_t i=0; i<mixvars.size(); i++)
    {
        avtMixedVariable *mv = mixvars[i];
        if (mv == NULL)
        {
            continue;
        }
        vtkDataArray *arr = outcd->GetArray(mv->GetVarname().c_str());
        if (arr == NULL)
        {
            debug1 << "INTERNAL ERROR IN MIR.  Asked to reconstruct a variable"
                   << " with mixed elements,\nbut could not find the original "
                   << "variable array." << endl;
            debug1 << "The mixed variable is " << mv->GetVarname().c_str() << endl;
            debug1 << "Variables in the VTK dataset are: ";
            for (int j = 0 ; j < outcd->GetNumberOfArrays() ; j++)
            {
                debug1 << outcd->GetArray(j)->GetName() << ", ";
            }
            debug1 << endl;
            continue;
        }
        if (arr->GetNumberOfComponents() != 1)
        {
            debug1 << "Can not operate on mixed vars that aren't scalars."
                   << endl;
            continue;
        }
        const float *buffer = mv->GetBuffer();
        float *outBuff = (float *) arr->GetVoidPointer(0);
        debug4 << "Overwriting mixed values for " << arr->GetName() << endl;
        int nvals = 0;
        for (int j=0; j<ncells; j++)
        {
            int mix_index = zonesList[cellList[j]].mix_index;
            if (mix_index >= 0)
            {
                outBuff[j] = buffer[mix_index];
                nvals++;
            }
        }
        debug4 << "Overwrote " << nvals << " values (by tet, not necessarily "
               << "by original zone)" << endl;
    }

    if (doMats)
    {
        //
        // Add an array that contains the material for each zone (which is now
        // clean after reconstruction).
        //
        vtkIntArray *outmat = vtkIntArray::New();
        outmat->SetName("avtSubsets");
        outmat->SetNumberOfTuples(ncells);
        int *buff = outmat->GetPointer(0);
        for (int i=0; i<ncells; i++)
        {
            int matno = zonesList[cellList[i]].mat;
            buff[i] = matno < 0 ? nOrigMaterials : mapUsedMatToMat[matno];
        }
        rv->GetCellData()->AddArray(outmat);
        outmat->Delete();
    }

    delete [] matFlag;
    delete [] cellList;

    visitTimer->StopTimer(timerHandle, "MIR: Getting clean dataset");
    visitTimer->DumpTimings();

    return rv;

}

// ****************************************************************************
//  Method:  DiscreteMIR::ReconstructCleanMesh
//
//  Purpose:
//    Main loop for interface reconstruction for any clean mesh.
//
//  Arguments:
//    mesh       the mesh
//    mat        the material
//    conn       the connectivity
//
//  Programmer:  John C. Anderson
//  Creation:    October 19, 2008
//
//  Note:  Copied from ZooMIR/TetMIR implementation by Jeremey Meredith.
//
// ****************************************************************************
bool
DiscreteMIR::ReconstructCleanMesh(vtkDataSet *mesh, avtMaterial *mat)
{
    int timerHandle = visitTimer->StartTimer();

    // no need to pack, so fake that part
    nMaterials = mat->GetNMaterials();
    mapMatToUsedMat.resize(mat->GetNMaterials(), -1);
    mapUsedMatToMat.resize(mat->GetNMaterials(), -1);
    for (int m=0; m<mat->GetNMaterials(); m++)
    {
        mapMatToUsedMat[m] = m;
        mapUsedMatToMat[m] = m;
    }

    // Can we skip the reconstruction entirely?
    if (mat->GetMixlen() <= 0 ||
        mat->GetMixMat() == NULL)
    {
        int ncells = mesh->GetNumberOfCells();
        const int *matlist = mat->GetMatlist();
        singleMat = matlist[0];
        for (int c=1; c<ncells; c++)
        {
            if (matlist[c] != singleMat)
            {
                singleMat = -1;
                break;
            }
        }
        if (singleMat >= 0)
        {
            visitTimer->StopTimer(timerHandle,
                               "MIR: Reconstructing clean mesh: one mat only");
            visitTimer->DumpTimings();
            return true;
        }
    }

    // Set the connectivity
    MIRConnectivity conn;
    conn.SetUpConnectivity(mesh);

    // extract coords
    SetUpCoords();

    // extract cells
    int        nCells  = conn.ncells;
    const int *matlist = mat->GetMatlist();
    int *conn_ptr = conn.connectivity;
    zonesList.resize(nCells);
    for (int c=0; c<nCells; c++)
    {
        int        nIds = *conn_ptr;
        const int *ids  = conn_ptr+1;

        ReconstructedZone &zone = zonesList[c];
        zone.origzone   = c;
        zone.mat        = matlist[c];
        zone.celltype   = conn.celltype[c];
        zone.nnodes     = nIds;
        zone.startindex = indexList.size();
        zone.mix_index  = -1;

        for (int n=0; n<nIds; n++)
            indexList.push_back(ids[n]);
        conn_ptr += nIds+1;
    }

    visitTimer->StopTimer(timerHandle, "MIR: Reconstructing clean mesh");
    visitTimer->DumpTimings();

    return true;
}



// ****************************************************************************
//  Function: SetUpCoords
//
//  Purpose:
//      Sets up the coordinates array.  Avoid using any VTK calls in its inner
//      loop.
//
//  Programmer: Hank Childs
//  Creation:   October 5, 2002
//
//  Modifications:
//
//  John C. Anderson, Sat Nov  1 17:17:41 2008
//  Save off the x, y, z spacings for use elsewhere.
//
// ****************************************************************************
void
DiscreteMIR::SetUpCoords()
{
    int timerHandle1 = visitTimer->StartTimer();

    int nPoints = mesh->GetNumberOfPoints();
    int i, j, k;

    origNPoints = nPoints;
    origXCoords.resize(nPoints);
    origYCoords.resize(nPoints);
    origZCoords.resize(nPoints);

    int dstype = mesh->GetDataObjectType();
    if (dstype == VTK_RECTILINEAR_GRID)
    {
        vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *) mesh;
        vtkDataArray *xc = rgrid->GetXCoordinates();
        int nx = xc->GetNumberOfTuples();
        xspacing = new float[nx];
        for (i = 0 ; i < nx ; i++)
        {
            xspacing[i] = xc->GetTuple1(i);
        }
        vtkDataArray *yc = rgrid->GetYCoordinates();
        int ny = yc->GetNumberOfTuples();
        yspacing = new float[ny];
        for (i = 0 ; i < ny ; i++)
        {
            yspacing[i] = yc->GetTuple1(i);
        }
        vtkDataArray *zc = rgrid->GetZCoordinates();
        int nz = zc->GetNumberOfTuples();
        zspacing = new float[nz];
        for (i = 0 ; i < nz ; i++)
        {
            zspacing[i] = zc->GetTuple1(i);
        }

        int pt = 0;
        for (k = 0 ; k < nz ; k++)
        {
            for (j = 0 ; j < ny ; j++)
            {
                for (i = 0 ; i < nx ; i++)
                {
                    origXCoords[pt] = xspacing[i];
                    origYCoords[pt] = yspacing[j];
                    origZCoords[pt] = zspacing[k];
                    pt++;
                }
            }
        }
    }
    else
    {
        vtkPointSet *ps = (vtkPointSet *) mesh;
        float *ptr = (float *) ps->GetPoints()->GetVoidPointer(0);
        for (int n=0; n<nPoints; n++)
        {
            origXCoords[n] = *ptr++;
            origYCoords[n] = *ptr++;
            origZCoords[n] = *ptr++;
        }
    }

    visitTimer->StopTimer(timerHandle1, "MIR: Copying coordinate list");
    visitTimer->DumpTimings();
}

// ***************************************************************************
//
//  Modifications:
//
//    Hank Childs, Fri Nov 27 07:08:22 CET 2009
//    Fix compilation error of some linux boxes.
//
// ***************************************************************************

unsigned char DiscreteMIR::get(size_t i, size_t j, size_t k) const
{
    if(!inside(i, j, k))
        return 255;

    Cell cell(i / DX, j / DY, k / DZ);

    unsigned char *labels = NULL;

    labels = m_labels[id(cell)];
    if(labels != NULL)
    {
        i %= DX;
        j %= DY;
        k %= DZ;

        return labels[DX * DY * k + DX * j + i];
    }
    else
    {
        return 0;
    }
}

void DiscreteMIR::optimize()
{
    // Temporary labels.
    unsigned char l, l2;
    unsigned char n, n2;

    // Labels.
    unsigned char *labels;

    // Cell.
    Cell cell;

    // Two sites.
    int r, r2, i, j, k, i2, j2, k2;

    int iterationK = 0;
    time_t starttime = time(NULL);
    while(time(NULL) - starttime < options.annealingTime)
    {
        for(int iteration = 0; iteration < 1000; ++iteration)
        {
            cell = m_mixedCells[RANDOM * m_mixedCells.size()];
            labels = m_labels[id(cell)];

            // Pick two sites within a cell with different labels.
            int incell = 10;
            do
            {
                r  = DX * DY * DZ * RANDOM;
                r2 = DX * DY * DZ * RANDOM;

                l  = labels[r];
                l2 = labels[r2];
            }
            while(l == l2 && incell--);

            if(l == l2)
                continue;

            // Compute the actual site locations.
            int bi = DX * cell.m_i;
            int bj = DY * cell.m_j;
            int bk = DZ * cell.m_k;
            i = bi + (r % DX);
            j = bj + ((r / DX) % DY);
            k = bk + (r / (DX * DY));
            i2 = bi + (r2 % DX);
            j2 = bj + ((r2 / DX) % DY);
            k2 = bk + (r2 / (DX * DY));

            // Compute the energy of:
            //   - current configuration, and
            //   - switched configuration.
            double e0 = 0.0, e1 = 0.0;
            float *weights = m_neighborhood;
            for(int z = -NZ; z <= NZ; ++z)
                for(int y = -NY; y <= NY; ++y)
                    for(int x = -NX; x <= NX; ++x)
                    {
                        float weight = *weights++;
                        if(weight == 0)
                            continue;

                        n  = get( i + x,  j + y,  k + z);
                        n2 = get(i2 + x, j2 + y, k2 + z);

                        if(n != 255)
                        {
                            e0 +=  l == n ? 0 : weight;
                            e1 += l2 == n ? 0 : weight;
                        }

                        if(n2 != 255)
                        {
                            e0 += l2 == n2 ? 0 : weight;
                            e1 +=  l == n2 ? 0 : weight;
                        }
                    }

            // Perhaps change the label.
            if(e1 < e0)
                std::swap(labels[r], labels[r2]);
            else
            {
                if(m_temperature > 0.0)
                {
                    if(RANDOM < exp(-(e1 - e0) / m_temperature))
                        std::swap(labels[r], labels[r2]);
                }
                else
                {
#if 1
                    if(e1 == e0 && RANDOM < 0.5)
                        std::swap(labels[r], labels[r2]);
#endif
                }
            }
        }
    }
}

bool DiscreteMIR::isValid(const Cell &cell) const
{
    if(0 <= cell.m_i && cell.m_i < dimensions[0] &&
       0 <= cell.m_j && cell.m_j < dimensions[1] &&
       0 <= cell.m_k && cell.m_k < dimensions[1])
        return true;
    else
        return false;
}

bool DiscreteMIR::isValid(const Node &node) const
{
    if(0 <= node.m_i && node.m_i <= dimensions[0] &&
       0 <= node.m_j && node.m_j <= dimensions[1] &&
       0 <= node.m_k && node.m_k <= dimensions[2])
        return true;
    else
        return false;
}
