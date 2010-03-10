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
VisIt_PointMesh_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(PointMesh_alloc,
                    int (*)(visit_handle*),
                    int (*cb)(visit_handle*),
                    (*cb)(obj))
}

int
VisIt_PointMesh_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(PointMesh_free,
                    int (*)(visit_handle), 
                    int (*cb)(visit_handle), 
                    (*cb)(obj));
}

int
VisIt_PointMesh_setCoordsXY(visit_handle obj, visit_handle x, visit_handle y)
{
    VISIT_DYNAMIC_EXECUTE(PointMesh_setCoordsXY,
                    int (*)(visit_handle,visit_handle,visit_handle), 
                    int (*cb)(visit_handle,visit_handle,visit_handle), 
                    (*cb)(obj,x,y));
}

int
VisIt_PointMesh_setCoordsXYZ(visit_handle obj, visit_handle x, visit_handle y, visit_handle z)
{
    VISIT_DYNAMIC_EXECUTE(PointMesh_setCoordsXYZ,
                    int (*)(visit_handle,visit_handle,visit_handle,visit_handle), 
                    int (*cb)(visit_handle,visit_handle,visit_handle,visit_handle), 
                    (*cb)(obj,x,y,z));
}

int
VisIt_PointMesh_setCoords(visit_handle obj, visit_handle c)
{
    VISIT_DYNAMIC_EXECUTE(PointMesh_setCoords,
                    int (*)(visit_handle,visit_handle), 
                    int (*cb)(visit_handle,visit_handle), 
                    (*cb)(obj,c));
}

/************************** Fortran callable routines *************************/
/* maxlen 012345678901234567890123456789                                      */
#define F_VISITPOINTMESHALLOC            F77_ID(visitpointmeshalloc_,visitpointmeshalloc,VISITPOINTMESHALLOC)
#define F_VISITPOINTMESHFREE             F77_ID(visitpointmeshfree_,visitpointmeshfree,VISITPOINTMESHFREE)
#define F_VISITPOINTMESHSETCOORDSXY      F77_ID(visitpointmeshsetcoordsxy_,visitpointmeshsetcoordsxy,VISITPOINTMESHSETCOORDSXY)
#define F_VISITPOINTMESHSETCOORDSXYZ     F77_ID(visitpointmeshsetcoordsxyz_,visitpointmeshsetcoordsxyz,VISITPOINTMESHSETCOORDSXYZ)
#define F_VISITPOINTMESHSETCOORDS        F77_ID(visitpointmeshsetcoords_,visitpointmeshsetcoords,VISITPOINTMESHSETCOORDS)

int
F_VISITPOINTMESHALLOC(visit_handle *obj)
{
    return VisIt_PointMesh_alloc(obj);
}

int
F_VISITPOINTMESHFREE(visit_handle *obj)
{
    return VisIt_PointMesh_free(*obj);
}

int
F_VISITPOINTMESHSETCOORDSXY(visit_handle *obj, visit_handle *x, visit_handle *y)
{
    return VisIt_PointMesh_setCoordsXY(*obj, *x, *y);
}

int
F_VISITPOINTMESHSETCOORDSXYZ(visit_handle *obj, visit_handle *x, visit_handle *y, visit_handle *z)
{
    return VisIt_PointMesh_setCoordsXYZ(*obj, *x, *y, *z);
}

int
F_VISITPOINTMESHSETCOORDS(visit_handle *obj, visit_handle *c)
{
    return VisIt_PointMesh_setCoords(*obj, *c);
}


