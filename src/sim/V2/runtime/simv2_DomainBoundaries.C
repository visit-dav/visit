#include <VisItDataInterfaceRuntime.h>
#include <VisItDataInterfaceRuntimeP.h>
#include <avtStructuredDomainBoundaries.h>

#include <simv2_DomainBoundaries.h>
#include <cstring>

#define CALLING_UNKNOWN            -1
#define CALLING_RECT_FUNCTIONS      0
#define CALLING_AMR_FUNCTIONS       1
#define CALLING_NEIGHBOR_FUNCTIONS  2

struct VisIt_DomainBoundaries : public VisIt_ObjectBase
{
    VisIt_DomainBoundaries();
    virtual ~VisIt_DomainBoundaries();

    avtStructuredDomainBoundaries *boundaries;
    int                            callingMode;
};

VisIt_DomainBoundaries::VisIt_DomainBoundaries() : 
    VisIt_ObjectBase(VISIT_DOMAIN_BOUNDARIES)
{
    boundaries = NULL;
    callingMode = CALLING_UNKNOWN;
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
        if(obj->callingMode == CALLING_UNKNOWN ||
           obj->callingMode == CALLING_AMR_FUNCTIONS)
        {
            int e[6];
            memcpy(e, extents, 6 * sizeof(int));
            obj->boundaries->SetIndicesForAMRPatch(patch, level, e);
            obj->callingMode = CALLING_AMR_FUNCTIONS;
            retval = VISIT_OKAY;
        }
        else
        {
            VisItError("DomainBoundaries_set_amrIndices cannot be called on the "
                       "DomainBoundaries object once DomainBoundaries_set_rectIndices "
                       "or functions to manually add neighbors have been called.");
        }
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
//   h : The handle to the domain boundaries object.
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
        if(obj->callingMode == CALLING_UNKNOWN ||
           obj->callingMode == CALLING_RECT_FUNCTIONS)
        {
            int e[6];
            memcpy(e, extents, 6 * sizeof(int));
            obj->boundaries->SetIndicesForRectGrid(dom, e);
            obj->callingMode = CALLING_RECT_FUNCTIONS;
            retval = VISIT_OKAY;
        }
        else
        {
            VisItError("DomainBoundaries_set_rectIndices cannot be called on the "
                       "DomainBoundaries object once DomainBoundaries_set_amrIndices "
                       "or functions to manually add neighbors have been called.");
        }
    }
    return retval;
}

// ****************************************************************************
// Method: simv2_DomainBoundaries_set_extents
//
// Purpose: 
//   Set domain extents.
//
// Arguments:
//   h : The handle to the domain boundaries object.
//   dom : The domain for which we're setting extents.
//   extents : The global zone indices for the domain e.g. 0, 10, 0, 10, 0, 10
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   2012
//
// Modifications:
//
// ****************************************************************************

int
simv2_DomainBoundaries_set_extents(visit_handle h, int dom, const int extents[6])
{
    int retval = VISIT_ERROR;
    VisIt_DomainBoundaries *obj = GetObject(h);
    if(obj != NULL && obj->boundaries != NULL)
    {
        if(obj->callingMode == CALLING_UNKNOWN ||
           obj->callingMode == CALLING_NEIGHBOR_FUNCTIONS)
        {
            int e[6];
            memcpy(e, extents, 6 * sizeof(int));
            obj->callingMode = CALLING_NEIGHBOR_FUNCTIONS;
            obj->boundaries->SetExtents(dom, e);
            retval = VISIT_OKAY;
        }
        else
        {
            VisItError("DomainBoundaries_set_extents cannot be called on the "
                       "DomainBoundaries object once DomainBoundaries_set_amrIndices "
                       "or DomainBoundaries_set_rectIndices functions have been called.");
        }
    }
    return retval;
}

// ****************************************************************************
// Method: simv2_DomainBoundaries_add_neighbor
//
// Purpose: 
//   Set domain extents.
//
// Arguments:
//   h   : The handle to the domain boundaries object.
//   dom : The current domain to add a neighbor for
//   d   : The domain number of the new neigbor
//   mi  : The current domain's index in the neighbor's neighbor list
//   orientation : The three orientation values
//   extents     : The extents of the matching boundary in the current domain
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   2012
//
// Modifications:
//
// ****************************************************************************

int
simv2_DomainBoundaries_add_neighbor(visit_handle h, int dom, int d, int mi, 
    const int orientation[3], const int extents[6])
{
    int retval = VISIT_ERROR;
    VisIt_DomainBoundaries *obj = GetObject(h);
    if(obj != NULL && obj->boundaries != NULL)
    {
        if(obj->callingMode == CALLING_UNKNOWN ||
           obj->callingMode == CALLING_NEIGHBOR_FUNCTIONS)
        {
            int o[3], e[6];
            memcpy(o, orientation, sizeof(int) * 3);
            memcpy(e, extents, sizeof(int) * 6);
            obj->callingMode = CALLING_NEIGHBOR_FUNCTIONS;
/*printf("add_neighbor: dom=%d, d=%d, mi=%d, orientation={%d,%d,%d}, extents={%d,%d,%d,%d,%d,%d}\n",
       dom, d, mi, orientation[0],orientation[1],orientation[2],
       e[0], e[1], e[2], e[3], e[4], e[5]);
*/
            obj->boundaries->AddNeighbor(dom, d, mi, o, e);
            retval = VISIT_OKAY;
        }
        else
        {
            VisItError("DomainBoundaries_add_neighbor cannot be called on the "
                       "DomainBoundaries object once DomainBoundaries_set_amrIndices "
                       "or DomainBoundaries_set_rectIndices functions have been called.");
        }
    }
    return retval;
}

// ****************************************************************************
// Method: simv2_DomainBoundaries_finish
//
// Purpose: 
//   Set domain extents.
//
// Arguments:
//   h : The handle to the domain boundaries object.
//   dom : The domain we're finishing.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   2012
//
// Modifications:
//
// ****************************************************************************

int
simv2_DomainBoundaries_finish(visit_handle h, int dom)
{
    int retval = VISIT_ERROR;
    VisIt_DomainBoundaries *obj = GetObject(h);
    if(obj != NULL && obj->boundaries != NULL)
    {
        if(obj->callingMode == CALLING_UNKNOWN ||
           obj->callingMode == CALLING_NEIGHBOR_FUNCTIONS)
        {
            obj->callingMode = CALLING_NEIGHBOR_FUNCTIONS;
            obj->boundaries->Finish(dom);
            retval = VISIT_OKAY;
        }
        else
        {
            VisItError("DomainBoundaries_finish cannot be called on the "
                       "DomainBoundaries object once DomainBoundaries_set_amrIndices "
                       "or DomainBoundaries_set_rectIndices functions have been called.");
        }
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
        // Only call CalculateBoundaries if we have not explicitly added neighbors.
        if(obj->callingMode != CALLING_NEIGHBOR_FUNCTIONS)
            obj->boundaries->CalculateBoundaries();

        retval = (void *)obj->boundaries;
        obj->boundaries = NULL;
    }
    return retval;
}
