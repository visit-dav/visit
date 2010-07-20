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

#ifndef SIMV2_SIMULATIONMETADATA_H
#define SIMV2_SIMULATIONMETADATA_H
#include <VisItSimV2Exports.h>
#include <VisItInterfaceTypes_V2.h>

// C-callable implementation of front end functions
#ifdef __cplusplus
extern "C"
{
#endif

SIMV2_API int simv2_SimulationMetaData_alloc(visit_handle*);
SIMV2_API int simv2_SimulationMetaData_free(visit_handle);

SIMV2_API int simv2_SimulationMetaData_setMode(visit_handle h, int mode);
SIMV2_API int simv2_SimulationMetaData_setCycleTime(visit_handle h, int cycle, double time);
SIMV2_API int simv2_SimulationMetaData_addMesh(visit_handle h, visit_handle obj);
SIMV2_API int simv2_SimulationMetaData_addVariable(visit_handle h, visit_handle obj);
SIMV2_API int simv2_SimulationMetaData_addMaterial(visit_handle h, visit_handle obj);
SIMV2_API int simv2_SimulationMetaData_addCurve(visit_handle h, visit_handle obj);
SIMV2_API int simv2_SimulationMetaData_addExpression(visit_handle h, visit_handle obj);
SIMV2_API int simv2_SimulationMetaData_addSpecies(visit_handle h, visit_handle obj);
SIMV2_API int simv2_SimulationMetaData_addGenericCommand(visit_handle h, visit_handle obj);
SIMV2_API int simv2_SimulationMetaData_addCustomCommand(visit_handle h, visit_handle obj);

#ifdef __cplusplus
}
#endif

SIMV2_API int simv2_SimulationMetaData_check(visit_handle h);

// Callable from within the runtime and SimV2
SIMV2_API int simv2_SimulationMetaData_getData(visit_handle, 
                  int &mode, int &cycle, double &time);
SIMV2_API int simv2_SimulationMetaData_getNumMeshes(visit_handle h, int &n);
SIMV2_API int simv2_SimulationMetaData_getMesh(visit_handle h, int i, visit_handle &obj);
SIMV2_API int simv2_SimulationMetaData_getNumVariables(visit_handle h, int &n);
SIMV2_API int simv2_SimulationMetaData_getVariable(visit_handle h, int i, visit_handle &obj);
SIMV2_API int simv2_SimulationMetaData_getNumMaterials(visit_handle h, int &n);
SIMV2_API int simv2_SimulationMetaData_getMaterial(visit_handle h, int i, visit_handle &obj);
SIMV2_API int simv2_SimulationMetaData_getNumCurves(visit_handle h, int &n);
SIMV2_API int simv2_SimulationMetaData_getCurve(visit_handle h, int i, visit_handle &obj);
SIMV2_API int simv2_SimulationMetaData_getNumExpressions(visit_handle h, int &n);
SIMV2_API int simv2_SimulationMetaData_getExpression(visit_handle h, int i, visit_handle &obj);
SIMV2_API int simv2_SimulationMetaData_getNumSpecies(visit_handle h, int &n);
SIMV2_API int simv2_SimulationMetaData_getSpecies(visit_handle h, int i, visit_handle &obj);
SIMV2_API int simv2_SimulationMetaData_getNumGenericCommands(visit_handle h, int &n);
SIMV2_API int simv2_SimulationMetaData_getGenericCommand(visit_handle h, int i, visit_handle &obj);
SIMV2_API int simv2_SimulationMetaData_getNumCustomCommands(visit_handle h, int &n);
SIMV2_API int simv2_SimulationMetaData_getCustomCommand(visit_handle h, int i, visit_handle &obj);

#endif
