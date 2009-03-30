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
#include <avtSimV2Writer.h>

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkFieldData.h>
#include <vtkFloatArray.h>
#include <vtkIdTypeArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

#include <avtDataAttributes.h>
#include <avtDatabaseMetaData.h>

#include <DebugStream.h>

// ****************************************************************************
// Method: avtSimV2Writer::avtSimV2Writer
//
// Purpose: 
//   Constructor
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 2 17:35:39 PST 2006
//
// Modifications:
//   
// ****************************************************************************

avtSimV2Writer::avtSimV2Writer() : avtDatabaseWriter(), objectName(), varList()
{
    metadata = 0;
}

// ****************************************************************************
// Method: avtSimV2Writer::~avtSimV2Writer
//
// Purpose: 
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 2 17:35:56 PST 2006
//
// Modifications:
//   
// ****************************************************************************

avtSimV2Writer::~avtSimV2Writer()
{
}

// ****************************************************************************
// Method: avtSimV2Writer::OpenFile
//
// Purpose: 
//   This method is called when we want to begin data export.
//
// Arguments:
//   objName : Typically the filename but this class uses it for the name of
//             the mesh that is exported to the application.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 2 17:36:09 PST 2006
//
// Modifications:
//    Jeremy Meredith, Tue Mar 27 15:10:21 EDT 2007
//    Added nblocks to this functin and save it so we don't have to 
//    trust the meta data.
//   
// ****************************************************************************

void
avtSimV2Writer::OpenFile(const std::string &objName, int nb)
{
    objectName = objName;
    numblocks = nb;

    // Remove leading "./" and "/". We'll allow all other slashes because
    // it will allow us to produce directory structures.
    if(objectName.substr(0, 2) == std::string("./"))
        objectName = objectName.substr(2, objectName.size()-2);
    else if(objectName.substr(0, 1) == std::string("/"))
        objectName = objectName.substr(1, objectName.size()-1);
    if(objectName.size() == 0) 
        objectName = "mesh";

    debug1 << "avtSimV2Writer::OpenFile(\"" << objName.c_str() << "\")\n";
    visit_invoke_WriteBegin(objName.c_str());
}

// ****************************************************************************
// Method: avtSimV2Writer::WriteHeaders
//
// Purpose: 
//   This method is called when the caller wants to write out a table of 
//   contents file. This class uses the method to make a list of the variables
//   that will be exported to the application.
//
// Arguments:
//   md : The metadata for the file.
//   scalars : The list of scalars to export.
//   vectors : The list of vectors to export.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 2 17:37:39 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtSimV2Writer::WriteHeaders(const avtDatabaseMetaData *md,
    std::vector<std::string> &scalars,
    std::vector<std::string> &vectors,
    std::vector<std::string> &)
{
    debug1 << "avtSimV2Writer::WriteHeaders(...)\n";
    varList = scalars;
    for(int i = 0; i < vectors.size(); ++i)
        varList.push_back(vectors[i]);
    metadata = md;
}

// ****************************************************************************
// Method: avtSimV2Writer::WriteChunk
//
// Purpose: 
//   This method is called to export one VTK dataset to the application.
//
// Arguments:
//   ds    : The dataset to export.
//   chunk : The chunk number.
//
// Note:       We create mesh metadata to pass the application here and then
//             call specialized methods to convert the VTK object into the
//             different types of SimV1 meshes.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 2 17:39:35 PST 2006
//
// Modifications:
//    Jeremy Meredith, Tue Mar 27 11:36:57 EDT 2007
//    Use the saved number of blocks instead of assuming the mmd was correct.
//   
// ****************************************************************************

void
avtSimV2Writer::WriteChunk(vtkDataSet *ds, int chunk)
{
    //
    // Assemble mesh metadata using the data attributes and the metadata.
    //
    const avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    const avtMeshMetaData *mmd = metadata->GetMesh(atts.GetMeshname());
    VisIt_MeshMetaData *vmmd = (VisIt_MeshMetaData*)malloc(sizeof(VisIt_MeshMetaData));
    memset(vmmd, 0, sizeof(VisIt_MeshMetaData));
    vmmd->name = strdup(objectName.c_str());
    vmmd->topologicalDimension = atts.GetTopologicalDimension();
    vmmd->spatialDimension = atts.GetSpatialDimension();

    if(mmd != 0)
    {
        vmmd->numBlocks = numblocks;
        vmmd->blockTitle = strdup(mmd->blockTitle.c_str());
        vmmd->blockPieceName = strdup(mmd->blockPieceName.c_str());

        vmmd->numGroups = 0;
        vmmd->groupTitle = strdup(mmd->groupTitle.c_str());
        vmmd->groupIds = 0;
    }
    else
    {
        vmmd->numBlocks = 1;
        vmmd->blockTitle = strdup("domains");
        vmmd->blockPieceName = strdup("domain");

        vmmd->numGroups = 0;
        vmmd->groupTitle = strdup("groups");
        vmmd->groupIds = 0;
    }
    vmmd->units = strdup(atts.GetXUnits().c_str());
    vmmd->xLabel = strdup(atts.GetXLabel().c_str());
    vmmd->yLabel = strdup(atts.GetYLabel().c_str());
    vmmd->zLabel = strdup(atts.GetZLabel().c_str());

    //
    // Translate the VTK input into SimV1 data structures and call back
    // into the simulation.
    //
    switch (ds->GetDataObjectType())
    {
    case VTK_UNSTRUCTURED_GRID:
        WriteUnstructuredMesh((vtkUnstructuredGrid *) ds, chunk, vmmd);
        break;
    case VTK_STRUCTURED_GRID:
        WriteCurvilinearMesh((vtkStructuredGrid *) ds, chunk, vmmd);
        break;
    case VTK_RECTILINEAR_GRID:
        WriteRectilinearMesh((vtkRectilinearGrid *) ds, chunk, vmmd);
        break;
    case VTK_POLY_DATA:
        WritePolyDataMesh((vtkPolyData *) ds, chunk, vmmd);
        break;
    default:
         EXCEPTION1(ImproperUseException, "Unsupported mesh type");
    }

    // Free the mesh metadata.
    VisIt_MeshMetaData_free(vmmd);
    free(vmmd);
}

