/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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
VisIt_CurvilinearMesh_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(CurvilinearMesh_alloc,
                    int, (visit_handle*),
                    (obj))
}

int
VisIt_CurvilinearMesh_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(CurvilinearMesh_free,
                    int, (visit_handle), 
                    (obj));
}

int
VisIt_CurvilinearMesh_setCoordsXY(visit_handle obj, int dims[2], visit_handle x, visit_handle y)
{
    VISIT_DYNAMIC_EXECUTE(CurvilinearMesh_setCoordsXY,
                    int, (visit_handle,int[2],visit_handle,visit_handle), 
                    (obj,dims,x,y));
}

int
VisIt_CurvilinearMesh_setCoordsXYZ(visit_handle obj, int dims[3], visit_handle x, visit_handle y, visit_handle z)
{
    VISIT_DYNAMIC_EXECUTE(CurvilinearMesh_setCoordsXYZ,
                    int, (visit_handle,int[3],visit_handle,visit_handle,visit_handle), 
                    (obj,dims,x,y,z));
}

int
VisIt_CurvilinearMesh_setCoords2(visit_handle obj, int dims[2], visit_handle c)
{
    VISIT_DYNAMIC_EXECUTE(CurvilinearMesh_setCoords2,
                    int, (visit_handle,int[2],visit_handle), 
                    (obj,dims,c));
}

int
VisIt_CurvilinearMesh_setCoords3(visit_handle obj, int dims[3], visit_handle c)
{
    VISIT_DYNAMIC_EXECUTE(CurvilinearMesh_setCoords3,
                    int, (visit_handle,int[3],visit_handle), 
                    (obj,dims,c));
}

int
VisIt_CurvilinearMesh_setRealIndices(visit_handle obj, int min[3], int max[3])
{
    VISIT_DYNAMIC_EXECUTE(CurvilinearMesh_setRealIndices,
                    int, (visit_handle,int[3], int[3]), 
                    (obj,min,max));
}

int
VisIt_CurvilinearMesh_setBaseIndex(visit_handle obj, int base_index[3])
{
    VISIT_DYNAMIC_EXECUTE(CurvilinearMesh_setBaseIndex,
                    int, (visit_handle,int[3]), 
                    (obj,base_index));
}

int
VisIt_CurvilinearMesh_setGhostCells(visit_handle obj, visit_handle gz)
{
    VISIT_DYNAMIC_EXECUTE(CurvilinearMesh_setGhostCells,
                    int, (visit_handle,visit_handle), 
                    (obj,gz));
}

int
VisIt_CurvilinearMesh_setGhostNodes(visit_handle obj, visit_handle gn)
{
    VISIT_DYNAMIC_EXECUTE(CurvilinearMesh_setGhostNodes,
                    int, (visit_handle,visit_handle), 
                    (obj,gn));
}

int
VisIt_CurvilinearMesh_getCoords(visit_handle obj, int *ndims, int dims[3],
    int *coordMode, 
    visit_handle *x, visit_handle *y, visit_handle *z, visit_handle *c)
{
    VISIT_DYNAMIC_EXECUTE(CurvilinearMesh_getCoords,
                    int, (visit_handle,int*,int[3],int*,visit_handle*,visit_handle*,visit_handle*,visit_handle*), 
                    (obj,ndims,dims,coordMode,x,y,z,c));
}

int
VisIt_CurvilinearMesh_getRealIndices(visit_handle obj, int min[3], int max[3])
{
    VISIT_DYNAMIC_EXECUTE(CurvilinearMesh_getRealIndices,
                    int, (visit_handle,int[3], int[3]), 
                    (obj,min,max));
}

int
VisIt_CurvilinearMesh_getBaseIndex(visit_handle obj, int base_index[3])
{
    VISIT_DYNAMIC_EXECUTE(CurvilinearMesh_getBaseIndex,
                    int, (visit_handle,int[3]), 
                    (obj,base_index));
}

int
VisIt_CurvilinearMesh_getGhostCells(visit_handle obj, visit_handle *gz)
{
    VISIT_DYNAMIC_EXECUTE(CurvilinearMesh_getGhostCells,
                    int, (visit_handle,visit_handle*), 
                    (obj,gz));
}

int
VisIt_CurvilinearMesh_getGhostNodes(visit_handle obj, visit_handle *gn)
{
    VISIT_DYNAMIC_EXECUTE(CurvilinearMesh_getGhostNodes,
                    int, (visit_handle,visit_handle*), 
                    (obj,gn));
}

/************************** Fortran callable routines *************************/
/* maxlen 012345678901234567890123456789                                      */
#define F_VISITCURVMESHALLOC             F77_ID(visitcurvmeshalloc_,visitcurvmeshalloc,VISITCURVMESHALLOC)
#define F_VISITCURVMESHFREE              F77_ID(visitcurvmeshfree_,visitcurvmeshfree,VISITCURVMESHFREE)
#define F_VISITCURVMESHSETCOORDSXY       F77_ID(visitcurvmeshsetcoordsxy_,visitcurvmeshsetcoordsxy,VISITCURVMESHSETCOORDSXY)
#define F_VISITCURVMESHSETCOORDSXYZ      F77_ID(visitcurvmeshsetcoordsxyz_,visitcurvmeshsetcoordsxyz,VISITCURVMESHSETCOORDSXYZ)
#define F_VISITCURVMESHSETCOORDS2        F77_ID(visitcurvmeshsetcoords2_,visitcurvmeshsetcoords2,VISITCURVMESHSETCOORDS2)
#define F_VISITCURVMESHSETCOORDS3        F77_ID(visitcurvmeshsetcoords3_,visitcurvmeshsetcoords3,VISITCURVMESHSETCOORDS3)
#define F_VISITCURVMESHSETBASEINDEX      F77_ID(visitcurvmeshsetbaseindex_,visitcurvmeshsetbaseindex,VISITCURVMESHSETBASEINDEX)
#define F_VISITCURVMESHSETREALINDICES    F77_ID(visitcurvmeshsetrealindices_,visitcurvmeshsetrealindices,VISITCURVMESHSETREALINDICES)
#define F_VISITCURVMESHSETGHOSTCELLS     F77_ID(visitcurvmeshsetghostcells_,visitcurvmeshsetghostcells,VISITCURVMESHSETGHOSTCELLS)
#define F_VISITCURVMESHSETGHOSTNODES     F77_ID(visitcurvmeshsetghostnodes_,visitcurvmeshsetghostnodes,VISITCURVMESHSETGHOSTNODES)

