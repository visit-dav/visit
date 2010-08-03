/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
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
VisIt_UnstructuredMesh_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(UnstructuredMesh_alloc,
                    int (*)(visit_handle*),
                    int (*cb)(visit_handle*),
                    (*cb)(obj))
}

int
VisIt_UnstructuredMesh_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(UnstructuredMesh_free,
                    int (*)(visit_handle), 
                    int (*cb)(visit_handle), 
                    (*cb)(obj));
}

int
VisIt_UnstructuredMesh_setCoordsXY(visit_handle obj, visit_handle x, visit_handle y)
{
    VISIT_DYNAMIC_EXECUTE(UnstructuredMesh_setCoordsXY,
                    int (*)(visit_handle,visit_handle,visit_handle), 
                    int (*cb)(visit_handle,visit_handle,visit_handle), 
                    (*cb)(obj,x,y));
}

int
VisIt_UnstructuredMesh_setCoordsXYZ(visit_handle obj, visit_handle x, visit_handle y, visit_handle z)
{
    VISIT_DYNAMIC_EXECUTE(UnstructuredMesh_setCoordsXYZ,
                    int (*)(visit_handle,visit_handle,visit_handle,visit_handle), 
                    int (*cb)(visit_handle,visit_handle,visit_handle,visit_handle), 
                    (*cb)(obj,x,y,z));
}

int
VisIt_UnstructuredMesh_setCoords(visit_handle obj, visit_handle c)
{
    VISIT_DYNAMIC_EXECUTE(UnstructuredMesh_setCoords,
                    int (*)(visit_handle,visit_handle), 
                    int (*cb)(visit_handle,visit_handle), 
                    (*cb)(obj,c));
}

int
VisIt_UnstructuredMesh_setConnectivity(visit_handle obj, int nzones, visit_handle c)
{
    VISIT_DYNAMIC_EXECUTE(UnstructuredMesh_setConnectivity,
                    int (*)(visit_handle,int,visit_handle), 
                    int (*cb)(visit_handle,int,visit_handle), 
                    (*cb)(obj,nzones,c));
}

int
VisIt_UnstructuredMesh_setRealIndices(visit_handle obj, int min, int max)
{
    VISIT_DYNAMIC_EXECUTE(UnstructuredMesh_setRealIndices,
                    int (*)(visit_handle,int,int), 
                    int (*cb)(visit_handle,int,int), 
                    (*cb)(obj,min,max));
}

int
VisIt_UnstructuredMesh_getCoords(visit_handle obj,
    int *ndims, int *coordMode,
    visit_handle *x, visit_handle *y, visit_handle *z, visit_handle *coords)
{
    VISIT_DYNAMIC_EXECUTE(UnstructuredMesh_getCoords,
                    int (*)(visit_handle,int*,int*,visit_handle*,visit_handle*,visit_handle*,visit_handle*), 
                    int (*cb)(visit_handle,int*,int*,visit_handle*,visit_handle*,visit_handle*,visit_handle*), 
                    (*cb)(obj,ndims,coordMode,x,y,z,coords));
}

int
VisIt_UnstructuredMesh_getConnectivity(visit_handle obj, int *nzones,
    visit_handle *conn)
{
    VISIT_DYNAMIC_EXECUTE(UnstructuredMesh_getConnectivity,
                    int (*)(visit_handle,int*,visit_handle*), 
                    int (*cb)(visit_handle,int*,visit_handle*), 
                    (*cb)(obj,nzones, conn));
}

int
VisIt_UnstructuredMesh_getRealIndices(visit_handle obj, int *min, int *max)
{
    VISIT_DYNAMIC_EXECUTE(UnstructuredMesh_getRealIndices,
                    int (*)(visit_handle,int*,int*), 
                    int (*cb)(visit_handle,int*,int*), 
                    (*cb)(obj,min,max));
}