// ****************************************************************************
// Method: avtSimV2Writer::CloseFile
//
// Purpose: 
//   This method is called when the export is completed.
//
// Note:       We call the application to let it know that exporting is done.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 2 17:41:02 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtSimV2Writer::CloseFile(void)
{
    debug1 << "avtSimV2Writer::CloseFile()\n";
    visit_invoke_WriteEnd(objectName.c_str());
}

// ****************************************************************************
// Method: avtSimV2Writer::WriteCurvilinearMesh
//
// Purpose: 
//   Converts a VTK dataset to a SimV1 curvilinear mesh and passes it to the
//   application.
//
// Arguments:
//   ds    : The VTK dataset to be transferred.
//   chunk : The chunk id of the dataset.
//   vmmd  : The mesh metadata that has been created for this dataset.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 2 17:41:33 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtSimV2Writer::WriteCurvilinearMesh(vtkStructuredGrid *ds, int chunk,
    VisIt_MeshMetaData *vmmd)
{
    debug1 << "avtSimV2Writer::WriteCurvilinearMesh(chunk=" << chunk << ")\n";

    // Set the mesh type in the sim1 metadata.
    vmmd->meshType = VISIT_MESHTYPE_CURVILINEAR;

    // Translate the VTK structure back into VisIt_CurvilinearMesh.
    VisIt_CurvilinearMesh *cmesh = (VisIt_CurvilinearMesh *)malloc(sizeof(VisIt_CurvilinearMesh));
    memset((void*)cmesh, 0, sizeof(VisIt_CurvilinearMesh));
    cmesh->ndims = ds->GetDataDimension();
    ds->GetDimensions(cmesh->dims);

    cmesh->baseIndex[0] = 0;
    cmesh->baseIndex[1] = 0;
    cmesh->baseIndex[2] = 0;

    cmesh->minRealIndex[0] = 0;
    cmesh->minRealIndex[1] = 0;
    cmesh->minRealIndex[2] = 0;
    
    cmesh->maxRealIndex[0] = cmesh->dims[0]-1;
    cmesh->maxRealIndex[1] = cmesh->dims[1]-1;
    cmesh->maxRealIndex[2] = cmesh->dims[2]-1;

    if(cmesh->ndims == 1)
    {
        double *x = (double*)malloc(ds->GetNumberOfPoints() * sizeof(double));
        for(vtkIdType i = 0; i < ds->GetNumberOfPoints(); ++i)
        {
            double *pt = ds->GetPoint(i);
            x[i] = pt[0];
        }
        cmesh->xcoords = VisIt_CreateDataArrayFromDouble(VISIT_OWNER_VISIT, x);
    }
    else if(cmesh->ndims == 2)
    {
        double *x = (double*)malloc(ds->GetNumberOfPoints() * sizeof(double));
        double *y = (double*)malloc(ds->GetNumberOfPoints() * sizeof(double));
        for(vtkIdType i = 0; i < ds->GetNumberOfPoints(); ++i)
        {
            double *pt = ds->GetPoint(i);
            x[i] = pt[0];
            y[i] = pt[1];
        }
        cmesh->xcoords = VisIt_CreateDataArrayFromDouble(VISIT_OWNER_VISIT, x);
        cmesh->ycoords = VisIt_CreateDataArrayFromDouble(VISIT_OWNER_VISIT, y);
    }
    else if(cmesh->ndims == 3)
    {
        double *x = (double*)malloc(ds->GetNumberOfPoints() * sizeof(double));
        double *y = (double*)malloc(ds->GetNumberOfPoints() * sizeof(double));
        double *z = (double*)malloc(ds->GetNumberOfPoints() * sizeof(double));
        for(vtkIdType i = 0; i < ds->GetNumberOfPoints(); ++i)
        {
            double *pt = ds->GetPoint(i);
            x[i] = pt[0];
            y[i] = pt[1];
            z[i] = pt[2];
        }
        cmesh->xcoords = VisIt_CreateDataArrayFromDouble(VISIT_OWNER_VISIT, x);
        cmesh->ycoords = VisIt_CreateDataArrayFromDouble(VISIT_OWNER_VISIT, y);
        cmesh->zcoords = VisIt_CreateDataArrayFromDouble(VISIT_OWNER_VISIT, z);
    }

    VisIt_MeshData *mesh = new VisIt_MeshData;
    memset((void *)mesh, 0, sizeof(VisIt_MeshData));
    mesh->meshType = VISIT_MESHTYPE_CURVILINEAR;
    mesh->cmesh = cmesh;

    // Call into the simulation to write the data back.
    int ret = visit_invoke_WriteMesh(objectName.c_str(), chunk, mesh, vmmd);
    if(ret != VISIT_OKAY)
    {
        debug1 << "WriteMesh callback returned " << ret
               << " instead of VISIT_OKAY." << endl;
    }

    VisIt_MeshData_free(mesh);
 
    // Write the data arrays into the simulation.
    WriteDataArrays(ds, chunk);   
}

