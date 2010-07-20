/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
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

#include <simv2_MeshMetaData.h>
#include <simv2_VariableMetaData.h>

#include <simv2_CurvilinearMesh.h>
#include <simv2_PointMesh.h>
#include <simv2_RectilinearMesh.h>
#include <simv2_UnstructuredMesh.h>
#include <simv2_VariableData.h>

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
    simv2_invoke_WriteBegin(objName.c_str());
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
// Creation:   Tue Mar  9 13:57:31 PST 2010
//
// Modifications:
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

    visit_handle vmmd = VISIT_INVALID_HANDLE;
    simv2_MeshMetaData_alloc(&vmmd);
    simv2_MeshMetaData_setName(vmmd, objectName.c_str());
    simv2_MeshMetaData_setTopologicalDimension(vmmd, atts.GetTopologicalDimension());
    simv2_MeshMetaData_setSpatialDimension(vmmd, atts.GetSpatialDimension());

    if(mmd != 0)
    {
        simv2_MeshMetaData_setNumDomains(vmmd, numblocks);
        simv2_MeshMetaData_setDomainTitle(vmmd, mmd->blockTitle.c_str());
        simv2_MeshMetaData_setDomainPieceName(vmmd, mmd->blockPieceName.c_str());

        simv2_MeshMetaData_setNumGroups(vmmd, 0);
        simv2_MeshMetaData_setGroupTitle(vmmd, mmd->groupTitle.c_str());
    }
    else
    {
        simv2_MeshMetaData_setNumDomains(vmmd, 1);
        simv2_MeshMetaData_setDomainTitle(vmmd, "domains");
        simv2_MeshMetaData_setDomainPieceName(vmmd, "domain");

        simv2_MeshMetaData_setNumGroups(vmmd, 0);
        simv2_MeshMetaData_setGroupTitle(vmmd, "groups");
    }

    simv2_MeshMetaData_setXUnits(vmmd, atts.GetXUnits().c_str());
    simv2_MeshMetaData_setYUnits(vmmd, atts.GetYUnits().c_str());
    simv2_MeshMetaData_setZUnits(vmmd, atts.GetZUnits().c_str());

    simv2_MeshMetaData_setXLabel(vmmd, atts.GetXLabel().c_str());
    simv2_MeshMetaData_setYLabel(vmmd, atts.GetYLabel().c_str());
    simv2_MeshMetaData_setZLabel(vmmd, atts.GetZLabel().c_str());

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
    simv2_MeshMetaData_free(vmmd);
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
    simv2_invoke_WriteEnd(objectName.c_str());
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
// Creation:   Thu Feb 25 16:08:38 PST 2010
//
// Modifications:
//   Eric Brugger, Wed Mar  3 10:53:11 PST 2010
//   I replaced the variables hx, hy and hz with hhx, hhy and hhz, so that
//   it would compile on AIX.
//   
// ****************************************************************************

