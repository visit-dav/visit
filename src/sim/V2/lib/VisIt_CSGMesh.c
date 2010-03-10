/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
VisIt_CSGMesh_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(CSGMesh_alloc,
                    int (*)(visit_handle*),
                    int (*cb)(visit_handle*),
                    (*cb)(obj))
}

int
VisIt_CSGMesh_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(CSGMesh_free,
                    int (*)(visit_handle), 
                    int (*cb)(visit_handle), 
                    (*cb)(obj));
}

int
VisIt_CSGMesh_setRegions(visit_handle obj, visit_handle boolops, visit_handle leftids, 
    visit_handle rightids)
{
    VISIT_DYNAMIC_EXECUTE(CSGMesh_setRegions,
                    int (*)(visit_handle,visit_handle,visit_handle,visit_handle), 
                    int (*cb)(visit_handle,visit_handle,visit_handle,visit_handle), 
                    (*cb)(obj,boolops,leftids,rightids));
}

int
VisIt_CSGMesh_setZonelist(visit_handle obj, visit_handle zl)
{
    VISIT_DYNAMIC_EXECUTE(CSGMesh_setZonelist,
                    int (*)(visit_handle,visit_handle), 
                    int (*cb)(visit_handle,visit_handle), 
                    (*cb)(obj,zl));
}

int
VisIt_CSGMesh_setBoundaryTypes(visit_handle obj, visit_handle boundaryTypes)
{
    VISIT_DYNAMIC_EXECUTE(CSGMesh_setBoundaryTypes,
                    int (*)(visit_handle,visit_handle), 
                    int (*cb)(visit_handle,visit_handle), 
                    (*cb)(obj,boundaryTypes));
}

int
VisIt_CSGMesh_setBoundaryCoeffs(visit_handle obj, visit_handle boundaryCoeffs)
{
    VISIT_DYNAMIC_EXECUTE(CSGMesh_setBoundaryCoeffs,
                    int (*)(visit_handle,visit_handle), 
                    int (*cb)(visit_handle,visit_handle), 
                    (*cb)(obj,boundaryCoeffs));
}

int
VisIt_CSGMesh_setExtents(visit_handle obj, double min[3], double max[3])
{
    VISIT_DYNAMIC_EXECUTE(CSGMesh_setExtents,
                    int (*)(visit_handle,double[3], double[3]), 
                    int (*cb)(visit_handle,double[3], double[3]), 
                    (*cb)(obj,min,max));
}

/************************** Fortran callable routines *************************/
/* maxlen 012345678901234567890123456789                                      */
#define F_VISITCSGMESHALLOC              F77_ID(visitcsgmeshalloc_,visitcsgmeshalloc,VISITCSGMESHALLOC)
#define F_VISITCSGMESHFREE               F77_ID(visitcsgmeshfree_,visitcsgmeshfree,VISITCSGMESHFREE)
#define F_VISITCSGMESHSETCOORDSXY        F77_ID(visitcsgmeshsetcoordsxy_,visitcsgmeshsetcoordsxy,VISITCSGMESHSETCOORDSXY)
#define F_VISITCSGMESHSETCOORDSXYZ       F77_ID(visitcsgmeshsetcoordsxyz_,visitcsgmeshsetcoordsxyz,VISITCSGMESHSETCOORDSXYZ)
#define F_VISITCSGMESHSETBASEINDEX       F77_ID(visitcsgmeshsetbaseindex_,visitcsgmeshsetbaseindex,VISITCSGMESHSETBASEINDEX)
#define F_VISITCSGMESHSETREALINDICES     F77_ID(visitcsgmeshsetrealindices_,visitcsgmeshsetrealindices,VISITCSGMESHSETREALINDICES)

int
F_VISITCSGMESHALLOC(visit_handle *obj)
{
    return VisIt_CSGMesh_alloc(obj);
}

int
F_VISITCSGMESHFREE(visit_handle *obj)
{
    return VisIt_CSGMesh_free(*obj);
}

int
F_VISITCSGMESHSETREGIONS(visit_handle *obj, visit_handle *boolops, visit_handle *leftids, visit_handle *rightids)
{
    return VisIt_CSGMesh_setRegions(*obj, *boolops, *leftids, *rightids);
}

int
F_VISITCSGMESHSETZONELIST(visit_handle *obj, visit_handle *zl)
{
    return VisIt_CSGMesh_setZonelist(*obj, *zl);
}

int
F_VISITCSGMESHSETBOUNDARYTYPES(visit_handle *obj, visit_handle *boundaryTypes)
{
    return VisIt_CSGMesh_setBoundaryTypes(*obj, *boundaryTypes);
}

int
F_VISITCSGMESHSETBOUNDARYCOEFFS(visit_handle *obj, visit_handle *boundaryCoeffs)
{
    return VisIt_CSGMesh_setBoundaryCoeffs(*obj, *boundaryCoeffs);
}

int
F_VISITCSGMESHSETEXTENTS(visit_handle *obj, double *mins, double *maxs)
{
    double tmpMin[3], tmpMax[3];
    tmpMin[0] = mins[0];
    tmpMin[1] = mins[1];
    tmpMin[2] = mins[2];
    tmpMax[0] = maxs[0];
    tmpMax[1] = maxs[1];
    tmpMax[2] = maxs[2];
    return VisIt_CSGMesh_setExtents(*obj, tmpMin, tmpMax);
}