// ****************************************************************************
// Method: avtSimV2Writer::WriteRectilinearMesh
//
// Purpose: 
//   Converts a VTK dataset to a SimV1 rectilinear mesh and passes it to the
//   application.
//
// Arguments:
//   ds    : The VTK dataset to be transferred.
//   chunk : The chunk id of the dataset.
//   vmmd  : The mesh metadata that has been created for this dataset.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 2 17:41:33 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtSimV2Writer::WriteRectilinearMesh(vtkRectilinearGrid *ds, int chunk, 
    VisIt_MeshMetaData *vmmd)
{
    debug1 << "avtSimV2Writer::WriteRectilinearMesh(chunk=" << chunk << ")\n";

    // Set the mesh type in the sim1 metadata.
    vmmd->meshType = VISIT_MESHTYPE_RECTILINEAR;

    // Translate the VTK structure back into VisIt_RectilinearMesh.
    VisIt_RectilinearMesh *rmesh = (VisIt_RectilinearMesh *)malloc(sizeof(VisIt_RectilinearMesh));
    memset((void*)rmesh, 0, sizeof(VisIt_RectilinearMesh));
    rmesh->ndims = ds->GetDataDimension();
    ds->GetDimensions(rmesh->dims);

    rmesh->baseIndex[0] = 0;
    rmesh->baseIndex[1] = 0;
    rmesh->baseIndex[2] = 0;

    rmesh->minRealIndex[0] = 0;
    rmesh->minRealIndex[1] = 0;
    rmesh->minRealIndex[2] = 0;
    
    rmesh->maxRealIndex[0] = rmesh->dims[0]-1;
    rmesh->maxRealIndex[1] = rmesh->dims[1]-1;
    rmesh->maxRealIndex[2] = rmesh->dims[2]-1;

    if(rmesh->ndims >= 1)
    {
        vtkDataArray *xc = ds->GetXCoordinates();
        double *x = (double*)malloc(xc->GetNumberOfTuples() * sizeof(double));
        for(vtkIdType i = 0; i < xc->GetNumberOfTuples(); ++i)
            x[i] = xc->GetTuple1(i);
        rmesh->xcoords = VisIt_CreateDataArrayFromDouble(VISIT_OWNER_VISIT, x);
    }

    if(rmesh->ndims >= 2)
    {
        vtkDataArray *yc = ds->GetYCoordinates();
        double *y = (double*)malloc(yc->GetNumberOfTuples() * sizeof(double));
        for(vtkIdType i = 0; i < yc->GetNumberOfTuples(); ++i)
            y[i] = yc->GetTuple1(i);
        rmesh->ycoords = VisIt_CreateDataArrayFromDouble(VISIT_OWNER_VISIT, y);
    }

    if(rmesh->ndims >= 3)
    {
        vtkDataArray *zc = ds->GetZCoordinates();
        double *z = (double*)malloc(zc->GetNumberOfTuples() * sizeof(double));
        for(vtkIdType i = 0; i < zc->GetNumberOfTuples(); ++i)
            z[i] = zc->GetTuple1(i);
        rmesh->zcoords = VisIt_CreateDataArrayFromDouble(VISIT_OWNER_VISIT, z);
    }

    VisIt_MeshData *mesh = (VisIt_MeshData *)malloc(sizeof(VisIt_MeshData));
    memset((void *)mesh, 0, sizeof(VisIt_MeshData));
    mesh->meshType = VISIT_MESHTYPE_RECTILINEAR;
    mesh->rmesh = rmesh;

    // Call into the simulation to write the data back.
    int ret = visit_invoke_WriteMesh(objectName.c_str(), chunk, mesh, vmmd);
    if(ret != VISIT_OKAY)
    {
        debug1 << "WriteRectilinearMesh callback returned " << ret
               << " instead of VISIT_OKAY." << endl;
    }

    VisIt_MeshData_free(mesh);

    // Write the data arrays into the simulation.
    WriteDataArrays(ds, chunk);
}

