#include <VisItDataInterface_V2.h>
#include "VisItDynamic.h"
#include "VisItFortran.h"

int
VisIt_DomainList_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(DomainList_alloc,
                    int (*)(visit_handle*),
                    int (*cb)(visit_handle*),
                    (*cb)(obj))
}

int
VisIt_DomainList_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(DomainList_free,
                    int (*)(visit_handle), 
                    int (*cb)(visit_handle), 
                    (*cb)(obj));
}

int
VisIt_DomainList_setDomains(visit_handle obj, int alldoms, visit_handle mydoms)
{
    VISIT_DYNAMIC_EXECUTE(DomainList_setDomains,
                    int (*)(visit_handle,int,visit_handle), 
                    int (*cb)(visit_handle,int,visit_handle), 
                    (*cb)(obj,alldoms,mydoms));
}

/************************** Fortran callable routines *************************/
/* maxlen 012345678901234567890123456789                                      */
#define F_VISITDOMAINLISTALLOC           F77_ID(visitdomainlistalloc_,visitdomainlistalloc,VISITDOMAINLISTALLOC)
#define F_VISITDOMAINLISTFREE            F77_ID(visitdomainlistfree_,visitdomainlistfree,VISITDOMAINLISTFREE)
#define F_VISITDOMAINLISTSETDOMAINS      F77_ID(visitdomainlistsetdomains_,visitdomainlistsetdomains,VISITDOMAINLISTSETDOMAINS)

int
F_VISITDOMAINLISTALLOC(visit_handle *obj)
{
    return VisIt_DomainList_alloc(obj);
}

int
F_VISITDOMAINLISTFREE(visit_handle *obj)
{
    return VisIt_DomainList_free(*obj);
}

int
F_VISITDOMAINLISTSETDOMAINS(visit_handle *obj, int *alldoms, visit_handle *mydoms)
{
    return VisIt_DomainList_setDomains(*obj, *alldoms, *mydoms);
}


