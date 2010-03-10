#include <snprintf.h>

#include "VisItDataInterfaceRuntime.h"
#include "VisItDataInterfaceRuntimeP.h"

#include "simv2_CurveData.h"
#include "simv2_VariableData.h"

struct VisIt_CurveData : public VisIt_ObjectBase
{
    VisIt_CurveData();
    virtual ~VisIt_CurveData();
    void FreeCoordinates();

    visit_handle xcoords;
    visit_handle ycoords;
};

VisIt_CurveData::VisIt_CurveData() : VisIt_ObjectBase(VISIT_CURVE_DATA)
{
    xcoords = VISIT_INVALID_HANDLE;
    ycoords = VISIT_INVALID_HANDLE;
}

VisIt_CurveData::~VisIt_CurveData()
{
    FreeCoordinates();
}

void
VisIt_CurveData::FreeCoordinates()
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
}

static VisIt_CurveData *
GetObject(visit_handle h, const char *fname)
{
    char tmp[100];
    VisIt_CurveData *obj = (VisIt_CurveData *)VisItGetPointer(h);
    if(obj != NULL)
    {
        if(obj->objectType() != VISIT_CURVE_DATA)
        {
            SNPRINTF(tmp, 100, "%s: The provided handle does not point to "
                "a CurveData object.", fname);
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
simv2_CurveData_alloc(visit_handle *h)
{
    *h = VisItStorePointer(new VisIt_CurveData);
    return (*h != VISIT_INVALID_HANDLE) ? VISIT_OKAY : VISIT_ERROR;
}

int
simv2_CurveData_free(visit_handle h)
{
    VisIt_CurveData *obj = GetObject(h, "simv2_CurveData_free");
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
simv2_CurveData_setCoords_helper(visit_handle h, visit_handle *cHandles, 
    int ndims, const char *fname)
{
    int retval = VISIT_ERROR;
    VisIt_CurveData *obj = GetObject(h, fname);

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
        if(!((i == 0 && dataType[i] == VISIT_DATATYPE_INT) ||
              dataType[i] == VISIT_DATATYPE_FLOAT ||
              dataType[i] == VISIT_DATATYPE_DOUBLE))
        {
            VisItError("Coordinates must contain int, float, or double data");
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
    }

    if(obj != NULL)
    {
        obj->FreeCoordinates();
        obj->xcoords = cHandles[0];
        obj->ycoords = cHandles[1];

        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_CurveData_setCoordsXY(visit_handle h, visit_handle x, visit_handle y)
{
    visit_handle cHandles[2];
    cHandles[0] = x;
    cHandles[1] = y;
    return simv2_CurveData_setCoords_helper(h, cHandles, 2,
               "simv2_CurveData_setCoordsXY");
}

/*******************************************************************************
 * C++ code callable from the SimV2 plugin and within the runtime
 ******************************************************************************/

int
simv2_CurveData_getData(visit_handle h, visit_handle &x, visit_handle &y)
{
    int retval = VISIT_ERROR;
    VisIt_CurveData *obj = GetObject(h, "simv2_CurveData_getData");
    if(obj != NULL)
    {
        x = obj->xcoords;
        y = obj->ycoords;

        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_CurveData_check(visit_handle h)
{
    int retval = VISIT_ERROR;
    VisIt_CurveData *obj = GetObject(h, "simv2_CurveData_check");
    if(obj != NULL)
    {
        if(obj->xcoords == VISIT_INVALID_HANDLE)
        {
            VisItError("No coordinates were supplied for the CurveData");
            return VISIT_ERROR;
        }
        retval = VISIT_OKAY;
    }
    return retval;
}
