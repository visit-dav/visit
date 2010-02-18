/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
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

#ifndef VISIT_DATA_INTERFACE_RUNTIME_H
#define VISIT_DATA_INTERFACE_RUNTIME_H

#include <VisItDataInterface_V2.h>
#include <VisItSimV2Exports.h>

#ifdef __cplusplus
extern "C" {
#endif

// ****************************************************************************
//  Library:  libsimV2runtime
//
//  Purpose:
//    Data-related simV2 runtime functions that we can call from the SimV2
//    reader or from libsim.
//
//  Programmer:  Brad Whitlock
//  Creation:    Fri Feb 13 16:06:04 PST 2009
//
//  Modifications:
//
// ****************************************************************************

// Data functions that let the control interface uses
SIMV2_API void simv2_set_ActivateTimestep(int (*cb) (void *), void *cbdata);
SIMV2_API void simv2_set_GetMetaData(int (*cb) (VisIt_SimulationMetaData *, void *), void *cbdata);
SIMV2_API void simv2_set_GetMesh(int (*cb) (int, const char *, VisIt_MeshData *, void *), void *cbdata);
SIMV2_API void simv2_set_GetMaterial(int (*cb) (int, const char *, VisIt_MaterialData *, void *), void *cbdata);
SIMV2_API void simv2_set_GetSpecies(int (*cb) (int, const char *, VisIt_SpeciesData *, void *), void *cbdata);
SIMV2_API void simv2_set_GetVariable(int (*cb) (int, const char *, VisIt_VariableData *, void *), void *cbdata);
SIMV2_API void simv2_set_GetMixedVariable(int (*cb) (int, const char *, VisIt_MixedVariableData *, void *), void *cbdata);
SIMV2_API void simv2_set_GetCurve(int (*cb) (const char *, VisIt_CurveData *, void *), void *cbdata);
SIMV2_API void simv2_set_GetDomainList(int (*cb) (VisIt_DomainList *, void *), void *cbdata);
SIMV2_API void simv2_set_GetDomainBoundaries(int (*cb) (const char *, visit_handle, void *), void *cbdata);
SIMV2_API void simv2_set_GetDomainNesting(int (*cb) (const char *, visit_handle, void *), void *cbdata);

SIMV2_API void simv2_set_WriteBegin(int (*cb)(const char *, void *), void *cbdata);
SIMV2_API void simv2_set_WriteEnd(int (*cb)(const char *, void *), void *cbdata);
SIMV2_API void simv2_set_WriteMesh(int (*cb)(const char *, int, const VisIt_MeshData *, const VisIt_MeshMetaData *, void *), void *cbdata);
SIMV2_API void simv2_set_WriteVariable(int (*cb)(const char *, const char *, int, int, void *, int, int, const VisIt_VariableMetaData *, void *), void *cbdata);

// Data functions callable from SimV2 reader
SIMV2_API int                       simv2_invoke_ActivateTimestep(void);
SIMV2_API VisIt_SimulationMetaData *simv2_invoke_GetMetaData(void);
SIMV2_API VisIt_MeshData           *simv2_invoke_GetMesh(int, const char *);
SIMV2_API VisIt_MaterialData       *simv2_invoke_GetMaterial(int, const char *);
SIMV2_API VisIt_SpeciesData        *simv2_invoke_GetSpecies(int, const char *);
SIMV2_API VisIt_VariableData       *simv2_invoke_GetVariable(int, const char *);
SIMV2_API VisIt_MixedVariableData  *simv2_invoke_GetMixedVariable(int, const char *);
SIMV2_API VisIt_CurveData          *simv2_invoke_GetCurve(const char *);
SIMV2_API VisIt_DomainList         *simv2_invoke_GetDomainList(void);
SIMV2_API visit_handle              simv2_invoke_GetDomainBoundaries(const char *name);
SIMV2_API visit_handle              simv2_invoke_GetDomainNesting(const char *name);

SIMV2_API int simv2_invoke_WriteBegin(const char *);
SIMV2_API int simv2_invoke_WriteEnd(const char *);
SIMV2_API int simv2_invoke_WriteMesh(const char *, int, const VisIt_MeshData *, const VisIt_MeshMetaData *);
SIMV2_API int simv2_invoke_WriteVariable(const char *, const char *, int, int, void *, int, int, const VisIt_VariableMetaData *);

#ifdef __cplusplus
}
#endif

#endif
