#include "VisItDataInterfaceRuntime.h"
#include "VisItDataInterfaceRuntimeP.h"

#include "simv2_CurvilinearMesh.h"
#include "simv2_VariableData.h"

#include <snprintf.h>

struct VisIt_CurvilinearMesh : public VisIt_ObjectBase
{
    VisIt_CurvilinearMesh();
    virtual ~VisIt_CurvilinearMesh();
    void FreeCoordinates();
    void FreeGhostCells();
    void FreeGhostNodes();

    int ndims;
    int dims[3];
    int baseIndex[3];
    int minRealIndex[3];
    int maxRealIndex[3];

    int          coordMode;
    visit_handle xcoords;
    visit_handle ycoords;
    visit_handle zcoords;
    visit_handle coords;
    visit_handle ghostCells;
    visit_handle ghostNodes;
};

VisIt_CurvilinearMesh::VisIt_CurvilinearMesh() : VisIt_ObjectBase(VISIT_CURVILINEAR_MESH)
{
    ndims = 0;
    dims[0] = dims[1] = dims[2] = 0;
    baseIndex[0] = baseIndex[1] = baseIndex[2] = 0;
    minRealIndex[0] = minRealIndex[1] = minRealIndex[2] = 0;
    maxRealIndex[0] = maxRealIndex[1] = maxRealIndex[2] = -1;

    coordMode = VISIT_COORD_MODE_SEPARATE;
    xcoords = VISIT_INVALID_HANDLE;
    ycoords = VISIT_INVALID_HANDLE;
    zcoords = VISIT_INVALID_HANDLE;
    coords = VISIT_INVALID_HANDLE;
    ghostCells = VISIT_INVALID_HANDLE;
    ghostNodes = VISIT_INVALID_HANDLE;
}

VisIt_CurvilinearMesh::~VisIt_CurvilinearMesh()
{
    FreeCoordinates();
    FreeGhostCells();
    FreeGhostNodes();
}

void
VisIt_CurvilinearMesh::FreeCoordinates()
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
VisIt_CurvilinearMesh::FreeGhostCells()
{
    if(ghostCells != VISIT_INVALID_HANDLE)
    {
        simv2_VariableData_free(ghostCells);
        ghostCells = VISIT_INVALID_HANDLE;
    }
}

void
VisIt_CurvilinearMesh::FreeGhostNodes()
{
    if(ghostNodes != VISIT_INVALID_HANDLE)
    {
        simv2_VariableData_free(ghostNodes);
        ghostNodes = VISIT_INVALID_HANDLE;
    }
}

