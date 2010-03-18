#include "VisItDataInterfaceRuntime.h"
#include "VisItDataInterfaceRuntimeP.h"

#include "simv2_VariableData.h"
#include "simv2_CSGMesh.h"
#include <float.h>

struct VisIt_CSGMesh : public VisIt_ObjectBase
{
    VisIt_CSGMesh();
    virtual ~VisIt_CSGMesh();
    void FreeRegions();
    void FreeZonelist();
    void FreeBoundaryTypes();
    void FreeBoundaryCoeffs();

    // Regions
    visit_handle typeflags;
    visit_handle leftids;
    visit_handle rightids;

    // Zonelist
    visit_handle zonelist;

    // Boundary types
    visit_handle boundaryTypes;

    // Boundary coeffs
    visit_handle boundaryCoeffs;

    // Extents
    double       min_extents[3];
    double       max_extents[3];
};

VisIt_CSGMesh::VisIt_CSGMesh() : VisIt_ObjectBase(VISIT_CSG_MESH)
{
    typeflags = VISIT_INVALID_HANDLE;
    leftids = VISIT_INVALID_HANDLE;
    rightids = VISIT_INVALID_HANDLE;

    zonelist = VISIT_INVALID_HANDLE;
    boundaryTypes = VISIT_INVALID_HANDLE;
    boundaryCoeffs = VISIT_INVALID_HANDLE;

    min_extents[0] = min_extents[1] = min_extents[2] = DBL_MAX;
    max_extents[0] = max_extents[1] = max_extents[2] = -DBL_MAX;
}

VisIt_CSGMesh::~VisIt_CSGMesh()
{
    FreeRegions();
    FreeZonelist();
    FreeBoundaryTypes();
    FreeBoundaryCoeffs();
}

void
VisIt_CSGMesh::FreeRegions()
{
    if(typeflags != VISIT_INVALID_HANDLE)
    {
        simv2_VariableData_free(typeflags);
        typeflags = VISIT_INVALID_HANDLE;
    }
    if(leftids != VISIT_INVALID_HANDLE)
    {
        simv2_VariableData_free(leftids);
        leftids = VISIT_INVALID_HANDLE;
    }
    if(rightids != VISIT_INVALID_HANDLE)
    {
        simv2_VariableData_free(rightids);
        rightids = VISIT_INVALID_HANDLE;
    }
}

void
VisIt_CSGMesh::FreeZonelist()
{
    if(zonelist != VISIT_INVALID_HANDLE)
    {
        simv2_VariableData_free(zonelist);
        zonelist = VISIT_INVALID_HANDLE;
    }
}

void
VisIt_CSGMesh::FreeBoundaryTypes()
{
    if(boundaryTypes != VISIT_INVALID_HANDLE)
    {
        simv2_VariableData_free(boundaryTypes);
        boundaryTypes = VISIT_INVALID_HANDLE;
    }
}

void
VisIt_CSGMesh::FreeBoundaryCoeffs()
{
    if(boundaryCoeffs != VISIT_INVALID_HANDLE)
    {
        simv2_VariableData_free(boundaryCoeffs);
        boundaryCoeffs = VISIT_INVALID_HANDLE;
    }
}


