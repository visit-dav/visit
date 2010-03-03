#include <VisItDataInterface_V2.h>
#include "VisItDynamic.h"
#include "VisItFortran.h"

int
VisIt_CurveData_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(CurveData_alloc,
                    int (*)(visit_handle*),
                    int (*cb)(visit_handle*),
                    (*cb)(obj))
}

int
VisIt_CurveData_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(CurveData_free,
                    int (*)(visit_handle), 
                    int (*cb)(visit_handle), 
                    (*cb)(obj));
}

int
VisIt_CurveData_setCoordsXY(visit_handle obj, visit_handle x, visit_handle y)
{
    VISIT_DYNAMIC_EXECUTE(CurveData_setCoordsXY,
                    int (*)(visit_handle,visit_handle,visit_handle), 
                    int (*cb)(visit_handle,visit_handle,visit_handle), 
                    (*cb)(obj,x,y));
}

/************************** Fortran callable routines *************************/
#define F_VISITCURVEDATAALLOC         F77_ID(visitcurvedataalloc_,visitcurvedataalloc,VISITCURVEDATAALLOC)
#define F_VISITCURVEDATAFREE          F77_ID(visitcurvedatafree_,visitcurvedatafree,VISITCURVEDATAFREE)
#define F_VISITCURVEDATASETCOORDSXY   F77_ID(visitcurvedatasetcoordsxy_,visitcurvedatasetcoordsxy,VISITCURVEDATASETCOORDSXY)

int
F_VISITCURVEDATAALLOC(visit_handle *obj)
{
    return VisIt_CurveData_alloc(obj);
}

int
F_VISITCURVEDATAFREE(visit_handle *obj)
{
    return VisIt_CurveData_free(*obj);
}

int
F_VISITCURVEDATASETCOORDSXY(visit_handle *obj, visit_handle *x, visit_handle *y)
{
    return VisIt_CurveData_setCoordsXY(*obj, *x, *y);
}


