#include <VisItDataInterfaceRuntime.h>
#include <VisItDataInterfaceRuntimeP.h>
#include <avtStructuredDomainNesting.h>

#include <simv2_DomainNesting.h>

struct VisIt_DomainNesting : public VisIt_ObjectBase
{
    VisIt_DomainNesting();
    virtual ~VisIt_DomainNesting();

    avtStructuredDomainNesting *nesting;
    int                         nDimensions;
};

VisIt_DomainNesting::VisIt_DomainNesting() : VisIt_ObjectBase(VISIT_DOMAIN_NESTING)
{
    nesting = NULL;
    nDimensions = 0;
}

VisIt_DomainNesting::~VisIt_DomainNesting()
{
    if(nesting != NULL)
        delete nesting;
}

static VisIt_DomainNesting *
GetObject(visit_handle h)
{
    VisIt_DomainNesting *obj = (VisIt_DomainNesting *)VisItGetPointer(h);
    if(obj != NULL)
    {
        if(obj->type != VISIT_DOMAIN_NESTING)
        {
            VisItError("The provided handle does not point to a DomainNesting object.");
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
simv2_DomainNesting_alloc(visit_handle *h)
{
    *h = VisItStorePointer(new VisIt_DomainNesting);
    return (*h != VISIT_INVALID_HANDLE) ? VISIT_OKAY : VISIT_ERROR;
}

int
simv2_DomainNesting_free(visit_handle h)
{
    VisIt_DomainNesting *obj = GetObject(h);
    if(obj != NULL)
    {
        delete obj;
        VisItFreePointer(h);
    }
}

int
simv2_DomainNesting_set_dimensions(visit_handle h, int nPatches, int nLevels, int nDimensions)
{
    int retval = VISIT_ERROR;
    VisIt_DomainNesting *obj = GetObject(h);
    if(obj != NULL)
    {
        if(obj->nesting != NULL)
            delete obj->nesting;
        obj->nesting = new avtStructuredDomainNesting(nPatches, nLevels);
        if(obj->nesting != NULL)
        {
            obj->nDimensions = nDimensions;
            retval = VISIT_OKAY;
        }
    }
    return retval;
}

int
simv2_DomainNesting_set_levelRefinement(visit_handle h, int level, int ratios[3])
{
    int retval = VISIT_ERROR;
    VisIt_DomainNesting *obj = GetObject(h);
    if(obj != NULL && obj->nesting != NULL)
    {
        std::vector<int> r;
        r.push_back(ratios[0]);
        r.push_back(ratios[1]);
        r.push_back(ratios[2]);
        obj->nesting->SetLevelRefinementRatios(level, r);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_DomainNesting_set_nestingForPatch(visit_handle h, int patch, int level, 
    const int *childPatches, int nChildPatches, int extents[6])
{
    int retval = VISIT_ERROR;
    VisIt_DomainNesting *obj = GetObject(h);
    if(obj != NULL && obj->nesting != NULL)
    {
        vector<int> cp;
        for(int i = 0; i < nChildPatches; ++i)
            cp.push_back(childPatches[i]);

        vector<int> logExts(6,0);
        logExts[0] = extents[0];
        logExts[1] = extents[1];
        logExts[2] = (obj->nDimensions == 3) ? extents[2] : 0;
        logExts[3] = extents[3];
        logExts[4] = extents[4];
        logExts[5] = (obj->nDimensions == 3) ? extents[5] : 0;

        obj->nesting->SetNestingForDomain(patch, level, cp, logExts);

        retval = VISIT_OKAY;
    }
    return retval;
}

// C++ code that exists in the runtime that we can use in the SimV2 reader
void *
simv2_DomainNesting_avt(visit_handle h)
{
    void *retval = NULL;
    VisIt_DomainNesting *obj = GetObject(h);
    if(obj != NULL && obj->nesting != NULL)
    {
        obj->nesting->SetNumDimensions(obj->nDimensions);
        retval = (void *)obj->nesting;
        obj->nesting = NULL;
    }
    return retval;
}
