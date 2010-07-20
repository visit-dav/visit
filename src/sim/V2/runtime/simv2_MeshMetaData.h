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

#ifndef SIMV2_MESHMETADATA_H
#define SIMV2_MESHMETADATA_H
#include <VisItSimV2Exports.h>
#include <VisItInterfaceTypes_V2.h>

// C-callable implementation of front end functions
#ifdef __cplusplus
extern "C" {
#endif

SIMV2_API int simv2_MeshMetaData_alloc(visit_handle *obj);
SIMV2_API int simv2_MeshMetaData_free(visit_handle obj);
SIMV2_API int simv2_MeshMetaData_setName(visit_handle h, const char *);
SIMV2_API int simv2_MeshMetaData_getName(visit_handle h, char **);
SIMV2_API int simv2_MeshMetaData_setMeshType(visit_handle h, int);
SIMV2_API int simv2_MeshMetaData_getMeshType(visit_handle h, int*);
SIMV2_API int simv2_MeshMetaData_setTopologicalDimension(visit_handle h, int);
SIMV2_API int simv2_MeshMetaData_getTopologicalDimension(visit_handle h, int*);
SIMV2_API int simv2_MeshMetaData_setSpatialDimension(visit_handle h, int);
SIMV2_API int simv2_MeshMetaData_getSpatialDimension(visit_handle h, int*);
SIMV2_API int simv2_MeshMetaData_setNumDomains(visit_handle h, int);
SIMV2_API int simv2_MeshMetaData_getNumDomains(visit_handle h, int*);
SIMV2_API int simv2_MeshMetaData_setDomainTitle(visit_handle h, const char *);
SIMV2_API int simv2_MeshMetaData_getDomainTitle(visit_handle h, char **);
SIMV2_API int simv2_MeshMetaData_setDomainPieceName(visit_handle h, const char *);
SIMV2_API int simv2_MeshMetaData_getDomainPieceName(visit_handle h, char **);
SIMV2_API int simv2_MeshMetaData_addDomainName(visit_handle h, const char *);
SIMV2_API int simv2_MeshMetaData_getNumDomainName(visit_handle h, int *);
SIMV2_API int simv2_MeshMetaData_getDomainName(visit_handle h, int, char **);
SIMV2_API int simv2_MeshMetaData_setNumGroups(visit_handle h, int);
SIMV2_API int simv2_MeshMetaData_getNumGroups(visit_handle h, int*);
SIMV2_API int simv2_MeshMetaData_setGroupTitle(visit_handle h, const char *);
SIMV2_API int simv2_MeshMetaData_getGroupTitle(visit_handle h, char **);
SIMV2_API int simv2_MeshMetaData_setGroupPieceName(visit_handle h, const char *);
SIMV2_API int simv2_MeshMetaData_getGroupPieceName(visit_handle h, char **);
SIMV2_API int simv2_MeshMetaData_addGroupId(visit_handle h, int);
SIMV2_API int simv2_MeshMetaData_getNumGroupId(visit_handle h, int *);
SIMV2_API int simv2_MeshMetaData_getGroupId(visit_handle h, int, int *);
SIMV2_API int simv2_MeshMetaData_setXUnits(visit_handle h, const char *);
SIMV2_API int simv2_MeshMetaData_getXUnits(visit_handle h, char **);
SIMV2_API int simv2_MeshMetaData_setYUnits(visit_handle h, const char *);
SIMV2_API int simv2_MeshMetaData_getYUnits(visit_handle h, char **);
SIMV2_API int simv2_MeshMetaData_setZUnits(visit_handle h, const char *);
SIMV2_API int simv2_MeshMetaData_getZUnits(visit_handle h, char **);
SIMV2_API int simv2_MeshMetaData_setXLabel(visit_handle h, const char *);
SIMV2_API int simv2_MeshMetaData_getXLabel(visit_handle h, char **);
SIMV2_API int simv2_MeshMetaData_setYLabel(visit_handle h, const char *);
SIMV2_API int simv2_MeshMetaData_getYLabel(visit_handle h, char **);
SIMV2_API int simv2_MeshMetaData_setZLabel(visit_handle h, const char *);
SIMV2_API int simv2_MeshMetaData_getZLabel(visit_handle h, char **);

#ifdef __cplusplus
};
#endif

// Callable from within the runtime and SimV2
SIMV2_API int simv2_MeshMetaData_check(visit_handle);

#endif
