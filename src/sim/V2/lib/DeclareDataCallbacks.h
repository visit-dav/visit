#ifndef DECLARE_DATA_CALLBACKS_H
#define DECLARE_DATA_CALLBACKS_H
#define NO_ARGS(A)
#define CB_ARGS(A) cb
#define FUNC_ARGS(A) cb_##A


#define DECLARE_DATA_CALLBACKS(DECL, ARG) \
   /* Read callbacks */ \
   DECL(ActivateTimestep,    int (*ARG(ActivateTimestep))(void *)) \
   DECL(GetMetaData,         visit_handle (*ARG(GetMetaData))(void *)) \
   DECL(GetMesh,             visit_handle (*ARG(GetMesh))(int, const char *, void *)) \
   DECL(GetMaterial,         visit_handle (*ARG(GetMaterial))(int, const char *, void *)) \
   DECL(GetSpecies,          visit_handle (*ARG(GetSpecies))(int, const char *, void *)) \
   DECL(GetVariable,         visit_handle (*ARG(GetVariable))(int, const char *, void *)) \
   DECL(GetMixedVariable,    visit_handle (*ARG(GetMixedVariable))(int, const char *, void *)) \
   DECL(GetCurve,            visit_handle (*ARG(GetCurve))(const char *, void *)) \
   DECL(GetDomainList,       visit_handle (*ARG(GetDomainList))(const char *, void *)) \
   DECL(GetDomainBoundaries, visit_handle (*ARG(GetDomainBoundaries))(const char *, void *)) \
   DECL(GetDomainNesting,    visit_handle (*ARG(GetDomainNesting))(const char *, void *)) \
   /* Write callbacks */ \
   DECL(WriteBegin,       int (*ARG(WriteBegin))(const char *, void *)) \
   DECL(WriteEnd,         int (*ARG(WriteEnd))(const char *, void *)) \
   DECL(WriteMesh,        int (*ARG(WriteMesh))(const char *, int, int, visit_handle, visit_handle, void *)) \
   DECL(WriteVariable,    int (*ARG(WriteVariable))(const char *, const char *, int, int, void *, int, int, visit_handle, void *))

#endif
