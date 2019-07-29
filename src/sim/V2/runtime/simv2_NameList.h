// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SIMV2_NAMELIST_H
#define SIMV2_NAMELIST_H
#include <VisItSimV2Exports.h>
#include <VisItInterfaceTypes_V2.h>

// C-callable implementation of front end functions
#ifdef __cplusplus
extern "C" {
#endif

SIMV2_API int simv2_NameList_alloc(visit_handle *obj);
SIMV2_API int simv2_NameList_free(visit_handle obj);
SIMV2_API int simv2_NameList_addName(visit_handle h, const char *);
SIMV2_API int simv2_NameList_getNumName(visit_handle h, int *);
SIMV2_API int simv2_NameList_getName(visit_handle h, int, char **);

#ifdef __cplusplus
};
#endif

// Callable from within the runtime and SimV2

#endif
