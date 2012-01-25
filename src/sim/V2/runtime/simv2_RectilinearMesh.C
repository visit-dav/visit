#include <snprintf.h>
#include "VisItDataInterfaceRuntime.h"
#include "VisItDataInterfaceRuntimeP.h"

#include "simv2_RectilinearMesh.h"
#include "simv2_VariableData.h"

struct VisIt_RectilinearMesh : public VisIt_ObjectBase
{
    VisIt_RectilinearMesh();
    virtual ~VisIt_RectilinearMesh();
    void FreeCoordinates();
    void FreeGhostCells();
    void FreeGhostNodes();

    int ndims;
    visit_handle xcoords;
    visit_handle ycoords;
    visit_handle zcoords;

    int baseIndex[3];
    int minRealIndex[3];
    int maxRealIndex[3];

    visit_handle ghostCells;
    visit_handle ghostNodes;
};

VisIt_RectilinearMesh::VisIt_RectilinearMesh() : VisIt_ObjectBase(VISIT_RECTILINEAR_MESH)
{
    ndims = 0;
    xcoords = VISIT_INVALID_HANDLE;
    ycoords = VISIT_INVALID_HANDLE;
    zcoords = VISIT_INVALID_HANDLE;
    baseIndex[0] = baseIndex[1] = baseIndex[2] = 0;
    minRealIndex[0] = minRealIndex[1] = minRealIndex[2] = 0;
    maxRealIndex[0] = maxRealIndex[1] = maxRealIndex[2] = -1;
    ghostCells = VISIT_INVALID_HANDLE;
    ghostNodes = VISIT_INVALID_HANDLE;
}

VisIt_RectilinearMesh::~VisIt_RectilinearMesh()
{
    FreeCoordinates();
    FreeGhostCells();
    FreeGhostNodes();
}

void
VisIt_RectilinearMesh::FreeCoordinates()
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
}

void
VisIt_RectilinearMesh::FreeGhostCells()
{
    if(ghostCells != VISIT_INVALID_HANDLE)
    {
        simv2_VariableData_free(ghostCells);
        ghostCells = VISIT_INVALID_HANDLE;
    }
}

void
VisIt_RectilinearMesh::FreeGhostNodes()
{
    if(ghostNodes != VISIT_INVALID_HANDLE)
    {
        simv2_VariableData_free(ghostNodes);
        ghostNodes = VISIT_INVALID_HANDLE;
    }
}

