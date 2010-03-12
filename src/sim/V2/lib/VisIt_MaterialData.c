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
VisIt_MaterialData_alloc(visit_handle *h)
{
    VISIT_DYNAMIC_EXECUTE(MaterialData_alloc,
                    int (*)(visit_handle*),
                    int (*cb)(visit_handle*),
                    (*cb)(h))
}

int
VisIt_MaterialData_free(visit_handle h)
{
    VISIT_DYNAMIC_EXECUTE(MaterialData_free,
                    int (*)(visit_handle), 
                    int (*cb)(visit_handle), 
                    (*cb)(h));
}

int
VisIt_MaterialData_appendCells(visit_handle h, int ncells)
{
    VISIT_DYNAMIC_EXECUTE(MaterialData_appendCells,
                    int (*)(visit_handle,int), 
                    int (*cb)(visit_handle,int), 
                    (*cb)(h,ncells));
}

int
VisIt_MaterialData_addMaterial(visit_handle h, const char *matName, int *matno)
{
    VISIT_DYNAMIC_EXECUTE(MaterialData_addMaterial,
                    int (*)(visit_handle,const char*,int *), 
                    int (*cb)(visit_handle,const char*, int *), 
                    (*cb)(h,matName,matno));
}

int
VisIt_MaterialData_addCleanCell(visit_handle h, int cell, int matno)
{
    VISIT_DYNAMIC_EXECUTE(MaterialData_addCleanCell,
                    int (*)(visit_handle,int,int), 
                    int (*cb)(visit_handle,int,int), 
                    (*cb)(h,cell,matno));
}

int
VisIt_MaterialData_addMixedCell(visit_handle h, int cell, 
   const int *matnos, const float *mixvf, int nmats)
{
    VISIT_DYNAMIC_EXECUTE(MaterialData_addMixedCell,
                    int (*)(visit_handle,int,const int*,const float*,int), 
                    int (*cb)(visit_handle,int,const int*,const float*,int), 
                    (*cb)(h,cell,matnos,mixvf,nmats));
}

int
VisIt_MaterialData_setMaterials(visit_handle h, visit_handle matlist)
{
    VISIT_DYNAMIC_EXECUTE(MaterialData_setMaterials,
                    int (*)(visit_handle,visit_handle), 
                    int (*cb)(visit_handle,visit_handle), 
                    (*cb)(h,matlist));
}

int
VisIt_MaterialData_setMixedMaterials(visit_handle h,
    visit_handle mix_mat, visit_handle mix_zone, 
    visit_handle mix_next, visit_handle mix_vf)
{
    VISIT_DYNAMIC_EXECUTE(MaterialData_setMixedMaterials,
                    int (*)(visit_handle,visit_handle,visit_handle,visit_handle,visit_handle), 
                    int (*cb)(visit_handle,visit_handle,visit_handle,visit_handle,visit_handle), 
                    (*cb)(h,mix_mat,mix_zone,mix_next,mix_vf));
}

/************************** Fortran callable routines *************************/
/* maxlen 012345678901234567890123456789                                      */
#define F_VISITMATDATAALLOC              F77_ID(visitmatdataalloc_,visitmatdataalloc,VISITMATDATAALLOC)
#define F_VISITMATDATAFREE               F77_ID(visitmatdatafree_,visitmatdatafree,VISITMATDATAFREE)
#define F_VISITMATDATAAPPENDCELLS        F77_ID(visitmatdataappendcells_,visitmatdataappendcells,VISITMATDATAAPPENDCELLS)
#define F_VISITMATDATAADDMAT             F77_ID(visitmatdataaddmat_,visitmatdataaddmat,VISITMATDATAADDMAT)
#define F_VISITMATDATAADDCLEANCELL       F77_ID(visitmatdataaddcleancell_,visitmatdataaddcleancell,VISITMATDATAADDCLEANCELL)
#define F_VISITMATDATAADDMIXEDCELL       F77_ID(visitmatdataaddmixedcell_,visitmatdataaddmixedcell,VISITMATDATAADDMIXEDCELL)
#define F_VISITMATDATASETMATS            F77_ID(visitmatdatasetmats_,visitmatdatasetmats,VISITMATDATASETMATS)
#define F_VISITMATDATASETMIXEDMATS       F77_ID(visitmatdatasetmixedmats_,visitmatdatasetmixedmats,VISITMATDATASETMIXEDMATS)

int
F_VISITMATDATAALLOC(visit_handle *h)
{
    return VisIt_MaterialData_alloc(h);
}

int
F_VISITMATDATAFREE(visit_handle *h)
{
    return VisIt_MaterialData_free(*h);
}

int
F_VISITMATDATAAPPENDCELLS(visit_handle *h, int *ncells)
{
    return VisIt_MaterialData_appendCells(*h, *ncells);
}

int
F_VISITMATDATAADDMAT(visit_handle *obj, VISIT_F77STRING matname, int *lmatname, int *matno)
{
    char *f_matname = NULL;
    int retval;

    COPY_FORTRAN_STRING(f_matname, matname, lmatname);
    retval = VisIt_MaterialData_addMaterial(*obj, f_matname, matno);
    FREE(f_matname);

    return retval;
}

int
F_VISITMATDATAADDCLEANCELL(visit_handle *h, int *cell, int *matno)
{
    return VisIt_MaterialData_addCleanCell(*h, *cell, *matno);
}

int
F_VISITMATDATAADDMIXEDCELL(visit_handle *h, int *cell, int *matnos, float *matvf, int *nmats)
{
    return VisIt_MaterialData_addMixedCell(*h, *cell, matnos, matvf, *nmats);
}

int
F_VISITMATDATASETMATS(visit_handle *obj, visit_handle *matlist)
{
    return VisIt_MaterialData_setMaterials(*obj, *matlist);
}

int
F_VISITMATDATASETMIXEDMATS(visit_handle *obj, visit_handle *mix_mat, 
    visit_handle *mix_zone, visit_handle *mix_next, visit_handle *mix_vf)
{
    return VisIt_MaterialData_setMixedMaterials(*obj, *mix_mat, *mix_zone, *mix_next, *mix_vf);
}
