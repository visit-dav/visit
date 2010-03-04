#include <snprintf.h>

#include "VisItDataInterfaceRuntime.h"
#include "VisItDataInterfaceRuntimeP.h"

#include "simv2_PointMesh.h"
#include "simv2_VariableData.h"

struct VisIt_PointMesh : public VisIt_ObjectBase
{
    VisIt_PointMesh();
    virtual ~VisIt_PointMesh();
    void FreeCoordinates();

    int ndims;
    int coordMode;
    visit_handle xcoords;
    visit_handle ycoords;
    visit_handle zcoords;
    visit_handle coords;
};

VisIt_PointMesh::VisIt_PointMesh() : VisIt_ObjectBase(VISIT_POINT_MESH)
{
    ndims = 0;
    coordMode = VISIT_COORD_MODE_SEPARATE;
    xcoords = VISIT_INVALID_HANDLE;
    ycoords = VISIT_INVALID_HANDLE;
    zcoords = VISIT_INVALID_HANDLE;
    coords = VISIT_INVALID_HANDLE;
}

VisIt_PointMesh::~VisIt_PointMesh()
{
    FreeCoordinates();
}

void
VisIt_PointMesh::FreeCoordinates()
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

static VisIt_PointMesh *
GetObject(visit_handle h, const char *fname)
{
    char tmp[100];
    VisIt_PointMesh *obj = (VisIt_PointMesh *)VisItGetPointer(h);
    if(obj != NULL)
    {
        if(obj->type != VISIT_POINT_MESH)
        {
            SNPRINTF(tmp, 100, "%s: The provided handle does not point to "
                "a PointMesh object.", fname);
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
simv2_PointMesh_alloc(visit_handle *h)
{
    *h = VisItStorePointer(new VisIt_PointMesh);
    return (*h != VISIT_INVALID_HANDLE) ? VISIT_OKAY : VISIT_ERROR;
}

int
simv2_PointMesh_free(visit_handle h)
{
    int retval = VISIT_ERROR;
    VisIt_PointMesh *obj = GetObject(h, "simv2_PointMesh_free");
    if(obj != NULL)
    {
        delete obj;
        VisItFreePointer(h);
    }
}

static int
simv2_PointMesh_setCoords_helper(visit_handle h, visit_handle *cHandles, 
    int ndims, const char *fname)
{
    int retval = VISIT_ERROR;
    VisIt_PointMesh *obj = GetObject(h, fname);

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
simv2_PointMesh_setCoordsXY(visit_handle h, visit_handle x, visit_handle y)
{
    visit_handle cHandles[2];
    cHandles[0] = x;
    cHandles[1] = y;
    return simv2_PointMesh_setCoords_helper(h, cHandles, 2,
               "simv2_PointMesh_setCoordsXY");
}

int
simv2_PointMesh_setCoordsXYZ(visit_handle h, visit_handle x, visit_handle y, 
    visit_handle z)
{
    visit_handle cHandles[3];
    cHandles[0] = x;
    cHandles[1] = y;
    cHandles[2] = z;
    return simv2_PointMesh_setCoords_helper(h, cHandles, 3,
               "simv2_PointMesh_setCoordsXYZ");
}

int
simv2_PointMesh_setCoords(visit_handle h, visit_handle coords)
{
    int retval = VISIT_ERROR;
    VisIt_PointMesh *obj = GetObject(h, "simv2_PointMesh_setCoords");

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

/*******************************************************************************
 * C++ code callable from the SimV2 plugin and within the runtime
 ******************************************************************************/

int
simv2_PointMesh_getData(visit_handle h, int &ndims, int &coordMode,
    visit_handle &x, visit_handle &y, visit_handle &z, visit_handle &coords)
{
    int retval = VISIT_ERROR;
    VisIt_PointMesh *obj = GetObject(h, "simv2_PointMesh_getData");
    if(obj != NULL)
    {
        ndims = obj->ndims;
        coordMode = obj->coordMode;
        x = obj->xcoords;
        y = obj->ycoords;
        z = obj->zcoords;
        coords = obj->coords;

        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_PointMesh_check(visit_handle h)
{
    int retval = VISIT_ERROR;
    VisIt_PointMesh *obj = GetObject(h, "simv2_PointMesh_check");
    if(obj != NULL)
    {
        if(obj->ndims == 0)
        {
            VisItError("No coordinates were supplied for the PointMesh");
            return VISIT_ERROR;
        }
        retval = VISIT_OKAY;
    }
    return retval;
}