static VisIt_RectilinearMesh *
GetObject(visit_handle h, const char *fname)
{
    char tmp[100];
    VisIt_RectilinearMesh *obj = (VisIt_RectilinearMesh *)VisItGetPointer(h);
    if(obj != NULL)
    {
        if(obj->objectType() != VISIT_RECTILINEAR_MESH)
        {
            SNPRINTF(tmp, 100, "%s: The provided handle does not point to a "
                "RectilinearMesh object.", fname);
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
simv2_RectilinearMesh_alloc(visit_handle *h)
{
    *h = VisItStorePointer(new VisIt_RectilinearMesh);
    return (*h != VISIT_INVALID_HANDLE) ? VISIT_OKAY : VISIT_ERROR;
}

int
simv2_RectilinearMesh_free(visit_handle h)
{
    VisIt_RectilinearMesh *obj = GetObject(h, "simv2_RectilinearMesh_free");
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
simv2_RectilinearMesh_setCoords_helper(visit_handle h, visit_handle *cHandles, 
    int ndims, const char *fname)
{
    int retval = VISIT_ERROR;
    VisIt_RectilinearMesh *obj = GetObject(h, fname);

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
        if(dataType[0] != dataType[i])
        {
            VisItError("Coordinates must be the same data type.");
            return VISIT_ERROR;
        }
    }

    if(obj != NULL)
    {
        obj->ndims = ndims;
        obj->FreeCoordinates();
        obj->xcoords = cHandles[0];
        obj->ycoords = cHandles[1];
        obj->zcoords = (ndims == 3) ?  cHandles[2] : VISIT_INVALID_HANDLE;

        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_RectilinearMesh_setCoordsXY(visit_handle h, visit_handle x, visit_handle y)
{
    visit_handle cHandles[2];
    cHandles[0] = x;
    cHandles[1] = y;
    return simv2_RectilinearMesh_setCoords_helper(h, cHandles, 2, 
               "simv2_RectilinearMesh_setCoordsXY");
}

int
simv2_RectilinearMesh_setCoordsXYZ(visit_handle h, visit_handle x, visit_handle y, 
    visit_handle z)
{
    visit_handle cHandles[3];
    cHandles[0] = x;
    cHandles[1] = y;
    cHandles[2] = z;
    return simv2_RectilinearMesh_setCoords_helper(h, cHandles, 3, 
               "simv2_RectilinearMesh_setCoordsXYZ");
}

int
simv2_RectilinearMesh_setRealIndices(visit_handle h, int min[3], int max[3])
{
    int retval = VISIT_ERROR;
    VisIt_RectilinearMesh *obj = GetObject(h, "simv2_RectilinearMesh_setRealIndices");
    if(obj != NULL)
    {
        if(obj->ndims < 1)
        {
            VisItError("setRealIndices called before the rectilinear mesh has coordinates.");
            return VISIT_ERROR;
        }

        for(int i = 0; i < 3; ++i)
        {
            if(i < obj->ndims)
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
                if(max[i] < min[i])
                {
                    VisItError("Min real index for a rectilinear mesh must be less "
                        "than max real index.");
                    return VISIT_ERROR;
                }

                obj->minRealIndex[i] = min[i];
                obj->maxRealIndex[i] = max[i];
            }
            else
            {
                obj->minRealIndex[i] = 0;
                obj->maxRealIndex[i] = -1;
            }
        }

        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_RectilinearMesh_setBaseIndex(visit_handle h, int base_index[3])
{
    int retval = VISIT_ERROR;
    VisIt_RectilinearMesh *obj = GetObject(h, "simv2_RectilinearMesh_setBaseIndex");
    if(obj != NULL)
    {
        if(obj->ndims < 1)
        {
            VisItError("setBaseIndex called before the rectilinear mesh has coordinates.");
            return VISIT_ERROR;
        }

        for(int i = 0; i < 3; ++i)
        {
            if(i < obj->ndims)
            {
                if(base_index[i] < 0)
                {
                    VisItError("Base index for a rectilinear mesh must be >= 0.");
                    return VISIT_ERROR;
                }
                obj->baseIndex[i] = base_index[i];
            }
            else
                obj->baseIndex[i] = 0;
        }

        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_RectilinearMesh_setGhostCells(visit_handle h, visit_handle gz)
{
    int retval = VISIT_ERROR;
    VisIt_RectilinearMesh *obj = GetObject(h, "simv2_RectilinearMesh_setGhostCells");
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
simv2_RectilinearMesh_setGhostNodes(visit_handle h, visit_handle gn)
{
    int retval = VISIT_ERROR;
    VisIt_RectilinearMesh *obj = GetObject(h, "simv2_RectilinearMesh_setGhostNodes");
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
simv2_RectilinearMesh_getCoords(visit_handle h,
    int *ndims,
    visit_handle *x, visit_handle *y, visit_handle *z)
{
    int retval = VISIT_ERROR;
    VisIt_RectilinearMesh *obj = GetObject(h, "simv2_RectilinearMesh_getCoords");
    if(obj != NULL)
    {
        *ndims = obj->ndims;
        *x = obj->xcoords;
        *y = obj->ycoords;
        *z = obj->zcoords;

        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_RectilinearMesh_getRealIndices(visit_handle h, int min[3], int max[3])
{
    int retval = VISIT_ERROR;
    VisIt_RectilinearMesh *obj = GetObject(h, "simv2_RectilinearMesh_getRealIndices");
    if(obj != NULL)
    {
        visit_handle cHandles[3];
        cHandles[0] = obj->xcoords;
        cHandles[1] = obj->ycoords;
        cHandles[2] = obj->zcoords;
        for(int i = 0; i < 3; ++i)
        {
            // Query the dimensions of the data arrays that make up the coordinates
            // so we can set the maxRealIndex if it has not been set.
            int owner, dataType, nComps, nTuples=1;
            void *data = NULL;
            if(i < obj->ndims)
            {
                simv2_VariableData_getData(cHandles[i], owner, dataType, nComps, 
                    nTuples, data);
            }
            min[i] = obj->minRealIndex[i];
            max[i] = (obj->maxRealIndex[i] == -1) ? (nTuples-1) : obj->maxRealIndex[i];
        }
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_RectilinearMesh_getBaseIndex(visit_handle h, int base_index[3])
{
    int retval = VISIT_ERROR;
    VisIt_RectilinearMesh *obj = GetObject(h, "simv2_RectilinearMesh_getBaseIndex");
    if(obj != NULL)
    {
        for(int i = 0; i < 3; ++i)
            base_index[i] = obj->baseIndex[i];
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_RectilinearMesh_getGhostCells(visit_handle h, visit_handle *gz)
{
    int retval = VISIT_ERROR;
    VisIt_RectilinearMesh *obj = GetObject(h, "simv2_RectilinearMesh_getGhostCells");
    if(obj != NULL)
    {
        *gz = obj->ghostCells;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_RectilinearMesh_getGhostNodes(visit_handle h, visit_handle *gn)
{
    int retval = VISIT_ERROR;
    VisIt_RectilinearMesh *obj = GetObject(h, "simv2_RectilinearMesh_getGhostNodes");
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
simv2_RectilinearMesh_check(visit_handle h)
{
    int retval = VISIT_ERROR;
    VisIt_RectilinearMesh *obj = GetObject(h, "simv2_RectilinearMesh_check");
    if(obj != NULL)
    {
        if(obj->ndims == 0)
        {
            VisItError("No coordinates were supplied for the RectilinearMesh");
            return VISIT_ERROR;
        }

        if(obj->ghostCells != VISIT_INVALID_HANDLE)
        {
            // Get the ghost cell information
            int owner, dataType, nComps, nTuples = 0;
            void *data = 0;
            simv2_VariableData_getData(obj->ghostCells, owner, dataType, nComps, nTuples, data);

            // Determine the number of cells in the mesh.
            int nCells = 1;
            for(int i = 0; i < obj->ndims; ++i)
            {
                int nCoordTuples = 0;
                if(i == 0)
                    simv2_VariableData_getData(obj->xcoords, owner, dataType, nComps, nCoordTuples, data);
                else if(i == 1)
                    simv2_VariableData_getData(obj->ycoords, owner, dataType, nComps, nCoordTuples, data);
                else
                    simv2_VariableData_getData(obj->zcoords, owner, dataType, nComps, nCoordTuples, data);
                nCells *= (nCoordTuples-1);
            }

            if(nCells != nTuples)
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

            // Determine the number of nodes in the mesh.
            int nNodes = 1;
            for(int i = 0; i < obj->ndims; ++i)
            {
                int nCoordTuples = 0;
                if(i == 0)
                    simv2_VariableData_getData(obj->xcoords, owner, dataType, nComps, nCoordTuples, data);
                else if(i == 1)
                    simv2_VariableData_getData(obj->ycoords, owner, dataType, nComps, nCoordTuples, data);
                else
                    simv2_VariableData_getData(obj->zcoords, owner, dataType, nComps, nCoordTuples, data);
                nNodes *= nCoordTuples;
            }

            if(nNodes != nTuples)
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
