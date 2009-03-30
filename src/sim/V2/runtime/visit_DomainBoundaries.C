#include <VisItDataInterfaceRuntime.h>
#include <VisItDataInterfaceRuntimeP.h>
#include <avtStructuredDomainBoundaries.h>

typedef struct
{
    VISIT_OBJECT_HEAD

    avtStructuredDomainBoundaries *boundaries;
} VisIt_DomainBoundaries;

static VisIt_DomainBoundaries *
GetObject(visit_handle h)
{
    VisIt_DomainBoundaries *obj = (VisIt_DomainBoundaries *)VisItGetPointer(h);
    if(obj != NULL)
    {
        if(!VISIT_OBJECT_CHECK_TYPE(obj, VISIT_DOMAIN_BOUNDARIES))
        {
            VisItError("The provided handle does not point to a DomainBoundaries object.");
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
visit_DomainBoundaries_alloc(visit_handle *h)
{
    VisIt_DomainBoundaries *obj = VISIT_OBJECT_ALLOCATE(VisIt_DomainBoundaries);
    VISIT_OBJECT_INITIALIZE(obj, VISIT_DOMAIN_BOUNDARIES);
    *h = VisItStorePointer(obj);
    return (*h != VISIT_INVALID_HANDLE) ? VISIT_OKAY : VISIT_ERROR;
}

int
visit_DomainBoundaries_free(visit_handle h)
{
    VisIt_DomainBoundaries *obj = GetObject(h);
    if(obj != NULL)
    {
        if(obj->boundaries != NULL)
            delete obj->boundaries;
        if(obj != NULL)
            free(obj);
        VisItFreePointer(h);
    }
}

int
visit_DomainBoundaries_set_type(visit_handle h, int type)
{
    int retval = VISIT_ERROR;
    VisIt_DomainBoundaries *obj = GetObject(h);
    if(obj != NULL)
    {
        if(obj->boundaries != NULL)
            delete obj->boundaries;
        if(type == 0)
            obj->boundaries = new avtRectilinearDomainBoundaries(true);
        else
            obj->boundaries = new avtCurvilinearDomainBoundaries(true);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
visit_DomainBoundaries_set_numDomains(visit_handle h, int numDomains)
{
    int retval = VISIT_ERROR;
    VisIt_DomainBoundaries *obj = GetObject(h);
    if(obj != NULL && obj->boundaries != NULL)
    {
        obj->boundaries->SetNumDomains(numDomains);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
visit_DomainBoundaries_set_amrIndices(visit_handle h, int patch, int level, int extents[6])
{
    int retval = VISIT_ERROR;
    VisIt_DomainBoundaries *obj = GetObject(h);
    if(obj != NULL && obj->boundaries != NULL)
    {
        obj->boundaries->SetIndicesForAMRPatch(patch, level, extents);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
visit_DomainBoundaries_set_rectIndices(visit_handle h, int dom, int extents[6])
{
    int retval = VISIT_ERROR;
    VisIt_DomainBoundaries *obj = GetObject(h);
    if(obj != NULL && obj->boundaries != NULL)
    {
        obj->boundaries->SetIndicesForRectGrid(dom, extents);
        retval = VISIT_OKAY;
    }
    return retval;
}

// C++ code that exists in the runtime that we can use in the SimV2 reader
void *
visit_DomainBoundaries_avt(visit_handle h)
{
    void *retval = NULL;
    VisIt_DomainBoundaries *obj = GetObject(h);
    if(obj != NULL && obj->boundaries != NULL)
    {
        retval = (void *)obj->boundaries;
        obj->boundaries = NULL;
    }
    return retval;
}
