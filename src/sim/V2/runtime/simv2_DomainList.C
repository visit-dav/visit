#include <snprintf.h>

#include "VisItDataInterfaceRuntime.h"
#include "VisItDataInterfaceRuntimeP.h"

#include "simv2_DomainList.h"
#include "simv2_VariableData.h"

struct VisIt_DomainList : public VisIt_ObjectBase
{
    VisIt_DomainList();
    virtual ~VisIt_DomainList();
    void FreeDomains();

    int          alldoms;
    visit_handle mydoms;
};

VisIt_DomainList::VisIt_DomainList() : VisIt_ObjectBase(VISIT_DOMAINLIST)
{
    alldoms = 0;
    mydoms = VISIT_INVALID_HANDLE;
}

VisIt_DomainList::~VisIt_DomainList()
{
    FreeDomains();
}

void
VisIt_DomainList::FreeDomains()
{
    if(mydoms != VISIT_INVALID_HANDLE)
    {
        simv2_VariableData_free(mydoms);
        mydoms = VISIT_INVALID_HANDLE;
    }
}

static VisIt_DomainList *
GetObject(visit_handle h, const char *fname)
{
    char tmp[100];
    VisIt_DomainList *obj = (VisIt_DomainList *)VisItGetPointer(h);
    if(obj != NULL)
    {
        if(obj->objectType() != VISIT_DOMAINLIST)
        {
            SNPRINTF(tmp, 100, "%s: The provided handle does not point to "
                "a DomainList object.", fname);
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
simv2_DomainList_alloc(visit_handle *h)
{
    *h = VisItStorePointer(new VisIt_DomainList);
    return (*h != VISIT_INVALID_HANDLE) ? VISIT_OKAY : VISIT_ERROR;
}

int
simv2_DomainList_free(visit_handle h)
{
    VisIt_DomainList *obj = GetObject(h, "simv2_DomainList_free");
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
simv2_DomainList_setDomains(visit_handle h, int alldoms, visit_handle mydoms)
{
    int retval = VISIT_ERROR;
    VisIt_DomainList *obj = GetObject(h, "simv2_DomainList_setDomains");

    if(alldoms < 1)
    {
        VisItError("There must be at least 1 domain.");
        return VISIT_ERROR;
    }
 
    // Get the domains
    int owner, dataType, nComps, nTuples;
    void *data = 0;
    if(simv2_VariableData_getData(mydoms, owner, dataType, nComps, 
       nTuples, data) == VISIT_ERROR)
    {
        return VISIT_ERROR;
    }

    // Error checking.
    if(nComps != 1)
    {
        VisItError("DomainList must have 1 component");
        return VISIT_ERROR;
    }
    if(dataType != VISIT_DATATYPE_INT)
    {
        VisItError("DomainList must contain int data");
        return VISIT_ERROR;
    }

    if(obj != NULL)
    {
        obj->FreeDomains();
        obj->mydoms = mydoms;
        obj->alldoms = alldoms;

        retval = VISIT_OKAY;
    }
    return retval;
}

/*******************************************************************************
 * C++ code callable from the SimV2 plugin and within the runtime
 ******************************************************************************/

int
simv2_DomainList_getData(visit_handle h, int &alldoms, visit_handle &mydoms)
{
    int retval = VISIT_ERROR;
    VisIt_DomainList *obj = GetObject(h, "simv2_DomainList_getData");
    if(obj != NULL)
    {
        alldoms = obj->alldoms;
        mydoms = obj->mydoms;

        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_DomainList_check(visit_handle h)
{
    int retval = VISIT_ERROR;
    VisIt_DomainList *obj = GetObject(h, "simv2_DomainList_check");
    if(obj != NULL)
    {
        if(obj->mydoms == VISIT_INVALID_HANDLE)
        {
            VisItError("No domains were supplied for the DomainList");
            return VISIT_ERROR;
        }
        int owner, dataType, nComps, nTuples;
        void *data = NULL;
        if(simv2_VariableData_getData(obj->mydoms, owner, dataType, nComps,
            nTuples, data) == VISIT_OKAY)
        {
            int *doms = (int*)data;
            for(int i = 0; i < nTuples; ++i)
            {
                if(doms[i] < 0 || doms[i] >= obj->alldoms)
                {
                    VisItError("The domain list contained out of range domain numbers");
                    return VISIT_ERROR;
                }
            }
        }
        retval = VISIT_OKAY;
    }
    return retval;
}
