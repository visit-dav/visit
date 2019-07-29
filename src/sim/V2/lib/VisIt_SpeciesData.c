// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <VisItDataInterface_V2.h>
#include "VisItDynamic.h"
#include "VisItFortran.h"

int
VisIt_SpeciesData_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(SpeciesData_alloc,
                    int, (visit_handle*),
                    (obj))
}

int
VisIt_SpeciesData_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(SpeciesData_free,
                    int, (visit_handle), 
                    (obj));
}

int
VisIt_SpeciesData_addSpeciesName(visit_handle h, visit_handle val)
{
    VISIT_DYNAMIC_EXECUTE(SpeciesData_addSpeciesName,
        int, (visit_handle, visit_handle),
        (h, val));
}

int
VisIt_SpeciesData_setSpecies(visit_handle h, visit_handle val)
{
    VISIT_DYNAMIC_EXECUTE(SpeciesData_setSpecies,
        int, (visit_handle, visit_handle),
        (h, val));
}

int
VisIt_SpeciesData_setSpeciesMF(visit_handle h, visit_handle val)
{
    VISIT_DYNAMIC_EXECUTE(SpeciesData_setSpeciesMF,
        int, (visit_handle, visit_handle),
        (h, val));
}

int
VisIt_SpeciesData_setMixedSpecies(visit_handle h, visit_handle val)
{
    VISIT_DYNAMIC_EXECUTE(SpeciesData_setMixedSpecies,
        int, (visit_handle, visit_handle),
        (h, val));
}


/************************** Fortran callable routines *************************/
/* maxlen 012345678901234567890123456789                                      */
#define F_VISITSPECDATAALLOC           F77_ID(visitspecdataalloc_,visitspecdataalloc,VISITSPECDATAALLOC)
#define F_VISITSPECDATAFREE            F77_ID(visitspecdatafree_,visitspecdatafree,VISITSPECDATAFREE)
#define F_VISITSPECDATAADDSPECIESNAME  F77_ID(visitspecdataaddspeciesname_,visitspecdataaddspeciesname, VISITSPECDATAADDSPECIESNAME)
#define F_VISITSPECDATASETSPECIES      F77_ID(visitspecdatasetspecies_,visitspecdatasetspecies, VISITSPECDATASETSPECIES)
#define F_VISITSPECDATASETSPECIESMF    F77_ID(visitspecdatasetspeciesmf_,visitspecdatasetspeciesmf, VISITSPECDATASETSPECIESMF)
#define F_VISITSPECDATASETMIXEDSPECIES F77_ID(visitspecdatasetmixedspecies_,visitspecdatasetmixedspecies, VISITSPECDATASETMIXEDSPECIES)

int
F_VISITSPECDATAALLOC(visit_handle *h)
{
    return VisIt_SpeciesData_alloc(h);
}

int
F_VISITSPECDATAFREE(visit_handle *h)
{
    return VisIt_SpeciesData_free(*h);
}

int
F_VISITSPECDATAADDSPECIESNAME(visit_handle *h, visit_handle *val)
{
    return VisIt_SpeciesData_addSpeciesName(*h, *val);
}

int
F_VISITSPECDATASETSPECIES(visit_handle *h, visit_handle *val)
{
    return VisIt_SpeciesData_setSpecies(*h, *val);
}

int
F_VISITSPECDATASETSPECIESMF(visit_handle *h, visit_handle *val)
{
    return VisIt_SpeciesData_setSpeciesMF(*h, *val);
}

int
F_VISITSPECDATASETMIXEDSPECIES(visit_handle *h, visit_handle *val)
{
    return VisIt_SpeciesData_setMixedSpecies(*h, *val);
}
