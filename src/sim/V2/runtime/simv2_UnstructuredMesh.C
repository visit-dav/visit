#include <snprintf.h>
#include "VisItDataInterfaceRuntime.h"
#include "VisItDataInterfaceRuntimeP.h"

#include "simv2_VariableData.h"
#include "simv2_UnstructuredMesh.h"

struct VisIt_UnstructuredMesh : public VisIt_ObjectBase
{
    VisIt_UnstructuredMesh();
    virtual ~VisIt_UnstructuredMesh();
    void FreeCoordinates();
    void FreeConnectivity();
    void FreeGhostCells();
    void FreeGhostNodes();

    int ndims;
    int coordMode;
    visit_handle xcoords;
    visit_handle ycoords;
    visit_handle zcoords;
    visit_handle coords;

    int nzones;
    int firstRealZone;
    int lastRealZone;
    visit_handle connectivity;

    visit_handle ghostCells;
    visit_handle ghostNodes;
};

VisIt_UnstructuredMesh::VisIt_UnstructuredMesh() : 
    VisIt_ObjectBase(VISIT_UNSTRUCTURED_MESH)
{
    ndims = 0;
    coordMode = VISIT_COORD_MODE_SEPARATE;
    xcoords = VISIT_INVALID_HANDLE;
    ycoords = VISIT_INVALID_HANDLE;
    zcoords = VISIT_INVALID_HANDLE;
    coords = VISIT_INVALID_HANDLE;

    nzones = 0;
    firstRealZone = 0;
    lastRealZone = -1;
    connectivity = VISIT_INVALID_HANDLE;

    ghostCells = VISIT_INVALID_HANDLE;
    ghostNodes = VISIT_INVALID_HANDLE;
}

VisIt_UnstructuredMesh::~VisIt_UnstructuredMesh()
{
    FreeCoordinates();
    FreeConnectivity();
    FreeGhostCells();
    FreeGhostNodes();
}

void
VisIt_UnstructuredMesh::FreeCoordinates()
{
    if(xcoords != VISIT_INVALID_HANDLE)
    {
        simv2_VariableData_free(xcoords);
        xcoords = VISIT_INVALID_HANDLE;
    }
    if(ycoords != VISIT_INVALID_HANDLE)
    {
        simv2_VariableData_free(ycoords);
        ycoords = VISIT_INVALID_HANDLE;
    }
    if(zcoords != VISIT_INVALID_HANDLE)
    {
        simv2_VariableData_free(zcoords);
        zcoords = VISIT_INVALID_HANDLE;
    }
    if(coords != VISIT_INVALID_HANDLE)
    {
        simv2_VariableData_free(coords);
        coords = VISIT_INVALID_HANDLE;
    }
}

void
VisIt_UnstructuredMesh::FreeConnectivity()
{
    if(connectivity != VISIT_INVALID_HANDLE)
    {
        simv2_VariableData_free(connectivity);
        connectivity = VISIT_INVALID_HANDLE;
    }
}

void
VisIt_UnstructuredMesh::FreeGhostCells()
{
    if(ghostCells != VISIT_INVALID_HANDLE)
    {
        simv2_VariableData_free(ghostCells);
        ghostCells = VISIT_INVALID_HANDLE;
    }
}

void
VisIt_UnstructuredMesh::FreeGhostNodes()
{
    if(ghostNodes != VISIT_INVALID_HANDLE)
    {
        simv2_VariableData_free(ghostNodes);
        ghostNodes = VISIT_INVALID_HANDLE;
    }
}

static VisIt_UnstructuredMesh *
GetObject(visit_handle h, const char *fname)
{
    char tmp[100];
    VisIt_UnstructuredMesh *obj = (VisIt_UnstructuredMesh *)VisItGetPointer(h);
    if(obj != NULL)
    {
        if(obj->objectType() != VISIT_UNSTRUCTURED_MESH)
        {
            SNPRINTF(tmp, 100, "%s: The provided handle does not point to an "
                "UnstructuredMesh object.", fname);
            VisItError(tmp);
            obj = NULL;
        }
    }
    else
    {
        SNPRINTF(tmp, 100, "%s: An invalid handle was provided.", fname);
        VisItError(tmp);
    }

    return obj;
}

/*******************************************************************************
 * Public functions, available to C 
 ******************************************************************************/

int
simv2_UnstructuredMesh_alloc(visit_handle *h)
{
    *h = VisItStorePointer(new VisIt_UnstructuredMesh);
    return (*h != VISIT_INVALID_HANDLE) ? VISIT_OKAY : VISIT_ERROR;
}

