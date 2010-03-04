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
}

VisIt_UnstructuredMesh::~VisIt_UnstructuredMesh()
{
    FreeCoordinates();
    FreeConnectivity();
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

static VisIt_UnstructuredMesh *
GetObject(visit_handle h)
{
    VisIt_UnstructuredMesh *obj = (VisIt_UnstructuredMesh *)VisItGetPointer(h);
    if(obj != NULL)
    {
        if(obj->type != VISIT_UNSTRUCTURED_MESH)
        {
            VisItError("The provided handle does not point to a VariableData object.");
            obj = NULL;
        }
    }
    else
    {
        VisItError("An invalid handle was provided.");
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
    int retval = VISIT_ERROR;
    VisIt_UnstructuredMesh *obj = GetObject(h);
    if(obj != NULL)
    {
        delete obj;
        VisItFreePointer(h);
    }
}

static int
simv2_UnstructuredMesh_setCoords_helper(visit_handle h, visit_handle *cHandles, int ndims)
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

    VisIt_UnstructuredMesh *obj = GetObject(h);
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
    return simv2_UnstructuredMesh_setCoords_helper(h, cHandles, 2);
}

int
simv2_UnstructuredMesh_setCoordsXYZ(visit_handle h, visit_handle x, visit_handle y, 
    visit_handle z)
{
    visit_handle cHandles[3];
    cHandles[0] = x;
    cHandles[1] = y;
    cHandles[2] = z;
    return simv2_UnstructuredMesh_setCoords_helper(h, cHandles, 3);
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

    VisIt_UnstructuredMesh *obj = GetObject(h);
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

    VisIt_UnstructuredMesh *obj = GetObject(h);
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

    VisIt_UnstructuredMesh *obj = GetObject(h);
    int retval = VISIT_ERROR;
    if(obj != NULL)
    {
        obj->firstRealZone = minval;
        obj->lastRealZone = minval;
        retval = VISIT_OKAY;
    }
    return retval;
}


/*******************************************************************************
 * C++ code callable from the SimV2 plugin and within the runtime
 ******************************************************************************/

int
simv2_UnstructuredMesh_getData(visit_handle h,
    int &ndims, 
    int &coordMode,
    visit_handle &x, visit_handle &y, visit_handle &z, visit_handle &coords,
    int &nzones, 
    int &firstRealZone, int &lastRealZone,
    visit_handle &conn)
{
    int retval = VISIT_ERROR;
    VisIt_UnstructuredMesh *obj = GetObject(h);
    if(obj != NULL)
    {
        ndims = obj->ndims;
        coordMode = obj->coordMode;
        x = obj->xcoords;
        y = obj->ycoords;
        z = obj->zcoords;
        coords = obj->coords;

        firstRealZone = obj->firstRealZone;
        lastRealZone = (obj->lastRealZone == -1) ? (obj->nzones-1) : obj->lastRealZone;
        nzones = obj->nzones;
        conn = obj->connectivity;

        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_UnstructuredMesh_check(visit_handle h)
{
    VisIt_UnstructuredMesh *obj = GetObject(h);
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

        retval = VISIT_OKAY;
    }

    return retval;
}