void
avtSimV2Writer::WriteCurvilinearMesh(vtkStructuredGrid *ds, int chunk,
    visit_handle vmmd)
{
    const char *mName = "avtSimV2Writer::WriteCurvilinearMesh: ";
    debug1 << mName << "(chunk=" << chunk << ")\n";

    // Translate the VTK structure back into VisIt_CurvilinearMesh.
    visit_handle h = VISIT_INVALID_HANDLE;
    if(simv2_CurvilinearMesh_alloc(&h) == VISIT_ERROR)
        return;

    int dims[3]={0,0,0}, baseIndex[3]={0,0,0};
    int minRealIndex[3]={0,0,0}, maxRealIndex[3]={0,0,0};
    ds->GetDimensions(dims);
    maxRealIndex[0] = dims[0]-1;
    maxRealIndex[1] = dims[1]-1;
    maxRealIndex[2] = dims[2]-1;

    if(ds->GetDataDimension() == 1)
    {
        debug1 << mName << "1D data not supported" << endl;
    }
    else if(ds->GetDataDimension() == 2)
    {
        double *x = (double*)malloc(ds->GetNumberOfPoints() * sizeof(double));
        double *y = (double*)malloc(ds->GetNumberOfPoints() * sizeof(double));
        for(vtkIdType i = 0; i < ds->GetNumberOfPoints(); ++i)
        {
            double *pt = ds->GetPoint(i);
            x[i] = pt[0];
            y[i] = pt[1];
        }
        visit_handle hhx, hhy;
        simv2_VariableData_alloc(&hhx);
        simv2_VariableData_alloc(&hhy);
        simv2_VariableData_setData(hhx, VISIT_OWNER_VISIT, VISIT_DATATYPE_DOUBLE,
            1, ds->GetNumberOfPoints(), x);
        simv2_VariableData_setData(hhy, VISIT_OWNER_VISIT, VISIT_DATATYPE_DOUBLE,
            1, ds->GetNumberOfPoints(), y);
        simv2_CurvilinearMesh_setCoordsXY(h, dims, hhx, hhy);
    }
    else if(ds->GetDataDimension() == 3)
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

        visit_handle hhx, hhy, hhz;
        simv2_VariableData_alloc(&hhx);
        simv2_VariableData_alloc(&hhy);
        simv2_VariableData_alloc(&hhz);
        simv2_VariableData_setData(hhx, VISIT_OWNER_VISIT, VISIT_DATATYPE_DOUBLE,
            1, ds->GetNumberOfPoints(), x);
        simv2_VariableData_setData(hhy, VISIT_OWNER_VISIT, VISIT_DATATYPE_DOUBLE,
            1, ds->GetNumberOfPoints(), y);
        simv2_VariableData_setData(hhz, VISIT_OWNER_VISIT, VISIT_DATATYPE_DOUBLE,
            1, ds->GetNumberOfPoints(), z);
        simv2_CurvilinearMesh_setCoordsXYZ(h, dims, hhx, hhy, hhz);
    }

    simv2_CurvilinearMesh_setRealIndices(h, minRealIndex, maxRealIndex);
    simv2_CurvilinearMesh_setBaseIndex(h, baseIndex);

    // Call into the simulation to write the data back.
    simv2_MeshMetaData_setMeshType(vmmd, VISIT_MESHTYPE_CURVILINEAR);
    int ret = simv2_invoke_WriteMesh(objectName.c_str(), chunk, 
        VISIT_MESHTYPE_CURVILINEAR, h, vmmd);
    if(ret != VISIT_OKAY)
    {
        debug1 << "WriteMesh callback returned " << ret
               << " instead of VISIT_OKAY." << endl;
    }

    // Free the mesh that we created.
    simv2_FreeObject(h);
 
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
// Creation:   Thu Feb 25 16:17:34 PST 2010
//
// Modifications:
//   Eric Brugger, Wed Mar  3 10:53:11 PST 2010
//   I replaced the variables hx, hy and hz with hhx, hhy and hhz, so that
//   it would compile on AIX.
//   
// ****************************************************************************

