/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