// ****************************************************************************
// Method: avtSimV2Writer::WriteUnstructuredMesh
//
// Purpose: 
//   Converts a VTK dataset to a SimV1 unstructured mesh of a point mesh and 
//   passes it to the application.
//
// Arguments:
//   ds    : The VTK dataset to be transferred.
//   chunk : The chunk id of the dataset.
//   vmmd  : The mesh metadata that has been created for this dataset.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 2 17:41:33 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtSimV2Writer::WriteUnstructuredMesh(vtkUnstructuredGrid *ds, int chunk,
    VisIt_MeshMetaData *vmmd)
{
    const char *mName = "avtSimV2Writer::WriteUnstructuredMesh: ";
    debug1 << mName << "chunk=" << chunk << ")\n";

    // The largest VTK cell type number is currently 67.
    int celltype_npts[100];
    memset(celltype_npts, 0, sizeof(int) * 100);
    celltype_npts[VTK_LINE]       = 2;
    celltype_npts[VTK_TRIANGLE]   = 3;
    celltype_npts[VTK_QUAD]       = 4;
    celltype_npts[VTK_TETRA]      = 4;
    celltype_npts[VTK_PYRAMID]    = 5;
    celltype_npts[VTK_WEDGE]      = 6;
    celltype_npts[VTK_HEXAHEDRON] = 8;
    celltype_npts[VTK_VERTEX]     = 1;

    int celltype_idtype[100];
    memset(celltype_idtype, 0, sizeof(int) * 100);
    celltype_idtype[VTK_LINE]       = VISIT_CELL_BEAM;
    celltype_idtype[VTK_TRIANGLE]   = VISIT_CELL_TRI;
    celltype_idtype[VTK_QUAD]       = VISIT_CELL_QUAD;
    celltype_idtype[VTK_TETRA]      = VISIT_CELL_TET;
    celltype_idtype[VTK_PYRAMID]    = VISIT_CELL_PYR;
    celltype_idtype[VTK_WEDGE]      = VISIT_CELL_WEDGE;
    celltype_idtype[VTK_HEXAHEDRON] = VISIT_CELL_HEX;
    celltype_idtype[VTK_VERTEX]     = VISIT_CELL_POINT;

    // Make a pass through to see if we have a point mesh.
    vtkUnsignedCharArray *ct = ds->GetCellTypesArray();
    bool onlyPoints = true;
    unsigned char *cptr = (unsigned char *)ct->GetVoidPointer(0);
    vtkIdType connLen = 0;
    vtkIdType *pts = 0;
    for(vtkIdType i = 0; i < ct->GetNumberOfTuples(); ++i)
    {
        int t = *cptr++;
        if(t != VTK_VERTEX)
            onlyPoints = false;
 
        debug5 << "cell[" << i << "] has " << celltype_npts[t] << " vertices." << endl;

        connLen += celltype_npts[t] + 1;
    }
    debug1 << mName << "Number of cells=" << ct->GetNumberOfTuples()
           << ", connLen=" << connLen << endl;

    if(onlyPoints)
    {
        debug1 << mName << "Write the mesh as a point mesh" << endl;

        // Do a point mesh callback.
        VisIt_PointMesh *pmesh = new VisIt_PointMesh;
        memset((void*)pmesh, 0, sizeof(VisIt_PointMesh));
        pmesh->ndims = 3;
        pmesh->nnodes = ds->GetNumberOfCells();
        double *x = (double*)malloc(pmesh->nnodes * sizeof(double));
        double *y = (double*)malloc(pmesh->nnodes * sizeof(double));
        double *z = (double*)malloc(pmesh->nnodes * sizeof(double));
        for(vtkIdType i = 0; i < ds->GetNumberOfCells(); ++i)
        {
            vtkIdType npts;
            double pt[3];

            ds->GetCellPoints(i, npts, pts);
            ds->GetPoint(pts[0], pt);

            x[i] = pt[0];
            y[i] = pt[1];
            z[i] = pt[2];
        }
        pmesh->xcoords = VisIt_CreateDataArrayFromDouble(VISIT_OWNER_VISIT, x);
        pmesh->ycoords = VisIt_CreateDataArrayFromDouble(VISIT_OWNER_VISIT, y);
        pmesh->zcoords = VisIt_CreateDataArrayFromDouble(VISIT_OWNER_VISIT, z);

        // Set the mesh type in the sim1 metadata.
        vmmd->meshType = VISIT_MESHTYPE_POINT;

        VisIt_MeshData *mesh = (VisIt_MeshData *)malloc(sizeof(VisIt_MeshData));
        memset((void *)mesh, 0, sizeof(VisIt_MeshData));
        mesh->meshType = VISIT_MESHTYPE_POINT;
        mesh->pmesh = pmesh;

        // Call into the simulation to write the data back.
        int ret = visit_invoke_WriteMesh(objectName.c_str(), chunk, mesh, vmmd);
        if(ret != VISIT_OKAY)
        {
            debug1 << "WritePointMesh callback returned " << ret
                   << " instead of VISIT_OKAY." << endl;
        }

        VisIt_MeshData_free(mesh);

        // Write the data arrays into the simulation.
        WriteDataArrays(ds, chunk);
    }
    else
    {
        debug1 << mName << "Write the mesh as an unstructured mesh" << endl;

        // Do a more general unstructured mesh callback.
        // Create an array of cell behaviors. 0=remove, 1=repeat once, 
        // 2=repeat 2x, ...
        unsigned char *cellCopy = new unsigned char[ct->GetNumberOfTuples()];
        memset(cellCopy, 0, ct->GetNumberOfTuples());

        // Now, assemble the connectivity.
        int *conn = (int*)malloc(connLen * sizeof(int));
        cptr = (unsigned char *)ct->GetVoidPointer(0);
        int *connPtr = conn;
        connLen = 0;
        int cellCount = 0;
        for(vtkIdType i = 0; i < ct->GetNumberOfTuples(); ++i)
        {
            vtkIdType npts, *pts = 0;
            ds->GetCellPoints(i, npts, pts);

            // Store the cell type in terms of the VISIT simulation types.
            int vtktype = *cptr++;
            switch(vtktype)
            {
            case VTK_LINE:
            case VTK_TRIANGLE:
            case VTK_QUAD:
            case VTK_TETRA:
            case VTK_PYRAMID:
            case VTK_WEDGE:
            case VTK_HEXAHEDRON:
            case VTK_VERTEX:
                {
                *connPtr++ = celltype_idtype[vtktype];

                // Store the connectivity.
                for(vtkIdType j = 0; j < npts; ++j)
                    *connPtr++ = pts[j];

                // Let's recalculate the connetivity size in case there
                // were cell types that we had to skip.
                connLen += (npts + 1);

                // Indicate that the cell should be copied.
                cellCopy[i] = 1;
                ++cellCount;
                }
                break;
            default:
                debug5 << mName << "Unsupported VTK cell type " << vtktype
                       << ". Skipping cell." << endl;
            }
        }

        if(cellCount > 0)
        {
            VisIt_UnstructuredMesh *umesh = (VisIt_UnstructuredMesh *)malloc(sizeof(VisIt_UnstructuredMesh));
            memset((void*)umesh, 0, sizeof(VisIt_UnstructuredMesh));
            umesh->ndims = 3;
            umesh->nnodes = ds->GetNumberOfPoints();
            umesh->nzones = cellCount;
            double *x = (double*)malloc(umesh->nnodes * sizeof(double));
            double *y = (double*)malloc(umesh->nnodes * sizeof(double));
            double *z = (double*)malloc(umesh->nnodes * sizeof(double));
            for(vtkIdType i = 0; i < ds->GetNumberOfPoints(); ++i)
            {
                double pt[3];
                ds->GetPoint(i, pt);
                x[i] = pt[0];
                y[i] = pt[1];
                z[i] = pt[2];
            }
            umesh->xcoords = VisIt_CreateDataArrayFromDouble(VISIT_OWNER_VISIT, x);
            umesh->ycoords = VisIt_CreateDataArrayFromDouble(VISIT_OWNER_VISIT, y);
            umesh->zcoords = VisIt_CreateDataArrayFromDouble(VISIT_OWNER_VISIT, z);
            umesh->connectivity = VisIt_CreateDataArrayFromInt(VISIT_OWNER_VISIT, conn);
            umesh->connectivityLen = connLen;

            // Set the mesh type in the sim1 metadata.
            vmmd->meshType = VISIT_MESHTYPE_UNSTRUCTURED;

            VisIt_MeshData *mesh = (VisIt_MeshData *)malloc(sizeof(VisIt_MeshData));
            memset((void *)mesh, 0, sizeof(VisIt_MeshData));
            mesh->meshType = VISIT_MESHTYPE_UNSTRUCTURED;
            mesh->umesh = umesh;

            // Call into the simulation to write the data back.
            int ret = visit_invoke_WriteMesh(objectName.c_str(), chunk, mesh, vmmd);
            if(ret != VISIT_OKAY)
            {
                debug1 << "WriteUnstructuredMesh callback returned " << ret
                       << " instead of VISIT_OKAY." << endl;
            }

            VisIt_MeshData_free(mesh);

            // Write the data arrays into the simulation.
            if(cellCount == ds->GetNumberOfCells())
                WriteDataArrays(ds, chunk);
            else
                WriteDataArraysConditionally(ds, chunk, cellCopy);
        }
        else
        {
            debug1 << mName << "WriteUnstructuredMesh function not called "
                   << "because the mesh had no cells that SimV1 could "
                   << "represent." << endl;
            free(conn);
        }

        delete [] cellCopy;
    }
}

