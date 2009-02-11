#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkFieldData.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkIdTypeArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVisItUtility.h>

#include <vtkCSGGrid.h>

#include <VisItDataInterface_V2.h>

#include <avtGhostData.h>

#include <DebugStream.h>
#include <ImproperUseException.h>

void FreeDataArray(VisIt_DataArray &da);

// ****************************************************************************
//  Function:  GetQuadGhostZones
//
//  Purpose:  Add ghost zone information to a dataset.
//    
//  Note:  stolen from the Silo file format method of the same name
//
// ****************************************************************************
static void 
GetQuadGhostZones(int nnodes, int ndims,
                  int *dims, int *min_index, int *max_index,
                  vtkDataSet *ds)
{
    //
    //  Determine if we have ghost points
    //
    int first[3];
    int last[3];
    bool ghostPresent = false;
    bool badIndex = false;
    for (int i = 0; i < 3; i++)
    {
        first[i] = (i < ndims ? min_index[i] : 0);
        last[i]  = (i < ndims ? max_index[i] : 0);

        if (first[i] < 0 || first[i] >= dims[i])
        {
            debug1 << "bad Index on first[" << i << "] dims is: " 
                   << dims[i] << endl;
            badIndex = true;
        }

        if (last[i] < 0 || last[i] >= dims[i])
        {
            debug1 << "bad Index on last[" << i << "] dims is: " 
                   << dims[i] << endl;
            badIndex = true;
        }

        if (first[i] != 0 || last[i] != dims[i] -1)
        {
            ghostPresent = true;
        }
    }

    //
    //  Create the ghost zones array if necessary
    //
    if (ghostPresent && !badIndex)
    {
        bool *ghostPoints = new bool[nnodes];
        //
        // Initialize as all ghost levels
        //
        for (int ii = 0; ii < nnodes; ii++)
            ghostPoints[ii] = true; 

        //
        // Set real values
        //
        for (int k = first[2]; k <= last[2]; k++)
            for (int j = first[1]; j <= last[1]; j++)
                for (int i = first[0]; i <= last[0]; i++)
                {
                    int index = k*dims[1]*dims[0] + j*dims[0] + i;
                    ghostPoints[index] = false; 
                }

        //
        //  okay, now we have ghost points, but what we really want
        //  are ghost cells ... convert:  if all points associated with
        //  cell are 'real' then so is the cell.
        //
        unsigned char realVal = 0;
        unsigned char ghostVal = 0;
        avtGhostData::AddGhostZoneType(ghostVal, 
                                       DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
        int ncells = ds->GetNumberOfCells();
        vtkIdList *ptIds = vtkIdList::New();
        vtkUnsignedCharArray *ghostCells = vtkUnsignedCharArray::New();
        ghostCells->SetName("avtGhostZones");
        ghostCells->Allocate(ncells);
 
        for (int i = 0; i < ncells; i++)
        {
            ds->GetCellPoints(i, ptIds);
            bool ghost = false;
            for (int idx = 0; idx < ptIds->GetNumberOfIds(); idx++)
                ghost |= ghostPoints[ptIds->GetId(idx)];

            if (ghost)
                ghostCells->InsertNextValue(ghostVal);
            else
                ghostCells->InsertNextValue(realVal);
 
        } 
        ds->GetCellData()->AddArray(ghostCells);
        delete [] ghostPoints;
        ghostCells->Delete();
        ptIds->Delete();

        vtkIntArray *realDims = vtkIntArray::New();
        realDims->SetName("avtRealDims");
        realDims->SetNumberOfValues(6);
        realDims->SetValue(0, first[0]);
        realDims->SetValue(1, last[0]);
        realDims->SetValue(2, first[1]);
        realDims->SetValue(3, last[1]);
        realDims->SetValue(4, first[2]);
        realDims->SetValue(5, last[2]);
        ds->GetFieldData()->AddArray(realDims);
        ds->GetFieldData()->CopyFieldOn("avtRealDims");
        realDims->Delete();

        ds->SetUpdateGhostLevel(0);
    }
}

// ****************************************************************************
// Method: SimV2_GetMesh_Curvilinear
//
// Purpose: 
//   Builds a VTK curvilinear mesh from the simulation mesh.
//
// Arguments:
//   cmesh : The simulation mesh.
//
// Returns:    A VTK dataset that represents the simulation mesh.
//
// Note:       
//
// Programmer: Jeremy Meredith
// Creation:   March 14, 2005
//
// Modifications:
//   
// ****************************************************************************

vtkDataSet *
SimV2_GetMesh_Curvilinear(VisIt_CurvilinearMesh *cmesh)
{
    if (!cmesh)
        return NULL;

    //
    // Create the VTK objects and connect them up.
    //
    vtkStructuredGrid    *sgrid   = vtkStructuredGrid::New(); 
    vtkPoints            *points  = vtkPoints::New();
    sgrid->SetPoints(points);
    points->Delete();

    //
    // Tell the grid what its dimensions are and populate the points array.
    //
    sgrid->SetDimensions(cmesh->dims);

    //
    // Populate the coordinates.
    //
    int ni = cmesh->dims[0];
    int nj = cmesh->dims[1];
    int nk = cmesh->dims[2];
    points->SetNumberOfPoints(ni * nj * nk);
    float *pts = (float *) points->GetVoidPointer(0);

    if (cmesh->xcoords.dataType == VISIT_DATATYPE_FLOAT)
    {
        int npts = 0;
        for (int i=0; i<ni; i++)
        {
            for (int j=0; j<nj; j++)
            {
                for (int k=0; k<nk; k++)
                {
                    pts[npts*3 + 0] = cmesh->xcoords.fArray[npts];
                    pts[npts*3 + 1] = cmesh->ycoords.fArray[npts];
                    if (cmesh->ndims==3)
                        pts[npts*3 + 2] = cmesh->zcoords.fArray[npts];
                    else
                        pts[npts*3 + 2] = 0;
                    npts++;
                }
            }
        }
    }
    else if (cmesh->xcoords.dataType == VISIT_DATATYPE_DOUBLE)
    {
        int npts = 0;
        for (int i=0; i<ni; i++)
        {
            for (int j=0; j<nj; j++)
            {
                for (int k=0; k<nk; k++)
                {
                    pts[npts*3 + 0] = cmesh->xcoords.dArray[npts];
                    pts[npts*3 + 1] = cmesh->ycoords.dArray[npts];
                    if (cmesh->ndims==3)
                        pts[npts*3 + 2] = cmesh->zcoords.dArray[npts];
                    else
                        pts[npts*3 + 2] = 0;
                    npts++;
                }
            }
        }
    }
    else
    {
        EXCEPTION1(ImproperUseException,
                   "Coordinate arrays must be float or double.\n");
    }

    GetQuadGhostZones(ni*nj*nk,
                      cmesh->ndims,
                      cmesh->dims,
                      cmesh->minRealIndex,
                      cmesh->maxRealIndex,
                      sgrid);

    vtkIntArray *arr = vtkIntArray::New();
    arr->SetNumberOfTuples(3);
    arr->SetValue(0, cmesh->baseIndex[0]);
    arr->SetValue(1, cmesh->baseIndex[1]);
    arr->SetValue(2, cmesh->baseIndex[2]);
    arr->SetName("base_index");
    sgrid->GetFieldData()->AddArray(arr);
    arr->Delete();

    return sgrid;
}

// ****************************************************************************
// Method: SimV2_GetMesh_Rectilinear
//
// Purpose: 
//   Builds a VTK rectilinear mesh from the simulation mesh.
//
// Arguments:
//  rmesh : The simulation mesh.
//
// Returns:    A VTK dataset that represents the simulation mesh.
//
// Note:       
//
// Programmer: Jeremy Meredith
// Creation:   March 14, 2005
//
// Modifications:
//   
// ****************************************************************************

vtkDataSet *
SimV2_GetMesh_Rectilinear(VisIt_RectilinearMesh *rmesh)
{
    if (!rmesh)
        return NULL;

    //
    // Create the VTK objects and connect them up.
    //
    vtkRectilinearGrid   *rgrid   = vtkRectilinearGrid::New(); 

    //
    // Tell the grid what its dimensions are and populate the points array.
    //
    rgrid->SetDimensions(rmesh->dims);

    //
    // Populate the coordinates.
    //
    int ni = rmesh->dims[0];
    int nj = rmesh->dims[1];
    int nk = rmesh->dims[2];

    vtkFloatArray *xcoords;
    vtkFloatArray *ycoords;
    vtkFloatArray *zcoords;

    xcoords = vtkFloatArray::New();
    xcoords->SetNumberOfTuples(ni);
    ycoords = vtkFloatArray::New();
    ycoords->SetNumberOfTuples(nj);
    zcoords = vtkFloatArray::New();
    zcoords->SetNumberOfTuples(nk);

    if (rmesh->xcoords.dataType == VISIT_DATATYPE_FLOAT)
    {
        for (int i=0; i<ni; i++)
            xcoords->SetComponent(i, 0, rmesh->xcoords.fArray[i]);

        for (int j=0; j<nj; j++)
            ycoords->SetComponent(j, 0, rmesh->ycoords.fArray[j]);

        if (rmesh->ndims==3)
        {
            for (int k=0; k<nk; k++)
                zcoords->SetComponent(k, 0, rmesh->zcoords.fArray[k]);
        }
        else
        {
            for (int k=0; k<nk; k++)
                zcoords->SetComponent(k, 0, 0);
        }
    }
    else if (rmesh->xcoords.dataType == VISIT_DATATYPE_DOUBLE)
    {
        for (int i=0; i<ni; i++)
            xcoords->SetComponent(i, 0, rmesh->xcoords.dArray[i]);

        for (int j=0; j<nj; j++)
            ycoords->SetComponent(j, 0, rmesh->ycoords.dArray[j]);

        if (rmesh->ndims==3)
        {
            for (int k=0; k<nk; k++)
                zcoords->SetComponent(k, 0, rmesh->zcoords.dArray[k]);
        }
        else
        {
            for (int k=0; k<nk; k++)
                zcoords->SetComponent(k, 0, 0);
        }
    }
    else
    {
        EXCEPTION1(ImproperUseException,
                   "Coordinate arrays must be float or double.\n");
    }

    rgrid->SetXCoordinates(xcoords);
    xcoords->Delete();
    rgrid->SetYCoordinates(ycoords);
    ycoords->Delete();
    rgrid->SetZCoordinates(zcoords);
    zcoords->Delete();

    GetQuadGhostZones(ni*nj*nk,
                      rmesh->ndims,
                      rmesh->dims,
                      rmesh->minRealIndex,
                      rmesh->maxRealIndex,
                      rgrid);

    vtkIntArray *arr = vtkIntArray::New();
    arr->SetNumberOfTuples(3);
    arr->SetValue(0, rmesh->baseIndex[0]);
    arr->SetValue(1, rmesh->baseIndex[1]);
    arr->SetValue(2, rmesh->baseIndex[2]);
    arr->SetName("base_index");
    rgrid->GetFieldData()->AddArray(arr);
    arr->Delete();

    return rgrid;
}

// ****************************************************************************
// Method: SimV2_GetMesh_Unstructured
//
// Purpose: 
//   Builds a VTK unstructured mesh from the simulation mesh.
//
// Arguments:
//   umesh : The simulation mesh.
//
// Returns:    A VTK dataset that represents the simulation mesh.
//
// Note:       
//
// Programmer: Jeremy Meredith
// Creation:   March 14, 2005
//
// Modifications:
//   
// ****************************************************************************

vtkDataSet *
SimV2_GetMesh_Unstructured(VisIt_UnstructuredMesh *umesh)
{
    if (!umesh)
        return NULL;

    if (umesh->connectivity.dataType != VISIT_DATATYPE_INT)
    {
        EXCEPTION1(ImproperUseException,
                   "Connectivity array must be ints.");
    }

    vtkUnstructuredGrid  *ugrid = vtkUnstructuredGrid::New();
    vtkPoints            *points  = vtkPoints::New();
    ugrid->SetPoints(points);
    points->Delete();

    //
    // Populate the coordinates.
    //
    int npts = umesh->nnodes;
    points->SetNumberOfPoints(npts);
    float *pts = (float *) points->GetVoidPointer(0);

    if (umesh->xcoords.dataType == VISIT_DATATYPE_FLOAT)
    {
        for (int i=0; i<npts; i++)
        {
            pts[i*3 + 0] = umesh->xcoords.fArray[i];
            pts[i*3 + 1] = umesh->ycoords.fArray[i];
            if (umesh->ndims==3)
                pts[i*3 + 2] = umesh->zcoords.fArray[i];
            else
                pts[i*3 + 2] = 0;
        }
    }
    else if (umesh->xcoords.dataType == VISIT_DATATYPE_DOUBLE)
    {
        for (int i=0; i<npts; i++)
        {
            pts[i*3 + 0] = umesh->xcoords.dArray[i];
            pts[i*3 + 1] = umesh->ycoords.dArray[i];
            if (umesh->ndims==3)
                pts[i*3 + 2] = umesh->zcoords.dArray[i];
            else
                pts[i*3 + 2] = 0;
        }
    }
    else
    {
        EXCEPTION1(ImproperUseException,
                   "Coordinate arrays must be float or double.\n");
    }

    int celltype_npts[10];
    celltype_npts[VISIT_CELL_BEAM]  = 2;
    celltype_npts[VISIT_CELL_TRI]   = 3;
    celltype_npts[VISIT_CELL_QUAD]  = 4;
    celltype_npts[VISIT_CELL_TET]   = 4;
    celltype_npts[VISIT_CELL_PYR]   = 5;
    celltype_npts[VISIT_CELL_WEDGE] = 6;
    celltype_npts[VISIT_CELL_HEX]   = 8;

    int celltype_idtype[10];
    celltype_idtype[VISIT_CELL_BEAM]  = VTK_LINE;
    celltype_idtype[VISIT_CELL_TRI]   = VTK_TRIANGLE;
    celltype_idtype[VISIT_CELL_QUAD]  = VTK_QUAD;
    celltype_idtype[VISIT_CELL_TET]   = VTK_TETRA;
    celltype_idtype[VISIT_CELL_PYR]   = VTK_PYRAMID;
    celltype_idtype[VISIT_CELL_WEDGE] = VTK_WEDGE;
    celltype_idtype[VISIT_CELL_HEX]   = VTK_HEXAHEDRON;

    vtkIdTypeArray *nlist = vtkIdTypeArray::New();
    nlist->SetNumberOfValues(umesh->connectivityLen);
    vtkIdType *nl = nlist->GetPointer(0);

    vtkUnsignedCharArray *cellTypes = vtkUnsignedCharArray::New();
    cellTypes->SetNumberOfValues(umesh->nzones);
    unsigned char *ct = cellTypes->GetPointer(0);

    vtkIdTypeArray *cellLocations = vtkIdTypeArray::New();
    cellLocations->SetNumberOfValues(umesh->nzones);
    int *cl = cellLocations->GetPointer(0);

    int numCells = 0;
    int offset = 0;
    while (offset < umesh->connectivityLen)
    {
        int celltype = umesh->connectivity.iArray[offset];

        int vtktype = celltype_idtype[celltype];
        int nelempts = celltype_npts[celltype];
        *ct++ = vtktype;
        *nl++ = nelempts;

        for (int j=0; j<nelempts; j++)
        {
            *nl++ = umesh->connectivity.iArray[offset+1+j];
        }

        numCells++;
        *cl++ = offset;
        offset += nelempts+1;
    }

    if (numCells != umesh->nzones)
    {
        EXCEPTION1(ImproperUseException,
                   "Number of zones and length of connectivity "
                   "array did not match!");
    }

    vtkCellArray *cells = vtkCellArray::New();
    cells->SetCells(umesh->nzones, nlist);
    nlist->Delete();

    ugrid->SetCells(cellTypes, cellLocations, cells);
    cellTypes->Delete();
    cellLocations->Delete();
    cells->Delete();

    int firstCell = umesh->firstRealZone;
    int lastCell  = umesh->lastRealZone;
    if (firstCell == 0 && lastCell == 0 )
    {
        debug5 << "Cannot tell if ghost zones are present because "
               << "min_index & max_index are both zero!" << endl;
    }
    else if (firstCell < 0 || firstCell >= numCells ||
             lastCell  < 0 || lastCell  >= numCells ||
             firstCell > lastCell)
    {
        // bad min or max index
        debug5 << "Invalid min/max index for determining ghost zones:  "
               << "\n\tnumCells: " << numCells
               << "\n\tfirstRealZone: " << firstCell
               << "\n\tlastRealZone: " << lastCell << endl;
    }
    else if (firstCell != 0 || lastCell != numCells -1)
    {
        int i;
        vtkUnsignedCharArray *ghostZones = vtkUnsignedCharArray::New();
        ghostZones->SetName("avtGhostZones");
        ghostZones->SetNumberOfTuples(numCells);
        unsigned char *tmp = ghostZones->GetPointer(0);
        for (i = 0; i < firstCell; i++)
        {
            //
            //  ghostZones at the begining of the zone list
            //
            unsigned char val = 0;
            avtGhostData::AddGhostZoneType(val, 
                                   DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
            *tmp++ = val;
        }
        for (i = firstCell; i <= lastCell; i++)
        {
            //
            // real zones
            //
            *tmp++ = 0;
        }
        for (i = lastCell+1; i < numCells; i++)
        {
            //
            //  ghostZones at the end of the zone list
            //
            unsigned char val = 0;
            avtGhostData::AddGhostZoneType(val, 
                                   DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
            *tmp++ = val;
        }
        ugrid->GetCellData()->AddArray(ghostZones);
        ghostZones->Delete();
        ugrid->SetUpdateGhostLevel(0);
    }

    return ugrid;
}

// ****************************************************************************
// Method: SimV2_GetMesh_Point
//
// Purpose: 
//   Builds a VTK point mesh from the simulation mesh.
//
// Arguments:
//   pmesh : The simulation mesh.
//
// Returns:    A VTK dataset that represents the simulation mesh.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb  5 16:24:29 PST 2009
//
// Modifications:
//   
// ****************************************************************************

vtkDataSet *
SimV2_GetMesh_Point(VisIt_PointMesh *pmesh)
{
    if (!pmesh)
        return NULL;

    vtkUnstructuredGrid  *ugrid = vtkUnstructuredGrid::New();
    vtkPoints    *points  = vtkPoints::New();
    ugrid->SetPoints(points);
    points->Delete();

    //
    // Populate the coordinates.
    //
    int npts = pmesh->nnodes;
    points->SetNumberOfPoints(npts);
    float *pts = (float *) points->GetVoidPointer(0);
    ugrid->Allocate(npts);
    vtkIdType onevertex[1];
    if (pmesh->xcoords.dataType == VISIT_DATATYPE_FLOAT)
    {
        for (int i=0; i<npts; i++)
        {
            pts[i*3 + 0] = pmesh->xcoords.fArray[i];
            pts[i*3 + 1] = pmesh->ycoords.fArray[i];
            if (pmesh->ndims==3)
                pts[i*3 + 2] = pmesh->zcoords.fArray[i];
            else
                pts[i*3 + 2] = 0.f;

            onevertex[0] = i;
            ugrid->InsertNextCell(VTK_VERTEX, 1, onevertex);
        }
    }
    else if (pmesh->xcoords.dataType == VISIT_DATATYPE_DOUBLE)
    {
        for (int i=0; i<npts; i++)
        {
            pts[i*3 + 0] = pmesh->xcoords.dArray[i];
            pts[i*3 + 1] = pmesh->ycoords.dArray[i];
            if (pmesh->ndims==3)
                pts[i*3 + 2] = pmesh->zcoords.dArray[i];
            else
                pts[i*3 + 2] = 0.f;

            onevertex[0] = i;
            ugrid->InsertNextCell(VTK_VERTEX, 1, onevertex);
        }
    }
    else
    {
        EXCEPTION1(ImproperUseException,
                   "Coordinate arrays must be float or double.\n");
    }

    return ugrid;
}

// ****************************************************************************
// Method: SimV2_GetMesh_CSG
//
// Purpose: 
//    Builds a VTK csg mesh from the simulation mesh.
//
// Arguments:
//   csgm : The simulation mesh.
//
// Returns:    A VTK dataset that represents the simulation mesh.
//
// Note:       Adapted from the Silo reader.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb  5 16:24:23 PST 2009
//
// Modifications:
//   
// ****************************************************************************

vtkDataSet *
SimV2_GetMesh_CSG(VisIt_CSGMesh *csgm)
{
    if(csgm == NULL)
        return NULL;

    // Do some checks on the input.
    if(csgm->typeflags.dataType != VISIT_DATATYPE_INT)
    {
        EXCEPTION1(ImproperUseException, "CSG typeflags array must be int type");
    }
    if(csgm->coeffs.dataType != VISIT_DATATYPE_DOUBLE &&
       csgm->coeffs.dataType != VISIT_DATATYPE_FLOAT)
    {
        EXCEPTION1(ImproperUseException, "CSG coeffs array must be float or double type");
    }
    if(csgm->zones.leftids.dataType != VISIT_DATATYPE_INT)
    {
        EXCEPTION1(ImproperUseException, "CSG zones leftids array must be int type");
    }
    if(csgm->zones.rightids.dataType != VISIT_DATATYPE_INT)
    {
        EXCEPTION1(ImproperUseException, "CSG zones rightids array must be int type");
    }
    if(csgm->zones.typeflags.dataType != VISIT_DATATYPE_INT)
    {
        EXCEPTION1(ImproperUseException, "CSG zones typeflags array must be int type");
    }
    if(csgm->zones.zonelist.dataType != VISIT_DATATYPE_INT)
    {
        EXCEPTION1(ImproperUseException, "CSG zones zonelist array must be int type");
    }

    //
    // Create the VTK objects and connect them up.
    //
    vtkCSGGrid *csggrid = vtkCSGGrid::New(); 

    double minX = -10.0, minY = -10.0, minZ = -10.0;
    double maxX =  10.0, maxY =  10.0, maxZ =  10.0;
    // set bounds *before* anything else
    if (!((csgm->min_extents[0] == 0.0 && csgm->max_extents[0] == 0.0 &&
           csgm->min_extents[1] == 0.0 && csgm->max_extents[1] == 0.0 &&
           csgm->min_extents[2] == 0.0 && csgm->max_extents[2] == 0.0) ||
          (csgm->min_extents[0] == -DBL_MAX && csgm->max_extents[0] == DBL_MAX &&
           csgm->min_extents[1] == -DBL_MAX && csgm->max_extents[1] == DBL_MAX &&
           csgm->min_extents[2] == -DBL_MAX && csgm->max_extents[2] == DBL_MAX)))
    {
        minX = csgm->min_extents[0];
        maxX = csgm->max_extents[0];
        minY = csgm->min_extents[1];
        maxY = csgm->max_extents[1];
        minZ = csgm->min_extents[2];
        maxZ = csgm->max_extents[2];
    }
    csggrid->SetBounds(minX, maxX, minY, maxY, minZ, maxZ);

    if (csgm->coeffs.dataType == VISIT_DATATYPE_DOUBLE)
        csggrid->AddBoundaries(csgm->nbounds, csgm->typeflags.iArray, csgm->lcoeffs, (double*) csgm->coeffs.dArray);
    else
        csggrid->AddBoundaries(csgm->nbounds, csgm->typeflags.iArray, csgm->lcoeffs, (float*) csgm->coeffs.fArray);
    csggrid->AddRegions(csgm->zones.nregs, csgm->zones.leftids.iArray, csgm->zones.rightids.iArray,
                        csgm->zones.typeflags.iArray, 0, 0);
    csggrid->AddZones(csgm->zones.nzones, csgm->zones.zonelist.iArray);

    return csggrid;
}