static VisIt_CurvilinearMesh *
GetObject(visit_handle h, const char *fname)
{
    char tmp[100];
    VisIt_CurvilinearMesh *obj = (VisIt_CurvilinearMesh *)VisItGetPointer(h);
    if(obj != NULL)
    {
        if(obj->objectType() != VISIT_CURVILINEAR_MESH)
        {
            SNPRINTF(tmp, 100, "%s: The provided handle does not point to "
                "a CurvilinearMesh object.", fname);
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
simv2_CurvilinearMesh_alloc(visit_handle *h)
{
    *h = VisItStorePointer(new VisIt_CurvilinearMesh);
    return (*h != VISIT_INVALID_HANDLE) ? VISIT_OKAY : VISIT_ERROR;
}

int
simv2_CurvilinearMesh_free(visit_handle h)
{
    VisIt_CurvilinearMesh *obj = GetObject(h, "simv2_CurvilinearMesh_free");
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
simv2_CurvilinearMesh_setCoords_helper(visit_handle h, visit_handle *cHandles, 
    int *dims, int ndims, const char *fname)
{
    int retval = VISIT_ERROR;

    // Get the coordinates
    char tmp[80];
    int owner[3], dataType[3], nComps[3], nTuples[3];
    void *data[3] = {0,0,0};
    int nnodes = 1;
    for(int i = 0; i < ndims; ++i)
    {
        nnodes *= dims[i];

        if(simv2_VariableData_getData(cHandles[i], owner[i], dataType[i], nComps[i], 
            nTuples[i], data[i]) == VISIT_ERROR)
        {
            return VISIT_ERROR;
        }

        // Error checking.
        if(dataType[i] != VISIT_DATATYPE_FLOAT &&
           dataType[i] != VISIT_DATATYPE_DOUBLE)
        {
            VisItError("Coordinates must contain float or double data");
            return VISIT_ERROR;
        }
    }

    for(int i = 1; i < ndims; ++i)
    {
        if(dataType[0] != dataType[i])
        {
            VisItError("Coordinates must be the same data type.");
            return VISIT_ERROR;
        }
    }
    for(int i = 0; i < ndims; ++i)
    {
        if(nTuples[i] != nnodes)
        {
            SNPRINTF(tmp, 80, "The provided %s coordinates contain %d nodes "
                     "instead of the required %d nodes.",
                     (i==0)?"X":((i==1)?"Y":"Z"), nTuples[i], nnodes);
            VisItError(tmp);
            return VISIT_ERROR;
        }
    }

    VisIt_CurvilinearMesh *obj = GetObject(h, fname);
    if(obj != NULL)
    {
        obj->ndims = ndims;
        obj->coordMode = VISIT_COORD_MODE_SEPARATE;
        obj->FreeCoordinates();
        obj->xcoords = cHandles[0];
        obj->ycoords = cHandles[1];
        obj->zcoords = (ndims == 3) ?  cHandles[2] : VISIT_INVALID_HANDLE;
        obj->coords = VISIT_INVALID_HANDLE;
        obj->dims[0] = dims[0];
        obj->dims[1] = dims[1];
        obj->dims[2] = (ndims == 3) ? dims[2] : 1;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_CurvilinearMesh_setCoordsXY(visit_handle h, int dims[2], visit_handle x, visit_handle y)
{
    visit_handle cHandles[2];
    cHandles[0] = x;
    cHandles[1] = y;
    return simv2_CurvilinearMesh_setCoords_helper(h, cHandles, dims, 2,
        "simv2_CurvilinearMesh_setCoordsXY");
}

int
simv2_CurvilinearMesh_setCoordsXYZ(visit_handle h, int dims[3], visit_handle x, visit_handle y, 
    visit_handle z)
{
    visit_handle cHandles[3];
    cHandles[0] = x;
    cHandles[1] = y;
    cHandles[2] = z;
    return simv2_CurvilinearMesh_setCoords_helper(h, cHandles, dims, 3,
        "simv2_CurvilinearMesh_setCoordsXYZ");
}

static int
simv2_CurvilinearMesh_setAllCoords_helper(visit_handle h, int ndims, int dims[3], 
    visit_handle coords, const char *fname)
{
    // Get the coordinates
    char tmp[100];
    int owner, dataType, nComps, nTuples;
    void *data = 0;
    if(simv2_VariableData_getData(coords, owner, dataType, nComps, nTuples, 
        data) == VISIT_ERROR)
    {
        return VISIT_ERROR;
    }

    // Error checking.
    if(nComps != ndims)
    {
        VisItError("Interleaved coordinates nComps must match the number of dimensions.");
        return VISIT_ERROR;
    }
    if(dataType != VISIT_DATATYPE_FLOAT &&
       dataType != VISIT_DATATYPE_DOUBLE)
    {
        VisItError("Coordinates must contain float or double data");
        return VISIT_ERROR;
    }
    int nnodes = 1;
    for(int i = 0; i < nComps; ++i)
        nnodes *= dims[i];
    if(nnodes != nTuples)
    {
        SNPRINTF(tmp, 100, "The product of the dimensions does not equal %d, "
            "the number of tuples in the coordinate array", nTuples);
        VisItError(tmp);
        return VISIT_ERROR;
    }

    VisIt_CurvilinearMesh *obj = GetObject(h, fname);
    int retval = VISIT_ERROR;
    if(obj != NULL)
    {
        obj->ndims = nComps;
        obj->coordMode = VISIT_COORD_MODE_INTERLEAVED;
        obj->FreeCoordinates();
        obj->coords = coords;
        obj->dims[0] = dims[0];
        obj->dims[1] = dims[1];
        obj->dims[2] = (ndims == 3) ? dims[2] : 1;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_CurvilinearMesh_setCoords2(visit_handle h, int dims[2], visit_handle coords)
{
    int tmp[3] = {0,0,1};
    tmp[0] = dims[0];
    tmp[1] = dims[1];
    return simv2_CurvilinearMesh_setAllCoords_helper(h, 2, tmp, coords,
        "simv2_CurvilinearMesh_setCoords2");
}

int
simv2_CurvilinearMesh_setCoords3(visit_handle h, int dims[3], visit_handle coords)
{
    return simv2_CurvilinearMesh_setAllCoords_helper(h, 3, dims, coords,
        "simv2_CurvilinearMesh_setCoords3");
}

int
simv2_CurvilinearMesh_setRealIndices(visit_handle h, int min[3], int max[3])
{
    int retval = VISIT_ERROR;
    VisIt_CurvilinearMesh *obj = GetObject(h, "simv2_CurvilinearMesh_setRealIndices");
    if(obj != NULL)
    {
        for(int i = 0; i < 3; ++i)
        {
            if(min[i] < 0)
            {
                VisItError("Min real index for a rectilinear mesh must be >= 0.");
                return VISIT_ERROR;
            }
            if(max[i] < 0)
            {
                VisItError("Max real index for a rectilinear mesh must be >= 0.");
                return VISIT_ERROR;
            }

            obj->minRealIndex[i] = min[i];
            obj->maxRealIndex[i] = max[i];
        }

        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_CurvilinearMesh_setBaseIndex(visit_handle h, int base_index[3])
{
    int retval = VISIT_ERROR;
    VisIt_CurvilinearMesh *obj = GetObject(h, "simv2_CurvilinearMesh_setBaseIndex");
    if(obj != NULL)
    {
        for(int i = 0; i < 3; ++i)
        {
            if(base_index[i] < 0)
            {
                VisItError("Base index for a rectilinear mesh must be >= 0.");
                return VISIT_ERROR;
            }
            obj->baseIndex[i] = base_index[i];
        }

        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_CurvilinearMesh_setGhostCells(visit_handle h, visit_handle gz)
{
    int retval = VISIT_ERROR;
    VisIt_CurvilinearMesh *obj = GetObject(h, "simv2_CurvilinearMesh_setGhostCells");
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
simv2_CurvilinearMesh_setGhostNodes(visit_handle h, visit_handle gn)
{
    int retval = VISIT_ERROR;
    VisIt_CurvilinearMesh *obj = GetObject(h, "simv2_CurvilinearMesh_setGhostNodes");
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
simv2_CurvilinearMesh_getCoords(visit_handle h, 
    int *ndims, int dims[3], int *coordMode, 
    visit_handle *x, visit_handle *y, visit_handle *z, visit_handle *c)
{
    int retval = VISIT_ERROR;
    VisIt_CurvilinearMesh *obj = GetObject(h, "simv2_CurvilinearMesh_getCoords");
    if(obj != NULL)
    {
        *ndims = obj->ndims;
        dims[0] = obj->dims[0];
        dims[1] = obj->dims[1];
        dims[2] = obj->dims[2];
        *coordMode = obj->coordMode;
        *x = obj->xcoords;
        *y = obj->ycoords;
        *z = obj->zcoords;
        *c = obj->coords;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_CurvilinearMesh_getRealIndices(visit_handle h, int min[3], int max[3])
{
    int retval = VISIT_ERROR;
    VisIt_CurvilinearMesh *obj = GetObject(h, "simv2_CurvilinearMesh_getRealIndices");
    if(obj != NULL)
    {
        for(int i = 0; i < 3; ++i)
        {
            min[i] = obj->minRealIndex[i];
            max[i] = (obj->maxRealIndex[i]==-1) ? (obj->dims[i]-1) :
                obj->maxRealIndex[i];
        }
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_CurvilinearMesh_getBaseIndex(visit_handle h, int base_index[3])
{
    int retval = VISIT_ERROR;
    VisIt_CurvilinearMesh *obj = GetObject(h, "simv2_CurvilinearMesh_getBaseIndex");
    if(obj != NULL)
    {
        for(int i = 0; i < 3; ++i)
            base_index[i] = obj->baseIndex[i];
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_CurvilinearMesh_getGhostCells(visit_handle h, visit_handle *gz)
{
    int retval = VISIT_ERROR;
    VisIt_CurvilinearMesh *obj = GetObject(h, "simv2_CurvilinearMesh_getGhostCells");
    if(obj != NULL)
    {
        *gz = obj->ghostCells;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_CurvilinearMesh_getGhostNodes(visit_handle h, visit_handle *gn)
{
    int retval = VISIT_ERROR;
    VisIt_CurvilinearMesh *obj = GetObject(h, "simv2_CurvilinearMesh_getGhostNodes");
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
simv2_CurvilinearMesh_check(visit_handle h)
{
    int retval = VISIT_ERROR;
    VisIt_CurvilinearMesh *obj = GetObject(h, "simv2_CurvilinearMesh_check");
    if(obj != NULL)
    {
        if(obj->ndims == 0)
        {
            VisItError("No coordinates were supplied for the CurvilinearMesh");
            return VISIT_ERROR;
        }

        if(obj->ghostCells != VISIT_INVALID_HANDLE)
        {
            // Get the ghost cell information
            int owner, dataType, nComps, nTuples = 0;
            void *data = 0;
            simv2_VariableData_getData(obj->ghostCells, owner, dataType, nComps, nTuples, data);

            // Get the number of cells
            int nCells = 1;
            for(int i = 0; i < obj->ndims; ++i)
                nCells *= (obj->dims[i]-1);

            if(nTuples != nCells)
            {
                 VisItError("The number of elements in the ghost cell array does "
                            "not match the number of mesh cells.");
                 return VISIT_ERROR;
            }
        }

        if(obj->ghostNodes != VISIT_INVALID_HANDLE)
        {
            // Get the ghost node information
            int owner, dataType, nComps, nTuples = 0;
            void *data = 0;
            simv2_VariableData_getData(obj->ghostNodes, owner, dataType, nComps, nTuples, data);

            // Get the number of nodes
            int nNodes = 1;
            for(int i = 0; i < obj->ndims; ++i)
                nNodes *= obj->dims[i];

            if(nTuples != nNodes)
            {
                 VisItError("The number of elements in the ghost node array does "
                            "not match the number of mesh nodes.");
                 return VISIT_ERROR;
            }
        }


        retval = VISIT_OKAY;
    }
    return retval;
}