// ****************************************************************************
// Method: avtSimV2Writer::WritePolyDataMesh
//
// Purpose: 
//   Converts a VTK dataset to a SimV1 point mesh or an unstructured mesh
//   and passes it to the application.
//
// Arguments:
//   ds    : The VTK dataset to be transferred.
//   chunk : The chunk id of the dataset.
//   vmmd  : The mesh metadata that has been created for this dataset.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 2 17:41:33 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtSimV2Writer::WritePolyDataMesh(vtkPolyData *ds, int chunk, VisIt_MeshMetaData *vmmd)
{
    debug1 << "avtSimV2Writer::WritePolyDataMesh(chunk=" << chunk << ")\n";

    // Build up an unstructured mesh from the types of data in the polydata.
    vtkIdType pts[100];
    double *x = (double*)malloc(ds->GetNumberOfPoints() * sizeof(double));
    double *y = (double*)malloc(ds->GetNumberOfPoints() * sizeof(double));
    double *z = (double*)malloc(ds->GetNumberOfPoints() * sizeof(double));
    for(vtkIdType i = 0; i < ds->GetNumberOfPoints(); ++i)
    {
        double pt[3];
        ds->GetPoint(i, pt);
        x[i] = pt[0];
        y[i] = pt[1];
        z[i] = pt[2];
    }

    debug1 << "polydata npts   = " << ds->GetNumberOfPoints() << endl;
    debug1 << "polydata nverts = " << ds->GetVerts()->GetNumberOfCells() << endl;
    debug1 << "polydata nlines = " << ds->GetLines()->GetNumberOfCells() << endl;
    debug1 << "polydata npolys = " << ds->GetPolys()->GetNumberOfCells() << endl;

    if(ds->GetVerts()->GetNumberOfCells() > 0 && 
       (ds->GetLines()->GetNumberOfCells() == 0 && 
        ds->GetPolys()->GetNumberOfCells() == 0))
    {
        // It's a point mesh.
        debug1 << "Write polydata as point mesh" << endl;
        VisIt_PointMesh *pmesh = (VisIt_PointMesh *)malloc(sizeof(VisIt_PointMesh));
        memset((void*)pmesh, 0, sizeof(VisIt_PointMesh));
        pmesh->ndims = 3;
        pmesh->nnodes = ds->GetNumberOfPoints();
        pmesh->xcoords = VisIt_CreateDataArrayFromDouble(VISIT_OWNER_VISIT, x);
        pmesh->ycoords = VisIt_CreateDataArrayFromDouble(VISIT_OWNER_VISIT, y);
        pmesh->zcoords = VisIt_CreateDataArrayFromDouble(VISIT_OWNER_VISIT, z);

        // Set the mesh type in the sim1 metadata.
        vmmd->meshType = VISIT_MESHTYPE_POINT;

        VisIt_MeshData *mesh = (VisIt_MeshData *)malloc(sizeof(VisIt_MeshData));
        memset((void *)mesh, 0, sizeof(VisIt_MeshData));
        mesh->meshType = VISIT_MESHTYPE_POINT;
        mesh->pmesh = pmesh;

        // Call into the simulation to write the data back.
        int ret = visit_invoke_WriteMesh(objectName.c_str(), chunk, mesh, vmmd);
        if(ret != VISIT_OKAY)
        {
            debug1 << "WritePointMesh callback returned " << ret
                   << " instead of VISIT_OKAY." << endl;
        }

        VisIt_MeshData_free(mesh);

        // Write the data arrays into the simulation.
        WriteDataArrays(ds, chunk);
    }
    else
    {
        debug1 << "Write polydata as unstructured mesh" << endl;
        // There are other primitives besides points, write out as a ucd mesh.
        debug1 << "verts: GetNumberOfConnectivityEntries="
               << ds->GetVerts()->GetNumberOfConnectivityEntries()
               << ", ncells=" << ds->GetVerts()->GetNumberOfCells() << endl;
        debug1 << "lines: GetNumberOfConnectivityEntries="
               << ds->GetLines()->GetNumberOfConnectivityEntries()
               << ", ncells=" << ds->GetLines()->GetNumberOfCells() << endl;
        debug1 << "polys: GetNumberOfConnectivityEntries="
               << ds->GetPolys()->GetNumberOfConnectivityEntries()
               << ", ncells=" << ds->GetPolys()->GetNumberOfCells() << endl;

        // Count the number of points in the line cells because we will have
        // to split them if there are more than 2 points.
        int beamCells = 0;
        for(vtkIdType i = 0; i < ds->GetLines()->GetNumberOfCells(); ++i)
        {
            vtkIdType *pts = 0, npts;
            ds->GetLines()->GetCell(i, npts, pts);
            beamCells += npts-1;
        }

        // Determine the length of the connectivity array that we'll need.
        int connLen = ds->GetVerts()->GetNumberOfConnectivityEntries() +
                      (3 * beamCells) + 
                      ds->GetPolys()->GetNumberOfConnectivityEntries();
        int *conn = (int*)malloc(connLen * sizeof(int));
        int *connPtr = conn;
        int cellCount = 0;

        // Get the points.
        for(vtkIdType i = 0; i < ds->GetVerts()->GetNumberOfCells(); ++i)
        {
            *connPtr++ = VISIT_CELL_POINT;

            vtkIdType *pts = 0, npts;
            ds->GetVerts()->GetCell(i, npts, pts);

            *connPtr++ = pts[0];
            ++cellCount;
        }

        // Get the lines and make beam cells from them.
        for(vtkIdType i = 0; i < ds->GetLines()->GetNumberOfCells(); ++i)
        {
            vtkIdType *pts = 0, npts;
            ds->GetLines()->GetCell(i, npts, pts);

            //
            // Note: This will end up changing the connectivity for line cells that
            //       have more than 2 points. If we run into those then we will 
            //       eventually need to change how the data are written back to 
            //       to reflect the change in connectivity.
            //
            for(vtkIdType j = 0; j < npts-1; ++j)
            {
                *connPtr++ = VISIT_CELL_BEAM;
                *connPtr++ = pts[j];
                *connPtr++ = pts[j+1];
                ++cellCount;
            }
        }
        
        // Get the polys and make triangles or quads from them.
        vtkIdType *pts = 0, npts;
        ds->GetPolys()->InitTraversal();
        while(ds->GetPolys()->GetNextCell(npts, pts) != 0)
        {
            if(npts == 3)
            {
                *connPtr++ = VISIT_CELL_TRI;
                *connPtr++ = pts[0];
                *connPtr++ = pts[1];
                *connPtr++ = pts[2];
                ++cellCount;
            }
            else if(npts == 4)
            {
                *connPtr++ = VISIT_CELL_QUAD;
                *connPtr++ = pts[0];
                *connPtr++ = pts[1];
                *connPtr++ = pts[2];
                *connPtr++ = pts[3];
                ++cellCount;
            }
            else
                debug5 << "Cell has " << npts << " points!" << endl;
        }

        VisIt_UnstructuredMesh *umesh = (VisIt_UnstructuredMesh *)malloc(sizeof(VisIt_UnstructuredMesh));
        memset((void*)umesh, 0, sizeof(VisIt_UnstructuredMesh));
        umesh->ndims = 3;
        umesh->nnodes = ds->GetNumberOfPoints();
        umesh->nzones = cellCount;
        umesh->xcoords = VisIt_CreateDataArrayFromDouble(VISIT_OWNER_VISIT, x);
        umesh->ycoords = VisIt_CreateDataArrayFromDouble(VISIT_OWNER_VISIT, y);
        umesh->zcoords = VisIt_CreateDataArrayFromDouble(VISIT_OWNER_VISIT, z);
        umesh->connectivity = VisIt_CreateDataArrayFromInt(VISIT_OWNER_VISIT, conn);
        umesh->connectivityLen = connPtr - conn;

        debug1 << "nzones = " << umesh->nzones << endl;
        debug1 << "connectivityLen = " << umesh->connectivityLen << endl;

        // Set the mesh type in the sim1 metadata.
        vmmd->meshType = VISIT_MESHTYPE_UNSTRUCTURED;

        VisIt_MeshData *mesh = (VisIt_MeshData *)malloc(sizeof(VisIt_MeshData));
        memset((void *)mesh, 0, sizeof(VisIt_MeshData));
        mesh->meshType = VISIT_MESHTYPE_UNSTRUCTURED;
        mesh->umesh = umesh;

        // Call into the simulation to write the data back.
        int ret = visit_invoke_WriteMesh(objectName.c_str(), chunk, mesh, vmmd);
        if(ret != VISIT_OKAY)
        {
            debug1 << "WriteUnstructuredMesh callback returned " << ret
                   << " instead of VISIT_OKAY." << endl;
        }
        else
        {
            // Write the data arrays into the simulation.
            WriteDataArrays(ds, chunk);
        } 

        VisIt_MeshData_free(mesh);
    }
}

