#ifndef DECLARE_DATA_CALLBACKS_H
#define DECLARE_DATA_CALLBACKS_H
#define NO_ARGS(A)
#define CB_ARGS(A) cb
#define FUNC_ARGS(A) cb_##A


#define DECLARE_DATA_CALLBACKS(DECL, ARG) \
   /* Read callbacks */ \
   DECL(ActivateTimestep,    int (*ARG(ActivateTimestep))(void *)) \
   DECL(GetMetaData,         int (*ARG(GetMetaData))(VisIt_SimulationMetaData *, void *)) \
   DECL(GetMesh,             visit_handle (*ARG(GetMesh))(int, const char *, void *)) \
   DECL(GetMaterial,         visit_handle (*ARG(GetMaterial))(int, const char *, void *)) \
   DECL(GetSpecies,          int (*ARG(GetSpecies))(int, const char *, VisIt_SpeciesData *, void *)) \
   DECL(GetVariable,         visit_handle (*ARG(GetVariable))(int, const char *, void *)) \
   DECL(GetMixedVariable,    visit_handle (*ARG(GetMixedVariable))(int, const char *, void *)) \
   DECL(GetCurve,            visit_handle (*ARG(GetCurve))(const char *, void *)) \
   DECL(GetDomainList,       visit_handle (*ARG(GetDomainList))(const char *, void *)) \
   DECL(GetDomainBoundaries, visit_handle (*ARG(GetDomainBoundaries))(const char *, void *)) \
   DECL(GetDomainNesting,    visit_handle (*ARG(GetDomainNesting))(const char *, void *)) \
   /* Write callbacks */ \
   DECL(WriteBegin,       int (*ARG(WriteBegin))(void *, const char *)) \
   DECL(WriteEnd,         int (*ARG(WriteEnd))(void *, const char *)) \
   DECL(WriteMesh,        int (*ARG(WriteMesh))(void *, const char *, int, visit_handle, const VisIt_MeshMetaData *)) \
   DECL(WriteVariable,    int (*ARG(WriteVariable))(void *, const char *, const char *, int, int, void *, int, int, const VisIt_VariableMetaData *))

#endif
