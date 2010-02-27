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
};

VisIt_CurvilinearMesh::VisIt_CurvilinearMesh() : VisIt_ObjectBase(VISIT_CURVILINEAR_MESH)
{
    ndims = 0;
    dims[0] = dims[1] = dims[2];
    baseIndex[0] = baseIndex[1] = baseIndex[2] = 0;
    minRealIndex[0] = minRealIndex[1] = minRealIndex[2] = 0;
    maxRealIndex[0] = maxRealIndex[1] = maxRealIndex[2] = 0;

    coordMode = VISIT_COORD_MODE_SEPARATE;
    xcoords = VISIT_INVALID_HANDLE;
    ycoords = VISIT_INVALID_HANDLE;
    zcoords = VISIT_INVALID_HANDLE;
    coords = VISIT_INVALID_HANDLE;
}

VisIt_CurvilinearMesh::~VisIt_CurvilinearMesh()
{
    FreeCoordinates();
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

static VisIt_CurvilinearMesh *
GetObject(visit_handle h, const char *fname)
{
    char tmp[100];
    VisIt_CurvilinearMesh *obj = (VisIt_CurvilinearMesh *)VisItGetPointer(h);
    if(obj != NULL)
    {
        if(obj->type != VISIT_CURVILINEAR_MESH)
        {
            SNPRINTF(tmp, 100, "%s: The provided handle does not point to "
                "a CurvilinearMesh object.", fname);
            VisItError(tmp);
            obj = NULL;
        }
    }
    else
    {
        SNPRINTF(tmp, 100, "An invalid handle was provided.", fname);
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
    int retval = VISIT_ERROR;
    VisIt_CurvilinearMesh *obj = GetObject(h, "simv2_CurvilinearMesh_free");
    if(obj != NULL)
    {
        delete obj;
        VisItFreePointer(h);
    }
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
        if(nComps[i] != ndims)
        {
            SNPRINTF(tmp, 80, "Coordinates must have %d components", ndims);
            VisItError(tmp);
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
simv2_CurvilinearMesh_setAllCoords_helper(visit_handle h, int dims[3], 
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
    if(nComps != 2 || nComps != 3)
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
    return simv2_CurvilinearMesh_setAllCoords_helper(h, tmp, coords,
        "simv2_CurvilinearMesh_setCoords2");
}

int
simv2_CurvilinearMesh_setCoords3(visit_handle h, int dims[3], visit_handle coords)
{
    return simv2_CurvilinearMesh_setAllCoords_helper(h, dims, coords,
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

/*******************************************************************************
 * C++ code callable from the SimV2 plugin and within the runtime
 ******************************************************************************/

int
simv2_CurvilinearMesh_getData(visit_handle h, 
    int &ndims, int dims[3], int min[3], int max[3], int base_index[3],
    int &coordMode, visit_handle &x, visit_handle &y, visit_handle &z,
    visit_handle &c)
{
    int retval = VISIT_ERROR;
    VisIt_CurvilinearMesh *obj = GetObject(h, "simv2_CurvilinearMesh_getData");
    if(obj != NULL)
    {
        ndims = obj->ndims;
        coordMode = obj->coordMode;
        x = obj->xcoords;
        y = obj->ycoords;
        z = obj->zcoords;
        c = obj->coords;
        for(int i = 0; i < 3; ++i)
        {
            dims[i] = obj->dims[i];
            min[i] = obj->minRealIndex[i];
            max[i] = obj->maxRealIndex[i];
            base_index[i] = obj->baseIndex[i];
        }
        retval = VISIT_OKAY;
    }
    return retval;
}

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
        retval = VISIT_OKAY;
    }
    return retval;
}