// ****************************************************************************
// Method: avtSimV2Writer::WriteOneDataArray
//
// Purpose: 
//   Transfers a VTK data array to the application.
//
// Arguments:
//   ds    : The VTK dataset to be transferred.
//   chunk : The chunk id of the dataset.
//   vmmd  : The mesh metadata that has been created for this dataset.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 2 17:41:33 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtSimV2Writer::WriteOneDataArray(vtkDataArray *arr, const std::string &objectName,
    int chunk, VisIt_VarCentering cent)
{
    debug1 << "avtSimV2Writer::WriteOneDataArray(chunk=" << chunk 
           << ", name=\"" << arr->GetName() << "\")\n";
    int t = -1;
    if(arr->GetDataType() == VTK_CHAR)
        t = VISIT_DATATYPE_CHAR;
    else if(arr->GetDataType() == VTK_INT)
        t = VISIT_DATATYPE_INT;
    else if(arr->GetDataType() == VTK_FLOAT)
        t = VISIT_DATATYPE_FLOAT;
    else if(arr->GetDataType() == VTK_DOUBLE)
        t = VISIT_DATATYPE_DOUBLE;
    else
    {
        debug1 << "Can't export type " << arr->GetDataType()
               << " to simulation." << endl;
        return;
    }

    // Assemble a scalar metadata object.
    const avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();    
    VisIt_VariableMetaData *smd = (VisIt_VariableMetaData*)malloc(
        sizeof(VisIt_VariableMetaData));
    memset(smd, 0, sizeof(VisIt_VariableMetaData));
    smd->name = strdup(arr->GetName());
    smd->meshName = strdup(objectName.c_str());
    smd->centering = cent;
    smd->treatAsASCII = atts.GetTreatAsASCII(arr->GetName());

    int ret = visit_invoke_WriteVariable(objectName.c_str(), arr->GetName(), chunk, t,
        arr->GetVoidPointer(0), arr->GetNumberOfTuples(),
        arr->GetNumberOfComponents(), smd);
    if(ret != VISIT_OKAY)
    {
        debug1 << "WriteDataArray callback returned " << ret
               << " instead of VISIT_OKAY." << endl;
    }

    // Free the variable metadata.
    VisIt_VariableMetaData_free(smd);
    free(smd);
}