#define F_VISITCURVMESHGETCOORDS         F77_ID(visitcurvmeshgetcoords_,visitcurvmeshgetcoords,VISITCURVMESHGETCOORDS)
#define F_VISITCURVMESHGETBASEINDEX      F77_ID(visitcurvmeshgetbaseindex_,visitcurvmeshgetbaseindex,VISITCURVMESHGETBASEINDEX)
#define F_VISITCURVMESHGETREALINDICES    F77_ID(visitcurvmeshgetrealindices_,visitcurvmeshgetrealindices,VISITCURVMESHGETREALINDICES)
#define F_VISITCURVMESHGETGHOSTCELLS     F77_ID(visitcurvmeshgetghostcells_,visitcurvmeshgetghostcells,VISITCURVMESHGETGHOSTCELLS)
#define F_VISITCURVMESHGETGHOSTNODES     F77_ID(visitcurvmeshgetghostnodes_,visitcurvmeshgetghostnodes,VISITCURVMESHGETGHOSTNODES)

int
F_VISITCURVMESHALLOC(visit_handle *obj)
{
    return VisIt_CurvilinearMesh_alloc(obj);
}

int
F_VISITCURVMESHFREE(visit_handle *obj)
{
    return VisIt_CurvilinearMesh_free(*obj);
}

int
F_VISITCURVMESHSETCOORDSXY(visit_handle *obj, int *dims, visit_handle *x, visit_handle *y)
{
    return VisIt_CurvilinearMesh_setCoordsXY(*obj, dims, *x, *y);
}

int
F_VISITCURVMESHSETCOORDSXYZ(visit_handle *obj, int *dims, visit_handle *x, visit_handle *y, visit_handle *z)
{
    return VisIt_CurvilinearMesh_setCoordsXYZ(*obj, dims, *x, *y, *z);
}

int
F_VISITCURVMESHSETCOORDS2(visit_handle *obj, int *dims, visit_handle *c)
{
    return VisIt_CurvilinearMesh_setCoords2(*obj, dims, *c);
}

int
F_VISITCURVMESHSETCOORDS3(visit_handle *obj, int *dims, visit_handle *c)
{
    return VisIt_CurvilinearMesh_setCoords3(*obj, dims, *c);
}

int
F_VISITCURVMESHSETBASEINDEX(visit_handle *obj, int *base_index)
{
    int tmp[3];
    tmp[0] = base_index[0];
    tmp[1] = base_index[1];
    tmp[2] = base_index[2];
    return VisIt_CurvilinearMesh_setBaseIndex(*obj, tmp);
}

int
F_VISITCURVMESHSETREALINDICES(visit_handle *obj, int *mins, int *maxs)
{
    int tmpMin[3], tmpMax[3];
    tmpMin[0] = mins[0];
    tmpMin[1] = mins[1];
    tmpMin[2] = mins[2];
    tmpMax[0] = maxs[0];
    tmpMax[1] = maxs[1];
    tmpMax[2] = maxs[2];
    return VisIt_CurvilinearMesh_setRealIndices(*obj, tmpMin, tmpMax);
}

int
F_VISITCURVMESHSETGHOSTCELLS(visit_handle *obj, visit_handle *gz)
{
    return VisIt_CurvilinearMesh_setGhostCells(*obj, *gz);
}

int
F_VISITCURVMESHSETGHOSTNODES(visit_handle *obj, visit_handle *gn)
{
    return VisIt_CurvilinearMesh_setGhostNodes(*obj, *gn);
}

int
F_VISITCURVMESHGETCOORDS(visit_handle *obj, int *ndims, int *dims,
    int *coordMode, 
    visit_handle *x, visit_handle *y, visit_handle *z, visit_handle *c)
{
    return VisIt_CurvilinearMesh_getCoords(*obj, ndims, dims, coordMode, x,y,z,c);
}

int
F_VISITCURVMESHGETBASEINDEX(visit_handle *obj, int *base_index)
{
    return VisIt_CurvilinearMesh_getBaseIndex(*obj, base_index);
}

int
F_VISITCURVMESHGETREALINDICES(visit_handle *obj, int *mins, int *maxs)
{
    return VisIt_CurvilinearMesh_getRealIndices(*obj, mins, maxs);
}

int
F_VISITCURVMESHGETGHOSTCELLS(visit_handle *obj, visit_handle *gz)
{
    return VisIt_CurvilinearMesh_getGhostCells(*obj, gz);
}

int
F_VISITCURVMESHGETGHOSTNODES(visit_handle *obj, visit_handle *gn)
{
    return VisIt_CurvilinearMesh_getGhostNodes(*obj, gn);
}
