/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
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
VisIt_DomainBoundaries_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(DomainBoundaries_alloc,
                    int, (visit_handle*),
                    (obj))
}

int
VisIt_DomainBoundaries_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(DomainBoundaries_free,
                    int, (visit_handle), 
                    (obj));
}

int
VisIt_DomainBoundaries_set_type(visit_handle obj, int type)
{
    VISIT_DYNAMIC_EXECUTE(DomainBoundaries_set_type,
                    int, (visit_handle,int), 
                    (obj, type));
}

int
VisIt_DomainBoundaries_set_numDomains(visit_handle obj, int numDomains)
{
    VISIT_DYNAMIC_EXECUTE(DomainBoundaries_set_numDomains,
                    int, (visit_handle,int), 
                    (obj, numDomains));
}

int
VisIt_DomainBoundaries_set_amrIndices(visit_handle obj, int patch, int level, const int extents[6])
{
    VISIT_DYNAMIC_EXECUTE(DomainBoundaries_set_amrIndices,
                    int, (visit_handle,int,int,const int[6]), 
                    (obj, patch, level, extents));
}

int
VisIt_DomainBoundaries_set_rectIndices(visit_handle obj, int dom, const int extents[6])
{
    VISIT_DYNAMIC_EXECUTE(DomainBoundaries_set_rectIndices,
                    int, (visit_handle,int,const int[6]), 
                    (obj, dom, extents));
}

/************************** Fortran callable routines *************************/
/* maxlen 012345678901234567890123456789                                      */
#define F_VISITDBOUNDSALLOC              F77_ID(visitdboundsalloc_,visitdboundsalloc,VISITDBOUNDSALLOC)
#define F_VISITDBOUNDSFREE               F77_ID(visitdboundsfree_,visitdboundsfree,VISITDBOUNDSFREE)
#define F_VISITDBOUNDSSETTYPE            F77_ID(visitdboundssettype_,visitdboundssettype,VISITDBOUNDSSETTYPE)
#define F_VISITDBOUNDSSETNUMDOMAINS      F77_ID(visitdboundssetnumdomains_,visitdboundssetnumdomains,VISITDBOUNDSSETNUMDOMAINS)
#define F_VISITDBOUNDSSETAMRINDICES      F77_ID(visitdboundssetamrindices_,visitdboundssetamrindices,VISITDBOUNDSSETAMRINDICES)
#define F_VISITDBOUNDSSETRECTINDICES     F77_ID(visitdboundssetrectindices_,visitdboundssetrectindices,VISITDBOUNDSSETRECTINDICES)

int
F_VISITDBOUNDSALLOC(visit_handle *obj)
{
    return VisIt_DomainBoundaries_alloc(obj);
}

int
F_VISITDBOUNDSFREE(visit_handle *obj)
{
    return VisIt_DomainBoundaries_free(*obj);
}

int
F_VISITDBOUNDSSETTYPE(visit_handle *obj, int *type)
{
    return VisIt_DomainBoundaries_set_type(*obj, *type);
}

int
F_VISITDBOUNDSSETNUMDOMAINS(visit_handle *obj, int *numDomains)
{
    return VisIt_DomainBoundaries_set_numDomains(*obj, *numDomains);
}

int
F_VISITDBOUNDSSETAMRINDICES(visit_handle *obj, int *patch, int *level, const int *extents)
{
    int i, e[6];
    for(i = 0; i < 6; ++i)
        e[i] = extents[i];
    return VisIt_DomainBoundaries_set_amrIndices(*obj, *patch, *level, e);
}

int
F_VISITDBOUNDSSETRECTINDICES(visit_handle *obj, int *domain, const int *extents)
{
    int i, e[6];
    for(i = 0; i < 6; ++i)
        e[i] = extents[i];
    return VisIt_DomainBoundaries_set_rectIndices(*obj, *domain, e);
}