// ****************************************************************************
// Method: ConditionalDataArrayCopy
//
// Purpose: 
//   Template function to create a version of a data array where the cell
//   connectivity has changed.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 2 17:45:02 PST 2006
//
// Modifications:
//   
// ****************************************************************************

template <class T>
inline void *
ConditionalDataArrayCopy(T *in, int in_size, int ncomps, unsigned int out_size, 
    const unsigned char *cellCopy)
{
    // Allocate memory for the new array.
    void *arr = malloc(out_size * ncomps * sizeof(T));
    T *out = (T *)arr;

    for(int i = 0; i < in_size; ++i)
    {
        for(int j = 0; j < int(cellCopy[i]); ++j)
        {
            for(int k = 0; k < ncomps; ++k)
                *out++ = in[i*ncomps + k];
        }
    }

    return arr;
}

// ****************************************************************************
// Method: avtSimV2Writer::WriteCellDataArrayConditionally
//
// Purpose: 
//   Creates a modified copy of the cell data and sends it to the application.
//
// Arguments:
//   arr        : The data array that we're sending.
//   objectName : The name of the mesh on which the data is defined.
//   chunk      : The chunk id of the data.
//   cellCopy   : A nCells sized array that lists the number of times each
//                cell data is to be copied.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 2 17:45:39 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtSimV2Writer::WriteCellDataArrayConditionally(vtkDataArray *arr,
    const std::string &objectName, int chunk, const unsigned char *cellCopy)
{
    debug1 << "avtSimV2Writer::WriteOneCellDataArrayConditionally(chunk=" << chunk 
           << ", name=\"" << arr->GetName() << "\")\n";

    // Sum up the size of the cellCopy array so we can size the array
    // copy that we'll need.
    unsigned int sum = 0;
    for(int i = 0; i < arr->GetNumberOfTuples(); ++i)
        sum += int(cellCopy[i]);

    int t = -1;
    void *S = 0;
    if(arr->GetDataType() == VTK_CHAR)
    {
        t = VISIT_DATATYPE_CHAR;
        S = ConditionalDataArrayCopy((char*)arr->GetVoidPointer(0), 
            arr->GetNumberOfTuples(), arr->GetNumberOfComponents(), sum, cellCopy);
    }
    else if(arr->GetDataType() == VTK_INT)
    {
        t = VISIT_DATATYPE_INT;
        S = ConditionalDataArrayCopy((int*)arr->GetVoidPointer(0), 
            arr->GetNumberOfTuples(), arr->GetNumberOfComponents(), sum, cellCopy);
    }
    else if(arr->GetDataType() == VTK_FLOAT)
    {
        t = VISIT_DATATYPE_FLOAT;
        S = ConditionalDataArrayCopy((float*)arr->GetVoidPointer(0), 
            arr->GetNumberOfTuples(), arr->GetNumberOfComponents(), sum, cellCopy);
    }
    else if(arr->GetDataType() == VTK_DOUBLE)
    {
        t = VISIT_DATATYPE_DOUBLE;
        S = ConditionalDataArrayCopy((double*)arr->GetVoidPointer(0), 
            arr->GetNumberOfTuples(), arr->GetNumberOfComponents(), sum, cellCopy);
    }
    else
    {
        debug1 << "Can't export type " << arr->GetDataType()
               << " to simulation." << endl;
        return;
    }

    // Assemble a scalar metadata object.
    const avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();    
    VisIt_VariableMetaData *smd = (VisIt_VariableMetaData*)malloc(
        sizeof(VisIt_VariableMetaData));
    memset(smd, 0, sizeof(VisIt_VariableMetaData));
    smd->name = strdup(arr->GetName());
    smd->meshName = strdup(objectName.c_str());
    smd->centering = VISIT_VARCENTERING_ZONE;
    smd->treatAsASCII = atts.GetTreatAsASCII(arr->GetName());

    int ret = visit_invoke_WriteVariable(objectName.c_str(), arr->GetName(), 
         chunk, t, S, sum, arr->GetNumberOfComponents(), smd);

    if(ret != VISIT_OKAY)
    {
        debug1 << "WriteDataArray callback returned " << ret
               << " instead of VISIT_OKAY." << endl;
    }

    // Free the temporary copy of the array.
    free(S);

    // Free the variable metadata.
    VisIt_VariableMetaData_free(smd);
    free(smd);
}