void
avtSimV2Writer::WriteRectilinearMesh(vtkRectilinearGrid *ds, int chunk, 
    visit_handle vmmd)
{
    debug1 << "avtSimV2Writer::WriteRectilinearMesh(chunk=" << chunk << ")\n";

    // Translate the VTK structure back into VisIt_RectilinearMesh.
    visit_handle h = VISIT_INVALID_HANDLE;
    if(simv2_RectilinearMesh_alloc(&h) == VISIT_ERROR)
        return;

    int dims[3]={0,0,0}, baseIndex[3]={0,0,0};
    int minRealIndex[3]={0,0,0}, maxRealIndex[3]={0,0,0};
    ds->GetDimensions(dims);
    maxRealIndex[0] = dims[0]-1;
    maxRealIndex[1] = dims[1]-1;
    maxRealIndex[2] = dims[2]-1;

    visit_handle hhx, hhy, hhz;
    if(ds->GetDataDimension() >= 1)
    {
        vtkDataArray *xc = ds->GetXCoordinates();
        double *x = (double*)malloc(xc->GetNumberOfTuples() * sizeof(double));
        for(vtkIdType i = 0; i < xc->GetNumberOfTuples(); ++i)
            x[i] = xc->GetTuple1(i);

        simv2_VariableData_alloc(&hhx);
        simv2_VariableData_setData(hhx, VISIT_OWNER_VISIT, VISIT_DATATYPE_DOUBLE,
            1, xc->GetNumberOfTuples(), x);
    }

    if(ds->GetDataDimension() >= 2)
    {
        vtkDataArray *yc = ds->GetYCoordinates();
        double *y = (double*)malloc(yc->GetNumberOfTuples() * sizeof(double));
        for(vtkIdType i = 0; i < yc->GetNumberOfTuples(); ++i)
            y[i] = yc->GetTuple1(i);

        simv2_VariableData_alloc(&hhy);
        simv2_VariableData_setData(hhy, VISIT_OWNER_VISIT, VISIT_DATATYPE_DOUBLE, 
            1, yc->GetNumberOfTuples(), y);
    }

    if(ds->GetDataDimension() >= 3)
    {
        vtkDataArray *zc = ds->GetZCoordinates();
        double *z = (double*)malloc(zc->GetNumberOfTuples() * sizeof(double));
        for(vtkIdType i = 0; i < zc->GetNumberOfTuples(); ++i)
            z[i] = zc->GetTuple1(i);

        simv2_VariableData_alloc(&hhz);
        simv2_VariableData_setData(hhz, VISIT_OWNER_VISIT, VISIT_DATATYPE_DOUBLE,
            1, zc->GetNumberOfTuples(), z);
    }

    if(ds->GetDataDimension() == 3)
        simv2_RectilinearMesh_setCoordsXYZ(h, hhx, hhy, hhz); 
    else
        simv2_RectilinearMesh_setCoordsXY(h, hhx, hhy); 
    simv2_RectilinearMesh_setRealIndices(h, minRealIndex, maxRealIndex);
    simv2_RectilinearMesh_setBaseIndex(h, baseIndex);

    // Call into the simulation to write the data back.
    simv2_MeshMetaData_setMeshType(vmmd, VISIT_MESHTYPE_RECTILINEAR);
    int ret = simv2_invoke_WriteMesh(objectName.c_str(), chunk, 
        VISIT_MESHTYPE_RECTILINEAR, h, vmmd);
    if(ret != VISIT_OKAY)
    {
        debug1 << "WriteRectilinearMesh callback returned " << ret
               << " instead of VISIT_OKAY." << endl;
    }

    simv2_FreeObject(h);

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
// Creation:   Thu Feb 25 16:31:10 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
avtSimV2Writer::WriteUnstructuredMesh(vtkUnstructuredGrid *ds, int chunk,
    visit_handle vmmd)
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
        visit_handle h = VISIT_INVALID_HANDLE;
        if(simv2_PointMesh_alloc(&h) == VISIT_ERROR)
            return;

        // Send the coordinates down to the writer interleaved.
        visit_handle hc;
        simv2_VariableData_alloc(&hc);
        simv2_VariableData_setData(hc, VISIT_OWNER_SIM, VISIT_DATATYPE_FLOAT, 3,
            ds->GetPoints()->GetNumberOfPoints(), 
            ds->GetPoints()->GetVoidPointer(0));
        simv2_PointMesh_setCoords(h, hc);

        // Call into the simulation to write the data back.
        simv2_MeshMetaData_setMeshType(vmmd, VISIT_MESHTYPE_POINT);
        int ret = simv2_invoke_WriteMesh(objectName.c_str(), chunk, 
             VISIT_MESHTYPE_POINT, h, vmmd);
        if(ret != VISIT_OKAY)
        {
            debug1 << "WritePointMesh callback returned " << ret
                   << " instead of VISIT_OKAY." << endl;
        }

        simv2_FreeObject(h);

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
            visit_handle h = VISIT_INVALID_HANDLE;
            if(simv2_UnstructuredMesh_alloc(&h) == VISIT_ERROR)
                return;

            // Send the coordinates down to the writer interleaved.
            visit_handle hc;
            simv2_VariableData_alloc(&hc);
            simv2_VariableData_setData(hc, VISIT_OWNER_SIM, VISIT_DATATYPE_FLOAT, 3,
                ds->GetPoints()->GetNumberOfPoints(),
                ds->GetPoints()->GetVoidPointer(0));
            simv2_UnstructuredMesh_setCoords(h, hc);

            // Send the connectivity down.
            visit_handle hconn;
            simv2_VariableData_alloc(&hconn);
            simv2_VariableData_setData(hconn, VISIT_OWNER_VISIT, 
                VISIT_DATATYPE_INT, 1, connLen, conn);
            simv2_UnstructuredMesh_setConnectivity(h, cellCount, hconn);

            // Call into the simulation to write the data back.
            simv2_MeshMetaData_setMeshType(vmmd, VISIT_MESHTYPE_UNSTRUCTURED);
            int ret = simv2_invoke_WriteMesh(objectName.c_str(), chunk, 
                VISIT_MESHTYPE_UNSTRUCTURED, h, vmmd);
            if(ret != VISIT_OKAY)
            {
                debug1 << "WriteUnstructuredMesh callback returned " << ret
                       << " instead of VISIT_OKAY." << endl;
            }

            simv2_FreeObject(h);

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
// Creation:   Thu Feb 25 16:35:38 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
avtSimV2Writer::WritePolyDataMesh(vtkPolyData *ds, int chunk, visit_handle vmmd)
{
    debug1 << "avtSimV2Writer::WritePolyDataMesh(chunk=" << chunk << ")\n";

    // Build up an unstructured mesh from the types of data in the polydata.
    vtkIdType pts[100];

    debug1 << "polydata npts   = " << ds->GetNumberOfPoints() << endl;
    debug1 << "polydata nverts = " << ds->GetVerts()->GetNumberOfCells() << endl;
    debug1 << "polydata nlines = " << ds->GetLines()->GetNumberOfCells() << endl;
    debug1 << "polydata npolys = " << ds->GetPolys()->GetNumberOfCells() << endl;

    if(ds->GetVerts()->GetNumberOfCells() > 0 && 
       (ds->GetLines()->GetNumberOfCells() == 0 && 
        ds->GetPolys()->GetNumberOfCells() == 0))
    {
        // It's a point mesh.
        visit_handle h = VISIT_INVALID_HANDLE;
        if(simv2_PointMesh_alloc(&h) == VISIT_ERROR)
            return;

        // Send the coordinates down to the writer interleaved.
        visit_handle hc;
        simv2_VariableData_alloc(&hc);
        simv2_VariableData_setData(hc, VISIT_OWNER_SIM, VISIT_DATATYPE_FLOAT, 3,
            ds->GetPoints()->GetNumberOfPoints(),
            ds->GetPoints()->GetVoidPointer(0));
        simv2_PointMesh_setCoords(h, hc);

        debug1 << "Write polydata as point mesh" << endl;

        // Call into the simulation to write the data back.
        simv2_MeshMetaData_setMeshType(vmmd, VISIT_MESHTYPE_POINT);
        int ret = simv2_invoke_WriteMesh(objectName.c_str(), chunk, 
            VISIT_MESHTYPE_POINT, h, vmmd);
        if(ret != VISIT_OKAY)
        {
            debug1 << "WritePointMesh callback returned " << ret
                   << " instead of VISIT_OKAY." << endl;
        }

        simv2_FreeObject(h);

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

        // It's an unstructured mesh.
        visit_handle h = VISIT_INVALID_HANDLE;
        if(simv2_UnstructuredMesh_alloc(&h) == VISIT_ERROR)
            return;

        // Send the coordinates down to the writer interleaved.
        visit_handle hc;
        simv2_VariableData_alloc(&hc);
        simv2_VariableData_setData(hc, VISIT_OWNER_SIM, VISIT_DATATYPE_FLOAT, 3,
            ds->GetPoints()->GetNumberOfPoints(),
            ds->GetPoints()->GetVoidPointer(0));
        simv2_UnstructuredMesh_setCoords(h, hc);

        // Send the connectivity down.
        visit_handle hconn;
        simv2_VariableData_alloc(&hconn);
        simv2_VariableData_setData(hconn, VISIT_OWNER_VISIT, VISIT_DATATYPE_INT,
            1, connPtr - conn, conn);
        simv2_UnstructuredMesh_setConnectivity(h, cellCount, hconn);

        debug1 << "nzones = " << cellCount << endl;
        debug1 << "connectivityLen = " << (connPtr - conn) << endl;

        // Call into the simulation to write the data back.
        simv2_MeshMetaData_setMeshType(vmmd, VISIT_MESHTYPE_UNSTRUCTURED);
        int ret = simv2_invoke_WriteMesh(objectName.c_str(), chunk, 
            VISIT_MESHTYPE_UNSTRUCTURED, h, vmmd);
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

        simv2_FreeObject(h);
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

    // Assemble a variable object.
    visit_handle data = VISIT_INVALID_HANDLE;
    simv2_VariableData_alloc(&data);
    if(arr->GetDataType() == VTK_CHAR)
        simv2_VariableData_setData(data, VISIT_OWNER_SIM, VISIT_DATATYPE_CHAR,
                                   arr->GetNumberOfComponents(),
                                   arr->GetNumberOfTuples(), 
                                   (char *)arr->GetVoidPointer(0));
    else if(arr->GetDataType() == VTK_INT)
        simv2_VariableData_setData(data, VISIT_OWNER_SIM, VISIT_DATATYPE_INT,
                                   arr->GetNumberOfComponents(),
                                   arr->GetNumberOfTuples(),
                                   (int *)arr->GetVoidPointer(0));
    else if(arr->GetDataType() == VTK_FLOAT)
        simv2_VariableData_setData(data, VISIT_OWNER_SIM, VISIT_DATATYPE_FLOAT,
                                   arr->GetNumberOfComponents(),
                                   arr->GetNumberOfTuples(),
                                   (float *)arr->GetVoidPointer(0));
    else if(arr->GetDataType() == VTK_DOUBLE)
        simv2_VariableData_setData(data, VISIT_OWNER_SIM,  VISIT_DATATYPE_DOUBLE,
                                   arr->GetNumberOfComponents(),
                                   arr->GetNumberOfTuples(), 
                                   (double*)arr->GetVoidPointer(0));
    else
    {
        debug1 << "Can't export type " << arr->GetDataType()
               << " to simulation." << endl;
        simv2_VariableData_free(data);
        return;
    }

    // Assemble a variable metadata object.
    const avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();    
    visit_handle metadata = VISIT_INVALID_HANDLE;
    simv2_VariableMetaData_alloc(&metadata);
    simv2_VariableMetaData_setName(metadata, arr->GetName());
    simv2_VariableMetaData_setMeshName(metadata, objectName.c_str());
    simv2_VariableMetaData_setCentering(metadata, cent);
    simv2_VariableMetaData_setTreatAsASCII(metadata, atts.GetTreatAsASCII(arr->GetName())?1:0);

    // Call back to the simulation to write the data.
    int ret = simv2_invoke_WriteVariable(objectName.c_str(), arr->GetName(), 
                                         chunk, data, metadata);
    if(ret != VISIT_OKAY)
    {
        debug1 << "WriteDataArray callback returned " << ret
               << " instead of VISIT_OKAY." << endl;
    }

    // Free the objects
    simv2_VariableData_free(data);
    simv2_VariableMetaData_free(metadata);
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

    // Store the data in a variabledata object
    visit_handle data = VISIT_INVALID_HANDLE;
    simv2_VariableData_alloc(&data);
    simv2_VariableData_setData(data, VISIT_OWNER_VISIT, t,
        arr->GetNumberOfComponents(), arr->GetNumberOfTuples(), S);

    // Assemble a scalar metadata object.
    const avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();

    visit_handle smd = VISIT_INVALID_HANDLE;
    simv2_VariableMetaData_alloc(&smd);
    simv2_VariableMetaData_setName(smd, arr->GetName());
    simv2_VariableMetaData_setMeshName(smd, objectName.c_str());
    simv2_VariableMetaData_setCentering(smd, VISIT_VARCENTERING_ZONE);
    simv2_VariableMetaData_setTreatAsASCII(smd, atts.GetTreatAsASCII(arr->GetName())?1:0);

    int ret = simv2_invoke_WriteVariable(objectName.c_str(), arr->GetName(), 
         chunk, data, smd);

    if(ret != VISIT_OKAY)
    {
        debug1 << "WriteDataArray callback returned " << ret
               << " instead of VISIT_OKAY." << endl;
    }

    // Free the objects
    simv2_VariableData_free(data);
    simv2_VariableMetaData_free(smd);
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
