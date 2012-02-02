#include <VisItDataInterfaceRuntime.h>
#include <VisItDataInterfaceRuntimeP.h>
#include <avtStructuredDomainBoundaries.h>

#include <simv2_DomainBoundaries.h>
#include <cstring>

struct VisIt_DomainBoundaries : public VisIt_ObjectBase
{
    VisIt_DomainBoundaries();
    virtual ~VisIt_DomainBoundaries();

    avtStructuredDomainBoundaries *boundaries;
};

VisIt_DomainBoundaries::VisIt_DomainBoundaries() : 
    VisIt_ObjectBase(VISIT_DOMAIN_BOUNDARIES)
{
    boundaries = NULL;
}

VisIt_DomainBoundaries::~VisIt_DomainBoundaries()
{
    if(boundaries != NULL)
        delete boundaries;
}

static VisIt_DomainBoundaries *
GetObject(visit_handle h)
{
    VisIt_DomainBoundaries *obj = (VisIt_DomainBoundaries *)VisItGetPointer(h);
    if(obj != NULL)
    {
        if(obj->objectType() != VISIT_DOMAIN_BOUNDARIES)
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
simv2_DomainBoundaries_alloc(visit_handle *h)
{
    *h = VisItStorePointer(new VisIt_DomainBoundaries);
    return (*h != VISIT_INVALID_HANDLE) ? VISIT_OKAY : VISIT_ERROR;
}

int
simv2_DomainBoundaries_free(visit_handle h)
{
    VisIt_DomainBoundaries *obj = GetObject(h);
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
simv2_DomainBoundaries_set_type(visit_handle h, int type)
{
    int retval = VISIT_ERROR;
    VisIt_DomainBoundaries *obj = GetObject(h);
    if(obj != NULL)
    {
        if(obj->boundaries != NULL)
            delete obj->boundaries;
        if(type == 0)
        {
            obj->boundaries = new avtRectilinearDomainBoundaries(true);
        }
        else
            obj->boundaries = new avtCurvilinearDomainBoundaries(true);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_DomainBoundaries_set_numDomains(visit_handle h, int numDomains)
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

// ****************************************************************************
// Method: simv2_DomainBoundaries_set_rectIndices
//
// Purpose: 
//   Set rectilinear domain indices so we know where the domain exists in
//   the entire dataset.
//
// Arguments:
//   h : The hansle to the domain boundaries object.
//   patch : The patch for which we're setting extents.
//   level : the level in which the patch exists.
//   extents : The global zone indices for the domain e.g. 0, 10, 0, 10, 0, 10
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   2010
//
// Modifications:
//   Brad Whitlock, Wed Feb  1 14:29:51 PST 2012
//   I added a missing const to the signature so the function will be exposed
//   to the front end.
//
// ****************************************************************************


int
simv2_DomainBoundaries_set_amrIndices(visit_handle h, int patch, int level, const int extents[6])
{
    int retval = VISIT_ERROR;
    VisIt_DomainBoundaries *obj = GetObject(h);
    if(obj != NULL && obj->boundaries != NULL)
    {
        int e[6];
        memcpy(e, extents, 6 * sizeof(int));
        obj->boundaries->SetIndicesForAMRPatch(patch, level, e);
        retval = VISIT_OKAY;
    }
    return retval;
}

// ****************************************************************************
// Method: simv2_DomainBoundaries_set_rectIndices
//
// Purpose: 
//   Set rectilinear domain indices so we know where the domain exists in
//   the entire dataset.
//
// Arguments:
//   h : The hansle to the domain boundaries object.
//   dom : The domain for which we're setting extents.
//   extents : The global zone indices for the domain e.g. 0, 10, 0, 10, 0, 10
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   2010
//
// Modifications:
//   Brad Whitlock, Wed Feb  1 14:29:51 PST 2012
//   I added a missing const to the signature so the function will be exposed
//   to the front end.
//
// ****************************************************************************

int
simv2_DomainBoundaries_set_rectIndices(visit_handle h, int dom, const int extents[6])
{
    int retval = VISIT_ERROR;
    VisIt_DomainBoundaries *obj = GetObject(h);
    if(obj != NULL && obj->boundaries != NULL)
    {
        int e[6];
        memcpy(e, extents, 6 * sizeof(int));
        obj->boundaries->SetIndicesForRectGrid(dom, e);
        retval = VISIT_OKAY;
    }
    return retval;
}

// C++ code that exists in the runtime that we can use in the SimV2 reader
void *
simv2_DomainBoundaries_avt(visit_handle h)
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
