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

#ifndef SIMV2_VARIABLEMETADATA_H
#define SIMV2_VARIABLEMETADATA_H
#include <VisItSimV2Exports.h>
#include <VisItInterfaceTypes_V2.h>

// C-callable implementation of front end functions
#ifdef __cplusplus
extern "C" {
#endif

SIMV2_API int simv2_VariableMetaData_alloc(visit_handle *obj);
SIMV2_API int simv2_VariableMetaData_free(visit_handle obj);
SIMV2_API int simv2_VariableMetaData_setName(visit_handle h, const char *);
SIMV2_API int simv2_VariableMetaData_getName(visit_handle h, char **);
SIMV2_API int simv2_VariableMetaData_setMeshName(visit_handle h, const char *);
SIMV2_API int simv2_VariableMetaData_getMeshName(visit_handle h, char **);
SIMV2_API int simv2_VariableMetaData_setUnits(visit_handle h, const char *);
SIMV2_API int simv2_VariableMetaData_getUnits(visit_handle h, char **);
SIMV2_API int simv2_VariableMetaData_setCentering(visit_handle h, int);
SIMV2_API int simv2_VariableMetaData_getCentering(visit_handle h, int*);
SIMV2_API int simv2_VariableMetaData_setType(visit_handle h, int);
SIMV2_API int simv2_VariableMetaData_getType(visit_handle h, int*);
SIMV2_API int simv2_VariableMetaData_setTreatAsASCII(visit_handle h, int);
SIMV2_API int simv2_VariableMetaData_getTreatAsASCII(visit_handle h, int*);
SIMV2_API int simv2_VariableMetaData_setHideFromGUI(visit_handle h, int);
SIMV2_API int simv2_VariableMetaData_getHideFromGUI(visit_handle h, int*);
SIMV2_API int simv2_VariableMetaData_setNumComponents(visit_handle h, int);
SIMV2_API int simv2_VariableMetaData_getNumComponents(visit_handle h, int*);

#ifdef __cplusplus
};
#endif

// Callable from within the runtime and SimV2
SIMV2_API int simv2_VariableMetaData_check(visit_handle);

#endif
