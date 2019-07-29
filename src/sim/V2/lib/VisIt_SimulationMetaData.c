// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <VisItDataInterface_V2.h>
#include "VisItDynamic.h"
#include "VisItFortran.h"

int
VisIt_SimulationMetaData_alloc(visit_handle *h)
{
    VISIT_DYNAMIC_EXECUTE(SimulationMetaData_alloc,
                    int, (visit_handle*),
                    (h))
}

int
VisIt_SimulationMetaData_free(visit_handle h)
{
    VISIT_DYNAMIC_EXECUTE(SimulationMetaData_free,
                    int, (visit_handle), 
                    (h));
}

int
VisIt_SimulationMetaData_setMode(visit_handle h, int mode)
{
    VISIT_DYNAMIC_EXECUTE(SimulationMetaData_setMode,
                    int, (visit_handle,int), 
                    (h,mode));
}

int
VisIt_SimulationMetaData_setCycleTime(visit_handle h, int cycle, double time)
{
    VISIT_DYNAMIC_EXECUTE(SimulationMetaData_setCycleTime,
                    int, (visit_handle,int,double), 
                    (h,cycle,time));
}

int
VisIt_SimulationMetaData_addMesh(visit_handle h, visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(SimulationMetaData_addMesh,
                    int, (visit_handle,visit_handle), 
                    (h,obj));
}

int
VisIt_SimulationMetaData_addVariable(visit_handle h, visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(SimulationMetaData_addVariable,
                    int, (visit_handle,visit_handle), 
                    (h,obj));
}

int
VisIt_SimulationMetaData_addMaterial(visit_handle h, visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(SimulationMetaData_addMaterial,
                    int, (visit_handle,visit_handle), 
                    (h,obj));
}

int
VisIt_SimulationMetaData_addCurve(visit_handle h, visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(SimulationMetaData_addCurve,
                    int, (visit_handle,visit_handle), 
                    (h,obj));
}

int
VisIt_SimulationMetaData_addExpression(visit_handle h, visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(SimulationMetaData_addExpression,
                    int, (visit_handle,visit_handle), 
                    (h,obj));
}

int
VisIt_SimulationMetaData_addSpecies(visit_handle h, visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(SimulationMetaData_addSpecies,
                    int, (visit_handle,visit_handle), 
                    (h,obj));
}

int
VisIt_SimulationMetaData_addGenericCommand(visit_handle h, visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(SimulationMetaData_addGenericCommand,
                    int, (visit_handle,visit_handle), 
                    (h,obj));
}

int
VisIt_SimulationMetaData_addCustomCommand(visit_handle h, visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(SimulationMetaData_addCustomCommand,
                    int, (visit_handle,visit_handle), 
                    (h,obj));
}

int
VisIt_SimulationMetaData_addMessage(visit_handle h, visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(SimulationMetaData_addMessage,
                    int, (visit_handle,visit_handle), 
                    (h,obj));
}


/************************** Fortran callable routines *************************/
/* maxlen 012345678901234567890123456789                                      */
#define F_VISITMDSIMALLOC              F77_ID(visitmdsimalloc_,visitmdsimalloc,VISITMDSIMALLOC)
#define F_VISITMDSIMFREE               F77_ID(visitmdsimfree_,visitmdsimfree,VISITMDSIMFREE)
#define F_VISITMDSIMSETMODE            F77_ID(visitmdsimsetmode_,visitmdsimsetmode,VISITMDSIMSETMODE)
#define F_VISITMDSIMSETCYCLETIME       F77_ID(visitmdsimsetcycletime_,visitmdsimsetcycletime,VISITMDSIMSETCYCLETIME)
#define F_VISITMDSIMADDMESH            F77_ID(visitmdsimaddmesh_,visitmdsimaddmesh,VISITMDSIMADDMESH)
#define F_VISITMDSIMADDVARIABLE        F77_ID(visitmdsimaddvariable_,visitmdsimaddvariable,VISITMDSIMADDVARIABLE)
#define F_VISITMDSIMADDMATERIAL        F77_ID(visitmdsimaddmaterial_,visitmdsimaddmaterial,VISITMDSIMADDMATERIAL)
#define F_VISITMDSIMADDCURVE           F77_ID(visitmdsimaddcurve_,visitmdsimaddcurve,VISITMDSIMADDCURVE)
#define F_VISITMDSIMADDEXPRESSION      F77_ID(visitmdsimaddexpression_,visitmdsimaddexpression,VISITMDSIMADDEXPRESSION)
#define F_VISITMDSIMADDSPECIES         F77_ID(visitmdsimaddspecies_,visitmdsimaddspecies,VISITMDSIMADDSPECIES)
#define F_VISITMDSIMADDGENERICCOMMAND  F77_ID(visitmdsimaddgenericcommand_,visitmdsimaddgenericcommand,VISITMDSIMADDGENERICCOMMAND)
#define F_VISITMDSIMADDCUSTOMCOMMAND   F77_ID(visitmdsimaddcustomcommand_,visitmdsimaddcustomcommand,VISITMDSIMADDCUSTOMCOMMAND)
#define F_VISITMDSIMADDMESSAGE         F77_ID(visitmdsimaddmessage_,visitmdsimaddmessage,VISITMDSIMADDMESSAGE)

int
F_VISITMDSIMALLOC(visit_handle *h)
{
    return VisIt_SimulationMetaData_alloc(h);
}

int
F_VISITMDSIMFREE(visit_handle *h)
{
    return VisIt_SimulationMetaData_free(*h);
}

int
F_VISITMDSIMSETMODE(visit_handle *h, int *mode)
{
    return VisIt_SimulationMetaData_setMode(*h, *mode);
}

int
F_VISITMDSIMSETCYCLETIME(visit_handle *h, int *cycle, double *time)
{
    return VisIt_SimulationMetaData_setCycleTime(*h, *cycle, *time);
}

int
F_VISITMDSIMADDMESH(visit_handle *h, visit_handle *md)
{
    return VisIt_SimulationMetaData_addMesh(*h, *md);
}

int
F_VISITMDSIMADDVARIABLE(visit_handle *h, visit_handle *md)
{
    return VisIt_SimulationMetaData_addVariable(*h, *md);
}

int
F_VISITMDSIMADDMATERIAL(visit_handle *h, visit_handle *md)
{
    return VisIt_SimulationMetaData_addMaterial(*h, *md);
}

int
F_VISITMDSIMADDCURVE(visit_handle *h, visit_handle *md)
{
    return VisIt_SimulationMetaData_addCurve(*h, *md);
}

int
F_VISITMDSIMADDEXPRESSION(visit_handle *h, visit_handle *md)
{
    return VisIt_SimulationMetaData_addExpression(*h, *md);
}

int
F_VISITMDSIMADDSPECIES(visit_handle *h, visit_handle *md)
{
    return VisIt_SimulationMetaData_addSpecies(*h, *md);
}

int
F_VISITMDSIMADDGENERICCOMMAND(visit_handle *h, visit_handle *md)
{
    return VisIt_SimulationMetaData_addGenericCommand(*h, *md);
}

int
F_VISITMDSIMADDCUSTOMCOMMAND(visit_handle *h, visit_handle *md)
{
    return VisIt_SimulationMetaData_addCustomCommand(*h, *md);
}

int
F_VISITMDSIMADDMESSAGE(visit_handle *h, visit_handle *md)
{
    return VisIt_SimulationMetaData_addMessage(*h, *md);
}