static VisIt_CSGMesh *
GetObject(visit_handle h)
{
    VisIt_CSGMesh *obj = (VisIt_CSGMesh *)VisItGetPointer(h);
    if(obj != NULL)
    {
        if(obj->objectType() != VISIT_CSG_MESH)
        {
            VisItError("The provided handle does not point to a CSGMesh object.");
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
simv2_CSGMesh_alloc(visit_handle *h)
{
    *h = VisItStorePointer(new VisIt_CSGMesh);
    return (*h != VISIT_INVALID_HANDLE) ? VISIT_OKAY : VISIT_ERROR;
}

int
simv2_CSGMesh_free(visit_handle h)
{
    VisIt_CSGMesh *obj = GetObject(h);
    int retval = VISIT_ERROR;
    if(obj != NULL)
    {
        delete obj;
        VisItFreePointer(h);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_CSGMesh_setRegions(visit_handle h, visit_handle typeflags, 
    visit_handle leftids, visit_handle rightids)
{
    int retval = VISIT_ERROR;
    visit_handle cHandles[3];
    cHandles[0] = typeflags;
    cHandles[1] = leftids;
    cHandles[2] = rightids;

    // Get the coordinates
    int owner[3], dataType[3], nComps[3], nTuples[3];
    void *data[3] = {0,0,0};
    for(int i = 0; i < 3; ++i)
    {
        if(simv2_VariableData_getData(cHandles[i], owner[i], dataType[i], nComps[i], 
            nTuples[i], data[i]) == VISIT_ERROR)
        {
            return VISIT_ERROR;
        }

        // Error checking.
        if(nComps[i] != 1)
        {
            VisItError("Region arrays must have 1 component");
            return VISIT_ERROR;
        }
        if(nTuples[i] <= 0)
        {
            VisItError("Region arrays must not be empty");
            return VISIT_ERROR;
        }
        if(dataType[i] != VISIT_DATATYPE_INT)
        {
            VisItError("Region arrays must contain integer data");
            return VISIT_ERROR;
        }
    }

    for(int i = 1; i < 3; ++i)
    {
        if(nTuples[0] != nTuples[i])
        {
            VisItError("Region arrays must contain the same number of tuples.");
            return VISIT_ERROR;
        }
    }

    VisIt_CSGMesh *obj = GetObject(h);
    if(obj != NULL)
    {
        obj->FreeRegions();
        obj->typeflags = typeflags;
        obj->leftids = leftids;
        obj->rightids = rightids;

        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_CSGMesh_setZonelist(visit_handle h, visit_handle zonelist)
{
    int retval = VISIT_ERROR;
    VisIt_CSGMesh *obj = GetObject(h);
    if(obj != NULL)
    {
        int owner, dataType, nComps, nTuples;
        void *data = NULL;
        if(simv2_VariableData_getData(zonelist, owner, dataType, nComps, 
            nTuples, data) == VISIT_ERROR)
        {
            return VISIT_ERROR;
        }
        // Error checking.
        if(nComps != 1)
        {
            VisItError("Zonelist array must have 1 component");
            return VISIT_ERROR;
        }
        if(nTuples <= 0)
        {
            VisItError("Zonelist array must not be empty");
            return VISIT_ERROR;
        }
        if(dataType != VISIT_DATATYPE_INT)
        {
            VisItError("Zonelist array must contain integer data");
            return VISIT_ERROR;
        }

        obj->FreeZonelist();
        obj->zonelist = zonelist;

        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_CSGMesh_setBoundaryTypes(visit_handle h, visit_handle boundaryTypes)
{
    int retval = VISIT_ERROR;
    VisIt_CSGMesh *obj = GetObject(h);
    if(obj != NULL)
    {
        int owner, dataType, nComps, nTuples;
        void *data = NULL;
        if(simv2_VariableData_getData(boundaryTypes, owner, dataType, nComps, 
            nTuples, data) == VISIT_ERROR)
        {
            return VISIT_ERROR;
        }
        // Error checking.
        if(nComps != 1)
        {
            VisItError("CSGMesh's boundary types array must have 1 component");
            return VISIT_ERROR;
        }
        if(nTuples <= 0)
        {
            VisItError("CSGMesh's boundary types array must not be empty");
            return VISIT_ERROR;
        }
        if(dataType != VISIT_DATATYPE_INT)
        {
            VisItError("CSGMesh's boundary types array must contain integer data");
            return VISIT_ERROR;
        }

        obj->FreeBoundaryTypes();
        obj->boundaryTypes = boundaryTypes;

        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_CSGMesh_setBoundaryCoeffs(visit_handle h, visit_handle boundaryCoeffs)
{
    int retval = VISIT_ERROR;
    VisIt_CSGMesh *obj = GetObject(h);
    if(obj != NULL)
    {
        int owner, dataType, nComps, nTuples;
        void *data = NULL;
        if(simv2_VariableData_getData(boundaryCoeffs, owner, dataType, nComps, 
            nTuples, data) == VISIT_ERROR)
        {
            return VISIT_ERROR;
        }
        // Error checking.
        if(nComps != 1)
        {
            VisItError("CSGMesh's boundary Coeffs array must have 1 component");
            return VISIT_ERROR;
        }
        if(nTuples <= 0)
        {
            VisItError("CSGMesh's boundary Coeffs array must not be empty");
            return VISIT_ERROR;
        }
        if(dataType != VISIT_DATATYPE_DOUBLE &&
           dataType != VISIT_DATATYPE_FLOAT)
        {
            VisItError("CSGMesh's boundary Coeffs array must contain float or double data");
            return VISIT_ERROR;
        }

        obj->FreeBoundaryCoeffs();
        obj->boundaryCoeffs = boundaryCoeffs;

        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_CSGMesh_setExtents(visit_handle h, double min[3], double max[3])
{
    int retval = VISIT_ERROR;
    VisIt_CSGMesh *obj = GetObject(h);
    if(obj != NULL)
    {
        for(int i = 0; i < 3; ++i)
        {
            if(min[i] >= max[i])
            { 
                VisItError("CSGMesh's min extents must be smaller than the max extents");
                return VISIT_ERROR;
            }
        }

        for(int i = 0; i < 3; ++i)
        {
            obj->min_extents[i] = min[i];
            obj->max_extents[i] = max[i];
        }

        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_CSGMesh_getRegions(visit_handle h, 
    visit_handle *typeflags, visit_handle *left, visit_handle *right)
{
    int retval = VISIT_ERROR;
    VisIt_CSGMesh *obj = GetObject(h);
    if(obj != NULL)
    {
        *typeflags = obj->typeflags;
        *left = obj->leftids;
        *right = obj->rightids;

        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_CSGMesh_getZonelist(visit_handle h, visit_handle *zl)
{
    int retval = VISIT_ERROR;
    VisIt_CSGMesh *obj = GetObject(h);
    if(obj != NULL)
    {
        *zl = obj->zonelist;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_CSGMesh_getBoundaryTypes(visit_handle h, visit_handle *bndtypes)
{
    int retval = VISIT_ERROR;
    VisIt_CSGMesh *obj = GetObject(h);
    if(obj != NULL)
    {
        *bndtypes = obj->boundaryTypes;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_CSGMesh_getBoundaryCoeffs(visit_handle h, visit_handle *bndcoeff)
{
    int retval = VISIT_ERROR;
    VisIt_CSGMesh *obj = GetObject(h);
    if(obj != NULL)
    {
        *bndcoeff = obj->boundaryCoeffs;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_CSGMesh_getExtents(visit_handle h, double min[3], double max[3])
{
    int retval = VISIT_ERROR;
    VisIt_CSGMesh *obj = GetObject(h);
    if(obj != NULL)
    {
        for(int i = 0; i < 3; ++i)
        {
            min[i] = obj->min_extents[i];
            max[i] = obj->max_extents[i];
        }
        retval = VISIT_OKAY;
    }
    return retval;
}

/*******************************************************************************
 * C++ code callable from the SimV2 plugin and within the runtime
 ******************************************************************************/

int
simv2_CSGMesh_check(visit_handle h)
{
    VisIt_CSGMesh *obj = GetObject(h);
    int retval = VISIT_ERROR;
    if(obj != NULL)
    {
        if(obj->typeflags == VISIT_INVALID_HANDLE)
        {
            VisItError("The CSGMesh object does not have regions.");
            return VISIT_ERROR;
        }
        if(obj->zonelist == VISIT_INVALID_HANDLE)
        {
            VisItError("The CSGMesh object does not a zone list.");
            return VISIT_ERROR;
        }

        if(obj->boundaryTypes == VISIT_INVALID_HANDLE)
        {
            VisItError("The CSGMesh object does not have boundary types.");
            return VISIT_ERROR;
        }

        if(obj->boundaryCoeffs == VISIT_INVALID_HANDLE)
        {
            VisItError("The CSGMesh object does not have boundary coefficients.");
            return VISIT_ERROR;
        }

        for(int i = 0; i < 3; ++i)
        {
            if(obj->min_extents[i] == DBL_MAX || obj->max_extents[i] == -DBL_MAX)
            {
                VisItError("The CSGMesh object does not have valid extents.");
                return VISIT_ERROR;
            }
        }

        retval = VISIT_OKAY;
    }

    return retval;
}

