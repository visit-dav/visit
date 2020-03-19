// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_DATA_INTERFACE_RUNTIME_H
#define VISIT_DATA_INTERFACE_RUNTIME_H

#include <VisItDataInterface_V2.h>
#include <VisItSimV2Exports.h>

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

#ifdef __cplusplus
extern "C" {
#endif

// Data functions that the control interface uses
SIMV2_API void simv2_set_ActivateTimestep(int (*cb) (void *), void *cbdata);
SIMV2_API void simv2_set_GetMetaData(visit_handle (*cb) (void *), void *cbdata);
SIMV2_API void simv2_set_GetMesh(visit_handle (*cb) (int, const char *, void *), void *cbdata);
SIMV2_API void simv2_set_GetMaterial(visit_handle (*cb) (int, const char *, void *), void *cbdata);
SIMV2_API void simv2_set_GetSpecies(visit_handle (*cb) (int, const char *, void *), void *cbdata);
SIMV2_API void simv2_set_GetVariable(visit_handle (*cb) (int, const char *, void *), void *cbdata);
SIMV2_API void simv2_set_GetMixedVariable(visit_handle (*cb) (int, const char *, void *), void *cbdata);
SIMV2_API void simv2_set_GetCurve(visit_handle (*cb) (const char *, void *), void *cbdata);
SIMV2_API void simv2_set_GetDomainList(visit_handle (*cb) (const char *, void *), void *cbdata);
SIMV2_API void simv2_set_GetDomainBoundaries(visit_handle (*cb) (const char *, void *), void *cbdata);
SIMV2_API void simv2_set_GetDomainNesting(visit_handle (*cb) (const char *, void *), void *cbdata);

SIMV2_API void simv2_set_WriteBegin(int (*cb)(const char *, void *), void *cbdata);
SIMV2_API void simv2_set_WriteEnd(int (*cb)(const char *, void *), void *cbdata);
SIMV2_API void simv2_set_WriteMesh(int (*cb)(const char *, int, int, visit_handle, visit_handle, void *), void *cbdata);
SIMV2_API void simv2_set_WriteVariable(int (*cb)(const char *, const char *, int, visit_handle, visit_handle, void *), void *cbdata);

#ifdef __cplusplus
}
#endif

// Data functions callable from SimV2 reader
SIMV2_API int          simv2_invoke_ActivateTimestep(void);
SIMV2_API visit_handle simv2_invoke_GetMetaData(void);
SIMV2_API visit_handle simv2_invoke_GetMesh(int, const char *);
SIMV2_API visit_handle simv2_invoke_GetMaterial(int, const char *);
SIMV2_API visit_handle simv2_invoke_GetSpecies(int, const char *);
SIMV2_API visit_handle simv2_invoke_GetVariable(int, const char *);
SIMV2_API visit_handle simv2_invoke_GetMixedVariable(int, const char *);
SIMV2_API visit_handle simv2_invoke_GetCurve(const char *);
SIMV2_API visit_handle simv2_invoke_GetDomainList(const char *);
SIMV2_API visit_handle simv2_invoke_GetDomainBoundaries(const char *name);
SIMV2_API visit_handle simv2_invoke_GetDomainNesting(const char *name);

SIMV2_API int simv2_invoke_WriteBegin(const char *);
SIMV2_API int simv2_invoke_WriteEnd(const char *);
SIMV2_API int simv2_invoke_WriteMesh(const char *name, int dom, int meshType, visit_handle mesh, visit_handle mmd);
SIMV2_API int simv2_invoke_WriteVariable(const char *, const char *, int, visit_handle, visit_handle);

#define VISIT_DOMAINLIST        12
#define VISIT_DOMAIN_BOUNDARIES 13
#define VISIT_DOMAIN_NESTING    14

#define VISIT_VARIABLE_DATA     15

#define VISIT_CURVILINEAR_MESH  20
#define VISIT_CSG_MESH          21
#define VISIT_POINT_MESH        22
#define VISIT_RECTILINEAR_MESH  23
#define VISIT_UNSTRUCTURED_MESH 24

#define VISIT_CURVE_DATA        30

#define VISIT_MATERIAL_DATA     40
#define VISIT_SPECIES_DATA      50

#define VISIT_SIMULATION_METADATA    100
#define VISIT_MESHMETADATA           101
#define VISIT_VARIABLEMETADATA       102
#define VISIT_MATERIALMETADATA       103
#define VISIT_CURVEMETADATA          104
#define VISIT_EXPRESSIONMETADATA     105
#define VISIT_SPECIESMETADATA        106
#define VISIT_NAMELIST               107
#define VISIT_COMMANDMETADATA        108
#define VISIT_OPTIONLIST             109

#define VISIT_MESSAGEMETADATA        110

#define VISIT_VIEW3D                 150
#define VISIT_VIEW2D                 151

SIMV2_API int simv2_ObjectType(visit_handle h);
SIMV2_API int simv2_FreeObject(visit_handle h);


#endif