// ****************************************************************************
// Method: avtSimV2Writer::WriteDataArrays
//
// Purpose: 
//   Sends all of the exported data arrays to the application.
//
// Arguments:
//   ds    : The VTK dataset that has the arrays.
//   chunk : The chunk id.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 2 17:47:38 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtSimV2Writer::WriteDataArrays(vtkDataSet *ds, int chunk)
{
    for (int v = 0 ; v < varList.size() ; v++)
    {
        vtkDataArray *arr = ds->GetCellData()->GetArray(varList[v].c_str());
        if (arr != 0)
        {
            WriteOneDataArray(arr, objectName, chunk, VISIT_VARCENTERING_ZONE);
        }
        else
        {
            arr = ds->GetPointData()->GetArray(varList[v].c_str());
            if (arr != 0)
            {
                WriteOneDataArray(arr, objectName, chunk, VISIT_VARCENTERING_NODE);
            }
            else
            {
                EXCEPTION1(ImproperUseException,
                           string("Couldn't find array ")+varList[v]+".");
            }
        }
    }
}

// ****************************************************************************
// Method: avtSimV2Writer::WriteDataArraysConditionally
//
// Purpose: 
//   Writes all of the exported arrays, taking special care of cell-centered
//   data when the cell connectivity has changed.
//
// Arguments:
//   ds       : The VTK dataset containing the data arrays to export.
//   chunk    : The chunk id.
//   cellCopy : An nCells sized array containing the number of times each cell
//              data should appear in the data sent to the application.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 2 17:48:29 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtSimV2Writer::WriteDataArraysConditionally(vtkDataSet *ds, int chunk,
    const unsigned char *cellCopy)
{
    for (int v = 0 ; v < varList.size() ; v++)
    {
        vtkDataArray *arr = ds->GetCellData()->GetArray(varList[v].c_str());
        if (arr != 0)
        {
            WriteCellDataArrayConditionally(arr, objectName, chunk, cellCopy);
        }
        else
        {
            arr = ds->GetPointData()->GetArray(varList[v].c_str());
            if (arr != 0)
            {
                WriteOneDataArray(arr, objectName, chunk, VISIT_VARCENTERING_NODE);
            }
            else
            {
                EXCEPTION1(ImproperUseException,
                           string("Couldn't find array ")+varList[v]+".");
            }
        }
    }
}