/************************** Fortran callable routines *************************/
/* maxlen 012345678901234567890123456789                                      */
#define F_VISITUCDMESHALLOC              F77_ID(visitucdmeshalloc_,visitucdmeshalloc,VISITUCDMESHALLOC)
#define F_VISITUCDMESHFREE               F77_ID(visitucdmeshfree_,visitucdmeshfree,VISITUCDMESHFREE)
#define F_VISITUCDMESHSETCOORDSXY        F77_ID(visitucdmeshsetcoordsxy_,visitucdmeshsetcoordsxy,VISITUCDMESHSETCOORDSXY)
#define F_VISITUCDMESHSETCOORDSXYZ       F77_ID(visitucdmeshsetcoordsxyz_,visitucdmeshsetcoordsxyz,VISITUCDMESHSETCOORDSXYZ)
#define F_VISITUCDMESHSETCOORDS          F77_ID(visitucdmeshsetcoords_,visitucdmeshsetcoords,VISITUCDMESHSETCOORDS)
#define F_VISITUCDMESHSETCONNECTIVITY    F77_ID(visitucdmeshsetconnectivity_,visitucdmeshsetconnectivity,VISITUCDMESHSETCONNECTIVITY)
#define F_VISITUCDMESHSETREALINDICES     F77_ID(visitucdmeshsetrealindices_,visitucdmeshsetrealindices,VISITUCDMESHSETREALINDICES)
#define F_VISITUCDMESHGETCOORDS          F77_ID(visitucdmeshgetcoords_,visitucdmeshgetcoords,VISITUCDMESHGETCOORDS)
#define F_VISITUCDMESHGETCONNECTIVITY    F77_ID(visitucdmeshgetconnectivity_,visitucdmeshgetconnectivity,VISITUCDMESHGETCONNECTIVITY)
#define F_VISITUCDMESHGETREALINDICES     F77_ID(visitucdmeshgetrealindices_,visitucdmeshgetrealindices,VISITUCDMESHGETREALINDICES)

int
F_VISITUCDMESHALLOC(visit_handle *obj)
{
    return VisIt_UnstructuredMesh_alloc(obj);
}

int
F_VISITUCDMESHFREE(visit_handle *obj)
{
    return VisIt_UnstructuredMesh_free(*obj);
}

int
F_VISITUCDMESHSETCOORDSXY(visit_handle *obj, visit_handle *x, visit_handle *y)
{
    return VisIt_UnstructuredMesh_setCoordsXY(*obj, *x, *y);
}

int
F_VISITUCDMESHSETCOORDSXYZ(visit_handle *obj, visit_handle *x, visit_handle *y, visit_handle *z)
{
    return VisIt_UnstructuredMesh_setCoordsXYZ(*obj, *x, *y, *z);
}

int
F_VISITUCDMESHSETCOORDS(visit_handle *obj, visit_handle *c)
{
    return VisIt_UnstructuredMesh_setCoords(*obj, *c);
}

int
F_VISITUCDMESHSETCONNECTIVITY(visit_handle *obj, int *nzones, visit_handle *c)
{
    return VisIt_UnstructuredMesh_setConnectivity(*obj, *nzones, *c);
}

int
F_VISITUCDMESHSETREALINDICES(visit_handle *obj, int *min, int *max)
{
    return VisIt_UnstructuredMesh_setRealIndices(*obj, *min, *max);
}

int
F_VISITUCDMESHGETCOORDS(visit_handle *obj, int *ndims, int *coordMode, 
    visit_handle *x, visit_handle *y, visit_handle *z, visit_handle *c)
{
    return VisIt_UnstructuredMesh_getCoords(*obj, ndims, coordMode, x, y, z, c);
}

int
F_VISITUCDMESHGETCONNECTIVITY(visit_handle *obj, int *nzones, visit_handle *c)
{
    return VisIt_UnstructuredMesh_getConnectivity(*obj, nzones, c);
}

int
F_VISITUCDMESHGETREALINDICES(visit_handle *obj, int *min, int *max)
{
    return VisIt_UnstructuredMesh_getRealIndices(*obj, min, max);
}

