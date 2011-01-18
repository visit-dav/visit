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
VisIt_DomainNesting_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(DomainNesting_alloc,
                    int (*)(visit_handle*),
                    int (*cb)(visit_handle*),
                    (*cb)(obj))
}

int
VisIt_DomainNesting_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(DomainNesting_free,
                    int (*)(visit_handle), 
                    int (*cb)(visit_handle), 
                    (*cb)(obj));
}

int
VisIt_DomainNesting_set_dimensions(visit_handle obj, int nPatches, int nLevels, int nDimensions)
{
    VISIT_DYNAMIC_EXECUTE(DomainNesting_set_dimensions,
                    int (*)(visit_handle,int,int,int),
                    int (*cb)(visit_handle,int,int,int),
                    (*cb)(obj, nPatches, nLevels, nDimensions))
}

int
VisIt_DomainNesting_set_levelRefinement(visit_handle obj, int level, int ratios[3])
{
    VISIT_DYNAMIC_EXECUTE(DomainNesting_set_levelRefinement,
                    int (*)(visit_handle,int,int[3]), 
                    int (*cb)(visit_handle,int,int[3]), 
                    (*cb)(obj, level, ratios));
}

int
VisIt_DomainNesting_set_nestingForPatch(visit_handle obj, int patch, int level, 
    const int *childPatches, int nChildPatches, int extents[6])
{
    VISIT_DYNAMIC_EXECUTE(DomainNesting_set_nestingForPatch,
                    int (*)(visit_handle,int,int,const int*,int,int[6]), 
                    int (*cb)(visit_handle,int,int,const int*,int,int[6]), 
                    (*cb)(obj, patch, level, childPatches, nChildPatches, extents));
}

/************************** Fortran callable routines *************************/
#define F_VISITDNESTINGALLOC                    F77_ID(visitdnestingalloc_,visitdnestingalloc,VISITDNESTINGALLOC)
#define F_VISITDNESTINGFREE                     F77_ID(visitdnestingfree_,visitdnestingfree,VISITDNESTINGFREE)
#define F_VISITDNESTINGSETDIMENSIONS            F77_ID(visitdnestingsetdimensions_,visitdnestingsetdimensions,VISITDNESTINGSETDIMENSIONS)
#define F_VISITDNESTINGSETLEVELREFINEMENT       F77_ID(visitdnestingsetlevelrefinement_,visitdnestingsetlevelrefinment,VISITDNESTINGSETLEVELREFINEMENT)
#define F_VISITDNESTINGSETNESTINGFORPATCH       F77_ID(visitdnestingsetnestingforpatch_,visitdnestingsetnestingforpatch,VISITDNESTINGSETNESTINGFORPATCH)

int
F_VISITDNESTINGALLOC(visit_handle *obj)
{
    return VisIt_DomainNesting_alloc(obj);
}

int
F_VISITDNESTINGFREE(visit_handle *obj)
{
    return VisIt_DomainNesting_free(*obj);
}

int
F_VISITDNESTINGSETDIMENSIONS(visit_handle *obj, int *nPatches, int *nLevels, int *nDimensions)
{
    return VisIt_DomainNesting_set_dimensions(*obj, *nPatches, *nLevels, *nDimensions);
}

int
F_VISITDNESTINGSETLEVELREFINEMENT(visit_handle *obj, int *level, const int *ratios)
{
    int r[3];
    r[0] = ratios[0];
    r[1] = ratios[1];
    r[2] = ratios[2];
    return VisIt_DomainNesting_set_levelRefinement(*obj, *level, r);
}

int
F_VISITDNESTINGSETNESTINGFORPATCH(visit_handle *obj, int *patch, int *level, 
    const int *childPatches, int *nChildPatches, const int *extents)
{
    int i, e[6];
    for(i = 0; i < 6; ++i)
        e[i] = extents[i];
    return VisIt_DomainNesting_set_nestingForPatch(*obj, *patch, *level, 
           childPatches, *nChildPatches, e);
}