int
simv2_UnstructuredMesh_free(visit_handle h)
{
    VisIt_UnstructuredMesh *obj = GetObject(h, "simv2_UnstructuredMesh_free");
    int retval = VISIT_ERROR;
    if(obj != NULL)
    {
        delete obj;
        VisItFreePointer(h);
        retval = VISIT_OKAY;
    }
    return retval;
}

static int
simv2_UnstructuredMesh_setCoords_helper(const char *fname,
    visit_handle h, visit_handle *cHandles, int ndims)
{
    int retval = VISIT_ERROR;

    // Get the coordinates
    int owner[3], dataType[3], nComps[3], nTuples[3];
    void *data[3] = {0,0,0};
    for(int i = 0; i < ndims; ++i)
    {
        if(simv2_VariableData_getData(cHandles[i], owner[i], dataType[i], nComps[i], 
            nTuples[i], data[i]) == VISIT_ERROR)
        {
            return VISIT_ERROR;
        }

        // Error checking.
        if(nComps[i] != 1)
        {
            VisItError("Coordinates must have 1 component");
            return VISIT_ERROR;
        }
        if(dataType[i] != VISIT_DATATYPE_FLOAT &&
           dataType[i] != VISIT_DATATYPE_DOUBLE)
        {
            VisItError("Coordinates must contain float or double data");
            return VISIT_ERROR;
        }
    }

    for(int i = 1; i < ndims; ++i)
    {
        if(nTuples[0] != nTuples[i])
        {
            VisItError("Coordinates must contain the same number of tuples.");
            return VISIT_ERROR;
        }
        if(dataType[0] != dataType[i])
        {
            VisItError("Coordinates must be the same data type.");
            return VISIT_ERROR;
        }
    }

    VisIt_UnstructuredMesh *obj = GetObject(h, fname);
    if(obj != NULL)
    {
        obj->ndims = ndims;
        obj->coordMode = VISIT_COORD_MODE_SEPARATE;
        obj->FreeCoordinates();
        obj->xcoords = cHandles[0];
        obj->ycoords = cHandles[1];
        obj->zcoords = (ndims == 3) ?  cHandles[2] : VISIT_INVALID_HANDLE;
        obj->coords = VISIT_INVALID_HANDLE;

        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_UnstructuredMesh_setCoordsXY(visit_handle h, visit_handle x, visit_handle y)
{
    visit_handle cHandles[2];
    cHandles[0] = x;
    cHandles[1] = y;
    return simv2_UnstructuredMesh_setCoords_helper("simv2_UnstructuredMesh_setCoordsXY", h, cHandles, 2);
}

int
simv2_UnstructuredMesh_setCoordsXYZ(visit_handle h, visit_handle x, visit_handle y, 
    visit_handle z)
{
    visit_handle cHandles[3];
    cHandles[0] = x;
    cHandles[1] = y;
    cHandles[2] = z;
    return simv2_UnstructuredMesh_setCoords_helper("simv2_UnstructuredMesh_setCoordsXYZ", h, cHandles, 3);
}

int
simv2_UnstructuredMesh_setCoords(visit_handle h, visit_handle coords)
{
    int retval = VISIT_ERROR;

    // Get the coordinates
    int owner, dataType, nComps, nTuples;
    void *data = 0;
    if(simv2_VariableData_getData(coords, owner, dataType, nComps, nTuples, 
        data) == VISIT_ERROR)
    {
        return VISIT_ERROR;
    }

    // Error checking.
    if(nComps != 2 && nComps != 3)
    {
        VisItError("Interleaved coordinates must have 2 or 3 components");
        return VISIT_ERROR;
    }
    if(dataType != VISIT_DATATYPE_FLOAT &&
       dataType != VISIT_DATATYPE_DOUBLE)
    {
        VisItError("Coordinates must contain float or double data");
        return VISIT_ERROR;
    }

    VisIt_UnstructuredMesh *obj = GetObject(h, "simv2_UnstructuredMesh_setCoords");
    if(obj != NULL)
    {
        obj->ndims = nComps;
        obj->coordMode = VISIT_COORD_MODE_INTERLEAVED;
        obj->FreeCoordinates();
        obj->xcoords = VISIT_INVALID_HANDLE;
        obj->ycoords = VISIT_INVALID_HANDLE;
        obj->zcoords = VISIT_INVALID_HANDLE;
        obj->coords = coords;

        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_UnstructuredMesh_setConnectivity(visit_handle h, int nzones, visit_handle conn)
{
    // Get the connectivity
    int owner, dataType, nComps, nTuples;
    void *data = 0;
    if(simv2_VariableData_getData(conn, owner, dataType, nComps, nTuples, 
        data) == VISIT_ERROR)
    {
        return VISIT_ERROR;
    }

    if(nComps != 1)
    {
        VisItError("The connectivity array must have 1 component.");
        return VISIT_ERROR;
    }
    if(nTuples <= 0)
    {
        VisItError("The connectivity array is empty.");
        return VISIT_ERROR;
    }
    if(dataType != VISIT_DATATYPE_INT)
    {
        VisItError("The connectivity array must contain integers.");
        return VISIT_ERROR;
    }
    if(nzones <= 0)
    {
        VisItError("The number of zones must be greater than zero.");
        return VISIT_ERROR;
    }

    VisIt_UnstructuredMesh *obj = GetObject(h, "simv2_UnstructuredMesh_setConnectivity");
    int retval = VISIT_ERROR;
    if(obj != NULL)
    {
        obj->nzones = nzones;
        if(obj->lastRealZone == -1)
            obj->lastRealZone = nzones - 1;
        obj->FreeConnectivity();
        obj->connectivity = conn;

        retval = VISIT_OKAY;
    }
    return retval;
}

/*******************************************************************************
 * Modifications:
 *
 *   Cihan Altinay, Tue Aug 31 18:18:46 PDT 2010
 *   Fix bug with setting ghost cells.
 *
 ******************************************************************************/
int
simv2_UnstructuredMesh_setRealIndices(visit_handle h, int minval, int maxval)
{
    if(minval < 0)
    {
        VisItError("The first real zone must be >= 0.");
        return VISIT_ERROR;
    }
    if(maxval < 0)
    {
        VisItError("The last real zone must be >= 0.");
        return VISIT_ERROR;
    }
    if(maxval < minval)
    {
        VisItError("The last real zone must greater than the first real zone.");
        return VISIT_ERROR;
    }

    VisIt_UnstructuredMesh *obj = GetObject(h, "simv2_UnstructuredMesh_setRealIndices");
    int retval = VISIT_ERROR;
    if(obj != NULL)
    {
        obj->firstRealZone = minval;
        obj->lastRealZone = maxval;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_UnstructuredMesh_setGhostCells(visit_handle h, visit_handle gz)
{
    int retval = VISIT_ERROR;
    VisIt_UnstructuredMesh *obj = GetObject(h, "simv2_UnstructuredMesh_setGhostCells");
    if(obj != NULL)
    {
        // Get the ghost cell information
        int owner, dataType, nComps, nTuples;
        void *data = 0;
        if(simv2_VariableData_getData(gz, owner, dataType, nComps, nTuples, data) == VISIT_ERROR)
        {
            VisItError("Could not obtain ghost cell information.");
            return VISIT_ERROR;
        }

        if(nComps != 1)
        {
            VisItError("Ghost cell arrays must have 1 component.");
            return VISIT_ERROR;
        }

        if(dataType != VISIT_DATATYPE_CHAR && dataType != VISIT_DATATYPE_INT)
        {
            VisItError("Ghost cell arrays must contain either char or int elements.");
            return VISIT_ERROR;
        }

        obj->FreeGhostCells();
        obj->ghostCells = gz;

        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_UnstructuredMesh_setGhostNodes(visit_handle h, visit_handle gn)
{
    int retval = VISIT_ERROR;
    VisIt_UnstructuredMesh *obj = GetObject(h, "simv2_UnstructuredMesh_setGhostNodes");
    if(obj != NULL)
    {
        // Get the ghost node information
        int owner, dataType, nComps, nTuples;
        void *data = 0;
        if(simv2_VariableData_getData(gn, owner, dataType, nComps, nTuples, data) == VISIT_ERROR)
        {
            VisItError("Could not obtain ghost node information.");
            return VISIT_ERROR;
        }

        if(nComps != 1)
        {
            VisItError("Ghost node arrays must have 1 component.");
            return VISIT_ERROR;
        }

        if(dataType != VISIT_DATATYPE_CHAR && dataType != VISIT_DATATYPE_INT)
        {
            VisItError("Ghost node arrays must contain either char or int elements.");
            return VISIT_ERROR;
        }

        obj->FreeGhostNodes();
        obj->ghostNodes = gn;

        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_UnstructuredMesh_getCoords(visit_handle h,
    int *ndims, int *coordMode,
    visit_handle *x, visit_handle *y, visit_handle *z, visit_handle *coords)
{
    int retval = VISIT_ERROR;
    VisIt_UnstructuredMesh *obj = GetObject(h, "simv2_UnstructuredMesh_getCoords");
    if(obj != NULL)
    {
        *ndims = obj->ndims;
        *coordMode = obj->coordMode;
        *x = obj->xcoords;
        *y = obj->ycoords;
        *z = obj->zcoords;
        *coords = obj->coords;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_UnstructuredMesh_getConnectivity(visit_handle h, int *nzones, visit_handle *conn)
{
    int retval = VISIT_ERROR;
    VisIt_UnstructuredMesh *obj = GetObject(h, "simv2_UnstructuredMesh_getConnectivity");
    if(obj != NULL)
    {
        *nzones = obj->nzones;
        *conn = obj->connectivity;

        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_UnstructuredMesh_getRealIndices(visit_handle h, int *firstRealZone, int *lastRealZone)
{
    int retval = VISIT_ERROR;
    VisIt_UnstructuredMesh *obj = GetObject(h, "simv2_UnstructuredMesh_getRealIndices");
    if(obj != NULL)
    {
        *firstRealZone = obj->firstRealZone;
        *lastRealZone = (obj->lastRealZone == -1) ? (obj->nzones-1) : obj->lastRealZone;

        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_UnstructuredMesh_getGhostCells(visit_handle h, visit_handle *gz)
{
    int retval = VISIT_ERROR;
    VisIt_UnstructuredMesh *obj = GetObject(h, "simv2_UnstructuredMesh_getGhostCells");
    if(obj != NULL)
    {
        *gz = obj->ghostCells;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_UnstructuredMesh_getGhostNodes(visit_handle h, visit_handle *gn)
{
    int retval = VISIT_ERROR;
    VisIt_UnstructuredMesh *obj = GetObject(h, "simv2_UnstructuredMesh_getGhostNodes");
    if(obj != NULL)
    {
        *gn = obj->ghostNodes;
        retval = VISIT_OKAY;
    }
    return retval;
}

/*******************************************************************************
 * C++ code callable from the SimV2 plugin and within the runtime
 ******************************************************************************/

int
simv2_UnstructuredMesh_check(visit_handle h)
{
    VisIt_UnstructuredMesh *obj = GetObject(h, "simv2_UnstructuredMesh_check");
    int retval = VISIT_ERROR;
    if(obj != NULL)
    {
        if(obj->nzones <= 0 || obj->connectivity == VISIT_INVALID_HANDLE)
        {
            VisItError("The UnstructuredMesh object does not have connectivity.");
            return VISIT_ERROR;
        }

        if(obj->coordMode == VISIT_COORD_MODE_SEPARATE &&
           obj->xcoords == VISIT_INVALID_HANDLE)
        {
            VisItError("The UnstructuredMesh's coordinates were not provided.");
            return VISIT_ERROR;
        }

        if(obj->ghostCells != VISIT_INVALID_HANDLE)
        {
            // Get the ghost cell information
            int owner, dataType, nComps, nTuples = 0;
            void *data = 0;
            simv2_VariableData_getData(obj->ghostCells, owner, dataType, nComps, nTuples, data);

            if(nTuples != obj->nzones)
            {
                 VisItError("The number of elements in the ghost cell array does "
                            "not match the number of cells in the mesh.");
                 return VISIT_ERROR;
            }
        }

        if(obj->ghostNodes != VISIT_INVALID_HANDLE)
        {
            // Get the ghost node information
            int owner, dataType, nComps, nTuples = 0, nNodes = 0;
            void *data = 0;
            if(obj->coordMode == VISIT_COORD_MODE_SEPARATE)
            {
                simv2_VariableData_getData(obj->xcoords, owner, dataType, nComps, nNodes, data);
            }
            else
            {
                simv2_VariableData_getData(obj->coords, owner, dataType, nComps, nNodes, data);
            }

            simv2_VariableData_getData(obj->ghostNodes, owner, dataType, nComps, nTuples, data);

            if(nTuples != nNodes)
            {
                 VisItError("The number of elements in the ghost node array does "
                            "not match the number of nodes in the mesh.");
                 return VISIT_ERROR;
            }
        }

        retval = VISIT_OKAY;
    }

    return retval;
}

